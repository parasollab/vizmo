// DeformEdge.cpp: implementation of the CModelEdge class.
//
//////////////////////////////////////////////////////////////////////

#include "ModelEdge.h"

namespace modelgraph
{

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CModelEdge::CModelEdge(int start, int end)
    {
        m_key[0]=start;
        m_key[1]=end;
        m_tri[0]=m_tri[1]=-1;
        m_next = NULL;
    }

} //end of namespace modelgraph
