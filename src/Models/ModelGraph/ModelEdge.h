#ifndef MODELEDGE_H_
#define MODELEDGE_H_

#include <stdlib.h>

namespace modelgraph {

  class ModelEdge {
    public:
      ModelEdge(int _start, int _end);

      bool IsEndPt(int _key) const {return _key == m_key[0] || _key == m_key[1];};

      int GetStartPt() const {return m_key[0];}
      int GetEndPt() const {return m_key[1];}
      int GetLeftTri() const {return m_tri[0];}
      int GetRightTri() const {return m_tri[1];}

      ModelEdge* GetNext() const {return m_next;}
      void SetNext(ModelEdge* _e) {m_next = _e;}

      void SetLeftTri(int _id)  {m_tri[0] = _id;}
      void SetRightTri(int _id) {m_tri[1] = _id;}

    private:
      //////////////////////////////////////////////////////////////////
      int m_key[2]; ///< 0->start, 1->end
      int m_tri[2]; ///< 0->left, 1->right

      ModelEdge* m_next;
  };

}

#endif
