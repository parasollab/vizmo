/////////////////////////////////////////////////////////////////////
//Implementation for Cfg  
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Cfg.h"
#include "Plum.h"
#include <limits.h>

namespace plum{

  //init m_invalidCfg and DOF
  CCfg CCfg::m_invalidCfg;
  int CCfg::m_dof = 0;

  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////

  CCfg::CCfg() {
    m_unknow1 = LONG_MAX;
    m_unknow2 = LONG_MAX;
    m_unknow3 = LONG_MAX;
    m_index = -1;
    m_shape=Point;
    m_coll = false;
    m_dofs.clear();
  }

  CCfg::~CCfg() {
  }

  CCfg::CCfg(const CCfg& _cfg) {
    m_index = _cfg.m_index;
    m_robot = _cfg.m_robot;
    m_cc = _cfg.m_cc;
    m_coll = _cfg.m_coll;
    m_dofs = _cfg.m_dofs;
    m_shape = _cfg.m_shape; 

    m_unknow1 = _cfg.m_unknow1;
    m_unknow2 = _cfg.m_unknow2;
    m_unknow3 = _cfg.m_unknow3;
  }

  void 
  CCfg::Set( int _index , OBPRMView_Robot* _robot, CCModelBase* _cc) {   
    m_index = _index;
    m_robot = _robot;
    m_cc = _cc;
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

    vector<double> cfg = m_dofs; //now vectors!  

    //back up
    
    vector<float> origColor = m_robot->GetColor();  

    m_robot->RestoreInitCfg();
    m_robot->BackUp();
    float* arr_m_RGBA = &m_RGBA[0];  
    glColor4fv(arr_m_RGBA);
    m_robot->SetRenderMode(m_RenderMode);
    m_robot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    m_robot->Scale(m_Scale[0],m_Scale[1],m_Scale[2]);
    m_robot->Configure(cfg);   

    //draw
    m_robot->Draw(GL_RENDER);

    //restore
    m_robot->Restore();
    m_robot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]); 
  }

  void 
  CCfg::DrawBox(){
    glEnable(GL_LIGHTING);
    glPushMatrix();
    float* arr_m_RGBA = &m_RGBA[0]; 
    glColor4fv(arr_m_RGBA);

    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    glTranslated( m_dofs[0], m_dofs[1], m_dofs[2] );
    glRotated( m_dofs[5]*360, 0, 0, 1 );
    glRotated( m_dofs[4]*360,  0, 1, 0 );
    glRotated( m_dofs[3]*360, 1, 0, 0 );
    glScale();
    glTransform();
    glEnable(GL_NORMALIZE);
    if(m_RenderMode == CPlumState::MV_SOLID_MODE)
      glutSolidCube(1);
    if(m_RenderMode == CPlumState::MV_WIRE_MODE)
      glutWireCube(1);
    glDisable(GL_NORMALIZE);
    glPopMatrix();
  }

  void 
  CCfg::DrawPoint(){
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    if(m_RenderMode == CPlumState::MV_SOLID_MODE ||
        m_RenderMode == CPlumState::MV_WIRE_MODE)
      glVertex3d( m_dofs[0], m_dofs[1], m_dofs[2] );
    glEnd();
  }

  void 
  CCfg::Draw(GLenum _mode){
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
  CCfg::DrawSelect() { 
    glColor3d(1,1,0);
    glDisable(GL_LIGHTING);
    switch(m_shape){

      case Robot: 
        if( m_robot!=NULL ){
          
          vector<double> cfg = m_dofs; 

          //backUp
          m_robot->BackUp();

          vector<float> origColor = m_robot->GetColor();   

          //change
          m_robot->SetColor(1,1,0,0);
          m_robot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
          m_robot->Scale(m_Scale[0],m_Scale[1],m_Scale[2]);
          m_robot->Configure(cfg);
          //delete[] cfg;

          //draw 
          m_robot->DrawSelect();

          //restore
          m_robot->Restore();  
          m_robot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]);  
        };
        break;

      case Box:         
        glLineWidth(2);
        glPushMatrix();
        glTranslated( m_dofs[0], m_dofs[1], m_dofs[2] );
        glRotated( m_dofs[5]*360, 0, 0, 1 );
        glRotated( m_dofs[4]*360,  0, 1, 0 );
        glRotated( m_dofs[3]*360, 1, 0, 0 );
        glutWireCube(1.1);
        glPopMatrix(); 
        break;

      case Point: 
        glPointSize(8);
        glDisable(GL_LIGHTING);
        glBegin(GL_POINTS);
        glVertex3d( m_dofs[0], m_dofs[1], m_dofs[2] );
        glEnd();
        break;
    }
  } //end of function

  bool 
  CCfg::operator==(const CCfg& _other) const {
    int dof=CCfg::m_dof;

    if( m_dofs[0] != _other.m_dofs[0] || 
        m_dofs[1] != _other.m_dofs[1] || 
        m_dofs[2] != _other.m_dofs[2] )
      return false;

    for(int i=0;i<dof-3;i++){
      if( m_dofs[i+3] != _other.m_dofs[i+3])
        return false;
    }

    return true;
  }

  // Fucntion not used:   
  // this information is controlled by VizmoRoadmapGUI::printNodeCfg() 
  // in roadmap.cpp, which calls CCfg::GetNodeInfo()

  list<string> 
  CCfg::GetInfo() const {  
    list<string> info; 
    int dof=CCfg::m_dof;
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


  list<string> 
  CCfg::GetNodeInfo() const {  
    list<string> info; 
    int dof=CCfg::m_dof;
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
  CCfg::Dump() {
    cout << "- ID = " << m_index << endl;
    cout << "- Position = (" << m_dofs[0] << ", " <<
      m_dofs[1] << ", " << m_dofs[2] << ")" << endl;
    cout << "- Orientation = (";
    int dof=CCfg::m_dof;
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

}//namespace plum 

