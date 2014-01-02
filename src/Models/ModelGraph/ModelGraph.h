#ifndef MODELGRAPH_H_
#define MODELGRAPH_H_

#include "ModelNode.h"
#include "ModelEdge.h"

#include <vector>
using namespace std;

#include <Vector.h>
using namespace mathtool;

namespace modelgraph {

  class ModelGraph {
    public:
      typedef vector<Point3d> PtVector;
      typedef Vector<int, 3> Tri;
      typedef vector<Tri> TriVector;

      ModelGraph();
      ~ModelGraph();

      size_t GetEdgeSize() const {return m_edgeSize;}
      ModelEdge* GetEdges() {return m_edges;}

      //convert from tri to graph
      void DoInit(const PtVector& _pts, const TriVector& _tris);

    private:
      size_t m_edgeSize;
      ModelEdge* m_edges;        //a list of edges
      vector<ModelNode*> m_nodes; //an array of nodes
  };

}

#endif
