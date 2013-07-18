// ModelNode.h: interface for the CModelNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ModelNode_H__EDFCE7A1_C33A_444C_8D15_5F5C9AE1B31A__INCLUDED_)
#define AFX_ModelNode_H__EDFCE7A1_C33A_444C_8D15_5F5C9AE1B31A__INCLUDED_

#include <stdlib.h>

//////////////////////////////////////////////////////////////////////
// Include ModelGraph Headers.
#include "ModelEdge.h"
class CParticle;

namespace modelgraph
{

    class CModelNode
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructors
        CModelNode(int Key);

        //////////////////////////////////////////////////////////////////////
        // Access Function
        CModelEdge * getEdge(CModelNode * nb) const
        {
            int Key=nb->m_Key;
            //linear search
            mylist * e= m_Edges;
            while(e!=NULL){
                if( e->pEdge->isEndPt(Key)==true ) return e->pEdge;
                e=e->pNext;
            }
            return NULL;
        }

        void addEdge( CModelEdge * e){
            if(e==NULL) return;
            mylist * listnode=new mylist();
            if( listnode!=NULL ){
                listnode->pEdge=e;
                listnode->pNext=m_Edges;
                m_Edges=listnode;
            }
        }

        int getKey() const { return m_Key; }

    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    private:
        class mylist{
        public:
            mylist(){ pEdge=NULL; pNext=NULL; }
            CModelEdge * pEdge;
            mylist * pNext;
        };

        int m_Key;
        mylist * m_Edges;
    };

}//end of namespace modelgraph

#endif // !defined(AFX_ModelNode_H__EDFCE7A1_C33A_444C_8D15_5F5C9AE1B31A__INCLUDED_)
