#ifndef _PLUM_CCMODEL_H_
#define _PLUM_CCMODEL_H_

#include <graph.h>
#include <algorithms/connected_components.h>

#include <iostream>
#include <vector> 
#include <string>


#include "GLModel.h"
#include "MapLoader.h"
#include "Cfg.h"
#include "SimpleCfg.h"
#include "src/EnvObj/Robot.h"

using namespace std;
using namespace stapl;

namespace plum{

  class CCModelBase : public CGLModel {
    
  public:
    //type for the shape of node representation
    enum Shape { Robot, Box, Point};

    // to know if change color of CC's
    bool newColor;
    int id;
    char m_shape; //'r' robot, 'b' box, 'p' point

    CCModelBase(unsigned int _ID){ 
      m_id = _ID; 
      id = _ID;
      m_enableSelection = true;
      m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
      //Set random Color
      m_RGBA[0]=((float)rand())/RAND_MAX;
      m_RGBA[1]=((float)rand())/RAND_MAX;
      m_RGBA[2]=((float)rand())/RAND_MAX;
      //size
      m_fRobotScale = 1;
      m_fBoxScale = 1;
      m_sNodeShape = Point;
      //display id
      m_idEdges = m_idRobot = m_idBox = m_idPt = -1;

      newColor = false;
    }

    virtual ~CCModelBase(){
      glDeleteLists(m_idEdges,1);
      glDeleteLists(m_idRobot,1);
      glDeleteLists(m_idBox,1);
      glDeleteLists(m_idPt,1);
    }

    void ReBuildAll(){
      glDeleteLists(m_idEdges,1);
      glDeleteLists(m_idRobot,1);
      glDeleteLists(m_idBox,1);
      glDeleteLists(m_idPt,1);
      m_idEdges=-1; 
      m_idRobot=-1; 
      m_idBox=-1;   
      m_idPt=-1;      
    }


    //////////////////////////////////////////////////////////////////////
    // Accss
    //////////////////////////////////////////////////////////////////////      

    void scaleNode( float _scale, Shape _s ) { 
      m_sNodeShape = _s; 
      if(m_sNodeShape == Robot && m_fRobotScale != _scale){
        m_fRobotScale = _scale;
        //glDeleteLists(m_idRobot,1);
        ReBuildAll();
        m_idRobot=-1; //need new id
      }
      else if(m_sNodeShape == Box && m_fBoxScale != _scale){
        m_fBoxScale = _scale;
        glDeleteLists(m_idBox,1);
        m_idBox = -1; //need new id
      }
    }

    void changeShape(Shape _s) { m_sNodeShape= _s; }

    /// Allow to change color of CC's
    void changeColor(float _r, float _g, float _b, Shape _s){ 
      ReBuildAll();
      m_sNodeShape = _s;
      CGLModel::SetColor(_r, _g, _b, 1); 

      m_RGBA[0] = _r;
      m_RGBA[1] = _g;
      m_RGBA[2] = _b;

    }

    /// OTHER:: Allow to change color of CC's
    void SetColor( float _r, float _g, float _b, float _a ){
      CGLModel::SetColor(_r, _g, _b, _a); 
      //force to rebuild robot
      glDeleteLists(m_idRobot, 1);
      m_idRobot = -1; //need new id
    } 

    //get id
    int ID() const { return m_id; }

    //get shape
    Shape getShape(){ return m_sNodeShape;}
    
    //get size
    float getRobotSize() {return  m_fRobotScale;}
    float getBoxSize() {return m_fBoxScale;}
    
    //get color
    float *getColor() {return m_RGBA;}

    protected:
    //CC ID
    int m_id;

    //to store the node size of the current selection
    float m_fRobotScale;
    float m_fBoxScale;

    //to store the "name" of the item selected
    Shape m_sNodeShape;

    //display ids
    int m_idEdges; //id for edges
    int m_idRobot; //id for original robots
    int m_idBox;   //id for box
    int m_idPt;    //id for points
  };

   /////////////////////////////////////////////////////////////////
   // CCModel
   ////////////////////////////////////////////////////////////////

  template <class Cfg, class WEIGHT> class CCModel : public CCModelBase {
    typedef typename CMapLoader<Cfg,WEIGHT>::Wg WG;

  public:

    //////////////////////////////////////////////////////////////////////
    // Constructor/Destructor
    //////////////////////////////////////////////////////////////////////
    CCModel(unsigned int _ID);
    virtual ~CCModel();

    //////////////////////////////////////////////////////////////////////
    // GL interface functions
    //////////////////////////////////////////////////////////////////////      
    bool BuildModels(); //not used, should not call this
    void Draw( GLenum _mode );
    void DrawSelect();
    void Select( unsigned int* _index, vector<gliObj>& _sel );
    typedef graph<DIRECTED,MULTIEDGES,Cfg,WEIGHT> WDG;
    typedef typename WDG::vertex_descriptor VID;
    typedef vector_property_map<WDG, size_t> color_map_t;
    color_map_t cmap;
    bool BuildModels( VID _id, WG* _g); //call this instread
    const string GetName() const;
    virtual list<string> GetInfo() const;

    void RobotModel(OBPRMView_Robot* _pRobot){
    m_pRobot = _pRobot;
  }

    ///////////////////////////////////////////////////
    // Access Functions
    //////////////////////////////////////////////////

    int GetNumNodes(){ return m_Nodes.size();}
    double GetNodeData() { return m_Nodes[0]->tx();}
    // Functions to be accessed to get nodes and edges info.
    //to write a new *.map file (this functions are 
    //currently accessed from vizmo2.ccp: vizmo::GetNodeInfo()
    vector<Cfg*> GetNodesInfo() {return m_Nodes;}
    vector<WEIGHT> GetEdgesInfo() { return m_Edges; }
    WG * GetGraph(){ return m_graph; }

    //add a new Edge (from the 'add edge' option) 
    //June 16-05
    void addEdge(Cfg* _c1, Cfg* _c2){
      typename WDG::vertex_iterator vi;
      typename WDG::adj_edge_iterator ei;
      typename  WDG::edge_descriptor ed(_c1->GetIndex(),_c2->GetIndex());
      m_graph->find_edge(ed, vi, ei);

      WEIGHT w  = (*ei).property();   
      w.Set(m_Edges.size(),_c1,_c2);
      m_Edges.push_back(w);
    }

    /// Allow to change color of CC's
    void changeColor(float _r, float _g, float _b, Shape _s){ 

      ReBuildAll();

      m_sNodeShape = _s;
      CGLModel::SetColor(_r, _g, _b, 1); 

      m_RGBA[0] = _r;
      m_RGBA[1] = _g;
      m_RGBA[2] = _b;

      typedef vector<CCfg>::iterator NIT;
      for(NIT i=m_Nodes.begin();i!=m_Nodes.end();i++){
        (*i).m_RGBA[0] = _r;
        (*i).m_RGBA[1] = _g;
        (*i).m_RGBA[2] = _b;
      }

      typedef vector<CSimpleEdge>::iterator EIT;
      for(EIT i=m_Edges.begin();i!=m_Edges.end();i++){
        (*i).m_RGBA[0] = _r;
        (*i).m_RGBA[1] = _g;
        (*i).m_RGBA[2] = _b;

      }
    }


    void SetColor(float _r, float _g, float _b, float _a) { 

      ReBuildAll();

      m_RGBA[0] = _r;
      m_RGBA[1] = _g;
      m_RGBA[2] = _b;

      typedef vector<CCfg>::iterator NIT;
      for(NIT i=m_Nodes.begin();i!=m_Nodes.end();i++) {
        (*i).m_RGBA[0] = _r;
        (*i).m_RGBA[1] = _g;
        (*i).m_RGBA[2] = _b;
      }

      typedef vector<CSimpleEdge>::iterator EIT;
      for(EIT i=m_Edges.begin();i!=m_Edges.end();i++) {
        (*i).m_RGBA[0] = _r;
        (*i).m_RGBA[1] = _g;
        (*i).m_RGBA[2] = _b;
      }

    }

    void change_properties(Shape _s, float _size, float* _color, bool _isNew){
      m_RenderMode = CPlumState::MV_SOLID_MODE;
      m_sNodeShape = _s;
      if(_s == 0){ 
        m_fRobotScale = _size;
        //force to rebuild robot
        glDeleteLists(m_idRobot,1);
        m_idRobot=-1; //need new id
      }
      else{
        m_fBoxScale = _size;
        glDeleteLists(m_idBox,1);
        m_idBox=-1; //need new id
      }
      glDeleteLists(m_idEdges,1);
      m_idEdges=-1; 
      glDeleteLists(m_idPt,1);
      m_idPt=-1;       
      if(_isNew)
        changeColor(_color[0], _color[1], _color[2], _s);

    }

    virtual void GetChildren( list<CGLModel*>& _models ){ 
      typedef vector<CCfg>::iterator NIT;
      for(NIT i=m_Nodes.begin();i!=m_Nodes.end();i++)
        _models.push_back(&*i);

      typedef vector<CSimpleEdge>::iterator EIT;
      for(EIT i=m_Edges.begin();i!=m_Edges.end();i++)
        _models.push_back(&(*i)); 
    }


  public:
    void BuildNodeModels(GLenum _mode);

    void DrawRobotNodes(GLenum _mode);
    void DrawBoxNodes(GLenum _mode);
    void DrawPointNodes(GLenum _mode);

    void BuildEdges();

    void ChangeColor(GLenum _mode);

  public:           
    vector<Cfg> m_Nodes;
    vector<WEIGHT> m_Edges;
    OBPRMView_Robot* m_pRobot;
    WG * m_graph;

  };


  /*********************************************************************
  *
  *      Implementation of CCModel
  *
  *********************************************************************/
  template <class Cfg, class WEIGHT>
  CCModel<Cfg, WEIGHT>::CCModel(unsigned int _ID) : CCModelBase(_ID){
    m_graph=NULL;
  }

  template <class Cfg, class WEIGHT>
  CCModel<Cfg, WEIGHT>::~CCModel() {
  }

  template <class Cfg, class WEIGHT>
  bool CCModel<Cfg, WEIGHT>::BuildModels() {
    //do not call this function
    cerr  << "CCModel<Cfg, WEIGHT>::BuildModels() : Do not call this function\n"
          << "call CCModel<Cfg, WEIGHT>::BuildModel(VID id,WG* g)"
          << " instead" << endl;
    return false;
  }

  template <class Cfg, class WEIGHT>
  bool CCModel<Cfg, WEIGHT>::BuildModels(VID _id, WG* _g) { 
    if( _g == NULL ){
      cout<<"Graph is null"<<endl;
      return false;
    }
    m_graph = _g;
    //Setup cc nodes        
    vector<VID> cc;
    cmap.reset();
    get_cc(*_g, cmap, _id, cc);
    int nSize=cc.size();    
    typename WDG::vertex_iterator cvi, cvi2, vi;
    typename WDG::adj_edge_iterator ei;
    for( int iN=0; iN<nSize; iN++ ){
      VID nid=cc[iN];    
      Cfg cfg = (_g->find_vertex(nid))->property();
      cfg.Set(nid,m_pRobot,this);
      m_Nodes.push_back(cfg);
    }

    //Setup edges
    vector< pair<VID,VID> > ccedges;

    cmap.reset();
    get_cc_edges(*_g, cmap, ccedges, _id);
    int eSize=ccedges.size();
    m_Edges.reserve(eSize/2);
    int edge_index=0;
    for( int iE=0; iE<eSize; iE++ ){
      if( ccedges[iE].first<ccedges[iE].second )
        continue;
      Cfg* cfg1 = &((_g->find_vertex(ccedges[iE].first) )->property() ) ;
      Cfg* cfg2 = &((_g->find_vertex(ccedges[iE].second) )->property() );          
      typename  WG::edge_descriptor ed(ccedges[iE].first,ccedges[iE].second);
      _g->find_edge(ed, vi, ei);
      WEIGHT w  = (*ei).property(); 
       w.Set(edge_index,cfg1,cfg2, m_pRobot);
      edge_index++;
      m_Edges.push_back(w);
    }

    return true;
  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::BuildNodeModels(GLenum _mode) {
    ReBuildAll(); 
    switch( m_sNodeShape ){
      case Robot: 
        m_idRobot = glGenLists(1);
        glNewList( m_idRobot, GL_COMPILE );
        DrawRobotNodes(_mode);
        break;
      
      case Box: 
        m_idBox = glGenLists(1);
        glNewList( m_idBox, GL_COMPILE );
        DrawBoxNodes(_mode);
        break;
      
      case Point: 
        m_idPt = glGenLists(1);
        glNewList( m_idPt, GL_COMPILE );
        DrawPointNodes(_mode);
        break;
    }
    glEndList();
  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::DrawRobotNodes(GLenum _mode) {

    if( m_pRobot==NULL ){
      cout << "m_pRobot is NULL" << endl;
      return;
    } //no robot given

    float origColor[4];
    memcpy(origColor,m_pRobot->GetColor(),4*sizeof(float));
    m_pRobot->BackUp();
    //m_idRobot = glGenLists(1);
    //glNewList( m_idRobot, GL_COMPILE );
    if( _mode==GL_RENDER ){
      //m_pRobot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],m_pRobot->GetColor()[3]);
      glEnable(GL_LIGHTING); 
    }

    int nSize = m_Nodes.size(); 
    //cout << " draw robot nodes called"  << endl;
    for( int iN=0; iN<nSize; iN++ ){ 
      //cout << "robot scale " << m_fRobotScale << endl; 
      m_Nodes[iN].Scale(m_fRobotScale,m_fRobotScale,m_fRobotScale);
      m_Nodes[iN].SetShape(CCfg::Robot); 
      m_pRobot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
      m_Nodes[iN].Draw(_mode);//draw robot;  
    }

    m_pRobot->Restore();
    m_pRobot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]);

  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::DrawBoxNodes(GLenum _mode) {
    glEnable(GL_LIGHTING);
    int nSize=m_Nodes.size(); 
    for( int iN=0; iN<nSize; iN++ ){
      //cout << "box scale " << m_fBoxScale << endl; 
      m_Nodes[iN].Scale(m_fBoxScale,m_fBoxScale,m_fBoxScale);
      m_Nodes[iN].SetShape(CCfg::Box);
      m_Nodes[iN].Draw(_mode); //draw box;
    }
  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::DrawPointNodes(GLenum _mode){ 
     glDisable(GL_LIGHTING);
     glPointSize(4);
     int nSize=m_Nodes.size();
     for( int iN=0; iN<nSize; iN++ ){
        m_Nodes[iN].Scale(1,1,1);
        m_Nodes[iN].SetShape(CCfg::Point);
        m_Nodes[iN].Draw(_mode);//draw point;
     }
     glEndList();
  }



  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::BuildEdges(){
    //build edges
    m_idEdges = glGenLists(1);
    glNewList( m_idEdges, GL_COMPILE );
    glDisable(GL_LIGHTING);
    {
      int eSize=m_Edges.size();
      for( int iE=0; iE<eSize; iE++ ){
         m_Edges[iE].SetCfgShape(m_shape);
         m_Edges[iE].Draw(m_RenderMode);
      }
    }   
    glEndList();
  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::ChangeColor(GLenum _mode){

    m_pRobot->BackUp();

    m_idRobot = glGenLists(1);

    glNewList( m_idRobot, GL_COMPILE );
    glEnable(GL_LIGHTING);
    int nSize=m_Nodes.size();

    for( int iN=0; iN<nSize; iN++ ){
       if(m_shape == 'r'){
          m_Nodes[iN].SetShape(CCfg::Robot);
          m_Nodes[iN].Scale(m_fRobotScale,m_fRobotScale,m_fRobotScale);
          //cout << "robot scale " << m_fRobotScale << endl; 
          m_Nodes[iN].Draw(_mode);//draw robot;
       }
       else if(m_shape == 'b'){
          m_Nodes[iN].SetShape(CCfg::Box);
          m_Nodes[iN].Scale(m_fBoxScale,m_fBoxScale,m_fBoxScale);
          //cout << "box scale " << m_fBoxScale << endl; 
          m_Nodes[iN].Draw(_mode);//draw box
       }
       else if(m_shape == 'p'){
          m_Nodes[iN].SetShape(CCfg::Point);
          m_Nodes[iN].Scale(1,1,1);
          m_Nodes[iN].Draw(_mode);//draw point
       }
    }

    glEndList();

    m_pRobot->Restore();

  }


   ///////////////////////////////////////////////////////////////////////////
  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::Draw( GLenum _mode ) {
    if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE )
      return;
    if( _mode==GL_SELECT && !m_enableSelection )
      return;

    ///////////////////////////////////////////////////////////////////////
    //glColor4fv(m_RGBA); //Set Color
    ////////////////////////////////////////////////////////////////////
    // Draw vertex
    //m_pRobot->size=m_fNodeScale;
    int list=-1;
    ReBuildAll();
    switch( m_sNodeShape ){ 
      case Robot: 
         if(m_idRobot==-1) BuildNodeModels(_mode);
         list=m_idRobot;
         m_shape='r'; 
         break;

      case Box: 
         if(m_idBox==-1) BuildNodeModels(_mode);
         list=m_idBox; 
         m_shape='b';
         break;

      case Point: 
         if(m_idPt==-1) BuildNodeModels(_mode);
         list=m_idPt; 
         m_shape='p';
         break;
    }

    //if(newColor) {ChangeColor(mode);}



    if( _mode == GL_SELECT )
      glPushName(1); //1 means nodes

    glCallList(list);

    if( _mode == GL_SELECT )
      glPopName();

    ////////////////////////////////////////////////////////
    // Draw edge

    //if( mode==GL_SELECT ) return; //no selection for edge
    //glColor3f(0.1f,0.1f,0.1f);
    //glColor3f(m_RGBA[0],m_RGBA[1],m_RGBA[2]);

    if( m_idEdges==-1 ) BuildEdges();
    //glColor3f(0.2f,0.2f,0.2f);
    //glColor3f(m_RGBA[0],m_RGBA[1],m_RGBA[2]);
    glLineWidth(1);

    if( _mode == GL_SELECT )
      glPushName(2); //2 means edge
    glCallList(m_idEdges);
    if( _mode == GL_SELECT )
      glPopName();
  }


  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::DrawSelect() {
    if( m_idEdges==-1 ) BuildEdges();
    glColor3f(1,1,0);
    glLineWidth(3);
    glCallList(m_idEdges);
    glLineWidth(1);
  }

  template <class Cfg, class WEIGHT>
  void CCModel<Cfg, WEIGHT>::
  Select( unsigned int* _index, vector<gliObj>& _sel ) { 
    typename WDG::vertex_iterator cvi;
    if( _index==NULL || m_graph==NULL )
      return;
    if(_index[0]==1){
      CCfg* cfg= &( (m_graph->find_vertex(_index[1]))->property());

      //cfg->SetShape((CCfg::Shape)m_sNodeShape);
      _sel.push_back(cfg);
    }
    else 
      _sel.push_back(&m_Edges[_index[1]]);
  }

  template <class Cfg, class WEIGHT>
  const string CCModel<Cfg, WEIGHT>::GetName() const { 
    ostringstream temp;
    temp<<"CC"<<m_id;
    return temp.str(); 
  }

  template <class Cfg, class WEIGHT>
  list<string> CCModel<Cfg, WEIGHT>::GetInfo() const { 

    list<string> info; 
    ostringstream temp,temp2;
    temp << "There are " << m_Nodes.size() << " nodes";
    info.push_back(temp.str());
    temp2 << "There are " << m_Edges.size() << " edges";
    info.push_back(temp2.str());
    return info;
  }

}//end of namespace plum

#endif //_PLUM_CCMODEL_H_


