#include <stdlib.h>
#include <limits.h>

#include "CCModel.h"
#include "CfgModel.h"

CfgModel CfgModel::m_invalidCfg;
int CfgModel::m_dof = 0;
double CfgModel::m_defaultDOF = 0;
CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
float CfgModel::m_pointScale = 10;
float CfgModel::m_boxScale = 1;
bool CfgModel::m_isPlanarRobot = false;
bool CfgModel::m_isVolumetricRobot = false;
bool CfgModel::m_isRotationalRobot = false;

CfgModel::CfgModel(){
  m_index = -1;
  m_coll = false;
  m_dofs.clear();
  m_robot = NULL;
  m_cc = NULL;
}

CfgModel::CfgModel(const CfgModel& _cfg) : Model(_cfg) {
  m_index = _cfg.m_index;
  m_robot = _cfg.m_robot;
  m_cc = _cfg.m_cc;
  m_coll = _cfg.m_coll;
  m_dofs = _cfg.m_dofs;
}

const string
CfgModel::GetName() const {

  ostringstream temp;
  temp << "Node" << m_index;
  return temp.str();
}

// Fucntion not used:
// this information is controlled by VizmoRoadmapGUI::printNodeCfg()
// in roadmap.cpp, which calls CfgModel::GetNodeInfo()
vector<string>
CfgModel::GetInfo() const {

  vector<string> info;
  int dof = CfgModel::m_dof;

  ostringstream temp;
  temp << "Node ID = " << m_index << " ";
  temp << " Cfg ( ";

  for(int i=0; i<dof;i++){
    temp << m_dofs[i];
    if(i == dof-1)
      temp << " )";
    else
      temp << ", ";
  }

  info.push_back(temp.str());

  if(m_coll)
    info.push_back("\t\t **** IS IN COLLISION!! ****");

  return info;
}

vector<string>
CfgModel::GetNodeInfo() const {

  vector<string> info;
  int dof = CfgModel::m_dof;

  ostringstream temp;
  temp << "Node ID = " << m_index << " ";

  for(int i=0; i<dof; i++){
    if(i < 3)
      temp << m_dofs[i];
    else
      temp << m_dofs[i];
    if(i == dof-1)
      temp << " )";
    else
      temp << ", ";
  }
  info.push_back(temp.str());
  return info;
}

int
CfgModel::GetCCID(){

  int i = m_cc->GetID();
  return i;
}

void
CfgModel::SetCfg(vector<double> _newCfg){
  m_dofs.assign(_newCfg.begin(), _newCfg.end());
  Translation()(m_dofs[0], m_dofs[1], m_isVolumetricRobot ? m_dofs[2] : 0);
}

bool
CfgModel::operator==(const CfgModel& _other) const{
  for(vector<double>::const_iterator dit1 = m_dofs.begin(), dit2 = _other.m_dofs.begin();
      dit1 != m_dofs.end() && dit2 != _other.m_dofs.end(); ++dit1, ++dit2){
    if(fabs(*dit1 - *dit2) > 0.0000001)
      return false;
  }
  return true;
}

void
CfgModel::Set(int _index , RobotModel* _robot, CCModel<CfgModel, EdgeModel>* _cc){
  m_index = _index;
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
  m_robot->SetColor(m_color);
  m_robot->Configure(m_dofs);
  m_robot->Draw(GL_RENDER);
  m_robot->Restore();
}

void
CfgModel::DrawBox(){

  glPushMatrix();
  glColor4fv(m_color);

  //If base is not FIXED, perform translations
  //Additionally, perform rotations if base is also ROTATIONAL
  if(m_isPlanarRobot){
    glTranslated(m_dofs[0], m_dofs[1], 0);
    if(m_isRotationalRobot){
      //2D planar robot can have one rotational DOF
      glRotated(m_dofs[2]*360, 0, 0, 1);
    }
  }
  else if(m_isVolumetricRobot){
    glTranslated(m_dofs[0], m_dofs[1], m_dofs[2]);
    if(m_isRotationalRobot){
      //3D volumetric robot can have 3 rotational DOFs
      glRotated(m_dofs[5]*360, 0, 0, 1);
      glRotated(m_dofs[4]*360, 0, 1, 0);
      glRotated(m_dofs[3]*360, 1, 0, 0);
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
  glColor4fv(GetColor());
  if(m_renderMode == SOLID_MODE ||
      m_renderMode == WIRE_MODE){
    if(m_isPlanarRobot)
      glVertex3d(m_dofs[0], m_dofs[1], 0);
    if(m_isVolumetricRobot)
      glVertex3d(m_dofs[0], m_dofs[1], m_dofs[2]);
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
        m_robot->Configure(m_dofs);
        m_robot->DrawSelect();
        m_robot->Restore();
      };
      break;

    case Box:
      glLineWidth(2);
      glPushMatrix();

      if(m_isPlanarRobot){
        glTranslated(m_dofs[0], m_dofs[1], 0);
        if(m_isRotationalRobot)
          glRotated(m_dofs[2]*360, 0, 0, 1);
      }

      else if(m_isVolumetricRobot){
        glTranslated(m_dofs[0], m_dofs[1], m_dofs[2]);
        if(m_isRotationalRobot){
          glRotated(m_dofs[5]*360, 0, 0, 1);
          glRotated(m_dofs[4]*360, 0, 1, 0);
          glRotated(m_dofs[3]*360, 1, 0, 0);
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
        glVertex3d(m_dofs[0], m_dofs[1], 0);
      else if(m_isVolumetricRobot)
        glVertex3d(m_dofs[0], m_dofs[1], m_dofs[2]);
      glEnd();
      break;
  }
} //end DrawSelect

ostream&
operator<<(ostream& _out, const CfgModel& _cfg){
  for(unsigned int i = 0; i < (_cfg.m_dofs).size(); i++)
    _out << (_cfg.m_dofs)[i] << " ";
  _out << " ";
  return _out;
}

istream&
operator>>(istream& _in, CfgModel& _cfg){

  _cfg.m_dofs.clear();

  //For now, read in and discard robot index;
  int robotIndex;
  _in >> robotIndex;

  for(int i = 0; i < CfgModel::m_dof; i++){
    double value;
    _in >> value;
    _cfg.m_dofs.push_back(value);
  }

  _cfg.Translation()(_cfg.m_dofs[0], _cfg.m_dofs[1],
      CfgModel::m_isVolumetricRobot ? _cfg.m_dofs[2] : 0);

  return _in;
}

