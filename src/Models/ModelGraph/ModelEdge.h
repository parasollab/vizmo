// DeformEdge.h: interface for the CModelEdge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFORMEDGE_H__F6B30E07_72A7_484C_B755_0CB8345B90B7__INCLUDED_)
#define AFX_DEFORMEDGE_H__F6B30E07_72A7_484C_B755_0CB8345B90B7__INCLUDED_

#include <stdlib.h>

namespace modelgraph
{
    class CModelEdge
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructors
        CModelEdge(int start, int end);

        //////////////////////////////////////////////////////////////////////
        // Access Function
        bool isEndPt( int key ) const{
            for( int i=0;i<2;i++)
                if( key==m_key[i] ) return true;
            return false;
        }
        int getStartPt()  const { return m_key[0]; }
        int getEndPt()    const { return m_key[1]; }
        int getLeftTri()  const { return m_tri[0]; }
        int getRightTri() const { return m_tri[1]; }

        //List Access
        CModelEdge * getNext() const { return m_next; }
        void setNext(CModelEdge * e) { m_next=e; }

        //set left and right id
        void setLeftTri(int id)  { m_tri[0]=id; }
        void setRightTri(int id) { m_tri[1]=id; }

    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    private:
        //////////////////////////////////////////////////////////////////
        int m_key[2]; ///< 0->start, 1->end
        int m_tri[2]; ///< 0->left, 1->right

        //list link
        CModelEdge * m_next;
    };

}//end modelgraph

#endif // !defined(AFX_DEFORMEDGE_H__F6B30E07_72A7_484C_B755_0CB8345B90B7__INCLUDED_)
