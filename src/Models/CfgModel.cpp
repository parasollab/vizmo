#include "CfgModel.h"

#include "RobotModel.h"
#include "Vizmo.h"

double CfgModel::m_defaultDOF = 0;
CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;
float CfgModel::m_boxScale = 1;
bool CfgModel::m_isPlanarRobot = false;
bool CfgModel::m_isVolumetricRobot = false;
bool CfgModel::m_isRotationalRobot = false;

CfgModel::CfgModel() : Model(""), Cfg() {
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}

CfgModel::CfgModel(const Cfg& _c) : Model(""), Cfg(_c) {
  m_index = -1;
  m_isValid = true;
  m_cc = NULL;
  m_isQuery = false;
}

void
CfgModel::SetName() {
  ostringstream temp;
  temp << "Node " << m_index;
  m_name = temp.str();
}

// Fucntion not used:
// this information is controlled by VizmoRoadmapGUI::printNodeCfg()
// in roadmap.cpp, which calls CfgModel::GetNodeInfo()
void
CfgModel::Print(ostream& _os) const {
  _os << "Node ID = " << m_index << endl
    << "Cfg ( " << *this << " )" << endl;

  if(!m_isValid)
    _os << "**** IS IN COLLISION!! ****" << endl;
}

void
CfgModel::SetCfg(const vector<double>& _newCfg) {
  m_v.assign(_newCfg.begin(), _newCfg.end());
}

void
CfgModel::Set(size_t _index, CCModel<CfgModel, EdgeModel>* _cc){
  m_index = _index;
  SetName();
  m_cc = _cc;
}

void
CfgModel::Scale(float _scale){

  m_pointScale = _scale*10;
  m_boxScale = _scale;
}

void
CfgModel::DrawRender(){
  if(m_renderMode == INVISIBLE_MODE)
    return;

  RobotModel* robot = GetVizmo().GetRobot();
  if(m_isValid) {
    glColor4fv(m_color);
    robot->SetColor(m_color);
  }
  else {
    Color4 ic(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0);
    glColor4fv(ic);
    robot->SetColor(ic); //Invert colors. Black case?
  }

  switch(m_shape){
    case Robot:
      robot->BackUp();
      robot->SetRenderMode(m_renderMode);
      robot->Configure(m_v);
      robot->DrawRender();
      robot->Restore();
      break;

    case Box:
      glPushMatrix();
      PerformBoxTranslation();
      glEnable(GL_NORMALIZE);
      if(m_renderMode == SOLID_MODE)
        glutSolidCube(m_boxScale);
      if(m_renderMode == WIRE_MODE)
        glutWireCube(m_boxScale);
      glDisable(GL_NORMALIZE);
      glPopMatrix();
      break;

    case Point:
      glBegin(GL_POINTS);
      glVertex3dv(GetPoint());
      glEnd();
      break;
  }
}

void
CfgModel::DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  switch(m_shape){
    case Robot:
      {
        RobotModel* robot = GetVizmo().GetRobot();
        robot->BackUp();
        robot->SetRenderMode(m_renderMode);
        robot->Configure(m_v);
        robot->DrawSelect();
        robot->Restore();
      }
      break;

    case Box:
      glPushMatrix();
      PerformBoxTranslation();
      if(m_renderMode == SOLID_MODE)
        glutSolidCube(m_boxScale);
      if(m_renderMode == WIRE_MODE)
        glutWireCube(m_boxScale);
      glPopMatrix();
      break;

    case Point:
      glBegin(GL_POINTS);
      glVertex3dv(GetPoint());
      glEnd();
      break;
  }
}

void
CfgModel::PerformBoxTranslation(){
  if(m_isPlanarRobot){
    glTranslated(m_v[0], m_v[1], 0);
    if(m_isRotationalRobot){
      glRotated(m_v[2]*360, 0, 0, 1);
    }
  }
  else if(m_isVolumetricRobot){
    glTranslated(m_v[0], m_v[1], m_v[2]);
    if(m_isRotationalRobot){
      glRotated(m_v[5]*360, 0, 0, 1);
      glRotated(m_v[4]*360, 0, 1, 0);
      glRotated(m_v[3]*360, 1, 0, 0);
    }
  }
}

void
CfgModel::DrawSelected(){
  glDisable(GL_LIGHTING);
  switch(m_shape){
    case Robot:
      {
        RobotModel* robot = GetVizmo().GetRobot();
        robot->BackUp();
        robot->SetRenderMode(WIRE_MODE);
        robot->Configure(m_v);
        robot->DrawSelected();
        robot->Restore();
      }
      break;

    case Box:
      glLineWidth(2);
      glPushMatrix();
      PerformBoxTranslation();
      glutWireCube(m_boxScale+0.1); //may need adjustment
      glPopMatrix();
      break;

    case Point:
      glPointSize(m_pointScale + 3);
      glDisable(GL_LIGHTING);
      glBegin(GL_POINTS);
      if(m_isPlanarRobot)
        glVertex3d(m_v[0], m_v[1], 0);
      else if(m_isVolumetricRobot)
        glVertex3d(m_v[0], m_v[1], m_v[2]);
      glEnd();
      break;
  }
} //end DrawSelect

