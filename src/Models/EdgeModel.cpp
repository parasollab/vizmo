#include <limits.h>
#include <stdlib.h>

#include "EdgeModel.h"

EdgeModel::EdgeModel() {
  m_lp = INT_MAX;
  m_weight = LONG_MAX;
  m_id = -1;
  m_edgeThickness = 1;
}

EdgeModel::EdgeModel(double _weight) {
  m_lp = INT_MAX;
  m_weight = _weight;
  m_id = -1;
  m_edgeThickness = 1;
}

EdgeModel::~EdgeModel() {
  m_lp = INT_MAX;
  m_weight = LONG_MAX;
}

const string
EdgeModel::GetName() const {

  ostringstream temp;
  temp << "Edge" << m_id;
  return temp.str();
}

vector<string>
EdgeModel::GetInfo() const {

  vector<string> info;
  ostringstream temp;

  temp << "Edge, ID= " << m_id << ", ";
  temp << "connects Node " << m_startCfg.GetIndex() << " and Node " << m_endCfg.GetIndex();
  temp << "Intermediates | ";

  typedef vector<CfgModel>::const_iterator CFGIT;
  for(CFGIT c = m_intermediateCfgs.begin(); c != m_intermediateCfgs.end(); c++)
    temp << *c << " | ";
  info.push_back(temp.str());

  return info;
}

vector<int>
EdgeModel::GetEdgeNodes(){

  vector<int> v;
  v.push_back(m_startCfg.GetIndex());
  v.push_back(m_endCfg.GetIndex());
  return v;
}

//Changing edge thickness: step 5
void
EdgeModel::SetThickness(size_t _thickness){
  m_edgeThickness = _thickness;
}

bool
EdgeModel::operator==(const EdgeModel& _other){

  if(m_lp != _other.m_lp || m_weight != _other.m_weight)
    return false;
  return true;
}

void
EdgeModel::Set(int _id, CfgModel* _c1, CfgModel* _c2, RobotModel* _robot){

  m_id = _id;
  m_startCfg = *_c1;
  m_endCfg = *_c2;

  typedef vector<CfgModel>::iterator CIT;
  for(CIT c = m_intermediateCfgs.begin();
      c != m_intermediateCfgs.end(); c++){
    c->SetRobot(_robot);
  }
}

//Changing edge thickness: final step. GL lines drawn with set width.
void
EdgeModel::Draw(GLenum _mode) {

  typedef vector<CfgModel>::iterator CFGIT;
  glPushName(m_id);

  if(m_renderMode == SOLID_MODE || m_renderMode == WIRE_MODE){

    glColor4fv(GetColor());
    glLineWidth(m_edgeThickness);

    glBegin(GL_LINES);
    glVertex3d(m_startCfg.tx(), m_startCfg.ty(), m_startCfg.tz());

    for(CFGIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++){
      glVertex3d (c->tx(), c->ty(), c->tz()); //ending point of prev line
      glVertex3d (c->tx(), c->ty(), c->tz()); //starting point of next line
    }

    glVertex3d(m_endCfg.tx(), m_endCfg.ty(), m_endCfg.tz());
    glEnd();

    //draw intermediate configurations
    if(m_cfgShape == CfgModel::Box || m_cfgShape == CfgModel::Robot){
      for(CFGIT c = m_intermediateCfgs.begin();
          c != m_intermediateCfgs.end(); c++){
          c->SetShape(m_cfgShape);
          c->SetRenderMode(WIRE_MODE);
          c->Draw(_mode);
      }
    }
  }
  glPopName();
}

void
EdgeModel::DrawSelect(){

  typedef vector<CfgModel>::iterator CFGIT;
  glColor3d(1,1,0);
  glLineWidth(m_edgeThickness + 4); //Ensure visibility around edge itself

  glBegin(GL_LINES);
  glVertex3d(m_startCfg.tx(), m_startCfg.ty(), m_startCfg.tz());

  for(CFGIT c = m_intermediateCfgs.begin();
      c != m_intermediateCfgs.end(); c++) {
    glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
    glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
  }

  glVertex3d( m_endCfg.tx(),m_endCfg.ty(),m_endCfg.tz() );
  glEnd();
}

ostream&
operator<<(ostream& _out, const CfgModel& _cfg){

  for(unsigned int i = 0; i < (_cfg.m_dofs).size(); i++)
    _out << (_cfg.m_dofs)[i] << " ";

  _out << " ";
  return _out;
}

istream&
operator>>(istream& _in, CfgModel& _cfg){

  (_cfg.m_dofs).clear();
  int dof = CfgModel::m_dof;

  //For now, read in and discard robot index;
  int robotIndex;
  _in >> robotIndex;

  for(int i = 0; i < dof; i++){
    double value;
    _in >> value;
    (_cfg.m_dofs).push_back(value);
  }

  return _in;
}

ostream&
operator<<(ostream& _out, const EdgeModel& _edge){

  _out << _edge.m_lp << " " << _edge.m_weight << " ";
  return _out;
}

istream&
operator>>(istream&  _in, EdgeModel& _edge){

  _edge.m_intermediateCfgs.clear();
  int numIntermediates = 0;
  CfgModel cfgtmp;

  _in >> numIntermediates;
  for(int i = 0; i < numIntermediates; i++){
    _in >> cfgtmp;
    _edge.m_intermediateCfgs.push_back(cfgtmp);
  }

  _in >> _edge.m_weight;
  return _in;
}


