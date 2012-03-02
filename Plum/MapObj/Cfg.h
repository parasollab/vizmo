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
  class CCfg : public CGLModel {
    friend ostream & operator<<( ostream& _out, const CCfg& _cfg );
    friend istream & operator>>( istream& _in, CCfg& _cfg );

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

    CCfg(const CCfg& _cfg);

    bool operator==( const CCfg & other ) const;
    void Set(int _index , OBPRMView_Robot* _robot, CCModelBase* _cc);

    virtual void SetColor( float _r, float _g, float _b, float _a ) {
       CGLModel::SetColor(_r,_g,_b,_a);
    }
    virtual void SetRenderMode( int _mode ){  m_RenderMode = _mode; }

    void DrawRobot();
    void DrawBox();
    //void DrawBox(double scale);
    void DrawPoint();

    //////////////////////////////////////////////////////////////////////
    bool BuildModels(){ /*do nothing*/ return true; }
    void Draw( GLenum _mode );
    void DrawSelect();

    const string GetName() const {

      ostringstream temp;
      temp<<"Node"<<m_index;
      return temp.str(); 
    }
    list<string> GetInfo() const;
    list<string> GetNodeInfo() const;

    //////////////////////////////////////////////////////////////////////
    //      Access Method
    //////////////////////////////////////////////////////////////////////
    static CCfg & InvalidData(){return m_invalidCfg;}
    virtual void Dump();

    int GetIndex() const {return m_index;}
    int GetCC_ID();
    vector<double> GetDataCfg() {
      return dofs;}
    //function accessed to write data into a new .map file
    vector<double> GetUnknown(){
      vector<double> v;
      v.push_back(m_unknow1); 
      v.push_back(m_unknow2);
      v.push_back(m_unknow3);
      return v;
    }
    static int GetDof(void) { return dof; }
    CCModelBase * GetCC() const { return m_cc; }
    OBPRMView_Robot* GetRobot() const {return m_robot;}

    static void SetDof(int d) { dof = d; }
    void SetShape(Shape shape){ m_shape=shape; }
    //set new values to dofs vector
    void SetCfg(vector<double> newCfg);
    void SetIndex(int _i) {m_index = _i;}
    void SetCCModel(CCModelBase* _cc);

    //function accessed from gliDataStructure
    void CopyCfg() {
      ObjCfg.clear();
      ObjCfg.assign(dofs.begin(), dofs.end());
    }

    ///Translation
    double& tx(){ ObjName="Node";  CopyCfg(); return dofs[0]; }
    double& ty(){ return dofs[1]; }
    double& tz(){ return dofs[2]; }
    const double& tx() const { return dofs[0]; }
    const double& ty() const { return dofs[1]; }
    const double& tz() const { return dofs[2]; }


    //////////////////////////////////////////////////////////////////////
    //      Protected Method & Data
    //////////////////////////////////////////////////////////////////////
  protected:

    vector<double> dofs;
    int m_index;
    double m_unknow1, m_unknow2, m_unknow3;
    Shape m_shape;
    CCModelBase * m_cc;       

    //////////////////////////////////////////////////////////////////////
    //      Private Method & Data
    //////////////////////////////////////////////////////////////////////
  private:
    static CCfg m_invalidCfg;
    friend class CSimpleEdge;

   };

   //////////////////////////////////////////////////////////////////////
   //
   //      CEdge
   //
   //////////////////////////////////////////////////////////////////////

  class CSimpleEdge : public CGLModel {
    friend ostream & operator<<( ostream& _out, const CSimpleEdge& _edge );
    friend istream & operator>>( istream& _in, CSimpleEdge& _edge );

  public:

    //////////////////////////////////////////////////////////////////////
    //      Constructor/Destructor
    //////////////////////////////////////////////////////////////////////

    CSimpleEdge();
    CSimpleEdge(double weight);
    ~CSimpleEdge();

    bool operator==( const CSimpleEdge& _other );
    void Set(int _id, CCfg* _c1, CCfg* _c2, OBPRMView_Robot* _robot=NULL); 

    //////////////////////////////////////////////////////////////////////
    bool BuildModels(){ return true; }
    void Draw( GLenum mode );
    void DrawSelect();

    const string GetName() const { 

    ostringstream temp;
    temp<< "Edge"<< m_id;
    return temp.str(); 

    }
    
    list<string> GetInfo() const;
    vector<int> GetEdgeNodes();

    void SetCfgShape(char _shape) {
      switch (_shape) {
        case 'r':
          m_cfgShape = CCfg::Robot;
          break;
    
        case 'b':
          m_cfgShape = CCfg::Box;
          break;

        case 'p':
          m_cfgShape = CCfg::Point;
          break;

        default:
          break;

      }
    }

    //////////////////////////////////////////////////////////////////////
    //      Access Method
    //////////////////////////////////////////////////////////////////////
    int & GetLP(){ return m_lp; }
    double & GetWeight(){ return m_weight; }
    double & Weight(){ return m_weight; }
    int GetID() { return m_id; }
    const CCfg & GetStartCfg() { return m_s; }
    
    //////////////////////////////////////////////////////////////////////
    //      Protected Method & Data
    //////////////////////////////////////////////////////////////////////
    
  protected:

    CCfg m_s, m_e;
    
    int m_lp;
    double m_weight;
    int m_id;

    CCfg::Shape m_cfgShape;

    //allow an edge to contain a sequence of cfgs
    vector <CCfg> m_intermediateCfgs;

   };

}//namespace plum

#endif 
