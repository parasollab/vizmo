// SimpleCfg.h: interface for the CSimpleCfg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_)
#define AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "src/EnvObj/Robot.h"

using namespace std;

namespace plum{
    
    //////////////////////////////////////////////////////////////////////
    //
    //      CSimpleCfg
    //
    //////////////////////////////////////////////////////////////////////
    
    class CSimpleCfg  
    {
        friend ostream & operator<<( ostream & out, const CSimpleCfg & cfg );
        friend istream & operator>>( istream &  in, CSimpleCfg & cfg );
        
    public:
        
        //////////////////////////////////////////////////////////////////////
        //      Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        
        CSimpleCfg();
        ~CSimpleCfg();
        
        bool operator==( const CSimpleCfg & other ) const;
        void Set( int index , OBPRMView_Robot* robot);

        void DrawRobot();
        void DrawBox(double size);
        void DrawPoint();
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        static CSimpleCfg & InvalidData(){return m_InvalidCfg;}
        int GetIndex() const {return m_index;}
        double GetX() const {return m_X;}
        double GetY() const {return m_Y;}
        double GetZ() const {return m_Z;}
        
        virtual void Dump();

        double GetAlpha() const {return m_Alpha*360;}
        double GetBeta() const {return m_Beta*360;}
        double GetGamma() const {return m_Gamma*360;}
        
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:
                
        double m_X, m_Y, m_Z;
        double m_Alpha, m_Beta, m_Gamma;
        double m_Unknow1, m_Unknow2, m_Unknow3;
        int m_index;
        
        OBPRMView_Robot* m_robot;
        
        //////////////////////////////////////////////////////////////////////
        //      Private Method & Data
        //////////////////////////////////////////////////////////////////////
    private:
        static CSimpleCfg m_InvalidCfg;
    };
    
    //////////////////////////////////////////////////////////////////////
    //
    //      CSimpleEdge
    //
    //////////////////////////////////////////////////////////////////////
    
    class CSimpleEdge
    {
        friend ostream & operator<<( ostream & out, const CSimpleEdge & edge );
        friend istream & operator>>( istream &  in, CSimpleEdge & edge );
        
    public:
        
        //////////////////////////////////////////////////////////////////////
        //      Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        
        CSimpleEdge();
        CSimpleEdge(double weight);
        ~CSimpleEdge();
        
        bool operator==( const CSimpleEdge & other );
        void Set(const Point3d& p1, const Point3d& p2){ m_s=p1; m_e=p2; }
        void Draw();
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        int & GetLP(){ return m_LP; }
        double & GetWeight(){ return m_Weight; }
        
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:
        
        Point3d m_s, m_e;

        int    m_LP;
        double m_Weight;
    };
    
}//namespace plum

#endif 
