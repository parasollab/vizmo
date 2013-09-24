#ifndef CCMODEL_H_
#define CCMODEL_H_

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

#include <graph.h>
#include <algorithms/connected_components.h>
using namespace stapl;

#include "CfgModel.h"
#include "EdgeModel.h"
#include "MapModel.h"
#include "RobotModel.h"
#include "Plum/GLModel.h"
#include "Utilities/Exceptions.h"

template<typename, typename>
class MapModel;

class CfgModel;

template <class CfgModel, class WEIGHT>
class CCModel : public GLModel{

  public:
    typedef typename MapModel<CfgModel, WEIGHT>::Wg WG;
    typedef typename WG::vertex_descriptor VID;
    typedef typename WG::edge_descriptor EID;
    typedef vector_property_map<WG, size_t> ColorMap;

    CCModel(unsigned int _id);
    ~CCModel();

    const string GetName() const;
    int GetID() const {return m_id;}
    int GetNumNodes(){ return m_nodes.size(); }
    int GetNumEdges(){ return m_edges.size(); }
    double GetNodeData() { return m_nodes[0]->tx(); }
    // Functions to be accessed to get nodes and edges info.
    //to write a new *.map file (this functions are
    //currently accessed from vizmo2.ccp: vizmo::GetNodeInfo()
    map<VID, CfgModel>& GetNodesInfo() { return m_nodes; }
    vector<WEIGHT>& GetEdgesInfo() { return m_edges; }
    WG* GetGraph(){ return m_graph; }
    void SetRobotModel(RobotModel* _robot){ m_robot = _robot; }
    void SetColorChanged(bool _isNew) { m_newColor = _isNew; }

    void BuildModels(); //not used, should not call this
    void Draw(GLenum _mode);
    void DrawSelect();
    void Select(unsigned int* _index, vector<GLModel*>& _sel);
    void BuildModels(VID _id, WG* _g); //call this instead
    virtual vector<string> GetInfo() const;

    void BuildNodeModels(GLenum _mode);
    void DrawNodes(GLenum _mode);
    void DrawEdges();
    void SetColor(const Color4& _c);
    void AddEdge(CfgModel* _c1, CfgModel* _c2);
    //void ChangeProperties(Shape _s, float _size, vector<float> _color, bool _isNew);
    virtual void GetChildren(list<GLModel*>& _models);

  private:
    int m_id; //CC ID
    bool m_newColor; //Have CC colors been changed?
    RobotModel* m_robot;
    WG* m_graph;
    ColorMap m_colorMap;
    map<VID, CfgModel> m_nodes;
    vector<WEIGHT> m_edges;
};

template <class CfgModel, class WEIGHT>
CCModel<CfgModel, WEIGHT>::CCModel(unsigned int _id){
  m_id = _id;
  m_renderMode = INVISIBLE_MODE;
  //Set random Color
  GLModel::SetColor(Color4(drand48(), drand48(), drand48(), 1));
  m_newColor = false;
  m_graph = NULL;
  m_robot = NULL;
}

template <class CfgModel, class WEIGHT>
CCModel<CfgModel, WEIGHT>::~CCModel(){}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::BuildModels() {
  cerr << "Error::CCModel.h::Calling wrong build models function." << endl;
  exit(1);
}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::BuildModels(VID _id, WG* _g){

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
    VID nid=cc[i];
    CfgModel cfg = (_g->find_vertex(nid))->property();
    cfg.Set(nid, m_robot,this);
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

    CfgModel* cfg1 = &((_g->find_vertex(ccedges[iE].first))->property());
    cfg1->SetIndex(ccedges[iE].first);
    CfgModel* cfg2 = &((_g->find_vertex(ccedges[iE].second))->property());
    cfg2->SetIndex(ccedges[iE].second);
    EID ed(ccedges[iE].first,ccedges[iE].second);
    _g->find_edge(ed, vi, ei);
    WEIGHT w  = (*ei).property();
    w.Set(edgeIdx++,cfg1,cfg2, m_robot);
    m_edges.push_back(w);
  }
}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::DrawNodes(GLenum _mode){

  switch(CfgModel::GetShape()){

    case CfgModel::Robot:
      if(m_robot == NULL)
        return;
      if(_mode == GL_RENDER)
        glEnable(GL_LIGHTING);
      glLineWidth(1);
    break;

    case CfgModel::Box:
      glEnable(GL_LIGHTING);
      glLineWidth(1);
    break;

    case CfgModel::Point:
      glDisable(GL_LIGHTING);
      glPointSize(CfgModel::GetPointSize());
    break;
  }

  typedef typename map<VID, CfgModel>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++){
    glPushName(cit->first);
    cit->second.Draw(_mode);
    glPopName();
  }
}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::SetColor(const Color4& _c){

  GLModel::SetColor(_c);

  typedef typename map<VID, CfgModel>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++)
    cit->second.SetColor(_c);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); eit++)
    eit->SetColor(_c);
}

//add a new Edge (from the 'add edge' option)
//June 16-05
template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::AddEdge(CfgModel* _c1, CfgModel* _c2){

  typename WG::vertex_iterator vi;
  typename WG::adj_edge_iterator ei;
  EID ed(_c1->GetIndex(),_c2->GetIndex());
  m_graph->find_edge(ed, vi, ei);

  WEIGHT w  = (*ei).property();
  w.Set(m_edges.size(),_c1,_c2);
  m_edges.push_back(w);
}

//Doesn't seem to be used anywhere...
/*template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::ChangeProperties(Shape _s, float _size, vector<float> _color, bool _isNew){

  m_renderMode = SOLID_MODE;
  m_cfgShape = _s;

  if(_s == CfgModel::Point)
    m_pointScale = _size;
  else
    m_boxScale = _size;

  if(_isNew)
    SetColor(_color[0], _color[1], _color[2], 1);
}
*/

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::GetChildren(list<GLModel*>& _models){

  typedef typename map<VID, CfgModel>::iterator CIT;
  for(CIT cit = m_nodes.begin(); cit != m_nodes.end(); cit++)
    _models.push_back(&cit->second);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit != m_edges.end(); eit++)
    _models.push_back(&*eit);
}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::DrawEdges(){

  glDisable(GL_LIGHTING);
  //Worth performance cost to antialias and prevent automatic
  //conversion to integer?
  glLineWidth(EdgeModel::m_edgeThickness);

  typedef typename vector<WEIGHT>::iterator EIT;
  for(EIT eit = m_edges.begin(); eit!=m_edges.end(); eit++){
    eit->SetCfgShape(CfgModel::GetShape());
    eit->Draw(m_renderMode);
  }
}

template <class CfgModel, class WEIGHT>
void CCModel<CfgModel, WEIGHT>::Draw(GLenum _mode) {

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

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::DrawSelect(){

  /*Disabled for now; later modifications likely*/
  //if(m_edgeID == -1)
  //  DrawEdges();

  //glLineWidth(3);
  //glCallList(m_edgeID);
  //glLineWidth(1);
}

template <class CfgModel, class WEIGHT>
void
CCModel<CfgModel, WEIGHT>::Select(unsigned int* _index, vector<GLModel*>& _sel){

  typename WG::vertex_iterator cvi;
  if(_index == NULL || m_graph == NULL)
    return;

  if(_index[0] == 1)
    _sel.push_back(&m_nodes[(VID)_index[1]]);
  else
    _sel.push_back(&m_edges[_index[1]]);
}

template <class CfgModel, class WEIGHT>
const string
CCModel<CfgModel, WEIGHT>::GetName() const{

  ostringstream temp;
  temp << "CC" << m_id;
  return temp.str();
}

template <class CfgModel, class WEIGHT>
vector<string>
CCModel<CfgModel, WEIGHT>::GetInfo() const{

  vector<string> info;
  ostringstream temp, temp2;
  temp << "There are " << m_nodes.size() << " nodes";
  info.push_back(temp.str());
  temp2 << "There are " << m_edges.size() << " edges";
  info.push_back(temp2.str());
  return info;
}

#endif //CCMODEL_H_


