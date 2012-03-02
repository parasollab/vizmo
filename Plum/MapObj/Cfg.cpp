
//
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Cfg.h"
#include "Plum.h"
#include <limits.h>



namespace plum{

  //init m_invalidCfg and DOF
  CCfg CCfg::m_invalidCfg;
  int CCfg::dof = 0;

  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////

  CCfg::CCfg() {
    m_unknow1 = LONG_MAX;
    m_unknow2 = LONG_MAX;
    m_unknow3 = LONG_MAX;
    m_index = -1;
    m_shape=Point;
    coll = false;
    dofs.clear();
  }

  CCfg::~CCfg() {
  }

  CCfg::CCfg(const CCfg& _cfg) {
  
    m_index = _cfg.m_index;
    m_robot = _cfg.m_robot;
    m_cc = _cfg.m_cc;
    coll = _cfg.coll;
    dofs = _cfg.dofs;
    m_shape = _cfg.m_shape; 

    m_unknow1 = _cfg.m_unknow1;
    m_unknow2 = _cfg.m_unknow2;
    m_unknow3 = _cfg.m_unknow3;
  }

  void CCfg::Set( int _index , OBPRMView_Robot* _robot, CCModelBase* _cc) {   
    m_index = _index;
    m_robot = _robot;
    m_cc = _cc;
  }

  int CCfg::GetCC_ID(){

    int i = m_cc->ID(); 
    return i; 
  }

  void CCfg::DrawRobot(){

    if( m_robot==NULL )
      return;
    
    int dof=CCfg::dof;
    double* cfg=new double[dof];
    for(int i=0;i<dof;i++){  
        cfg[i] = dofs[i]; 
    }
    
    //back up
    float origColor[4];
    memcpy(origColor,m_robot->GetColor(),4*sizeof(float));
    m_robot->RestoreInitCfg();
    m_robot->BackUp();
    glColor4fv(m_RGBA);
    m_robot->SetRenderMode(m_RenderMode);
    m_robot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    m_robot->Scale(m_Scale[0],m_Scale[1],m_Scale[2]);
    m_robot->Configure(cfg);
    delete[] cfg;
    
    //draw
    m_robot->Draw(GL_RENDER);
    
    //restore
    m_robot->Restore();
    m_robot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]); 
  }

  void CCfg::DrawBox(){
    glEnable(GL_LIGHTING);
    glPushMatrix();
    glColor4fv(m_RGBA);
    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    glTranslated( dofs[0], dofs[1], dofs[2] );
    glRotated( dofs[5]*360, 0, 0, 1 );
    glRotated( dofs[4]*360,  0, 1, 0 );
    glRotated( dofs[3]*360, 1, 0, 0 );
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

  void CCfg::DrawPoint(){
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    glColor4f(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
    if(m_RenderMode == CPlumState::MV_SOLID_MODE ||
      m_RenderMode == CPlumState::MV_WIRE_MODE)
        glVertex3d( dofs[0], dofs[1], dofs[2] );
    glEnd();
  }

  void CCfg::Draw(GLenum _mode) {
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

  void CCfg::DrawSelect() { 
    glColor3d(1,1,0);
    glDisable(GL_LIGHTING);
    switch(m_shape){
      
      case Robot: 
        if( m_robot!=NULL ){
          int dof=CCfg::dof;
          double* cfg=new double[dof];
          for(int i=0; i<dof;i++) cfg[i] = dofs[i];
          
          //backUp
          m_robot->BackUp();
          float origColor[4];
          memcpy(origColor,m_robot->GetColor(),4*sizeof(float));
          
          //change
          m_robot->SetColor(1,1,0,0);
          m_robot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
          m_robot->Scale(m_Scale[0],m_Scale[1],m_Scale[2]);
          m_robot->Configure(cfg);
          delete[] cfg;
          
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
        glTranslated( dofs[0], dofs[1], dofs[2] );
        glRotated( dofs[5]*360, 0, 0, 1 );
        glRotated( dofs[4]*360,  0, 1, 0 );
        glRotated( dofs[3]*360, 1, 0, 0 );
        glutWireCube(1.1);
        glPopMatrix(); 
        break;
      
      case Point: 
        glPointSize(8);
        glDisable(GL_LIGHTING);
        glBegin(GL_POINTS);
        glVertex3d( dofs[0], dofs[1], dofs[2] );
        glEnd();
        break;
    }
  } //end of function

  bool CCfg::operator==(const CCfg& _other) const {
    int dof=CCfg::dof;
      
    if( dofs[0] != _other.dofs[0] || 
        dofs[1] != _other.dofs[1] || 
        dofs[2] != _other.dofs[2] )
        return false;
    
    for(int i=0;i<dof-3;i++){
      if( dofs[i+3] != _other.dofs[i+3])
        return false;
    }

    return true;
  }

  // Fucntion not used:   
  // this information is controlled by VizmoRoadmapGUI::printNodeCfg() 
  // in roadmap.cpp, which calls CCfg::GetNodeInfo()

  list<string> CCfg::GetInfo() const {  
    list<string> info; 
    int dof=CCfg::dof;
    ostringstream temp;

    temp<<"Node ID = "<<m_index<< " ";
    temp << " Cfg ( ";

    for(int i=0; i<dof;i++){
      if(i < 3)
        temp << dofs[i];
      else
        temp << dofs[i];
      if(i == dof-1)
        temp << " )";
      else
        temp << ", ";
    }
    
    info.push_back(temp.str());

    if(coll)
      info.push_back("\t\t **** IS IN COLLISION!! ****");

    return info;
  }


  list<string> CCfg::GetNodeInfo() const {  
    list<string> info; 
    int dof=CCfg::dof;
    ostringstream temp;

    temp<<"Node ID = "<<m_index<< " ";

    for(int i=0; i<dof;i++){
      if(i < 3)
          temp << dofs[i];
      else
          temp << dofs[i];
      if(i == dof-1)
          temp << " )";
      else
          temp << ", ";
    }
    info.push_back(temp.str());
    return info;
  }

  void CCfg::Dump() {
    cout << "- ID = " << m_index <<endl;
    cout << "- Position = ("<<dofs[0] << ", " <<
      dofs[1] << ", " << dofs[2]<<")"<<endl;
    cout << "- Orientation = (";
    int dof=CCfg::dof;
    for(int i=0; i<dof-3;i++){
        printf("%f ", dofs[i+3]*360);
    }
    cout<<")"<<endl;
  }

  void CCfg::SetCfg(vector<double> _newCfg){
    dofs.assign(_newCfg.begin(), _newCfg.end());
    m_unknow1 = m_unknow2 = m_unknow3 = -1;
  }   

  void CCfg::SetCCModel(CCModelBase* _cc){
    m_cc = _cc;
  }


  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////

  CSimpleEdge::CSimpleEdge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
    m_id=-1;
  }

  CSimpleEdge::CSimpleEdge(double _weight) {
    m_lp = INT_MAX;
    m_weight = _weight;
    m_id=-1;
  }

  void
  CSimpleEdge::Set(int _id,
  CCfg* _c1, CCfg* _c2,
  OBPRMView_Robot* _robot) {
    m_id = _id;
    m_s = *_c1;
    m_e = *_c2;

    typedef vector<CCfg>::iterator CIT;
    for(CIT c = m_intermediateCfgs.begin();
    c != m_intermediateCfgs.end(); c++) {
      c->m_robot = _robot;
    }
  }

  void
  CSimpleEdge::Draw(GLenum _mode) {

    typedef vector<CCfg>::iterator CFGIT;
    glPushName(m_id);
    if(m_RenderMode == CPlumState::MV_SOLID_MODE ||
    m_RenderMode == CPlumState::MV_WIRE_MODE){
      glColor4fv(m_RGBA);
      glBegin(GL_LINES);
      glVertex3d( m_s.tx(),m_s.ty(),m_s.tz() );

      for(CFGIT c = m_intermediateCfgs.begin();
      c != m_intermediateCfgs.end(); c++) {
        glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
        glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
      }
      
      glVertex3d( m_e.tx(),m_e.ty(),m_e.tz() );
      glEnd();

      //draw intermediate configurations
      if(m_cfgShape == CCfg::Box || m_cfgShape == CCfg::Robot) {
        for(CFGIT c = m_intermediateCfgs.begin();
          c != m_intermediateCfgs.end(); c++) { 
          c->SetShape(m_cfgShape);
          c->SetRenderMode(CPlumState::MV_WIRE_MODE);
          c->Draw(_mode);
        }
      } 
    }
    glPopName();
  }
  
  void CSimpleEdge::DrawSelect() {
    typedef vector<CCfg>::iterator CFGIT;
    glColor3d(1,1,0);
    glLineWidth(4);
    
    glBegin( GL_LINES );
    glVertex3d( m_s.tx(),m_s.ty(),m_s.tz() );
    for(CFGIT c = m_intermediateCfgs.begin();
    c != m_intermediateCfgs.end(); c++) {
      glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
      glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
    }
    glVertex3d( m_e.tx(),m_e.ty(),m_e.tz() );
    glEnd();
  }

  CSimpleEdge::~CSimpleEdge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
  }


  bool CSimpleEdge::operator==( const CSimpleEdge& _other ){
    if( m_lp != _other.m_lp || m_weight != _other.m_weight ) 
      return false;        
    return true;
  }

  list<string> CSimpleEdge::GetInfo() const {  
    list<string> info; 
    ostringstream temp;
    temp<<"Edge, ID= "<<m_id<<", ";
    temp<<"connects Node "<<m_s.GetIndex()<<" and Node "<<m_e.GetIndex();
    info.push_back(temp.str());
    return info;
  }

  vector<int> CSimpleEdge::GetEdgeNodes(){
    vector<int> v;
    v.push_back(m_s.GetIndex());
    v.push_back(m_e.GetIndex());
    return v;
  }

  //////////////////////////////////////////////////////////////////////
  // Opers
  //////////////////////////////////////////////////////////////////////

  ostream & operator<<( ostream& _out, const CCfg& _cfg ) {
    for( unsigned int iC=0; iC < _cfg.dofs.size(); iC++ ){
      _out << _cfg.dofs[iC] << " ";
    }
    _out << " " <<
    _cfg.m_unknow1 << " " <<
    _cfg.m_unknow2 << " " <<
    _cfg.m_unknow3;
    return _out;
  }

  istream & operator>>(istream& _in, CCfg& _cfg) {
    _cfg.dofs.clear();
    int dof=CCfg::dof;

    for(int i = 0; i < dof; i++){
        double value;
        _in >> value;
        _cfg.dofs.push_back(value); 
    }

    _in >> _cfg.m_unknow1 >> _cfg.m_unknow2 >> _cfg.m_unknow3; //not used
    return _in;
  }

  ostream&
  operator<<(ostream& _out, const CSimpleEdge& _edge) {
    _out << _edge.m_lp << " " << _edge.m_weight << " ";
    return _out;
  }

  istream&
  operator>>( istream&  _in, CSimpleEdge& _edge ) {

    int numIntermediates = 0;
    CCfg cfgtmp = _edge.GetStartCfg();
    _in >> numIntermediates;

    for(int i = 0; i < numIntermediates; i++) {
      _in >> cfgtmp;
      _edge.m_intermediateCfgs.push_back(cfgtmp);
    }

    _in >> _edge.m_weight;
    return _in;
  }
}//namespace plum


