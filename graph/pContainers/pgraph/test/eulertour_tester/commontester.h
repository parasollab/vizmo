/* common ancester class for pET_Tester and sET_Tester
 * which includes a method GenLineTree (which is for best case, that the tree is actually a line)
 * in parallel ET tester and sequential ET testers, they both work on the generated tree here.
 */
#include "common/BaseTester.h"
#include "../task.h"
#include "Graph.h"
#include "pGraph.h"
#include "pContainers/pgraph/pET.h"
#include <vector>

using namespace stapl;

typedef Graph<UG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task, Weight2> TREE;
typedef pGraph<PUG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task,
Weight2> PTREE;

bool euleredge_compare(const euleredge& e1, const euleredge& e2) {
  int k1 = e1.GetRank(); int k2 = e2.GetRank();
  return (k1>k2);
}

template <class TREE>
class ETTester : public BaseTester {

public:
  virtual ETTester* CreateCopy() {
    BaseTester::CreateCopy();
  }
 
  virtual void Test(TREE& g) {}
  virtual void Test(TREE& g) const {}


};
