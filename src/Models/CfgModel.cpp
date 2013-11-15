#include "CfgModel.h"

#include "CCModel.h"

CfgModel CfgModel::m_invalidCfg;
double CfgModel::m_defaultDOF = 0;
CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;
float CfgModel::m_boxScale = 1;
bool CfgModel::m_isPlanarRobot = false;
bool CfgModel::m_isVolumetricRobot = false;
bool CfgModel::m_isRotationalRobot = false;

CfgModel::CfgModel() : Model("") {
  m_index = -1;
  m_inColl = false;
  m_robot = NULL;
  m_cc = NULL;
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
    << "Cfg ( ";

  for(size_t i = 0; i < m_v.size(); ++i){
    _os << m_v[i];
    _os << (i == m_v.size() - 1 ? " )" : ", ");
  }

  _os << endl;

  if(m_inColl)
    _os << "**** IS IN COLLISION!! ****" << endl;
}

int
CfgModel::GetCCID(){

  int i = m_cc->GetID();
  return i;
}

void
CfgModel::SetCfg(const vector<double>& _newCfg) {
  m_v.assign(_newCfg.begin(), _newCfg.end());
}

void
CfgModel::Set(int _index , RobotModel* _robot, CCModel<CfgModel, EdgeModel>* _cc){
  m_index = _index;
  SetName();
  m_robot = _robot;
  m_cc = _cc;
}

void
CfgModel::Scale(float _scale){

  m_pointScale = _scale*10;
  m_boxScale = _scale;
}

void
CfgModel::Draw(GLenum _mode){

  glPushName(m_index);
  switch(m_shape){
    case Robot:
      DrawRobot();
      break;

    case Box:
      DrawBox();
      break;

    case Point:
      DrawPoint();
      break;
  }
  glPopName();
}

void
CfgModel::DrawRobot(){
  if(m_robot == NULL)
    return;

  m_robot->BackUp();
  m_robot->SetRenderMode(m_renderMode);

  if(m_inColl)
    m_robot->SetColor(Color4(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0)); //Invert colors. Black case?
  else
    m_robot->SetColor(m_color);

  m_robot->Configure(m_v);
  m_robot->Draw(GL_RENDER);
  m_robot->Restore();
}

void
CfgModel::DrawBox(){

  glPushMatrix();

  if(m_inColl)
    glColor4fv(Color4(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0));
  else
    glColor4fv(m_color);

  //If base is not FIXED, perform translations
  //Additionally, perform rotations if base is also ROTATIONAL
  if(m_isPlanarRobot){
    glTranslated(m_v[0], m_v[1], 0);
    if(m_isRotationalRobot){
      //2D planar robot can have one rotational DOF
      glRotated(m_v[2]*360, 0, 0, 1);
    }
  }
  else if(m_isVolumetricRobot){
    glTranslated(m_v[0], m_v[1], m_v[2]);
    if(m_isRotationalRobot){
      //3D volumetric robot can have 3 rotational DOFs
      glRotated(m_v[5]*360, 0, 0, 1);
      glRotated(m_v[4]*360, 0, 1, 0);
      glRotated(m_v[3]*360, 1, 0, 0);
    }
  }

  glEnable(GL_NORMALIZE);
  if(m_renderMode == SOLID_MODE)
    glutSolidCube(m_boxScale);
  if(m_renderMode == WIRE_MODE)
    glutWireCube(m_boxScale);
  glDisable(GL_NORMALIZE);
  glPopMatrix();
}

void
CfgModel::DrawPoint(){

  glBegin(GL_POINTS);

  if(m_inColl)
    glColor4fv(Color4(1.0-m_color[0], 1.0-m_color[1], 1.0-m_color[2], 0.0));
  else
    glColor4fv(GetColor());

  if(m_renderMode == SOLID_MODE ||
      m_renderMode == WIRE_MODE){
    if(m_isPlanarRobot)
      glVertex3d(m_v[0], m_v[1], 0);
    if(m_isVolumetricRobot)
      glVertex3d(m_v[0], m_v[1], m_v[2]);
  }
  glEnd();
}

void
CfgModel::DrawSelect(){

  glDisable(GL_LIGHTING);
  switch(m_shape){
    case Robot:
      if(m_robot != NULL){
        m_robot->BackUp();
        m_robot->SetColor(GetColor());
        m_robot->Configure(m_v);
        m_robot->DrawSelect();
        m_robot->Restore();
      }
      break;

    case Box:
      glLineWidth(2);
      glPushMatrix();

      if(m_isPlanarRobot){
        glTranslated(m_v[0], m_v[1], 0);
        if(m_isRotationalRobot)
          glRotated(m_v[2]*360, 0, 0, 1);
      }

      else if(m_isVolumetricRobot){
        glTranslated(m_v[0], m_v[1], m_v[2]);
        if(m_isRotationalRobot){
          glRotated(m_v[5]*360, 0, 0, 1);
          glRotated(m_v[4]*360, 0, 1, 0);
          glRotated(m_v[3]*360, 1, 0, 0);
        }
      }
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

