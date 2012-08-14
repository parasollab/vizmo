////////////////////////////////////////////////////////////////
//Class Edge interface. Moved from Cfg.h and renamed from 'CSimpleEdge'   
////////////////////////////////////////////////////////////////

#ifndef EDGE_H_
#define EDGE_H_ 
      
#ifdef WIN32
#include <windows.h>
#endif
          
#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include "Cfg.h"                
#include "src/EnvObj/Robot.h"
                 
using namespace std;
                   
namespace plum {
  class CCfg;
  class Edge : public CGLModel {
    friend ostream& operator<<( ostream& _out, const Edge& _edge );
    friend istream& operator>>( istream& _in, Edge& _edge );
    
    public:

    //Constructor/Destructor///////////////////////////////////////

      Edge();
      Edge(double _weight);
      ~Edge();

      bool operator==( const Edge& _other );
      void Set(int _id, CCfg* _c1, CCfg* _c2, OBPRMView_Robot* _robot=NULL); 

    /////////////////////////////////////////////////////////////////
      
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
      int& GetLP(){ return m_lp; }
      double& GetWeight(){ return m_weight; }
      double& Weight(){ return m_weight; }
      int GetID() { return m_id; }
      const CCfg& GetStartCfg() { return m_s; }

      //////////////////////////////////////////////////////////////////////
      //      Protected Method & Data
      //////////////////////////////////////////////////////////////////////
    
    protected:

      CCfg m_s, m_e;
    
      int m_lp;
      double m_weight;
      int m_id;

      size_t m_edgeThickness; 

      CCfg::Shape m_cfgShape;

      //allow an edge to contain a sequence of cfgs
      vector <CCfg> m_intermediateCfgs;

    private:
      friend class Ccfg; 

  }; 

} //namespace plum

#endif 


















