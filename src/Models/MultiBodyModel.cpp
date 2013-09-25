#include "MultiBodyModel.h"

#include <algorithms/connected_components.h>

#include "BodyModel.h"
#include "ConnectionModel.h"
#include "Models/CfgModel.h"
#include "Utilities/IOUtils.h"

class IsConnectionGloballyFirst {
  public:
    bool operator()(const ConnectionModel* _a, const ConnectionModel* _b) const {
      return _a->GetGlobalIndex() < _b->GetGlobalIndex();
    }
} connectionComparator;

MultiBodyModel::MultiBodyModel() : m_active(false), m_surface(false), m_radius(false) {}

MultiBodyModel::~MultiBodyModel() {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    delete *bit;
}

vector<string>
MultiBodyModel::GetInfo() const{
  vector<string> info;
  ostringstream temp, os;

  if(m_active){
    info.push_back(string("Robot"));
    temp << m_bodies.size();
  }
  else {
    info.push_back(string("Obstacle"));
    temp << "Position ( "<< Translation() << ")";
  }
  info.push_back(temp.str());
  return info;
}

void
MultiBodyModel::GetChildren(list<Model*>& _models) {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    _models.push_back(*bit);
}

void
MultiBodyModel::SetRenderMode(RenderMode _mode){
  Model::SetRenderMode(_mode);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetRenderMode(_mode);
}

void
MultiBodyModel::SetColor(const Color4& _c) {
  Model::SetColor(_c);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetColor(_c);
}

void
MultiBodyModel::BuildModels() {
  //build for each body and compute com
  m_com(0, 0, 0);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    m_com += (*bit)->GetTransform().translation();
  m_com /= m_bodies.size();

  //set position of multi-body as com
  Translation() = m_com;

  //compute radius
  m_radius = 0;
  for(BodyIter bit = Begin(); bit!=End(); ++bit) {
    double dist = ((*bit)->GetTransform().translation()-m_com).norm() + (*bit)->GetRadius();
    if(m_radius < dist)
      m_radius = dist;
  }

  BuildRobotStructure();
}

void
MultiBodyModel::Select(unsigned int* _index, vector<Model*>& sel){
  if(!_index)
    sel.push_back(this);
}

void
MultiBodyModel::Draw(GLenum _mode){
  glColor4fv(GetColor());
  glPushMatrix();

  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->Draw(_mode);

  glPopMatrix();
}

void
MultiBodyModel::DrawSelect(){
  glPushMatrix();

  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawSelect();

  glPopMatrix();
}

void
MultiBodyModel::ParseMultiBody(istream& _is, const string& _modelDir) {
  string multibodyType = ReadFieldString(_is, WHERE,
      "Failed reading Multibody type.");

  if(multibodyType == "ACTIVE")
    m_active = true;
  else if(multibodyType == "SURFACE")
    m_surface = true;

  if(multibodyType == "ACTIVE") {
    size_t numberOfBody = ReadField<size_t>(_is, WHERE,
        "Failed reading body count");

    Color4 color = GetColorFromComment(_is);

    for(size_t i = 0; i < numberOfBody && _is; i++) {
      BodyModel* b = new BodyModel();
      b->ParseActiveBody(_is, _modelDir, color);
      m_bodies.push_back(b);
    }

    //Get connection info
    string connectionTag = ReadFieldString(_is, WHERE,
        "Failed reading connections tag.");
    size_t numberOfConnections = ReadField<size_t>(_is, WHERE,
        "Failed reading number of connections");

    if(numberOfConnections != 0){
      for(size_t i = 0; i < numberOfConnections; ++i) {
        ConnectionModel* c = new ConnectionModel();
        _is >> *c;
        //Increment m_numberOfConnection for each body
        BodyModel* previousBody = m_bodies[c->GetPreviousIndex()];
        previousBody->AddConnection(c);
        m_joints.push_back(c);
      }
    }
  }
  else if(multibodyType == "INTERNAL" ||
      multibodyType == "SURFACE" ||
      multibodyType == "PASSIVE") {

    Color4 color = GetColorFromComment(_is);

    BodyModel* b = new BodyModel(m_surface);
    b->ParseOtherBody(_is, _modelDir, color);
    m_bodies.push_back(b);
  }
  else
    throw ParseException(WHERE,
        "Unsupported body type '" + multibodyType +
        "'. Choices are Active, Passive, Internal, or Surface.");
}

void
MultiBodyModel::BuildRobotStructure() {

  m_dof = 0;

  //only active robots have DOF
  if(!IsActive())
    return;

  //add a node to the robot graph for each body
  for(size_t i = 0; i < m_bodies.size(); i++)
    m_robotGraph.add_vertex(i);

  //Total amount of bodies in environment: free + fixed
  for(BodyIter bit = Begin(); bit != End(); ++bit)
    //For each body, find forward connections and connect them
    for(BodyModel::ConnectionIter cit = (*bit)->Begin(); cit!=(*bit)->End(); ++cit)
      m_robotGraph.add_edge(bit - Begin(), (*cit)->GetNextIndex());

  //Robot ID typedef
  typedef RobotGraph::vertex_descriptor RID;
  vector<pair<size_t, RID> > ccs;
  stapl::vector_property_map<RobotGraph, size_t> cmap;

  //Initialize CC information
  get_cc_stats(m_robotGraph, cmap, ccs);
  for(size_t i = 0; i < ccs.size(); i++){
    cmap.reset();
    vector<RID> cc;
    //Find CCs, construct robot objects
    get_cc(m_robotGraph, cmap, ccs[i].second, cc);
    size_t baseIndx = -1;

    for(size_t j = 0; j < cc.size(); j++) {
      size_t index = m_robotGraph.find_vertex(cc[j])->property();
      if((*(Begin()+index))->IsBase()) {
        baseIndx = index;
        break;
      }
    }

    if(baseIndx == size_t(-1))
      throw ParseException(WHERE, "Robot does not have base.");

    BodyModel* base = *(Begin() + baseIndx);
    if(base->IsBasePlanar()){
      m_dof += 2;
      if(base->IsBaseRotational())
        m_dof += 1;
    }
    else if(base->IsBaseVolumetric()){
      m_dof += 3;
      if(base->IsBaseRotational())
        m_dof += 3;
    }

    Joints jm;
    for(size_t j = 0; j < cc.size(); ++j){
      size_t index = m_robotGraph.find_vertex(cc[j])->property();
      typedef Joints::const_iterator MIT;
      for(MIT mit = m_joints.begin(); mit != m_joints.end(); ++mit){
        if((*mit)->GetPreviousIndex() == index){
          jm.push_back(*mit);
          if((*mit)->GetJointType() == ConnectionModel::REVOLUTE)
            m_dof += 1;
          else if((*mit)->GetJointType() == ConnectionModel::SPHERICAL)
            m_dof += 2;
        }
      }
    }

    sort(jm.begin(), jm.end(), connectionComparator);
    m_robots.push_back(make_pair(base, jm));
  }

  CfgModel::SetDOF(m_dof);
  CfgModel::SetIsPlanarRobot(m_robots[0].first->IsBasePlanar() ? true : false);
  CfgModel::SetIsVolumetricRobot(m_robots[0].first->IsBaseVolumetric() ? true : false);
  CfgModel::SetIsRotationalRobot(m_robots[0].first->IsBaseRotational() ? true : false);
}

