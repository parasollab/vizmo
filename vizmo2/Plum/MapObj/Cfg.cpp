// Cfg.cpp: implementation of the CCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "Cfg.h"
#include "Plum.h"
#include <limits.h>

namespace plum{
    
  //init m_InvalidCfg and DOF
  CCfg CCfg::m_InvalidCfg;
  int CCfg::dof = 0;

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    CCfg::CCfg()
    {
      m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
      m_index = -1; m_Shape=Point;
    }
    
    CCfg::~CCfg()
    {
      //CCfg::dof = 0;
      m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
    }
    
  //void CCfg::Set( int index , OBPRMView_Robot* robot) 
  void CCfg::Set( int index , OBPRMView_Robot* robot, CCModelBase* cc) 
  {    
        m_index = index;
        m_robot = robot;
	m_CC=cc;
  }
  
  void CCfg::DrawRobot(){
      	  
      if( m_robot==NULL ) return;
      //Dump();
      int dof=CCfg::dof;
      double * cfg=new double[dof];
      for(int i=0;i<dof;i++) cfg[i]=dofs[i];
      //CopyCfg();
      m_robot->Scale(sx(),sy(),sz());
      m_robot->Configure(cfg);
	  delete [] cfg;
      m_robot->Draw(GL_RENDER);
  }
  
    void CCfg::DrawBox(){
      //Dump();
      glEnable(GL_LIGHTING);
      glPushMatrix();
      
      glTranslated( dofs[0], dofs[1], dofs[2] );
      glRotated( dofs[5]*360, 0, 0, 1 );
      glRotated( dofs[4]*360,  0, 1, 0 );
      glRotated( dofs[3]*360, 1, 0, 0 );
      glScale();
      //glTransform();
      glEnable(GL_NORMALIZE);
      glutSolidCube(1);
      glDisable(GL_NORMALIZE);
      glPopMatrix();
    }
    
    void CCfg::DrawPoint(){
      glBegin(GL_POINTS);
      glVertex3d( dofs[0], dofs[1], dofs[2] );
      glEnd();
    }

  void CCfg::Draw( GLenum mode )
  {
    glPushName(m_index);
    switch(m_Shape){
    case Robot: DrawRobot(); break;
    case Box: DrawBox(); break;
    case Point: DrawPoint(); break;
    }
    glPopName();
  }

  void CCfg::DrawSelect()
  {
    glColor3d(1,1,0);
    glDisable(GL_LIGHTING);
    switch(m_Shape){
    case Robot: 
      if( m_robot!=NULL ){

	int dof=CCfg::dof;
	double * cfg=new double[dof];
	for(int i=0; i<dof;i++) cfg[i]=dofs[i];
	//CopyCfg();
	
	//backUp
	m_robot->BackUp();
	float o_c[4]; memcpy(o_c,m_robot->GetColor(),4*sizeof(float));
	//change
	m_robot->SetColor(1,1,0,0);
	m_robot->Scale(sx(),sy(),sz());
	m_robot->Configure(cfg);
	delete [] cfg;
	m_robot->DrawSelect();
	m_robot->SetColor(o_c[0],o_c[1],o_c[2],o_c[3]);	
	//restore
	m_robot->Restore();	

      }
      break;
    case Box:         
      glLineWidth(2);
      glPushMatrix();
      glTranslated( dofs[0], dofs[1], dofs[2] );
      glRotated( dofs[5]*360, 0, 0, 1 );
      glRotated( dofs[4]*360,  0, 1, 0 );
      glRotated( dofs[3]*360, 1, 0, 0 );

      //glTransform();
      glScale();

      glutWireCube(1.1);
      //glScale();
      glPopMatrix(); 
      break;
    case Point: 
      glPointSize(8);
      DrawPoint();
      break;
    }
  } //end of function
    
  bool CCfg::operator==( const CCfg & other ) const {
    int dof=CCfg::dof;
    
    if( dofs[0] != other.dofs[0] || 
	dofs[1] != other.dofs[1] || 
	dofs[2] != other.dofs[2] )
      return false;
    for(int i=0;i<dof-3;i++){
      if( dofs[i+3] != other.dofs[i+3])
	return false;
    }
           
    return true;
  }
    
    
  list<string> CCfg::GetInfo() const 
  { 	
    list<string> info; 
    {
      ostringstream temp;
      temp<<"Node ID= "<<m_index;
      info.push_back(temp.str());
    }
    
    return info;
  }

  void CCfg::Dump()
  {
    cout << "- ID = " << m_index <<endl;
    cout << "- Position = ("<<dofs[0] << ", " << dofs[1] << ", " << dofs[2]<<")"<<endl;
    cout << "- Orientation = (";
    int dof=CCfg::dof;
    for(int i=0; i<dof-3;i++){
      printf("%f ", dofs[i+3]*360);
    }
    cout<<")"<<endl;
  }
  
  
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    CSimpleEdge::CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
	m_ID=-1;
    }
    
    CSimpleEdge::CSimpleEdge(double weight)
    {
        m_LP = INT_MAX;
        m_Weight = weight;
	m_ID=-1;
    }
    
  void CSimpleEdge::Draw(GLenum mode) 
  {
    
    glPushName(m_ID);
    glBegin( GL_LINES );
    glVertex3d( m_s->tx(),m_s->ty(),m_s->tz() );
    glVertex3d( m_e->tx(),m_e->ty(),m_e->tz() );
    glEnd();
    glPopName();
  }
  
  void CSimpleEdge::DrawSelect()
  {
    glColor3d(1,1,0);
    glLineWidth(4);
    glBegin( GL_LINES );
    glVertex3d( m_s->tx(),m_s->ty(),m_s->tz() );
    glVertex3d( m_e->tx(),m_e->ty(),m_e->tz() );
    glEnd();
  }
  
  CSimpleEdge::~CSimpleEdge()
  {
    m_LP = INT_MAX;
    m_Weight = LONG_MAX;
  }
  
 
  bool CSimpleEdge::operator==( const CSimpleEdge & other ){
    if( m_LP != other.m_LP || m_Weight != other.m_Weight ) 
      return false;        
    return true;
  }

  list<string> CSimpleEdge::GetInfo() const 
  { 	
    list<string> info; 
    {
      ostringstream temp;
      temp<<"Edge, ID= "<<m_ID<<", ";
      temp<<"connects Node "<<m_s->GetIndex()<<" and Node "<<m_e->GetIndex();
      info.push_back(temp.str());
    }
    
    return info;
  }
  
  //////////////////////////////////////////////////////////////////////
  // Opers
  //////////////////////////////////////////////////////////////////////
//   ostream & operator<<( ostream & out, const CCfg & cfg )
//   {
//     for( int iC=0; iC<cfg.dofs.size(); iC++ ){
//       out<<"cfg["<<iC<<"]"<<cfg.dofs[iC]<<endl;
//     }

//     out << " " << cfg.m_Unknow1 << " " << cfg.m_Unknow2 << " " << cfg.m_Unknow3;
    
//     return out;
//   }

  ostream & operator<<( ostream & out, const CCfg & cfg )
  {
    for( int iC=0; iC<cfg.dofs.size(); iC++ ){
      out<<cfg.dofs[iC];
    }


    out << " " << cfg.m_Unknow1 << " " << cfg.m_Unknow2 << " " << cfg.m_Unknow3;
    
    return out;
  }
    
    
  istream & operator>>( istream &  in, CCfg & cfg )  
    {

      cfg.dofs.clear();

      int dof=CCfg::dof;

      for( int i=0;i<dof;i++ ){
	double value;
	in >> value;
	cfg.dofs.push_back(value);
      }
      
      in >> cfg.m_Unknow1 >> cfg.m_Unknow2 >> cfg.m_Unknow3; //not used
      
      //cfg.tx()= cfg.dofs[0]; cfg.ty()= cfg.dofs[1]; cfg.tz()= cfg.dofs[2];

      return in;
    }
    
    ostream & operator<<( ostream & out, const CSimpleEdge & edge )
    {
        out << edge.m_LP << " " << edge.m_Weight;
        return out;
    }
    
    istream & operator>>( istream &  in, CSimpleEdge & edge )
    {
        in >> edge.m_LP >> edge.m_Weight;
        return in;
    }
}//namespace plum


