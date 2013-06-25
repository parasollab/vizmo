///////////////////////
//Class Edge interface.   
///////////////////////

#ifndef EDGE_H_
#define EDGE_H_ 
          
#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include "Cfg.h"                
#include "EnvObj/Robot.h"
                 
using namespace std;
                   
namespace plum {
  
  class Cfg;
  class Edge : public CGLModel{
    
    friend ostream& operator<<(ostream& _out, const Edge& _edge);
    friend istream& operator>>(istream& _in, Edge& _edge);
    
    public:
      
      Edge();
      Edge(double _weight);
      ~Edge();

      bool operator==(const Edge& _other);
      void Set(int _id, Cfg* _c1, Cfg* _c2, OBPRMView_Robot* _robot=NULL); 
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
            m_cfgShape = Cfg::Robot;
            break;
    
          case 'b':
            m_cfgShape = Cfg::Box;
            break;

          case 'p':
            m_cfgShape = Cfg::Point;
            break;

          default:
            break;
         }
      } 

      int& GetLP(){ return m_lp; }
      double& GetWeight(){ return m_weight; }
      double& Weight(){ return m_weight; }
      int GetID() { return m_id; }
      const Cfg& GetStartCfg() { return m_startCfg; }
    
    protected:

      Cfg m_startCfg, m_endCfg;
    
      int m_lp;
      double m_weight;
      int m_id;
      size_t m_edgeThickness; 

      Cfg::Shape m_cfgShape;
      vector <Cfg> m_intermediateCfgs;

    private:
      friend class Ccfg; 
  }; 
} //namespace plum

#endif 


















