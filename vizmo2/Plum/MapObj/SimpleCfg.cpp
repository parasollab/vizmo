// SimpleCfg.cpp: implementation of the CSimpleCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "SimpleCfg.h"
#include "Plum.h"
#include <limits.h>

namespace plum{

    //init m_InvalidCfg
    CSimpleCfg CSimpleCfg::m_InvalidCfg;

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CSimpleCfg::CSimpleCfg()
    {
        m_X = LONG_MAX; m_Y = LONG_MAX; m_Z = LONG_MAX;
        m_Alpha = LONG_MAX; m_Beta = LONG_MAX; m_Gamma = LONG_MAX;
        m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
        
        m_DisplayListIndex = -1;
        m_index = -1;
        m_bSelected = false;
        m_RenderMode=CPlumState::MV_SOLID_MODE;

    }

    CSimpleCfg::~CSimpleCfg()
    {
        m_X = LONG_MAX; m_Y = LONG_MAX; m_Z = LONG_MAX;
        m_Alpha = LONG_MAX; m_Beta = LONG_MAX; m_Gamma = LONG_MAX;
        m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
        
        m_DisplayListIndex = -1;
	m_DisplayListIndexPoint = -1;
        m_index = -1;
        m_bSelected = false;
    }

  //bool CSimpleCfg::BuildModel( int index ) 
  bool CSimpleCfg::BuildModel( int index , OBPRMView_Robot* robot) 
  {    
    m_index = index;
    m_robot = robot;

    cube();
    point();

    return true;
  }
	
  void CSimpleCfg::Draw( GLenum mode ) 
  {
    if( m_RenderMode==CPlumState::MV_INVISIBLE_MODE ) return;

    if(m_NodeShape == "Box"){

    ///////////////////////////////////////////////////////////////////////////    
    // draw solid part
      if( m_RenderMode==CPlumState::MV_SOLID_MODE ){
	glLineWidth(4);
	glPolygonMode( GL_FRONT, GL_FILL );
	glEnable( GL_POLYGON_OFFSET_FILL );
        #ifdef WIN32
	  glPolygonOffset( 1.0, 1.0 );
        #endif
	glCallList(m_DisplayListIndex);
	glDisable( GL_POLYGON_OFFSET_FILL );
      }
    //////////////////////////////////////////////////////////////////////////////
    // draw putline
      glPolygonMode( GL_FRONT, GL_LINE );
      glCallList(m_DisplayListIndex);
    }
    else if(m_NodeShape == "Point"){
      glCallList(m_DisplayListIndexPoint);
    }
    else{
      glPushMatrix();
      RobotModel();
      glPopMatrix();
    }
  }
  
  
  bool CSimpleCfg::operator==( const CSimpleCfg & other ){
    
    if( m_X != other.m_X || m_Y != other.m_Y || m_Z != other.m_Z )
      return false;
        if( m_Alpha != other.m_Alpha || m_Beta != other.m_Beta || m_Gamma != other.m_Gamma )
	  return false;
        
        return true;
  }
  
  void CSimpleCfg::Dump()
  {
    cout << "- ID = " << m_index <<endl;
    cout << "- Location = ("<<m_X << ", " << m_Y << ", " << m_Z<<")"<<endl;
    cout << "- Orientation = (" << m_Alpha*360 << ", " << m_Beta*360 << ", " << m_Gamma*360<<")"<<endl;
  }
  
  
  void CSimpleCfg::RobotModel(){
    if( m_robot==NULL ) return;
    double cfg[6];
    cfg[0]=GetX(); 
    cfg[1]=GetY(); 
    cfg[2]=GetZ(); 
    cfg[3]=GetAlpha(); 
    cfg[4]=GetBeta(); 
    cfg[5]=GetGamma();
    
    m_robot->Configure(cfg);			
    m_robot->Draw(GL_RENDER);
  }

  void CSimpleCfg::cube(void){
    m_DisplayListIndex = glGenLists(1);
    glNewList( m_DisplayListIndex, GL_COMPILE );
    glPushMatrix();
    glTranslated( GetX(), GetY(), GetZ() );
    glRotated( m_Alpha*360, 1, 0, 0 );
    glRotated( m_Beta*360,  0, 1, 0 );
    glRotated( m_Gamma*360, 0, 0, 1 );
    SolidCube( 0.3f );
    glPopMatrix();
    glEndList();
  }

  void CSimpleCfg::point(void){
    m_DisplayListIndexPoint = glGenLists(1);
    glNewList( m_DisplayListIndexPoint, GL_COMPILE );
    glPushMatrix();
    glTranslated( GetX(), GetY(), GetZ() );
    glRotated( m_Alpha*360, 1, 0, 0 );
    glRotated( m_Beta*360,  0, 1, 0 );
    glRotated( m_Gamma*360, 0, 0, 1 );
    glutSolidSphere( 0.1f, 10, 8 );
    glPopMatrix();
    glEndList();
  }

  void CSimpleCfg::SolidCube( float size )
  {

    GLdouble vertice[]=
    { -size, -size, -size,
      size, -size, -size,
      size, -size,  size,
      -size, -size,  size,
      -size,  size, -size,
      size,  size, -size,
      size,  size,  size,
      -size,  size,  size};
    
    //Face index
    GLubyte id1[] = { 0, 1, 2, 3 }; //buttom
    GLubyte id2[] = { 7, 6, 5, 4 }; //top
    GLubyte id3[] = { 1, 5, 6, 2 }; //left
    GLubyte id4[] = { 3, 7, 4, 0 }; //right
    GLubyte id5[] = { 0, 4, 5, 1 }; //back
    GLubyte id6[] = { 3, 2, 6, 7 }; //front
    
    //setup points
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, vertice);
    
    glNormal3d(0,-1,0);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id1 );
    glNormal3d(0, 1,0);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id2 );
    glNormal3d( 1,0,0);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id3 );
    glNormal3d(-1,0,0);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id4 );
    glNormal3d(0,0,-1);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id5 );
    glNormal3d(0,0, 1);
    glDrawElements( GL_QUADS, 4, GL_UNSIGNED_BYTE, id6 );
  }

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CSimpleEdge::CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
        m_EndIndex = -1;
        m_StartIndex = -1;
        m_DisplayListIndex=-1;
    }

    CSimpleEdge::CSimpleEdge(double weight)
    {
        m_LP = INT_MAX;
        m_Weight = weight;
        m_EndIndex = -1;
        m_StartIndex = -1;
        m_DisplayListIndex=-1;
    }

    CSimpleEdge::~CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
    }

  bool CSimpleEdge::BuildModel( CSimpleCfg & start, CSimpleCfg & end ) 
  {
    m_StartIndex = start.GetIndex();
    m_EndIndex = end.GetIndex();
    
    m_DisplayListIndex = glGenLists(1);
    glNewList( m_DisplayListIndex, GL_COMPILE );
        glBegin( GL_LINES );
        glVertex3d( start.GetX(), start.GetY(), start.GetZ() );
        glVertex3d( end.GetX(),   end.GetY(),   end.GetZ() );
        glEnd();
        glEndList();
        
        return true;
    }

  void CSimpleEdge::Draw( GLenum mode ) 
  {
    glCallList(m_DisplayListIndex);
    glLineWidth(1);
  }

  bool CSimpleEdge::operator==( const CSimpleEdge & other ){
        if( m_LP != other.m_LP || m_Weight != other.m_Weight ) 
            return false;
        
        if( m_StartIndex != other.m_StartIndex || m_EndIndex != other.m_EndIndex)
            return false;
        
        return true;
    }
    //////////////////////////////////////////////////////////////////////
    // Opers
    //////////////////////////////////////////////////////////////////////
    ostream & operator<<( ostream & out, const CSimpleCfg & cfg )
    {
        out << cfg.m_X << " " << cfg.m_Y << " " << cfg.m_Z;
        out << " " << cfg.m_Alpha << " " << cfg.m_Beta << " " << cfg.m_Gamma;
        out << " " << cfg.m_Unknow1 << " " << cfg.m_Unknow2 << " " << cfg.m_Unknow3;
        
        return out;
    }

    istream & operator>>( istream &  in, CSimpleCfg & cfg )
    {
        in >> cfg.m_X >> cfg.m_Y >> cfg.m_Z;
        in >> cfg.m_Alpha >> cfg.m_Beta >> cfg.m_Gamma;
        in >> cfg.m_Unknow1 >> cfg.m_Unknow2 >> cfg.m_Unknow3;
        
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

