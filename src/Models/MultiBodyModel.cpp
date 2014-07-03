#include "MultiBodyModel.h"

#include <GraphAlgo.h>

#include "BodyModel.h"
#include "BoundingBoxModel.h"
#include "BoundaryModel.h"
#include "BoundingSphereModel.h"
#include "CfgModel.h"
#include "ConnectionModel.h"
#include "EnvModel.h"
#include "Vizmo.h"
#include "Utilities/IO.h"

vector<MultiBodyModel::DOFInfo> MultiBodyModel::m_dofInfo = vector<MultiBodyModel::DOFInfo>();

class IsConnectionGloballyFirst {
  public:
    bool operator()(const ConnectionModel* _a, const ConnectionModel* _b) const {
      return _a->GetGlobalIndex() < _b->GetGlobalIndex();
    }
} connectionComp;

MultiBodyModel::MultiBodyModel(EnvModel* _env) : Model("MultiBody"),
  m_envModel(_env), m_active(false), m_surface(false), m_radius(0) {
  }

MultiBodyModel::MultiBodyModel(EnvModel* _env,
    const string& _modelDataDir, const string& _filename,
    const Transformation& _t) : Model("MultiBody"),
  m_envModel(_env), m_active(false), m_surface(false) {
    m_bodies.push_back(new BodyModel(_modelDataDir, _filename, _t));
    Build();
  }

MultiBodyModel::MultiBodyModel(const MultiBodyModel& _m) : Model(_m),
  m_envModel(_m.m_envModel),
  m_active(_m.m_active), m_surface(_m.m_surface) {
    //copy all bodies
    for(BodyIter bit = _m.Begin(); bit != _m.End(); ++bit)
      m_bodies.push_back(new BodyModel(**bit));
    //copy all connections
    typedef vector<ConnectionModel*>::const_iterator CIT;
    for(CIT cit = _m.m_joints.begin(); cit != _m.m_joints.end(); ++cit)
      m_joints.push_back(new ConnectionModel(**cit));
    //build models
    Build();
  }

MultiBodyModel::~MultiBodyModel() {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    delete *bit;
}

void
MultiBodyModel::SetRenderMode(RenderMode _mode){
  Model::SetRenderMode(_mode);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetRenderMode(_mode);
}

void
MultiBodyModel::SetSelectable(bool _s){
  m_selectable = _s;
  for(BodyIter bit = Begin(); bit != End(); bit++)
    (*bit)->SetSelectable(_s);
}

void
MultiBodyModel::SetColor(const Color4& _c) {
  Model::SetColor(_c);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->SetColor(_c);
}

void
MultiBodyModel::Build() {
  //build for each body and compute com
  m_com(0, 0, 0);
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    m_com += (*bit)->GetTransform().translation();
  m_com /= m_bodies.size();

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
  if(m_selectable && _index)
    sel.push_back(this);
}

void
MultiBodyModel::DrawRender() {
  glColor4fv(GetColor());
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawRender();
}

void
MultiBodyModel::DrawSelect() {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawSelect();
}

void
MultiBodyModel::DrawSelected(){
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawSelected();
}

void
MultiBodyModel::
DrawHaptics() {
  for(BodyIter bit = Begin(); bit!=End(); ++bit)
    (*bit)->DrawHaptics();
}

void
MultiBodyModel::Print(ostream& _os) const {
  _os << Name() << endl;
  if(m_active) {
    _os << "Active" << endl;
    _os << m_bodies.size() << endl;
  }
  else {
    _os << "Passive" << endl;
    m_bodies[0]->Print(_os);
  }
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
MultiBodyModel::AddJoint(ConnectionModel* _c, int _indexBase, int _indexJoint, int _bodyNum){
  m_robots.at(_indexBase).second.insert(m_robots.at(_indexBase).second.begin()+_indexJoint, _c);
  m_joints.insert(m_joints.begin()+(_bodyNum-(_indexBase+1)), _c);
}

void
MultiBodyModel::AddBody(BodyModel* _b, int _index){
  if(_index <= (int)m_bodies.size())
    m_bodies.insert(m_bodies.begin()+_index, _b);
  else
    m_bodies.push_back(_b);
}

void
MultiBodyModel::DeleteBody(int _index){
  m_bodies.erase(m_bodies.begin()+_index);
}

void
MultiBodyModel::DeleteJoint(int _indexBase, int _indexJoint, int _bodyNumber){
  m_joints.erase(m_joints.begin()+(_bodyNumber-(_indexBase+1)));
  m_robots.at(_indexBase).second.erase(m_robots.at(_indexBase).second.begin()+_indexJoint);
}

void
MultiBodyModel::DeleteJoints(){
  while(!m_joints.empty())
    m_joints.pop_back();
}

void
MultiBodyModel::DeleteRobot(int _index){
  m_robots.erase(m_robots.begin()+_index);
}

void
MultiBodyModel::AddBase(BodyModel* _newBase){
  m_robots.push_back(make_pair(_newBase, NULL));
}

void
MultiBodyModel::BuildRobotStructure() {

  m_dof = 0;

  if(!IsActive())
    return;

  vector<pair<double, double> > dofRanges = m_envModel->GetBoundary()->GetRanges();

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
  stapl::sequential::vector_property_map<RobotGraph, size_t> cmap;

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
      m_dofInfo.push_back(DOFInfo("Base X Translation ",
            dofRanges[0].first, dofRanges[0].second));
      m_dofInfo.push_back(DOFInfo("Base Y Translation ",
            dofRanges[1].first, dofRanges[1].second));
      if(base->IsBaseRotational()){
        m_dof += 1;
        m_dofInfo.push_back(DOFInfo("Base Rotation ", -1.0, 1.0));
      }
    }
    else if(base->IsBaseVolumetric()){
      m_dof += 3;
      m_dofInfo.push_back(DOFInfo("Base X Translation ",
            dofRanges[0].first, dofRanges[0].second));
      m_dofInfo.push_back(DOFInfo("Base Y Translation ",
            dofRanges[1].first, dofRanges[1].second));
      m_dofInfo.push_back(DOFInfo("Base Z Translation ",
            dofRanges[2].first, dofRanges[2].second));
      if(base->IsBaseRotational()){
        m_dof += 3;
        m_dofInfo.push_back(DOFInfo("Base X Rotation ", -1.0, 1.0));
        m_dofInfo.push_back(DOFInfo("Base Y Rotation ", -1.0, 1.0));
        m_dofInfo.push_back(DOFInfo("Base Z Rotation ", -1.0, 1.0));
      }
    }

    Joints jm;
    for(size_t j = 0; j < cc.size(); ++j){
      size_t index = m_robotGraph.find_vertex(cc[j])->property();
      typedef Joints::const_iterator MIT;
      int jointIdx = 0;
      for(MIT mit = m_joints.begin(); mit != m_joints.end(); ++mit){
        if((*mit)->GetPreviousIndex() == index){
          jm.push_back(*mit);
          if((*mit)->GetJointType() == ConnectionModel::REVOLUTE){
            m_dof += 1;
            ostringstream oss;
            oss << "Revolute Joint " << jointIdx << " Angle ";
            m_dofInfo.push_back(DOFInfo(oss.str(), -1.0, 1.0));
          }
          else if((*mit)->GetJointType() == ConnectionModel::SPHERICAL){
            m_dof += 2;
            ostringstream oss;
            oss << "Spherical Joint " << jointIdx << " Angle 1 ";
            m_dofInfo.push_back(DOFInfo(oss.str(), -1.0, 1.0));
            oss.str("");
            oss.clear();
            oss << "Spherical Joint " << jointIdx << " Angle 2 ";
            m_dofInfo.push_back(DOFInfo(oss.str(), -1.0, 1.0));
          }
        }
        jointIdx++;
      }
    }

    sort(jm.begin(), jm.end(), connectionComp);
    m_robots.push_back(make_pair(base, jm));
  }

  CfgModel::SetDOF(m_dof);
  CfgModel::SetIsPlanarRobot(m_robots[0].first->IsBasePlanar() ? true : false);
  CfgModel::SetIsVolumetricRobot(m_robots[0].first->IsBaseVolumetric() ? true : false);
  CfgModel::SetIsRotationalRobot(m_robots[0].first->IsBaseRotational() ? true : false);
}

void
MultiBodyModel::ChangeDOF(int _dof){
  m_dof=_dof;
  CfgModel::SetDOF(m_dof);
}
