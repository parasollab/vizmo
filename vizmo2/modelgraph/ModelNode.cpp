// ModelNode.cpp: implementation of the CModelNode class.
//
//////////////////////////////////////////////////////////////////////

#include "ModelNode.h"

namespace modelgraph
{

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CModelNode::CModelNode(int Key)
    {
        m_Key=Key;
        m_Edges=NULL;
    }

}//namespace modelgraph