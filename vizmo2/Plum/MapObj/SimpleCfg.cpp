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
        m_index = -1;
    }
    
    CSimpleCfg::~CSimpleCfg()
    {
        m_X = LONG_MAX; m_Y = LONG_MAX; m_Z = LONG_MAX;
        m_Alpha = LONG_MAX; m_Beta = LONG_MAX; m_Gamma = LONG_MAX;
        m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
    }
    
    void CSimpleCfg::Set( int index , OBPRMView_Robot* robot) 
    {    
        m_index = index;
        m_robot = robot;
    }
    
    void CSimpleCfg::DrawRobot(){
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
    
    void CSimpleCfg::DrawBox(double scale){
        glPushMatrix();
        glTranslated( GetX(), GetY(), GetZ() );
        glRotated( m_Alpha*360, 1, 0, 0 );
        glRotated( m_Beta*360,  0, 1, 0 );
        glRotated( m_Gamma*360, 0, 0, 1 );
        glutSolidCube( scale );
        glPopMatrix();
    }
    
    void CSimpleCfg::DrawPoint(){
        glBegin(GL_POINTS);
        glVertex3d( GetX(), GetY(), GetZ() );
        glEnd();
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
    
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    CSimpleEdge::CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
    }
    
    CSimpleEdge::CSimpleEdge(double weight)
    {
        m_LP = INT_MAX;
        m_Weight = weight;
    }
    
    CSimpleEdge::~CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
    }
    
    void CSimpleEdge::Draw() 
    {
        glBegin( GL_LINES );
        glVertex3d( m_s[0],m_s[1],m_s[2] );
        glVertex3d( m_e[0],m_e[1],m_e[2] );
        glEnd();
    }
    
    bool CSimpleEdge::operator==( const CSimpleEdge & other ){
        if( m_LP != other.m_LP || m_Weight != other.m_Weight ) 
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