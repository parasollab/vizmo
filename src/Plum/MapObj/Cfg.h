///////////////////////////////////////
// Cfg.h: interface for the Cfg class.  
///////////////////////////////////////

#ifndef CFG_H_
#define CFG_H_ 

#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "EnvObj/Robot.h"

using namespace std;

namespace plum{

  class CCModelBase;
  class Cfg : public CGLModel{
    
    friend ostream& operator<<(ostream& _out, const Cfg& _cfg);
    friend istream& operator>>(istream& _in, Cfg& _cfg);

    public:
      enum Shape { Robot, Box, Point }; //Node representation 
      static int m_dof;
      bool m_coll; //For collision detection 
      
      OBPRMView_Robot* m_robot; //Testing 

      Cfg();
      ~Cfg();
      Cfg(const Cfg& _cfg);

      bool operator==(const Cfg& _other) const;
      void Set(int _index, OBPRMView_Robot* _robot, CCModelBase* _cc);

      virtual void 
      SetColor(float _r, float _g, float _b, float _a){
        CGLModel::SetColor(_r,_g,_b,_a);
      }
      
      void DrawRobot();
      void DrawBox();
      void DrawPoint();
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

      static Cfg& InvalidData(){ return m_invalidCfg; }
      virtual void Dump();

      int GetIndex() const { return m_index; }  
      int GetCC_ID();
      vector<double> GetDataCfg() { return m_dofs; }
      //Function accessed to write data into a new .map file
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
      OBPRMView_Robot* GetRobot() const { return m_robot; }

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
      double& tx(); 
      double& ty(); 
      double& tz(); 
      const double& tx() const; 
      const double& ty() const; 
      const double& tz() const; 

      static Shape m_shape;

    protected:

      vector<double> m_dofs;
      int m_index;
      double m_unknow1, m_unknow2, m_unknow3;
      CCModelBase* m_cc;       

    private:
      static Cfg m_invalidCfg;
      static double m_defaultDOF;
      static bool m_isPlanarRobot;
      static bool m_isVolumetricRobot;
      static bool m_isRotationalRobot; 
  };
}//namespace plum

#endif 

