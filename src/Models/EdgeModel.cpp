#include "EdgeModel.h"

#include <limits.h>
#include <cstdlib>

#include "CfgModel.h"

double EdgeModel::m_edgeThickness = 1;

EdgeModel::EdgeModel(double _weight) : Model(""), DefaultWeight<CfgModel>("", _weight), m_id(-1) {
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
    << "Connects Nodes: " << m_startCfg->GetIndex() << " and " << m_endCfg->GetIndex() << endl
    << "Intermediates: ";

  typedef vector<CfgModel>::const_iterator CFGIT;
  for(CFGIT c = m_intermediates.begin(); c != m_intermediates.end(); c++)
    _os << *c << " | ";

  _os << endl;
}

void
EdgeModel::Set(int _id, CfgModel* _c1, CfgModel* _c2, RobotModel* _robot){

  m_id = _id;
  m_startCfg = _c1;
  m_endCfg = _c2;

  SetName();

  typedef vector<CfgModel>::iterator CIT;
  for(CIT c = m_intermediates.begin();
      c != m_intermediates.end(); c++){
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
    glVertex3dv(m_startCfg->GetPoint());
    for(CFGIT c = m_intermediates.begin();
        c != m_intermediates.end(); c++){
      glVertex3dv(c->GetPoint()); //starting point of next line
    }
    glVertex3dv(m_endCfg->GetPoint());
    glEnd();

    //draw intermediate configurations
    if(CfgModel::GetShape() == CfgModel::Box || CfgModel::GetShape() == CfgModel::Robot){
      for(CFGIT c = m_intermediates.begin();
          c != m_intermediates.end(); c++){
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
    glVertex3dv(m_startCfg->GetPoint());
    for(CFGIT c = m_intermediates.begin();
        c != m_intermediates.end(); c++){
      glVertex3dv(c->GetPoint()); //starting point of next line
    }
    glVertex3dv(m_endCfg->GetPoint());
  glEnd();
}

