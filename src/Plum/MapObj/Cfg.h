///////////////////////////////////////////////////////////////////////////////
// Cfg.h: interface for the CCfg class. Edge class moved from here to its
// own files. 
///////////////////////////////////////////////////////////////////////////////

#ifndef CFG_H_
#define CFG_H_ 

#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "EnvObj/Robot.h"

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
    friend ostream& operator<<( ostream& _out, const CCfg& _cfg );
    friend istream& operator>>( istream& _in, CCfg& _cfg );

    public:
    //type for the shape of node representation
      enum Shape { Robot, Box, Point};

    //////////////////////////////////////////
    // DOF variable
    /////////////////////////////////////////

      static int m_dof;

    /////////////////////////
    // for collison detection 
    //////////////////////////

      bool m_coll;

    //testing:
      OBPRMView_Robot* m_robot;

    //////////////////////////////////////////////////////////////////////
    //      Constructor/Destructor
    //////////////////////////////////////////////////////////////////////

      CCfg();
      ~CCfg();

      CCfg(const CCfg& _cfg);

      bool operator==(const CCfg& _other) const;
      void Set(int _index , OBPRMView_Robot* _robot, CCModelBase* _cc);

      virtual void 
      SetColor( float _r, float _g, float _b, float _a ) {
        CGLModel::SetColor(_r,_g,_b,_a);
      }
      
      void DrawRobot();
      void DrawBox();
      //void DrawBox(double scale);
      void DrawPoint();

      //////////////////////////////////////////////////////////////////////
      bool BuildModels(){ /*do nothing*/ return true; }
      void Draw(GLenum _mode);
      void DrawSelect();

      const string 
      GetName() const {
        ostringstream temp;
        temp << "Node" << m_index;
        return temp.str(); 
      } 
      
      vector<string> GetInfo() const;
      vector<string> GetNodeInfo() const;

      //////////////////////////////////////////////////////////////////////
      //      Access Method
      //////////////////////////////////////////////////////////////////////
      static CCfg& InvalidData(){ return m_invalidCfg; }
      virtual void Dump();

      int GetIndex() const { return m_index; }  
      int GetCC_ID();
      vector<double> GetDataCfg() { return m_dofs; }
      //function accessed to write data into a new .map file
      vector<double> 
      GetUnknown(){
        vector<double> v;
        v.push_back(m_unknow1); 
        v.push_back(m_unknow2);
        v.push_back(m_unknow3);
        return v;
      }
      
      static int GetDof(void) { return m_dof; }
      CCModelBase* GetCC() const { return m_cc; }
      OBPRMView_Robot* GetRobot() const {return m_robot;}

      static void SetDof(int _d) { m_dof = _d; }
      void SetShape(Shape _shape){ m_shape =_shape; }
      //set new values to dofs vector
      void SetCfg(vector<double> _newCfg);
      void SetIndex(int _i) { m_index = _i; } 
      void SetCCModel(CCModelBase* _cc);

      //function accessed from gliDataStructure
      void 
      CopyCfg() {
        ObjCfg.clear();
        ObjCfg.assign(m_dofs.begin(), m_dofs.end());
      }

      ///Translation
      double& tx(){ ObjName="Node";  CopyCfg(); return m_dofs[0]; }
      double& ty(){ return m_dofs[1]; }
      double& tz(){ if(m_dofs.size()>2) return m_dofs[2]; else return
        m_defaultDOF;}
      const double& tx() const { return m_dofs[0]; }
      const double& ty() const { return m_dofs[1]; }
      const double& tz() const { if(m_dofs.size()>2) return m_dofs[2]; else
        return m_defaultDOF;}


      static Shape m_shape;
    //////////////////////////////////////////////////////////////////////
    //      Protected Method & Data
    //////////////////////////////////////////////////////////////////////

    protected:

      vector<double> m_dofs;
      int m_index;
      double m_unknow1, m_unknow2, m_unknow3;
      CCModelBase* m_cc;       

    //////////////////////////////////////////////////////////////////////
    //      Private Method & Data
    //////////////////////////////////////////////////////////////////////
    private:
      static CCfg m_invalidCfg;
      static double m_defaultDOF;

  };

}//namespace plum

#endif 

