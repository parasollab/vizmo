#ifndef MODELNODE_H_
#define MODELNODE_H_

#include <cstdlib>

#include "ModelEdge.h"

namespace modelgraph {

  class ModelNode {
    public:
      ModelNode(int _key);

      int GetKey() const {return m_key;}

      ModelEdge* GetEdge(ModelNode* _n) const;

      void AddEdge(ModelEdge* _e);

    private:
      struct ListNode{
          ListNode();

          ModelEdge* m_edge;
          ListNode* m_next;
      };

      int m_key;
      ListNode* m_edges;
  };

}

#endif
