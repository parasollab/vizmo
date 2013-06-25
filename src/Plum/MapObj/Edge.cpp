/////////////////////////// 
//Implementation for Edge. 
//////////////////////////

#include "Edge.h"
#include "Plum/Plum.h"

#include <limits.h> 
#include <stdlib.h>

namespace plum{
  
  Edge::Edge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
    m_id=-1;
    m_edgeThickness = 1;
  }

  Edge::Edge(double _weight) {
    m_lp = INT_MAX;
    m_weight = _weight;
    m_id=-1;
    m_edgeThickness = 1;
  }

  void
  Edge::Set(int _id, Cfg* _c1, Cfg* _c2, OBPRMView_Robot* _robot){
    m_id = _id;
    m_startCfg = *_c1;
    m_endCfg = *_c2;

    typedef vector<Cfg>::iterator CIT;
    for(CIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++) {
      c->m_robot = _robot;
    }
  }

  //Changing edge thickness: final step. GL lines drawn with set width. 
  void
  Edge::Draw(GLenum _mode) {

    typedef vector<Cfg>::iterator CFGIT;
    glPushName(m_id);
    
    if(m_renderMode == SOLID_MODE || m_renderMode == WIRE_MODE){
    
      float* arr_m_RGBA = &m_RGBA[0];
      glColor4fv(arr_m_RGBA);
      glLineWidth(m_edgeThickness); 
        
      glBegin(GL_LINES);
      glVertex3d(m_startCfg.tx(), m_startCfg.ty(), m_startCfg.tz());

      for(CFGIT c = m_intermediateCfgs.begin();
          c != m_intermediateCfgs.end(); c++) {
          glVertex3d (c->tx(), c->ty(), c->tz()); //ending point of prev line
          glVertex3d (c->tx(), c->ty(), c->tz()); //starting point of next line
      }

      glVertex3d(m_endCfg.tx(), m_endCfg.ty(), m_endCfg.tz());
      glEnd();

      //draw intermediate configurations
      if(m_cfgShape == Cfg::Box || m_cfgShape == Cfg::Robot){
        for(CFGIT c = m_intermediateCfgs.begin(); 
            c != m_intermediateCfgs.end(); c++){ 
            c->SetShape(m_cfgShape);
            c->SetRenderMode(WIRE_MODE);
            c->Draw(_mode);
        }
      } 
    }
    glPopName();
  }

  //Changing edge thickness: step 5
  void
  Edge::SetThickness(size_t _thickness){
    m_edgeThickness = _thickness; 
  }

  void 
  Edge::DrawSelect() {
    
    typedef vector<Cfg>::iterator CFGIT;
    glColor3d(1,1,0);
    glLineWidth(m_edgeThickness + 4); //Ensure visibility around edge itself 

    glBegin(GL_LINES);
    glVertex3d(m_startCfg.tx(), m_startCfg.ty(), m_startCfg.tz());
    
    for(CFGIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++) {
        glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
        glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
    }
    
    glVertex3d( m_endCfg.tx(),m_endCfg.ty(),m_endCfg.tz() );
    glEnd();
  }

  Edge::~Edge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
  }

  bool 
  Edge::operator==(const Edge& _other){
    if(m_lp != _other.m_lp || m_weight != _other.m_weight) 
      return false;        
    return true;
  }

  vector<string> 
  Edge::GetInfo() const {  
    
    vector<string> info; 
    ostringstream temp;
    temp << "Edge, ID= " << m_id << ", ";
    temp << "connects Node " << m_startCfg.GetIndex() << " and Node " << m_endCfg.GetIndex();
    temp << "Intermediates | ";
    typedef vector<Cfg>::const_iterator CFGIT;
    for(CFGIT c = m_intermediateCfgs.begin(); c!=m_intermediateCfgs.end(); c++)
      temp << *c << " | ";
    info.push_back(temp.str());
    return info;
  }

  vector<int> 
  Edge::GetEdgeNodes(){
    vector<int> v;
    v.push_back(m_startCfg.GetIndex());
    v.push_back(m_endCfg.GetIndex());
    return v;
  }

  ostream& 
  operator<<(ostream& _out, const Cfg& _cfg){    
    for(unsigned int iC = 0; iC < _cfg.m_dofs.size(); iC++){
      _out << _cfg.m_dofs[iC] << " ";
    }
    _out << " ";
    return _out;
  }

  istream& 
  operator>>(istream& _in, Cfg& _cfg){
    _cfg.m_dofs.clear();
    int dof=Cfg::m_dof;

    //For now, read in and discard robot index;
    int robotIndex;
    _in >> robotIndex;

    for(int i = 0; i < dof; i++){
      double value;
      _in >> value;
      _cfg.m_dofs.push_back(value); 
    }

    return _in;
  }

  ostream&
  operator<<(ostream& _out, const Edge& _edge){
    _out << _edge.m_lp << " " << _edge.m_weight << " ";
    return _out;
  }

  istream&
  operator>>(istream&  _in, Edge& _edge){
    _edge.m_intermediateCfgs.clear();
    int numIntermediates = 0;
    Cfg cfgtmp;
    _in >> numIntermediates;

    for(int i = 0; i < numIntermediates; i++){
      _in >> cfgtmp;
      _edge.m_intermediateCfgs.push_back(cfgtmp);
    }

    _in >> _edge.m_weight;
      return _in;
  }
}//namespace plum 


