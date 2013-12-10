#ifndef CCMODEL_H_
#define CCMODEL_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

#include <include/Graph.h>
#include <include/GraphAlgo.h>

#include "Model.h"
#include "MapModel.h"
#include "RobotModel.h"
#include "Utilities/VizmoExceptions.h"

template<typename, typename>
class MapModel;

template <class CFG, class WEIGHT>
class CCModel : public Model {

  public:
    typedef MapModel<CFG, WEIGHT> MM;
    typedef typename MM::Wg WG;
    typedef typename WG::vertex_descriptor VID;
    typedef typename WG::edge_descriptor EID;
    typedef typename MM::ColorMap ColorMap;

    CCModel(unsigned int _id);
    ~CCModel();

    void SetName();
    int GetID() const {return m_id;}
    int GetNumNodes(){ return m_nodes.size(); }
    int GetNumEdges(){ return m_edges.size(); }
    double GetNodeData() { return m_nodes[0]->tx(); }
    // Functions to be accessed to get nodes and edges info.
    //to write a new *.map file (this functions are
    //currently accessed from vizmo2.ccp: vizmo::GetNodeInfo()
    map<VID, CFG>& GetNodesInfo() { return m_nodes; }
    vector<WEIGHT>& GetEdgesInfo() { return m_edges; }
    WG* GetGraph(){ return m_graph; }
    void SetRobotModel(RobotModel* _robot){ m_robot = _robot; }

    void BuildModels(); //not used, should not call this
    void BuildModels(VID _id, WG* _g); //call this instead
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw(GLenum _mode);
    void DrawSelect();
    void Print(ostream& _os) const;
    void BuildNodeModels(GLenum _mode);
    void DrawNodes(GLenum _mode);
    void DrawEdges();
    void SetColor(const Color4& _c);
    //void AddEdge(CFG* _c1, CFG* _c2);
    //void ChangeProperties(Shape _s, float _size, vector<float> _color, bool _isNew);
    virtual void GetChildren(list<Model*>& _models);

  private:
    int m_id; //CC ID
    RobotModel* m_robot;
    WG* m_graph;
    ColorMap m_colorMap;
    map<VID, CFG> m_nodes;
    vector<WEIGHT> m_edges;

    static map<VID, Color4> m_colorIndex;
};

template <class CFG, class WEIGHT>
map<typename CCModel<CFG, WEIGHT>::VID, Color4> CCModel<CFG, WEIGHT>::m_colorIndex = map<VID, Color4>();

template <class CFG, class WEIGHT>
CCModel<CFG, WEIGHT>::CCModel(unsigned int _id) : Model("") {
  m_id = _id;
  SetName();
  m_renderMode = INVISIBLE_MODE;
  m_graph = NULL;
  m_robot = NULL;
}

template <class CFG, class WEIGHT>
CCModel<CFG, WEIGHT>::~CCModel(){}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::BuildModels() {
  cerr << "Error::CCModel.h::Calling wrong build models function." << endl;
  exit(1);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::BuildModels(VID _id, WG* _g){

  if(!_g)
    throw BuildException(WHERE, "Passed in null graph");

  m_graph = _g;

  //Set up CC nodes
  vector<VID> cc;
  m_colorMap.reset();
  get_cc(*_g, m_colorMap, _id, cc);

  int nSize = cc.size();
  typename WG::vertex_iterator cvi, cvi2, vi;
  typename WG::adj_edge_iterator ei;
  m_nodes.clear();

  for(int i = 0; i < nSize; i++){
    VID nid = cc[i];
    CFG cfg = (_g->find_vertex(nid))->property();
    cfg.Set(nid, m_robot, this);
    m_nodes[nid] = cfg;
  }

  //Set up edges
  vector< pair<VID,VID> > ccedges;

  m_colorMap.reset();
  get_cc_edges(*_g, m_colorMap, ccedges, _id);
  int eSize=ccedges.size(), edgeIdx = 0;

  m_edges.clear();
  for(int iE=0; iE<eSize; iE++){
    if(ccedges[iE].first<ccedges[iE].second)
      continue;

    CFG* cfg1 = &((_g->find_vertex(ccedges[iE].first))->property());
    cfg1->SetIndex(ccedges[iE].first);
    CFG* cfg2 = &((_g->find_vertex(ccedges[iE].second))->property());
    cfg2->SetIndex(ccedges[iE].second);
    EID ed(ccedges[iE].first,ccedges[iE].second);
    _g->find_edge(ed, vi, ei);
    WEIGHT w  = (*ei).property();
    w.Set(edgeIdx++,cfg1,cfg2, m_robot);
    m_edges.push_back(w);
  }

  //If user changes a CC's color, color at associated index is changed
  VID key = _id;
  if(m_colorIndex.find(key) == m_colorIndex.end())
    m_colorIndex[key] = Color4(drand48(), drand48(), drand48(), 1);
  SetColor(m_colorIndex[key]);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::DrawNodes(GLenum _mode){

  switch(CFG::GetShape()){

    case CFG::Robot:
      if(m_robot == NULL)
        return;
      if(_mode == GL_RENDER)
        glEnable(GL_LIGHTING);
      glLineWidth(1);
    break;

    case CFG::Box:
      glEnable(GL_LIGHTING);
      glLineWidth(1);
    break;

    case CFG::Point:
      glDisable(GL_LIGHTING);
      glPointSize(CFG::GetPointSize());
    break;
  }

  typedef typename map<VID, CFG>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++){
    glPushName(cit->first);
    cit->second.Draw(_mode);
    glPopName();
  }
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::SetColor(const Color4& _c){

  m_colorIndex[m_id] = _c;
  Model::SetColor(_c);

  typedef typename map<VID, CFG>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++)
    cit->second.SetColor(_c);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); eit++)
    eit->SetColor(_c);
}

//add a new Edge (from the 'add edge' option)
//June 16-05

//Throws bad_alloc and seems to only apply to one cc anyway.
//Not using for new add edge utility, which will be in
//map and can apply to nodes from different CCs

/*template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::AddEdge(CFG* _c1, CFG* _c2){

  typename WG::vertex_iterator vi;
  typename WG::adj_edge_iterator ei;
  EID ed(_c1->GetIndex(),_c2->GetIndex());
  m_graph->find_edge(ed, vi, ei);

  WEIGHT w  = (*ei).property();
  w.Set(m_edges.size(),_c1,_c2);
  m_edges.push_back(w);
}
*/
//Doesn't seem to be used anywhere...
/*template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::ChangeProperties(Shape _s, float _size, vector<float> _color, bool _isNew){

  m_renderMode = SOLID_MODE;
  m_cfgShape = _s;

  if(_s == CFG::Point)
    m_pointScale = _size;
  else
    m_boxScale = _size;

  if(_isNew)
    SetColor(_color[0], _color[1], _color[2], 1);
}
*/

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::GetChildren(list<Model*>& _models){

  typedef typename map<VID, CFG>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++)
    _models.push_back(&cit->second);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); eit++)
    _models.push_back(&*eit);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::DrawEdges(){
  glDisable(GL_LIGHTING);
  //Worth performance cost to antialias and prevent automatic
  //conversion to integer?
  glLineWidth(WEIGHT::m_edgeThickness);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit!=m_edges.end(); eit++)
    eit->Draw(m_renderMode);
}

template <class CFG, class WEIGHT>
void CCModel<CFG, WEIGHT>::Draw(GLenum _mode) {

  if(m_renderMode == INVISIBLE_MODE)
    return;

  if(_mode == GL_SELECT)
    glPushName(1); //1 means nodes

  DrawNodes(_mode);

  if(_mode == GL_SELECT)
    glPopName();

  if(_mode == GL_SELECT)
    glPushName(2); //2 means edge

  DrawEdges();

  if(_mode == GL_SELECT)
    glPopName();
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::DrawSelect(){

  /*Disabled for now; later modifications likely*/
  //if(m_edgeID == -1)
  //  DrawEdges();

  //glLineWidth(3);
  //glCallList(m_edgeID);
  //glLineWidth(1);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::Select(GLuint* _index, vector<Model*>& _sel){

  typename WG::vertex_iterator cvi;
  if(_index == NULL || m_graph == NULL)
    return;

  if(_index[0] == 1)
    _sel.push_back(&m_nodes[(VID)_index[1]]);
  else
    _sel.push_back(&m_edges[_index[1]]);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::SetName() {
  ostringstream temp;
  temp << "CC " << m_id;
  m_name = temp.str();
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::Print(ostream& _os) const {
  _os << Name() << endl
    << m_nodes.size() << " nodes" << endl
    << m_edges.size() << " edges" << endl;
}

#endif //CCMODEL_H_


