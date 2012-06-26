///////////////////////////////////////////////////////////////////////
//Implementation for Edge. Moved from Cfg.cpp and renamed from CSimpleEdge 
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "Edge.h"
#include "Plum.h"
#include <limits.h> 

namespace plum{

  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////

  Edge::Edge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
    m_id=-1;
  }

  Edge::Edge(double _weight) {
    m_lp = INT_MAX;
    m_weight = _weight;
    m_id=-1;
  }

  void
  Edge::Set(int _id,
        CCfg* _c1, CCfg* _c2,
        OBPRMView_Robot* _robot) {
    m_id = _id;
    m_s = *_c1;
    m_e = *_c2;

    typedef vector<CCfg>::iterator CIT;
     
    for(CIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++) {
        c->m_robot = _robot;
    }
        }

  void
  Edge::Draw(GLenum _mode) {

      typedef vector<CCfg>::iterator CFGIT;
      glPushName(m_id);
      if(m_RenderMode == CPlumState::MV_SOLID_MODE ||
          m_RenderMode == CPlumState::MV_WIRE_MODE){
        
        float* arr_m_RGBA = &m_RGBA[0]; 
        glColor4fv(arr_m_RGBA);
        
        glBegin(GL_LINES);
        glVertex3d( m_s.tx(),m_s.ty(),m_s.tz() );

        for(CFGIT c = m_intermediateCfgs.begin();
            c != m_intermediateCfgs.end(); c++) {
          glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
          glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
        }

        glVertex3d( m_e.tx(),m_e.ty(),m_e.tz() );
        glEnd();

        //draw intermediate configurations
        if(m_cfgShape == CCfg::Box || m_cfgShape == CCfg::Robot) {
          for(CFGIT c = m_intermediateCfgs.begin(); 
              c != m_intermediateCfgs.end(); c++) { 
            c->SetShape(m_cfgShape);
            c->SetRenderMode(CPlumState::MV_WIRE_MODE);
            c->Draw(_mode);
          }
        } 
      }
      glPopName();
    }

  void 
  Edge::DrawSelect() {
    typedef vector<CCfg>::iterator CFGIT;
    glColor3d(1,1,0);
    glLineWidth(4);

    glBegin( GL_LINES );
    glVertex3d( m_s.tx(),m_s.ty(),m_s.tz() );
    for(CFGIT c = m_intermediateCfgs.begin();
        c != m_intermediateCfgs.end(); c++) {
      glVertex3d (c->tx(), c->ty(), c->tz() ); //ending point of prev line
      glVertex3d (c->tx(), c->ty(), c->tz() ); //starting point of next line
    }
    glVertex3d( m_e.tx(),m_e.ty(),m_e.tz() );
    glEnd();
  }

  Edge::~Edge() {
    m_lp = INT_MAX;
    m_weight = LONG_MAX;
  }


  bool 
  Edge::operator==( const Edge& _other ){
    if( m_lp != _other.m_lp || m_weight != _other.m_weight ) 
      return false;        
    return true;
  }

  list<string> 
  Edge::GetInfo() const {  
    list<string> info; 
    ostringstream temp;
    temp << "Edge, ID= " << m_id << ", ";
    temp << "connects Node " << m_s.GetIndex() << " and Node " << m_e.GetIndex();
    info.push_back(temp.str());
    return info;
  }

  vector<int> 
  Edge::GetEdgeNodes(){
    vector<int> v;
    v.push_back(m_s.GetIndex());
    v.push_back(m_e.GetIndex());
    return v;
  }

  //////////////////////////////////////////////////////////////////////
  // Opers
  //////////////////////////////////////////////////////////////////////

  ostream& 
  operator<<( ostream& _out, const CCfg& _cfg ) {
    for( unsigned int iC=0; iC < _cfg.m_dofs.size(); iC++ ){
      _out << _cfg.m_dofs[iC] << " ";
    }
    _out << " ";
    return _out;
  }

  istream& 
  operator>>(istream& _in, CCfg& _cfg) {
    _cfg.m_dofs.clear();
    int dof=CCfg::m_dof;

    for(int i = 0; i < dof; i++){
      double value;
      _in >> value;
      _cfg.m_dofs.push_back(value); 
    }

    return _in;
  }

  ostream&
  operator<<(ostream& _out, const Edge& _edge) {
      _out << _edge.m_lp << " " << _edge.m_weight << " ";
      return _out;
    }

  istream&
  operator>>( istream&  _in, Edge& _edge ) {
      int numIntermediates = 0;
      CCfg cfgtmp = _edge.GetStartCfg();
      _in >> numIntermediates;

      for(int i = 0; i < numIntermediates; i++) {
        _in >> cfgtmp;
        _edge.m_intermediateCfgs.push_back(cfgtmp);
      }

      _in >> _edge.m_weight;
      return _in;
    }
}//namespace plum 


