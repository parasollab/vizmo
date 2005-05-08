// Cfg.h: interface for the CCfg class.
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
    //      CCfg 
    //      To support articulated robots.
    //
    //////////////////////////////////////////////////////////////////////
  class CCModelBase;
  class CCfg : public CGLModel  
    {
        friend ostream & operator<<( ostream & out, const CCfg & cfg );
        friend istream & operator>>( istream &  in, CCfg & cfg );
        
    public:
	//type for the shape of node representation
        enum Shape { Robot, Box, Point};

	//////////////////////////////////////////
	// DOF variable
	/////////////////////////////////////////

	static int dof;

	/////////////////////////
	// for collison detection 
	//////////////////////////

	bool coll;

	//testing:
	OBPRMView_Robot* m_robot;
        
        //////////////////////////////////////////////////////////////////////
        //      Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        
        CCfg();
        ~CCfg();
        
        bool operator==( const CCfg & other ) const;
        void Set( int index , OBPRMView_Robot* robot, CCModelBase* cc);

	virtual void SetColor( float r, float g, float b, float a ){
	  CGLModel::SetColor(r,g,b,a);
	  m_robot->SetColor(r,g,b,a);
	}
        void DrawRobot();
        void DrawBox();
	//void DrawBox(double scale);
        void DrawPoint();

	//////////////////////////////////////////////////////////////////////
	bool BuildModels(){ /*do nothing*/ return true; }
        void Draw( GLenum mode );
	void DrawSelect();
	const string GetName() const {return "Node";}
	list<string> GetInfo() const;
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        static CCfg & InvalidData(){return m_InvalidCfg;}
        int GetIndex() const {return m_index;}
        virtual void Dump();
	static void SetDof(int d) { dof = d; }
	static int GetDof(void) { return dof; }
	void SetShape(Shape shape){ m_Shape=shape; }
	CCModelBase * GetCC() const { return m_CC; }
	vector<double> GetDataCfg() {return dofs;}
	//function accessed to write data into a new .map file
	vector<double> GetUnknown(){
	  vector<double> v;
	  v.push_back(m_Unknow1); v.push_back(m_Unknow2); v.push_back(m_Unknow3);
	  return v;
	}

	//function accessed from gliDataStructure
	void CopyCfg() {
	  //cout<<"Vector dofs size::"<<dofs.size()<<endl;
	  ObjCfg.clear();
/* 	  typedef vector<double>::iterator ID; */
/* 	  for(ID i=dofs.begin(); i!=dofs.end(); i++) */
/* 	    ObjCfg.push_back(*i); */

	  ObjCfg.assign(dofs.begin(), dofs.end());
	  //cout<<"Vector ObjCfg size::"<<ObjCfg.size()<<endl;
	  //ObjCfg = dofs;
	}

	///Translation
	//@{
	double& tx(){ ObjName="Node";  CopyCfg(); return dofs[0]; }
	double& ty(){ return dofs[1]; }
	double& tz(){ return dofs[2]; }
	const double& tx() const { return dofs[0]; }
	const double& ty() const { return dofs[1]; }
	const double& tz() const { return dofs[2]; }
	///@}

        
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:
                
        vector<double> dofs;
        int m_index;
	double m_Unknow1, m_Unknow2, m_Unknow3;
	//original declaration:
	//	OBPRMView_Robot* m_robot;
	Shape m_Shape;
	CCModelBase * m_CC;       
        
	//////////////////////////////////////////////////////////////////////
        //      Private Method & Data
        //////////////////////////////////////////////////////////////////////
    private:
        static CCfg m_InvalidCfg;
	//tmp
	friend class CSimpleEdge;
    };
  
    //////////////////////////////////////////////////////////////////////
    //
    //      CEdge
    //
    //////////////////////////////////////////////////////////////////////
    
    class CSimpleEdge : public CGLModel
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
        //void Set(const Point3d& p1, const Point3d& p2){ m_s=p1; m_e=p2; }
	void Set(int id, CCfg * c1, CCfg * c2){ m_ID=id; m_s=c1; m_e=c2; }

	//////////////////////////////////////////////////////////////////////
        bool BuildModels(){ /*do nothing*/ return true; }
        void Draw( GLenum mode );
	void DrawSelect();
	
        const string GetName() const { return "Edge"; }
	list<string> GetInfo() const;
	vector<int> GetEdgeNodes();
	
        
        //////////////////////////////////////////////////////////////////////
        //      Access Method
        //////////////////////////////////////////////////////////////////////
        int & GetLP(){ return m_LP; }
        double & GetWeight(){ return m_Weight; }
        int GetID() { return m_ID; }
        //////////////////////////////////////////////////////////////////////
        //      Protected Method & Data
        //////////////////////////////////////////////////////////////////////
    protected:
        
        //Point3d m_s, m_e;
	CCfg * m_s, * m_e;

        int    m_LP;
        double m_Weight;
	int m_ID;
    };
    
}//namespace plum

#endif 
