// SimpleCfg.h: interface for the CSimpleCfg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_)
#define AFX_SIMPLECFG_H__2A4319F9_0BA6_4F63_85B4_AE6226B6BC69__INCLUDED_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <iostream>
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
        virtual ~CSimpleCfg();
        
        bool operator==( const CSimpleCfg & other );
        virtual bool BuildModel( int index );
        virtual void Draw( GLenum mode );
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        
        static CSimpleCfg & InvalidData(){
            return m_InvalidCfg;
        }
        
        int GetIndex() const {return m_index;}
        double GetX() const {return m_X;}
        double GetY() const {return m_Y;}
        double GetZ() const {return m_Z;}
        virtual void Select(bool bSel){ m_bSelected=bSel; }
        virtual bool isSelect() const { return m_bSelected; }
        
        virtual void Dump();
        virtual void SetRenderMode( int mode ){ m_RenderMode=mode; }
        
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:

        void SolidCube( float size );

        bool m_bSelected; //is this node selected
        int m_RenderMode; //render mode , wire or solid
        int m_DisplayListIndex;
        
        double m_X, m_Y, m_Z;
        double m_Alpha, m_Beta, m_Gamma;
        double m_Unknow1, m_Unknow2, m_Unknow3;
        int m_index;
        
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
        virtual ~CSimpleEdge();
        
        bool operator==( const CSimpleEdge & other );
        
        virtual bool BuildModel( CSimpleCfg & start, CSimpleCfg & end );
        virtual void Draw( GLenum mode );
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        virtual int & GetLP(){ return m_LP; }
        virtual double & GetWeight(){ return m_Weight; }
        
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:
        
        int  m_StartIndex;
        int  m_EndIndex;

        int m_DisplayListIndex;
        
        int    m_LP;
        double m_Weight;
    };

}//namespace plum

#endif 
