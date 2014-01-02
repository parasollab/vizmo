#include "ModelGraph.h"

namespace modelgraph {

  ModelGraph::ModelGraph() : m_edgeSize(0), m_edges(NULL) {}

  ModelGraph::~ModelGraph() {
    //Free edges
    while(m_edges != NULL){
      ModelEdge* tmp = m_edges->GetNext();
      delete m_edges;
      m_edges = tmp;
    }

    //Free Nodes
    for(size_t i=0; i < m_nodes.size(); ++i)
      delete m_nodes[i];
  }

  //convert from tri to graph
  void
    ModelGraph::DoInit(const PtVector& _pts, const TriVector& _tris) {
      ModelEdge* tail = NULL;

      //create nodes
      m_nodes.reserve(_pts.size());
      for(size_t i = 0; i < _pts.size(); ++i)
        m_nodes.push_back(new ModelNode(i));

      //create edge from triangles
      for(size_t i = 0; i < _tris.size(); ++i) {
        for(size_t j = 0; j < 3; ++j) {
          int a = _tris[i][j];
          int b = _tris[i][(j+1)%3];
          //int c = _tris[i][(j+2)%3];
          ModelEdge* e = m_nodes[a]->GetEdge(m_nodes[b]);
          if(e !=NULL) //edge a->b is in a
            e->SetRightTri(i);
          else {
            e = new ModelEdge(a, b);
            e->SetLeftTri(i);
            m_nodes[a]->AddEdge(e);
            m_nodes[b]->AddEdge(e);
            //push new edge to end of list
            if(m_edges == NULL)
              m_edges = e;
            else if(tail)
              tail->SetNext(e);
            tail = e;
            m_edgeSize++;
          }
        }
      }

      if(tail)
        tail->SetNext(NULL);
    }

}
