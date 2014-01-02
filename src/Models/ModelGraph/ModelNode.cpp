#include "ModelNode.h"

namespace modelgraph {

  ModelNode::ModelNode(int _key) : m_key(_key), m_edges(NULL) {}

  ModelEdge*
    ModelNode::GetEdge(ModelNode* _n) const {
      int key = _n->m_key;
      ListNode* e = m_edges;
      while(e) {
        if(e->m_edge->IsEndPt(key))
          return e->m_edge;
        e = e->m_next;
      }
      return NULL;
    }

  void
    ModelNode::AddEdge(ModelEdge* _e) {
      if(!_e)
        return;

      ListNode* listnode = new ListNode();
      listnode->m_edge = _e;
      listnode->m_next = m_edges;
      m_edges = listnode;
    }

  ModelNode::ListNode::ListNode() : m_edge(NULL), m_next(NULL) {}

}
