#include <stdlib.h>
#include <limits.h>

#include "CfgModel.h"
#include "Plum/EnvObj/RobotInfo.h"
#include "CCModel.h"


CfgModel CfgModel::m_invalidCfg;
int CfgModel::m_dof = 0;
double CfgModel::m_defaultDOF = 0;
CfgModel::Shape CfgModel::m_shape = CfgModel::Point;
bool CfgModel::m_isPlanarRobot = false;
bool CfgModel::m_isVolumetricRobot = false;
bool CfgModel::m_isRotationalRobot = false;

CfgModel::CfgModel(){

  m_unknown1 = LONG_MAX;
  m_unknown2 = LONG_MAX;
  m_unknown3 = LONG_MAX;
  m_index = -1;
  m_coll = false;
  m_dofs.clear();
}

CfgModel::CfgModel(const CfgModel& _cfg){

  m_index = _cfg.m_index;
  m_robot = _cfg.m_robot;
  m_cc = _cfg.m_cc;
  m_coll = _cfg.m_coll;
  m_dofs = _cfg.m_dofs;
  m_isPlanarRobot = _cfg.m_isPlanarRobot;
  m_isVolumetricRobot = _cfg.m_isVolumetricRobot;
  m_isRotationalRobot = _cfg.m_isRotationalRobot;

  m_unknown1 = _cfg.m_unknown1;
  m_unknown2 = _cfg.m_unknown2;
  m_unknown3 = _cfg.m_unknown3;
}

CfgModel::~CfgModel() {}

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

//Function accessed to write data into a new .map file
vector<double>
CfgModel::GetUnknowns(){

  vector<double> v;
  v.push_back(m_unknown1);
  v.push_back(m_unknown2);
  v.push_back(m_unknown3);
  return v;
}

void
CfgModel::SetCfg(vector<double> _newCfg){

  m_dofs.assign(_newCfg.begin(), _newCfg.end());
  m_unknown1 = m_unknown2 = m_unknown3 = -1;
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

  if(m_robot != NULL){
    vector<struct Robot>& exampleBots = m_robot->m_envModel->GetRobots();
    const struct Robot& exampleBot = *(exampleBots.begin());
    m_isPlanarRobot = (exampleBot.m_base == Robot::PLANAR) ? true : false;
    m_isVolumetricRobot = (exampleBot.m_base == Robot::VOLUMETRIC) ? true : false;
    m_isRotationalRobot = (exampleBot.m_baseMovement == Robot::ROTATIONAL) ? true : false;
  }
  else cout<<"NULL ROBOT in CfgModel::Set!"<<endl;
}

void
CfgModel::Draw(GLenum _mode){

  glPushName(m_index);
  Shape shape = (m_cc == NULL ? m_shape : (Shape)m_cc->GetShape());
  switch(shape){
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

  vector<double> cfg = m_dofs;
  Color4 origColor = m_robot->GetColor();

  m_robot->RestoreInitCfg();
  m_robot->BackUp();
  glColor4fv(GetColor());
  m_robot->SetRenderMode(m_renderMode);
  m_robot->SetColor(GetColor());
  m_robot->Scale(m_scale[0], m_scale[1], m_scale[2]);
  m_robot->Configure(cfg);
  m_robot->Draw(GL_RENDER);
  m_robot->Restore();
  m_robot->SetColor(origColor);
}

void
CfgModel::DrawBox(){

  glEnable(GL_LIGHTING);
  glPushMatrix();
  glColor4fv(GetColor());

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

  //glScale(); What does this call do?
  glTransform();
  glEnable(GL_NORMALIZE);
  if(m_renderMode == SOLID_MODE)
    glutSolidCube(m_scale[0]);
  if(m_renderMode == WIRE_MODE)
    glutWireCube(m_scale[0]);
  glDisable(GL_NORMALIZE);
  glPopMatrix();
}

void
CfgModel::DrawPoint(){

  glDisable(GL_LIGHTING);
  glPointSize(m_scale[0]);
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

  glColor3d(1,1,0);
  glDisable(GL_LIGHTING);
  Shape shape = (m_cc == NULL ? m_shape : (Shape)m_cc->GetShape());
  switch(shape){
    case Robot:
      if(m_robot!=NULL){
        vector<double> cfg = m_dofs;
        m_robot->BackUp();
        Color4 origColor = m_robot->GetColor();
        //change
        m_robot->SetColor(GetColor());
        m_robot->Scale(m_scale[0], m_scale[1], m_scale[2]);
        m_robot->Configure(cfg);
        //delete[] cfg;
        //draw
        m_robot->DrawSelect();
        m_robot->Restore();
        m_robot->SetColor(origColor);
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
          glRotated(m_dofs[4]*360,  0, 1, 0);
          glRotated(m_dofs[3]*360, 1, 0, 0);
        }
      }

      //TODO:Fix different paces of item and selection box
      //outline rescaling
      glutWireCube(m_scale[0]+0.1);

      glPopMatrix();
      break;

    case Point:
      glPointSize(m_scale[0] + 3);
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

void
CfgModel::Dump(){

  cout << "- ID = " << m_index << endl;
  cout << "- Position = (" << m_dofs[0] << ", "
       << m_dofs[1] << ", " << m_dofs[2] << ")" << endl;
  cout << "- Orientation = (";

  int dof = CfgModel::m_dof;
  for(int i=0; i<dof-3; i++)
    printf("%f ", m_dofs[i+3]*360);
  cout << ")" << endl;
}

void
CfgModel::CopyCfg() {

  ObjCfg.clear();
  ObjCfg.assign(m_dofs.begin(), m_dofs.end());
}

double&
CfgModel::tx(){

  ObjName = "Node";
  CopyCfg();
  return m_dofs[0];
}

double&
CfgModel::ty(){

  return m_dofs[1];
}

double&
CfgModel::tz(){

  if(m_isVolumetricRobot){
    return m_dofs[2];
  }
  else
    return m_defaultDOF; //Planar robot; m_dofs[2] is possible rotation
}

const double&
CfgModel::tx() const {

  return m_dofs[0];
}

const double&
CfgModel::ty() const {

  return m_dofs[1];
}

const double&
CfgModel::tz() const {

  if(m_isVolumetricRobot)
    return m_dofs[2];
  else
    return m_defaultDOF; //Planar robot; m_dofs[2] is possible rotation
}


