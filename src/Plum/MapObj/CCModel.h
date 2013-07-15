#ifndef CCMODEL_H_
#define CCMODEL_H_

#include <graph.h>
#include <algorithms/connected_components.h>

#include <iostream>
#include <vector> 
#include <map> 
#include <string>


#include "Plum/GLModel.h"
#include "CfgModel.h"
#include "EdgeModel.h" 
#include "MapModel.h" 
#include "EnvObj/Robot.h"

using namespace std;
using namespace stapl;
using namespace plum;

namespace plum{

  template<typename, typename>
  class MapModel; 
  
  class CCModelBase : public GLModel{
    
  public:
    enum Shape {Robot, Box, Point}; //Node representation 

    bool newColor; //Have CC colors been changed?
    char m_shape; //'r' robot, 'b' box, 'p' point

    int ID() const {return m_id;}
    Shape getShape(){return m_nodeShape;}
    float getRobotSize() {return  m_robotScale;}
    float getBoxSize() {return m_boxScale;}
    float GetPointSize() {return m_pointScale;}   
    vector<float> getColor() {return m_RGBA;}

  protected:
    int m_id; //CC ID 

    //to store the node size of the current selection
    float m_robotScale;
    float m_boxScale;
    float m_pointScale;  

    size_t m_edgeThickness; 
    Shape m_nodeShape;

    //display ids
    int m_idEdges; //id for edges
    int m_idRobot; //id for original robots
    int m_idBox;   //id for box
    int m_idPt;    //id for points

  public:   
    //CCModelBase:: ***Eventually, these will belong outside the class!
    //Putting them inside as temporary linker error fix 
    CCModelBase(unsigned int _ID){ 
      
      m_id = _ID; 
      m_enableSelection = true;
      m_renderMode = INVISIBLE_MODE;
      //Set random Color
      m_RGBA.clear(); 
      m_RGBA.push_back(((float)rand())/RAND_MAX);
      m_RGBA.push_back(((float)rand())/RAND_MAX);
      m_RGBA.push_back(((float)rand())/RAND_MAX);  
      //size
      m_robotScale = 1;
      m_boxScale = 1;
      m_pointScale = 10;  
      m_nodeShape = Point;
      //display id
      m_idEdges = m_idRobot = m_idBox = m_idPt = -1;

      newColor = false;
      m_edgeThickness = 1;
    }
    
    virtual ~CCModelBase(){
  
      glDeleteLists(m_idEdges,1);
      glDeleteLists(m_idRobot,1);
      glDeleteLists(m_idBox,1);
      glDeleteLists(m_idPt,1);
    }

    void 
    ReBuildAll(){
        
      glDeleteLists(m_idEdges,1);
      glDeleteLists(m_idRobot,1);
      glDeleteLists(m_idBox,1);
      glDeleteLists(m_idPt,1);
      m_idEdges=-1; 
      m_idRobot=-1; 
      m_idBox=-1;   
      m_idPt=-1;      
    }

    void 
    ScaleNode(float _scale, Shape _s){
        
      m_nodeShape = _s; 

      if(m_nodeShape == Point && m_pointScale != _scale){
        m_pointScale = _scale*10;
        glDeleteLists(m_idPt, 1); 
        m_idPt = -1; 
      }
        
      else if(m_nodeShape == Box && m_boxScale != _scale){
        m_boxScale = _scale;
        glDeleteLists(m_idBox, 1); 
        m_idBox = -1; 
      }
    }

    void
    ScaleEdges(size_t _scale){
      
      m_edgeThickness = _scale; 
      glDeleteLists(m_idEdges, 1); 
      m_idEdges = -1; 
    }

    void 
    ChangeShape(Shape _s){ 
      m_nodeShape= _s; 
    }
  };
  
  /////////////////////////////////////////////////////////////////
  // CCModel
  ////////////////////////////////////////////////////////////////

  template <class CfgModel, class WEIGHT> 
  class CCModel : public CCModelBase{
  
    public:
      typedef typename MapModel<CfgModel, WEIGHT>::Wg WG; 
      typedef typename WG::vertex_descriptor VID;
      typedef typename WG::edge_descriptor EID;
      typedef vector_property_map<WG, size_t> color_map_t;
        
      CCModel(unsigned int _ID);
      virtual ~CCModel();

      // GL interface functions
      bool BuildModels(); //not used, should not call this
      void Draw(GLenum _mode);
      void DrawSelect();
      void Select(unsigned int* _index, vector<gliObj>& _sel);
      bool BuildModels(VID _id, WG* _g); //call this instread
      const string GetName() const;
      virtual vector<string> GetInfo() const;
      void RobotModel(OBPRMView_Robot* _pRobot){m_pRobot = _pRobot;}

      // Access Functions
      int GetNumNodes(){ return m_nodes.size();}
      int GetNumEdges(){ return m_edges.size();}
      double GetNodeData() { return m_nodes[0]->tx();}
      // Functions to be accessed to get nodes and edges info.
      //to write a new *.map file (this functions are 
      //currently accessed from vizmo2.ccp: vizmo::GetNodeInfo()
      map<VID, CfgModel>& GetNodesInfo() {return m_nodes;}
      vector<WEIGHT>& GetEdgesInfo() {return m_edges;}
      WG* GetGraph(){return m_graph;}

      void BuildRobotNodes(GLenum _mode); 
      void BuildBoxNodes(GLenum _mode); 
      void BuildPointNodes(GLenum _mode); 
      void BuildEdges();   
      void SetColor(float _r, float _g, float _b, float _a); 
      void DrawRobotNodes(GLenum _mode);
      void DrawBoxNodes(GLenum _mode);
      void DrawPointNodes(GLenum _mode);

      OBPRMView_Robot* m_pRobot;
      WG* m_graph;
      color_map_t m_cmap;

        //add a new Edge (from the 'add edge' option) 
        //June 16-05
        void addEdge(CfgModel* _c1, CfgModel* _c2){
          typename WG::vertex_iterator vi;
          typename WG::adj_edge_iterator ei;
          EID ed(_c1->GetIndex(),_c2->GetIndex());
          m_graph->find_edge(ed, vi, ei);

          WEIGHT w  = (*ei).property();   
          w.Set(m_edges.size(),_c1,_c2);
          m_edges.push_back(w);
        }

        void change_properties(Shape _s, float _size, vector<float> _color, bool _isNew){ 
          
          m_renderMode = SOLID_MODE;
          m_nodeShape = _s;
          if(_s == Point){
            m_pointScale = _size; 
            glDeleteLists(m_idPt, 1); 
            m_idPt = -1; 
          }
          else{
            m_boxScale = _size;
            glDeleteLists(m_idBox,1);
            m_idBox = -1; //need new id
          }
          
          glDeleteLists(m_idEdges,1);
          m_idEdges = -1; 
          glDeleteLists(m_idPt,1);
          m_idPt = -1;       
          
          if(_isNew)
            SetColor(_color[0], _color[1], _color[2], 1); 
        }

        virtual void GetChildren(list<GLModel*>& _models){ 
          typedef typename map<VID, CfgModel>::iterator CIT;
          for(CIT cit=m_nodes.begin(); cit!=m_nodes.end(); cit++)
            _models.push_back(&cit->second);

          typedef typename vector<WEIGHT>::iterator EIT;
          for(EIT eit=m_edges.begin(); eit!=m_edges.end(); eit++)
            _models.push_back(&*eit); 
        }

        void BuildNodeModels(GLenum _mode);

        map<VID, CfgModel> m_nodes;
        vector<WEIGHT> m_edges;
    };


  /*********************************************************************
  *
  *      Implementation of CCModel
  *
  *********************************************************************/
  template <class CfgModel, class WEIGHT>
  CCModel<CfgModel, WEIGHT>::CCModel(unsigned int _ID) : CCModelBase(_ID){
    m_graph = NULL;
  }

  template <class CfgModel, class WEIGHT>
  CCModel<CfgModel, WEIGHT>::~CCModel() {
  }

  template <class CfgModel, class WEIGHT>
  bool 
  CCModel<CfgModel, WEIGHT>::BuildModels() {
    //do not call this function
    cerr  << "CCModel<CfgModel, WEIGHT>::BuildModels() : Do not call this function\n"
          << "call CCModel<CfgModel, WEIGHT>::BuildModel(VID id,WG* g)"
          << " instead" << endl;
    return false;
  }

  template <class CfgModel, class WEIGHT>
  bool 
  CCModel<CfgModel, WEIGHT>::BuildModels(VID _id, WG* _g){ 
    if(_g == NULL){
      cerr<<"Graph is null"<<endl;
      return false;
    }
    m_graph = _g;
    //Setup cc nodes        
    vector<VID> cc;
    m_cmap.reset();
    get_cc(*_g, m_cmap, _id, cc);
    
    int nSize=cc.size();   
    typename WG::vertex_iterator cvi, cvi2, vi;
    typename WG::adj_edge_iterator ei;
    m_nodes.clear();
    for(int iN=0; iN<nSize; iN++){
      VID nid=cc[iN];    
      CfgModel cfg = (_g->find_vertex(nid))->property();
      cfg.Set(nid,m_pRobot,this);
      m_nodes[nid] = cfg;
    }

    //Setup edges
    vector< pair<VID,VID> > ccedges;

    m_cmap.reset();
    get_cc_edges(*_g, m_cmap, ccedges, _id);
    int eSize=ccedges.size(), edgeIdx = 0;

    m_edges.clear();
    for(int iE=0; iE<eSize; iE++){
      if(ccedges[iE].first<ccedges[iE].second)
        continue;

      CfgModel* cfg1 = &((_g->find_vertex(ccedges[iE].first))->property());
      cfg1->SetIndex(ccedges[iE].first); 
      CfgModel* cfg2 = &((_g->find_vertex(ccedges[iE].second))->property());
      cfg2->SetIndex(ccedges[iE].second); 
      EID ed(ccedges[iE].first,ccedges[iE].second);
      _g->find_edge(ed, vi, ei);
      WEIGHT w  = (*ei).property(); 
      w.Set(edgeIdx++,cfg1,cfg2, m_pRobot);
      m_edges.push_back(w);
    }
    return true;
  }

  template <class CfgModel, class WEIGHT>
  void
  CCModel<CfgModel, WEIGHT>::BuildRobotNodes(GLenum _mode){

    glDeleteLists(m_idRobot, 1); 
    m_idRobot = glGenLists(1); 
    glNewList(m_idRobot, GL_COMPILE); 
    DrawRobotNodes(_mode);
    glEndList(); 
  }

  template <class CfgModel, class WEIGHT>
  void
  CCModel<CfgModel, WEIGHT>::BuildBoxNodes(GLenum _mode){

    glDeleteLists(m_idBox, 1); 
    m_idBox = glGenLists(1); 
    glNewList(m_idBox, GL_COMPILE); 
    DrawBoxNodes(_mode); 
    glEndList(); 
  }

  template <class CfgModel, class WEIGHT>
  void
  CCModel<CfgModel, WEIGHT>::BuildPointNodes(GLenum _mode){

    glDeleteLists(m_idPt, 1); 
    m_idPt = glGenLists(1); 
    glNewList(m_idPt, GL_COMPILE); 
    DrawPointNodes(_mode); 
    glEndList(); 
  }

  template <class CfgModel, class WEIGHT>
  void
  CCModel<CfgModel, WEIGHT>::SetColor(float _r, float _g, float _b, float _a){
    
    ReBuildAll();

    m_RGBA.clear(); 
    m_RGBA.push_back(_r);
    m_RGBA.push_back(_g);
    m_RGBA.push_back(_b);

    typedef typename map<VID, CfgModel>::iterator CIT;
    for(CIT cit=m_nodes.begin(); cit!=m_nodes.end(); cit++){
      cit->second.m_RGBA.clear(); 
      cit->second.m_RGBA.push_back(_r);
      cit->second.m_RGBA.push_back(_g);
      cit->second.m_RGBA.push_back(_b);
    }

    typedef typename vector<WEIGHT>::iterator EIT;
    for(EIT eit=m_edges.begin(); eit!=m_edges.end(); eit++){
      eit->m_RGBA.clear(); 
      eit->m_RGBA.push_back(_r);
      eit->m_RGBA.push_back(_g);
      eit->m_RGBA.push_back(_b);
    }
  }

  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::DrawRobotNodes(GLenum _mode){
  
    if(m_pRobot == NULL)
      return;
    
    vector<float> origColor = m_pRobot->GetColor();
    m_pRobot->BackUp();
    
    if(_mode == GL_RENDER)  
      glEnable(GL_LIGHTING);
    
    typedef typename map<VID, CfgModel>::iterator CIT;
    for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++){      
      glPushName(cit->first);
      cit->second.Scale(m_robotScale,m_robotScale,m_robotScale);
      cit->second.SetShape(CfgModel::Robot); 
      m_pRobot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],1);
      cit->second.Draw(_mode);//draw robot; 
      glPopName();
    }

    m_pRobot->Restore();
    m_pRobot->SetColor(origColor[0],origColor[1],origColor[2],origColor[3]);
  }

  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::DrawBoxNodes(GLenum _mode){
    
    glEnable(GL_LIGHTING);
    typedef typename map<VID, CfgModel>::iterator CIT;
    
    for(CIT cit = m_nodes.begin(); cit!=m_nodes.end(); cit++){      
      glPushName(cit->first);
      cit->second.Scale(m_boxScale,m_boxScale,m_boxScale);
      cit->second.SetShape(CfgModel::Box);
      cit->second.Draw(_mode); 
      glPopName();
    }
  }

  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::DrawPointNodes(GLenum _mode){ 
    
    glDisable(GL_LIGHTING);
    glPointSize(m_pointScale); 
    typedef typename map<VID, CfgModel>::iterator CIT;
    for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++){      
      glPushName(cit->first);
      cit->second.Scale(m_pointScale, m_pointScale, m_pointScale); 
      cit->second.SetShape(CfgModel::Point);
      cit->second.Draw(_mode); 
      glPopName();
    }
    glEndList();
  }

  //Changing edge thickness: step 4
  //This function sets the thickness member of the edge itself
  //and then calls Edge's Draw
  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::BuildEdges(){
    //build edges
    m_idEdges = glGenLists(1);
    glNewList(m_idEdges, GL_COMPILE);
    glDisable(GL_LIGHTING);
      typedef typename vector<WEIGHT>::iterator EIT;
      for(EIT eit = m_edges.begin(); eit!=m_edges.end(); eit++){
        eit->SetThickness(m_edgeThickness); 
        eit->SetCfgShape(m_shape);
        eit->Draw(m_renderMode);
      }
    glEndList();
  }

  template <class CfgModel, class WEIGHT>
  void CCModel<CfgModel, WEIGHT>::Draw(GLenum _mode) {
    if(m_renderMode == INVISIBLE_MODE)
      return;
    if(_mode==GL_SELECT && !m_enableSelection)
      return;

    int list = -1;  

    switch(m_nodeShape){ 
      case Robot:
        glDeleteLists(m_idRobot, 1); 
        m_idRobot = -1; 
        BuildRobotNodes(_mode); 
        list = m_idRobot; 
        m_shape = 'r'; 
      break;

      case Box:
        glDeleteLists(m_idBox, 1); 
        m_idBox = -1; 
        BuildBoxNodes(_mode); 
        list = m_idBox; 
        m_shape='b';
      break;

      case Point:
        glDeleteLists(m_idPt, 1); 
        m_idPt = -1; 
        BuildPointNodes(_mode); 
        list = m_idPt; 
        m_shape = 'p';
      break;
    }
    glDeleteLists(m_idEdges, 1); 
    m_idEdges = -1; 

    if(_mode == GL_SELECT)
      glPushName(1); //1 means nodes

    glCallList(list);

    if(_mode == GL_SELECT)
      glPopName();

    // Draw edge
    if(m_idEdges == -1)  
      BuildEdges();
    
    glLineWidth(1);

    if(_mode == GL_SELECT)
      glPushName(2); //2 means edge
    glCallList(m_idEdges);
    if(_mode == GL_SELECT)
      glPopName();
  }


  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::DrawSelect(){
    if(m_idEdges==-1) 
      BuildEdges();
    glColor3f(1,1,0);
    glLineWidth(3);
    glCallList(m_idEdges);
    glLineWidth(1);
  }

  template <class CfgModel, class WEIGHT>
  void 
  CCModel<CfgModel, WEIGHT>::Select(unsigned int* _index, vector<gliObj>& _sel){ 
    typename WG::vertex_iterator cvi;
    if(_index==NULL || m_graph==NULL)
      return;
    if(_index[0]==1){
      _sel.push_back(&m_nodes[(VID)_index[1]]);
    }
    else 
      _sel.push_back(&m_edges[_index[1]]);
  }

  template <class CfgModel, class WEIGHT>
  const string 
  CCModel<CfgModel, WEIGHT>::GetName() const{ 
    ostringstream temp;
    temp<<"CC"<<m_id;
    return temp.str(); 
  }

  template <class CfgModel, class WEIGHT>
  vector<string> 
  CCModel<CfgModel, WEIGHT>::GetInfo() const{ 

    vector<string> info; 
    ostringstream temp,temp2;
    temp << "There are " << m_nodes.size() << " nodes";
    info.push_back(temp.str());
    temp2 << "There are " << m_edges.size() << " edges";
    info.push_back(temp2.str());
    return info;
  }
}//end of namespace plum

#endif //_PLUM_CCMODEL_H_


