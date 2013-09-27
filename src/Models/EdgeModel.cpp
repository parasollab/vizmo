#include <limits.h>
#include <stdlib.h>

#include "EdgeModel.h"

double EdgeModel::m_edgeThickness = 1;

EdgeModel::EdgeModel() : Model("") {
  m_lp = INT_MAX;
  m_weight = LONG_MAX;
  m_id = -1;
  m_cfgShape = CfgModel::Point;
}

EdgeModel::EdgeModel(double _weight) : Model("") {
  m_lp = INT_MAX;
  m_weight = _weight;
  m_id = -1;
  m_cfgShape = CfgModel::Point;
}

void
EdgeModel::SetName() {
  ostringstream temp;
  temp << "Edge " << m_id;
  m_name = temp.str();
}

void
EdgeModel::Print(ostream& _os) const {
  _os << "Edge ID= " << m_id << endl
    << "Connects Nodes: " << m_startCfg.GetIndex() << " and " << m_endCfg.GetIndex() << endl
    << "Intermediats: ";

  typedef vector<CfgModel>::const_iterator CFGIT;
  for(CFGIT c = m_intermediateCfgs.begin(); c != m_intermediateCfgs.end(); c++)
    _os << *c << " | ";

  _os << endl;
}

vector<int>
EdgeModel::GetEdgeNodes(){

  vector<int> v;
  v.push_back(m_startCfg.GetIndex());
  v.push_back(m_endCfg.GetIndex());
  return v;
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

void
EdgeModel::Draw(GLenum _mode) {

  typedef vector<CfgModel>::iterator CFGIT;
  glPushName(m_id);

  if(m_renderMode == SOLID_MODE || m_renderMode == WIRE_MODE){

    glColor4fv(GetColor());
    glBegin(GL_LINE_STRIP);
    glVertex3dv(m_startCfg.Translation());
    for(CFGIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++){
      glVertex3dv(c->Translation()); //starting point of next line
    }
    glVertex3dv(m_endCfg.Translation());
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
  glLineWidth(m_edgeThickness + 4);

  glBegin(GL_LINE_STRIP);
    glVertex3dv(m_startCfg.Translation());
    for(CFGIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++){
      glVertex3dv(c->Translation()); //starting point of next line
    }
    glVertex3dv(m_endCfg.Translation());
  glEnd();
}

ostream&
operator<<(ostream& _out, const EdgeModel& _edge){
  _out << _edge.m_lp << " " << _edge.m_weight << " ";
  return _out;
}

istream&
operator>>(istream&  _is, EdgeModel& _edge){

  _edge.m_intermediateCfgs.clear();
  int numIntermediates = 0;
  CfgModel cfgtmp;

  _is >> numIntermediates;
  for(int i = 0; i < numIntermediates && _is; i++){
    _is >> cfgtmp;
    _edge.m_intermediateCfgs.push_back(cfgtmp);
  }

  _is >> _edge.m_weight;
  return _is;
}


