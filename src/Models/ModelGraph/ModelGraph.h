// ModelGraph.h: interface for the CModelGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ModelGraph_H__FF303688_9EB8_4844_B0F7_0C4AEEBAE826__INCLUDED_)
#define AFX_ModelGraph_H__FF303688_9EB8_4844_B0F7_0C4AEEBAE826__INCLUDED_

#include "ModelNode.h"
#include "ModelEdge.h"

#include <vector>
using namespace std;

#include <Vector.h>
using namespace mathtool;

namespace modelgraph
{

    class CModelGraph
    {
    public:

        typedef vector<Point3d> PtVector;
        typedef Vector<int, 3>     Tri;
        typedef vector<Tri>     TriVector;

        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructors
        CModelGraph();
        ~CModelGraph();

        //////////////////////////////////////////////////////////////////////
        // Core Function
        //convert from tri to graph
        bool doInit(const PtVector& pts,const TriVector& tri)
        {
            CModelEdge * tail=NULL; //tail of m_pEdge

            //create nodes
            int psize=pts.size();
            m_pNode.reserve(psize);
            for( int iP=0;iP<psize;iP++ )
                m_pNode.push_back(new CModelNode(iP));

            //create edge from triangles
            int trisize=tri.size();
            for( int iT=0;iT<trisize; iT++ ){
                for( int iE=0;iE<3;iE++ ){ //for each edge
                    int a=tri[iT][iE]; int b=tri[iT][(iE+1)%3]; //int c=tri[iT][(iE+2)%3];
                    CModelEdge * e=m_pNode[a]->getEdge(m_pNode[b]);
                    if( e!=NULL ){ //edge a->b is in a
                        e->setRightTri(iT);
                    }
                    else{
                        if( (e=new CModelEdge(a,b))==NULL ) return false;
                        e->setLeftTri(iT);
                        m_pNode[a]->addEdge(e); m_pNode[b]->addEdge(e);
                        //push new edge to end of list
                        if(m_pEdge==NULL) m_pEdge=e;
                        else tail->setNext(e);
                        tail=e; m_EdgeSize++;
                    } //end of if
                }//end of for(iE)
            }//end of for(iT)
            tail->setNext(NULL);

            return true;
        }

        //////////////////////////////////////////////////////////////////////
        // Access Function
        int getEdgeSize() const { return m_EdgeSize; }
        CModelEdge * getEdges(){ return m_pEdge; }

    //////////////////////////////////////////////////////////////////////
    // Private stuff
    private:
        int m_EdgeSize;
        CModelEdge * m_pEdge;        //a list of edges
        vector<CModelNode*> m_pNode; //an array of nodes
    };

} //end of namespace modelgraph

#endif // !defined(AFX_ModelGraph_H__FF303688_9EB8_4844_B0F7_0C4AEEBAE826__INCLUDED_)
