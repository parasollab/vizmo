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
                if( key==m_Key[i] ) return true;
            return false;
        }
        int getStartPt()  const { return m_Key[0]; }
        int getEndPt()    const { return m_Key[1]; }
        int getLeftTri()  const { return m_Tri[0]; }
        int getRightTri() const { return m_Tri[1]; }

        //List Access
        CModelEdge * getNext() const { return m_Next; }
        void setNext(CModelEdge * e) { m_Next=e; }

        //set left and right id
        void setLeftTri(int id)  { m_Tri[0]=id; }
        void setRightTri(int id) { m_Tri[1]=id; }

    //////////////////////////////////////////////////////////////////////
    // Private Stuff
    private:
        //////////////////////////////////////////////////////////////////
        int m_Key[2]; ///< 0->start, 1->end
        int m_Tri[2]; ///< 0->left, 1->right

        //list link
        CModelEdge * m_Next;
    };

}//end modelgraph

#endif // !defined(AFX_DEFORMEDGE_H__F6B30E07_72A7_484C_B755_0CB8345B90B7__INCLUDED_)
