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
        m_Key[0]=start;
        m_Key[1]=end;
        m_Tri[0]=m_Tri[1]=-1;
    }

} //end of namespace modelgraph
