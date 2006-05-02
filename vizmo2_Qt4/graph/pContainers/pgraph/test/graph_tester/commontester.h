/* Basic Tester class for pGraph,
 * inheritted from BaseTester 
 */
#include "common/BaseTester.h"
#include "../task.h"
#include "Graph.h"
#include "pGraph.h"

using namespace stapl;



typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> DGRAPH;
typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;
typedef pGraph<PUG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PUGRAPH;
typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PDGRAPH;


template <class GRAPH> //can be either pGraph or sGraph
class GraphTester : public BaseTester {

public:
  virtual GraphTester* CreateCopy() {
    BaseTester::CreateCopy();
  }

  /* the testing case is a graph very well distributed, with few cross edges */
  template <class WEIGHT>
  void GenEdge(vector<_StaplTriple<VID,VID,WEIGHT> >& edgevec, GRAPH& g, int deg) {
    typename GRAPH::VI vi;
    int gsize = g.size();
    //Notice: pGraph::begin() is actually local_begin()
    //so we can use "begin()" for either pGraph or sGraph
    for (vi=g.begin(); vi!=g.end(); ++vi) {
      if (vi->vid < gsize-deg) {
        for (int j=vi->vid+deg;  j>vi->vid; j--) {
	  _StaplTriple<VID,VID,WEIGHT> tmp(vi->vid, j, vi->vid*j); 
	  edgevec.push_back(tmp);
        }
      }
    }
  }

  virtual void Test(GRAPH& g) { } 
  virtual void Test(GRAPH& g) const { }
};


