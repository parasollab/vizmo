// ModelGraph.cpp: implementation of the CModelGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "ModelGraph.h"

namespace modelgraph
{

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CModelGraph::CModelGraph()
    {
        m_pEdge=NULL;
        m_EdgeSize=0;
    }

    CModelGraph::~CModelGraph()
    {
        //Free edges
        while( m_pEdge!=NULL ){
            CModelEdge * tmp=m_pEdge->getNext();
            delete m_pEdge;
            m_pEdge=tmp;
        }

        //Free Nodes
        int nsize=m_pNode.size();
        for( int iN=0;iN<nsize;iN++ )
            delete m_pNode[iN];
        m_pNode.clear();
    }

}//end of namespace modelgraph
