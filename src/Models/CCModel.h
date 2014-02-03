#ifndef CCMODEL_H_
#define CCMODEL_H_

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include "Model.h"
#include "MapModel.h"

template<typename, typename>
class MapModel;

template <class CFG, class WEIGHT>
class CCModel : public Model {

  public:
    typedef MapModel<CFG, WEIGHT> MM;
    typedef typename MM::Graph Graph;
    typedef typename MM::VID VID;
    typedef typename MM::VI VI;
    typedef typename MM::EID EID;
    typedef typename MM::EI EI;
    typedef typename MM::ColorMap ColorMap;

    CCModel(size_t _id, VID _rep, Graph* _graph);

    void SetName();
    size_t GetID() const {return m_id;}

    void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw();
    void DrawSelect();
    void Print(ostream& _os) const;
    void DrawNodes();
    void DrawEdges();
    void SetColor(const Color4& _c);
    virtual void GetChildren(list<Model*>& _models);

  private:
    CFG& GetCfg(VID _v);
    WEIGHT& GetEdge(EID _e);

    size_t m_id;
    VID m_rep;
    Graph* m_graph;
    ColorMap m_colorMap;
    vector<VID> m_nodes;
    vector<EID> m_edges;

    static map<VID, Color4> m_colorIndex;
};

template <class CFG, class WEIGHT>
map<typename CCModel<CFG, WEIGHT>::VID, Color4> CCModel<CFG, WEIGHT>::m_colorIndex = map<VID, Color4>();

template <class CFG, class WEIGHT>
CCModel<CFG, WEIGHT>::CCModel(size_t _id, VID _rep, Graph* _graph) :
  Model(""), m_id(_id), m_rep(_rep), m_graph(_graph) {
    SetName();
    m_renderMode = INVISIBLE_MODE;

    BuildModels();
  }

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::BuildModels() {

  //Set up CC nodes
  m_nodes.clear();
  m_edges.clear();
  m_colorMap.reset();
  get_cc(*m_graph, m_colorMap, m_rep, m_nodes);

  VI vi;
  EI ei;

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    GetCfg(*vit).Set(*vit, this);

  //Set up edges
  vector<pair<VID, VID> > ccedges;

  m_colorMap.reset();
  get_cc_edges(*m_graph, m_colorMap, ccedges, m_rep);
  int edgeIdx = 0;

  typedef typename vector<pair<VID, VID> >::iterator EIT;
  for(EIT eit = ccedges.begin(); eit != ccedges.end(); ++eit) {
    if(eit->first < eit->second)
      continue;

    CFG* cfg1 = &GetCfg(eit->first);
    CFG* cfg2 = &GetCfg(eit->second);
    EID ed(eit->first, eit->second);
    GetEdge(ed).Set(edgeIdx++, cfg1, cfg2);
    m_edges.push_back(ed);
  }

  //If user changes a CC's color, color at associated index is changed
  if(m_colorIndex.find(m_rep) == m_colorIndex.end())
    m_colorIndex[m_rep] = Color4(drand48(), drand48(), drand48(), 1);
  SetColor(m_colorIndex[m_rep]);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::DrawNodes(){
  switch(CFG::GetShape()){
    case CFG::Robot:
    case CFG::Box:
      glEnable(GL_LIGHTING);
      glLineWidth(1);
      break;

    case CFG::Point:
      glDisable(GL_LIGHTING);
      glPointSize(CFG::GetPointSize());
      break;
  }

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit){
    glPushName(*vit);
    GetCfg(*vit).Draw();
    glPopName();
  }
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::SetColor(const Color4& _c){
  Model::SetColor(_c);
  m_colorIndex[m_rep] = _c;

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    GetCfg(*vit).SetColor(_c);

  typedef typename vector<EID>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); ++eit)
    GetEdge(*eit).SetColor(_c);
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::GetChildren(list<Model*>& _models){
  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    _models.push_back(&GetCfg(*vit));

  typedef typename vector<EID>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); ++eit)
    _models.push_back(&GetEdge(*eit));
}

template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::DrawEdges(){
  glDisable(GL_LIGHTING);
  glLineWidth(WEIGHT::m_edgeThickness);

  typedef typename vector<EID>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit!=m_edges.end(); ++eit) {
    glPushName(eit-m_edges.begin());
    CFG* cfg1 = &GetCfg(eit->source());
    CFG* cfg2 = &GetCfg(eit->target());
    GetEdge(*eit).Set(distance(m_edges.begin(), eit), cfg1, cfg2);
    GetEdge(*eit).Draw();
    glPopName();
  }
}

template <class CFG, class WEIGHT>
void CCModel<CFG, WEIGHT>::Draw() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //Names: 1 = Nodes, 2 = Edges
  glPushName(1);
  DrawNodes();
  glPopName();

  glPushName(2);
  DrawEdges();
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
  if(!m_selectable || _index == NULL)
    return;

  if(_index[0] == 1)
    _sel.push_back(&GetCfg(_index[1]));
  else
    _sel.push_back(&GetEdge(m_edges[_index[1]]));
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

template<class CFG, class WEIGHT>
CFG&
CCModel<CFG, WEIGHT>::GetCfg(VID _v) {
  return m_graph->find_vertex(_v)->property();
}

template<class CFG, class WEIGHT>
WEIGHT&
CCModel<CFG, WEIGHT>::GetEdge(EID _e) {
  VI vi;
  EI ei;
  m_graph->find_edge(_e, vi, ei);
  return (*ei).property();
}

#endif
