#include "ModelEdge.h"

namespace modelgraph {

  ModelEdge::ModelEdge(int _start, int _end) : m_next(NULL) {
    m_key[0] = _start;
    m_key[1] = _end;
    m_tri[0] = m_tri[1] = -1;
  }

}
