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
        m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
        m_index = -1;
		m_Shape=Point;
    }
    
    CSimpleCfg::~CSimpleCfg()
    {
        m_Unknow1 = LONG_MAX; m_Unknow2 = LONG_MAX; m_Unknow3 = LONG_MAX;
    }
    
    void CSimpleCfg::Set
	( int index , OBPRMView_Robot* robot, CCModelBase* cc) 
    {    
        m_index = index;
        m_robot = robot;
		m_CC=cc;
    }
    
    void CSimpleCfg::DrawRobot(){
        if( m_robot==NULL ) return;
        double cfg[6]={tx(),ty(),tz(),rx(),ry(),rz()};
        m_robot->Scale(sx(),sy(),sz());
        m_robot->Configure(cfg);
        m_robot->Draw(GL_RENDER);
    }
    
    void CSimpleCfg::DrawBox(){
		glEnable(GL_LIGHTING);
        glPushMatrix();
        glTransform();
		glEnable(GL_NORMALIZE);
        glutSolidCube(1);
		glDisable(GL_NORMALIZE);
        glPopMatrix();
    }
    
    void CSimpleCfg::DrawPoint(){
        glBegin(GL_POINTS);
        glVertex3d( tx(), ty(), tz() );
        glEnd();
    }

	void CSimpleCfg::Draw( GLenum mode )
	{
		glPushName(m_index);
		switch(m_Shape){
		case Robot: DrawRobot(); break;
		case Box: DrawBox(); break;
		case Point: DrawPoint(); break;
		}
		glPopName();
	}

	void CSimpleCfg::DrawSelect()
	{
		glColor3d(1,1,0);
		glDisable(GL_LIGHTING);
		switch(m_Shape){
		case Robot: 
if( m_robot!=NULL ){
//backup
double cfg[6]={tx(),ty(),tz(),rx(),ry(),rz()};
double o_t[3]={m_robot->tx(),m_robot->ty(),m_robot->tz()};
double o_r[3]={m_robot->rx(),m_robot->ry(),m_robot->rz()};
double o_s[3]={m_robot->sx(),m_robot->sy(),m_robot->sz()};
float o_c[4]; memcpy(o_c,m_robot->GetColor(),4*sizeof(float));
//change
m_robot->Scale(sx(),sy(),sz());
m_robot->SetColor(1,1,0,0);
m_robot->Configure(cfg);
m_robot->DrawSelect();
//put back
m_robot->tx()=o_t[0]; m_robot->ty()=o_t[1]; m_robot->tz()=o_t[2];
m_robot->rx()=o_r[0]; m_robot->ry()=o_r[1]; m_robot->rz()=o_r[2];
m_robot->sx()=o_s[0]; m_robot->sy()=o_s[1]; m_robot->sz()=o_s[2];
m_robot->SetColor(o_c[0],o_c[1],o_c[2],o_c[3]);
}
			break;
		case Box:         
			glLineWidth(2);
			glPushMatrix();
			glTransform();
			glutWireCube(1.1);
			glPopMatrix(); 
			break;
		case Point: 
			glPointSize(8);
			DrawPoint();
			break;
		}
	} //end of function
    
    bool CSimpleCfg::operator==( const CSimpleCfg & other ) const {

        if( tx() != other.tx() || ty() != other.ty() || tz() != other.tz() )
            return false;
        if( rx() != other.rx() || ry() != other.ry() || rz() != other.rz() )
            return false;
        
        return true;
    }

    
	list<string> CSimpleCfg::GetInfo() const 
    { 	
        list<string> info; 
        {
            ostringstream temp;
            temp<<"Node ID= "<<m_index;
            info.push_back(temp.str());
        }
		
        return info;
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
    
    CSimpleEdge::~CSimpleEdge()
    {
        m_LP = INT_MAX;
        m_Weight = LONG_MAX;
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
    ostream & operator<<( ostream & out, const CSimpleCfg & cfg )
    {
        out << cfg.tx() << " " << cfg.ty() << " " << cfg.tz();
        out << " " << cfg.rx() << " " << cfg.ry() << " " << cfg.rz();
        out << " " << cfg.m_Unknow1 << " " << cfg.m_Unknow2 << " " << cfg.m_Unknow3;
        
        return out;
    }
    
    istream & operator>>( istream &  in, CSimpleCfg & cfg )
    {

        in >> cfg.tx() >> cfg.ty() >> cfg.tz();
        in >> cfg.rx() >> cfg.ry() >> cfg.rz();
        in >> cfg.m_Unknow1 >> cfg.m_Unknow2 >> cfg.m_Unknow3;
        cfg.Euiler2Quaternion(); //compute quaternion

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
