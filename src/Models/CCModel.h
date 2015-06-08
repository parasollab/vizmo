#ifndef CC_MODEL_H_
#define CC_MODEL_H_

#include <iostream>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

#include "Model.h"
#include "MapModel.h"

template<typename, typename>
class MapModel;

////////////////////////////////////////////////////////////////////////////////
/// \brief Provides a connected component model that is used to ensure all
///        members of a CC are rendered with the same color.
////////////////////////////////////////////////////////////////////////////////
template <class CFG, class WEIGHT>
class CCModel : public Model {

  public:

    // Local types
    typedef MapModel<CFG, WEIGHT> MM;
    typedef typename MM::Graph Graph;
    typedef typename MM::VID VID;
    typedef typename MM::VI VI;
    typedef typename MM::EID EID;
    typedef typename MM::EI EI;
    typedef typename MM::ColorMap ColorMap;

    // Construction
    CCModel(size_t _id, VID _rep, Graph* _graph);

    void SetName();                     ///< Set the name of this CC.
    size_t GetID() const {return m_id;} ///< Get the ID of this CC.

    // Model functions
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;
    void SetColor(const Color4& _c);
    virtual void GetChildren(list<Model*>& _models);

  private:

    CFG& GetCfg(VID _v);

    size_t m_id;         ///< The ID of this CC.
    VID m_rep;           ///< The ID of a reference node in this CC.
    Graph* m_graph;      ///< Pointer to the graph.
    ColorMap m_colorMap; ///< Auxiliary structure for stapl graph.
    vector<VID> m_nodes; ///< A list of the VIDs in this CC.

    static map<VID, Color4> m_colorIndex; ///< Cfg colors by VID.
};


template <class CFG, class WEIGHT>
map<typename CCModel<CFG, WEIGHT>::VID, Color4>
CCModel<CFG, WEIGHT>::
m_colorIndex = map<VID, Color4>();


template <class CFG, class WEIGHT>
CCModel<CFG, WEIGHT>::
CCModel(size_t _id, VID _rep, Graph* _graph) : Model(""), m_id(_id), m_rep(_rep),
    m_graph(_graph) {
  SetName();
  m_renderMode = INVISIBLE_MODE;

  Build();
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
Build() {
  //Set up CC nodes
  m_nodes.clear();
  m_colorMap.reset();
  get_cc(*m_graph, m_colorMap, m_rep, m_nodes);

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    GetCfg(*vit).Set(*vit, this);

  //Set up edges
  int edgeIdx = 0;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    VI v = m_graph->find_vertex(*vit);
    for(EI ei = v->begin(); ei != v->end(); ++ei) {
      if((*ei).source() > (*ei).target())
        continue;

      CFG* cfg2 = &GetCfg((*ei).target());
      WEIGHT& edge = (*ei).property();
      edge.Set(edgeIdx++, &v->property(), cfg2);
    }
  }

  //If user changes a CC's color, color at associated index is changed
  if(m_colorIndex.find(m_rep) == m_colorIndex.end())
    m_colorIndex[m_rep] = Color4(drand48(), drand48(), drand48(), 1);
  SetColor(m_colorIndex[m_rep]);
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
SetColor(const Color4& _c) {
  Model::SetColor(_c);
  m_colorIndex[m_rep] = _c;

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    GetCfg(*vit).SetColor(_c);

  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    VI v = m_graph->find_vertex(*vit);
    for(EI ei = v->begin(); ei != v->end(); ++ei) {
      if((*ei).source() > (*ei).target())
        continue;
      (*ei).property().SetColor(_c);
    }
  }
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
GetChildren(list<Model*>& _models) {
  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
    _models.push_back(&GetCfg(*vit));

  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    VI v = m_graph->find_vertex(*vit);
    for(EI ei = v->begin(); ei != v->end(); ++ei) {
      if((*ei).source() > (*ei).target())
        continue;
      _models.push_back(&(*ei).property());
    }
  }
}


template <class CFG, class WEIGHT>
void CCModel<CFG, WEIGHT>::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  glColor4fv(GetColor());

  switch(CFG::GetShape()){
    case CFG::Robot:
      glEnable(GL_LIGHTING);
      glLineWidth(1);
      typedef typename vector<VID>::iterator VIT;
      for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
        GetCfg(*vit).DrawRender();
      break;

    case CFG::Point:
      glDisable(GL_LIGHTING);
      glPointSize(CFG::GetPointSize());
      glBegin(GL_POINTS);
      typedef typename vector<VID>::iterator VIT;
      for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit)
        glVertex3dv(GetCfg(*vit).GetPoint());
      glEnd();
      break;
  }

  //draw edges
  glDisable(GL_LIGHTING);
  glLineWidth(WEIGHT::m_edgeThickness);

  glBegin(GL_LINES);
  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    VI v = m_graph->find_vertex(*vit);
    for(EI ei = v->begin(); ei != v->end(); ++ei) {
      if((*ei).source() > (*ei).target())
        continue;

      CFG* cfg2 = &GetCfg((*ei).target());
      WEIGHT& edge = (*ei).property();
      edge.Set(&v->property(), cfg2);
      edge.DrawRenderInCC();
    }
  }
  glEnd();
}


template <class CFG, class WEIGHT>
void CCModel<CFG, WEIGHT>::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //Names: 1 = Nodes, 2 = Edges

  //draw nodes
  glPushName(1);
  switch(CFG::GetShape()){
    case CFG::Robot:
      glLineWidth(1);
      break;

    case CFG::Point:
      glPointSize(CFG::GetPointSize());
      break;
  }

  typedef typename vector<VID>::iterator VIT;
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    glPushName(*vit);
    GetCfg(*vit).DrawSelect();
    glPopName();
  }
  glPopName();

  //draw edges
  glPushName(2);
  glLineWidth(WEIGHT::m_edgeThickness);
  for(VIT vit = m_nodes.begin(); vit != m_nodes.end(); ++vit) {
    glPushName(*vit);
    VI v = m_graph->find_vertex(*vit);
    for(EI ei = v->begin(); ei != v->end(); ++ei) {
      if((*ei).source() > (*ei).target())
        continue;
      glPushName((*ei).target());
      CFG* cfg2 = &GetCfg((*ei).target());
      WEIGHT& edge = (*ei).property();
      edge.Set(&v->property(), cfg2);
      edge.DrawSelect();
      glPopName();
    }
    glPopName();
  }
  glPopName();
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
DrawSelected(){
  /// \warning Disabled for now; later modifications likely.
  //if(m_edgeID == -1)
  //  DrawEdges();

  //glLineWidth(3);
  //glCallList(m_edgeID);
  //glLineWidth(1);
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(!m_selectable || _index == NULL)
    return;

  if(_index[0] == 1)
    _sel.push_back(&GetCfg(_index[1]));
  else {
    VI vi;
    EI ei;
    m_graph->find_edge(EID(_index[1], _index[2]), vi, ei);
    _sel.push_back(&(*ei).property());
  }
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
SetName() {
  ostringstream temp;
  temp << "CC " << m_id;
  m_name = temp.str();
}


template <class CFG, class WEIGHT>
void
CCModel<CFG, WEIGHT>::
Print(ostream& _os) const {
  /// \todo Add in num edges again
  _os << Name() << endl
      << m_nodes.size() << " nodes" << endl;
}


template<class CFG, class WEIGHT>
CFG&
CCModel<CFG, WEIGHT>::
GetCfg(VID _v) {
  return m_graph->find_vertex(_v)->property();
}

#endif
