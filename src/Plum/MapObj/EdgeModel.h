///////////////////////
//Class EdgeModel interface.   
///////////////////////

#ifndef EDGE_MODEL_H_
#define EDGE_MODEL_H_ 
          
#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include "CfgModel.h"                
#include "EnvObj/Robot.h"
                 
using namespace std;
                   
namespace plum {
  
  class CfgModel;
  class EdgeModel : public CGLModel{
    
    friend ostream& operator<<(ostream& _out, const EdgeModel& _edge);
    friend istream& operator>>(istream& _in, EdgeModel& _edge);
    
    public:
      
      EdgeModel();
      EdgeModel(double _weight);
      ~EdgeModel();

      bool operator==(const EdgeModel& _other);
      void Set(int _id, CfgModel* _c1, CfgModel* _c2, OBPRMView_Robot* _robot=NULL); 
      bool BuildModels(){ return true; }
      void SetThickness(size_t); 
      void Draw(GLenum _mode);
      void DrawSelect();

      const string 
      GetName() const { 
        ostringstream temp;
        temp << "Edge" << m_id;
        return temp.str(); 
      } 
    
      vector<string> GetInfo() const; 
      vector<int> GetEdgeNodes(); 
      
      void 
      SetCfgShape(char _shape) {
        switch (_shape) {
          case 'r':
            m_cfgShape = CfgModel::Robot;
            break;
    
          case 'b':
            m_cfgShape = CfgModel::Box;
            break;

          case 'p':
            m_cfgShape = CfgModel::Point;
            break;

          default:
            break;
         }
      } 

      int& GetLP(){ return m_lp; }
      double& GetWeight(){ return m_weight; }
      double& Weight(){ return m_weight; }
      int GetID() { return m_id; }
      const CfgModel& GetStartCfg() { return m_startCfg; }
    
    protected:

      CfgModel m_startCfg, m_endCfg;
    
      int m_lp;
      double m_weight;
      int m_id;
      size_t m_edgeThickness; 

      CfgModel::Shape m_cfgShape;
      vector <CfgModel> m_intermediateCfgs;

    private:
      friend class Ccfg; 
  }; 
} //namespace plum

#endif 


















