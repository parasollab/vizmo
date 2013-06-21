#include "Cfg.h"

#include <stdlib.h>
#include <limits.h>

#include "Plum/Plum.h"
#include "Plum/EnvObj/RobotInfo.h"   
#include "CCModel.h"

namespace plum{

  //init m_invalidCfg and DOF
  CCfg CCfg::m_invalidCfg;
  int CCfg::m_dof = 0;
  double CCfg::m_defaultDOF = 0;
  CCfg::Shape CCfg::m_shape = CCfg::Point;
  bool CCfg::m_isPlanarRobot = false; 
  bool CCfg::m_isVolumetricRobot = false; 
  bool CCfg::m_isRotationalRobot = false; 

  CCfg::CCfg(){
    
    m_unknow1 = LONG_MAX;
    m_unknow2 = LONG_MAX;
    m_unknow3 = LONG_MAX;
    m_index = -1;
    m_coll = false;
    m_dofs.clear();
  }

  CCfg::~CCfg() {}

  CCfg::CCfg(const CCfg& _cfg){
    m_index = _cfg.m_index;
    m_robot = _cfg.m_robot;
    m_cc = _cfg.m_cc;
    m_coll = _cfg.m_coll;
    m_dofs = _cfg.m_dofs;
    m_isPlanarRobot = _cfg.m_isPlanarRobot; 
    m_isVolumetricRobot = _cfg.m_isVolumetricRobot; 
    m_isRotationalRobot = _cfg.m_isRotationalRobot; 

    m_unknow1 = _cfg.m_unknow1;
    m_unknow2 = _cfg.m_unknow2;
    m_unknow3 = _cfg.m_unknow3;
  }

  void 
  CCfg::Set(int _index , OBPRMView_Robot* _robot, CCModelBase* _cc){   
    m_index = _index;
    m_robot = _robot;
    m_cc = _cc;
    
    vector<struct Robot>& exampleBots = m_robot->m_pEnvLoader->GetRobots(); 
    const struct Robot& exampleBot = *(exampleBots.begin());  
    m_isPlanarRobot = (exampleBot.m_base == Robot::PLANAR) ? true : false; 
    m_isVolumetricRobot = (exampleBot.m_base == Robot::VOLUMETRIC) ? true : false;
    m_isRotationalRobot = (exampleBot.m_baseMovement == Robot::ROTATIONAL) ? true : false; 
  }

  int 
  CCfg::GetCC_ID(){
    int i = m_cc->ID(); 
    return i; 
  }

  void 
  CCfg::DrawRobot(){
    
    if(m_robot==NULL)
      return;

    vector<double> cfg = m_dofs;  
    vector<float> origColor = m_robot->GetColor();  
    
    m_robot->RestoreInitCfg();
    m_robot->BackUp();
    float* rgbaArray = &m_RGBA[0];  
    glColor4fv(rgbaArray);
    m_robot->SetRenderMode(m_renderMode);
    m_robot->SetColor(m_RGBA[0], m_RGBA[1], m_RGBA[2], 1);
    m_robot->Scale(m_Scale[0], m_Scale[1], m_Scale[2]);
    m_robot->Configure(cfg);   
    m_robot->Draw(GL_RENDER);
    m_robot->Restore();
    m_robot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]); 
  }

  void 
  CCfg::DrawBox(){

    glEnable(GL_LIGHTING);
    glPushMatrix();
    float* rgbaArray = &m_RGBA[0]; 
    glColor4fv(rgbaArray);
    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
   
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
      glutSolidCube(m_Scale[0]); 
    if(m_renderMode == WIRE_MODE)
      glutWireCube(m_Scale[0]); 
    glDisable(GL_NORMALIZE);
    glPopMatrix();
  }

  void 
  CCfg::DrawPoint(){
    
    glDisable(GL_LIGHTING);
    glPointSize(m_Scale[0]); 
    glBegin(GL_POINTS);
    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
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
  CCfg::Draw(GLenum _mode){
    
    glPushName(m_index);
    Shape shape = (m_cc == NULL ? m_shape : (Shape)m_cc->getShape());
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
  CCfg::DrawSelect(){ 
    
    glColor3d(1,1,0);
    glDisable(GL_LIGHTING);
    Shape shape = (m_cc == NULL ? m_shape : (Shape)m_cc->getShape());
    switch(shape){
      case Robot:   
        if(m_robot!=NULL){
          vector<double> cfg = m_dofs; 
          m_robot->BackUp();
          vector<float> origColor = m_robot->GetColor();   
          //change
          m_robot->SetColor(1,1,0,0);
          m_robot->SetColor(m_RGBA[0], m_RGBA[1], m_RGBA[2], 1);
          m_robot->Scale(m_Scale[0], m_Scale[1], m_Scale[2]);
          m_robot->Configure(cfg);
          //delete[] cfg;
          //draw 
          m_robot->DrawSelect();
          m_robot->Restore();  
          m_robot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]);  
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
        glutWireCube(m_Scale[0]+0.1);
        
        glPopMatrix(); 
        break;

      case Point: 
        glPointSize(m_Scale[0] + 3); 
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

  bool 
  CCfg::operator==(const CCfg& _other) const{

    for(vector<double>::const_iterator dit1 = m_dofs.begin(), dit2 = _other.m_dofs.begin();
        dit1 != m_dofs.end() && dit2 != _other.m_dofs.end(); ++dit1, ++dit2){
      if(fabs(*dit1 - *dit2) > 0.0000001)
        return false;
    }
    return true;
  }

  // Fucntion not used:   
  // this information is controlled by VizmoRoadmapGUI::printNodeCfg() 
  // in roadmap.cpp, which calls CCfg::GetNodeInfo()

  vector<string> 
  CCfg::GetInfo() const{
    
    vector<string> info; 
    int dof = CCfg::m_dof;
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
  CCfg::GetNodeInfo() const{
    
    vector<string> info; 
    int dof = CCfg::m_dof;
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

  void 
  CCfg::Dump(){
    
    cout << "- ID = " << m_index << endl;
    cout << "- Position = (" << m_dofs[0] << ", " <<
      m_dofs[1] << ", " << m_dofs[2] << ")" << endl;
    cout << "- Orientation = (";
    int dof = CCfg::m_dof;
    for(int i=0; i<dof-3; i++){
      printf("%f ", m_dofs[i+3]*360);
    }
    cout << ")" << endl;
  }

  void 
  CCfg::SetCfg(vector<double> _newCfg){
    m_dofs.assign(_newCfg.begin(), _newCfg.end());
    m_unknow1 = m_unknow2 = m_unknow3 = -1;
  }   

  void 
  CCfg::SetCCModel(CCModelBase* _cc){
    m_cc = _cc;
  }

  double& 
  CCfg::tx(){ 
    ObjName="Node";  
    CopyCfg(); 
    return m_dofs[0]; 
  }
  
  double& 
  CCfg::ty(){ 
    return m_dofs[1]; 
  }
  
  double& 
  CCfg::tz(){
    if(m_isVolumetricRobot){ 
      return m_dofs[2]; 
    }
    else
      return m_defaultDOF; //Planar robot; m_dofs[2] is possible rotation
  }
  
  const double& 
  CCfg::tx() const{ 
    return m_dofs[0]; 
  }
  
  const double& 
  CCfg::ty() const{ 
    return m_dofs[1]; 
  }
  
  const double& 
  CCfg::tz() const{ 
    if(m_isVolumetricRobot)
      return m_dofs[2]; 
    else
      return m_defaultDOF; //Planar robot; m_dofs[2] is possible rotation
  }

}//namespace plum 

