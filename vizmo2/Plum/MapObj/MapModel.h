// MapModel.h: interface for the CMapModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#ifdef WIN32
#pragma warning( disable: 4018 )
#endif

#include "MapLoader.h"
#include "GLModel.h"
#include "PlumState.h"

namespace plum{

    template <class Cfg, class WEIGHT>
    class CMapModel : public CGLModel
    {
        
    public:
        void DumpSelected();
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapModel();
        virtual ~CMapModel();
        
        //////////////////////////////////////////////////////////////////////
        // Access functions
        //////////////////////////////////////////////////////////////////////
        void SetMapLoader( CMapLoader<Cfg,WEIGHT>  * mapLoader ){ m_mapLoader=mapLoader; }
        
        vector< Cfg > & GetMapNodes(){ return m_Nodes; }
        vector< pair< pair<VID,VID>, WEIGHT> > & GetMapEdges(){ return m_Edges; }
        
        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        
        bool BuildModels();
        void Draw( GLenum mode );
        //void Select( unsigned int * index );
        
    private:
        CMapLoader<Cfg,WEIGHT> * m_mapLoader;
        vector< Cfg > m_Nodes;
        vector< pair< pair<VID,VID>, WEIGHT> > m_Edges;
        
    protected:
        //virtual void DumpNode();
        //virtual void SelectNode( bool bSel );
    };

    /*********************************************************************
    *
    *      Implementation of CMapLoader
    *
    *********************************************************************/

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    template <class Cfg, class WEIGHT>
    CMapModel<Cfg, WEIGHT>::CMapModel()
    {
        m_mapLoader = NULL;
        //m_SelectedID=-1;
        m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
    }

    template <class Cfg, class WEIGHT>
    CMapModel<Cfg, WEIGHT>::~CMapModel()
    {
        //m_SelectedID=-1;
    }

    template <class Cfg, class WEIGHT>
    bool CMapModel<Cfg, WEIGHT>::BuildModels()
    {
        if( m_mapLoader==NULL ) return false;
        
        //create Model for nodes and edges
        WeightedMultiDiGraph< Cfg, WEIGHT> * graph = m_mapLoader->GetGraph();
        if( graph==NULL ) return false;
        
        m_Nodes = graph->GetVerticesData();
        int nSize=m_Nodes.size();
        for( int iN=0; iN<nSize; iN++ ){
            if( m_Nodes[iN].BuildModel( graph->GetVID(m_Nodes[iN]) )==false )
                return false;
        }
        
        m_Edges = graph->GetEdges();
        int eSize=m_Edges.size();
        for( int iE=0; iE<eSize; iE++ ){
            if( m_Edges[iE].first.first > m_Edges[iE].first.second ){
                Cfg start = graph->GetData(m_Edges[iE].first.first);
                Cfg end = graph->GetData(m_Edges[iE].first.second);
                if( m_Edges[iE].second.BuildModel(start, end)==false )
                    return false;
            }
        }

        m_mapLoader->KillGraph();
        return true;
    }

    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_EnableSeletion ) return;

        ////////////////////////////////////////////////////////////////////////////////
        // Draw vertex
        for( int iN=0; iN<m_Nodes.size(); iN++ ){
            if( mode==GL_SELECT ) glPushName( iN );
            m_Nodes[iN].Draw( mode );
            if( mode==GL_SELECT ) glPopName();
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        // Draw edge
        if( mode==GL_SELECT ) return; //no selection for edge
        glLineWidth(2);
        glColor3f(0.3f,0.3f,0.3f);
        for( int iE=0; iE<m_Edges.size(); iE++ ){
            if( m_Edges[iE].first.first > m_Edges[iE].first.second ){
                m_Edges[iE].second.Draw( mode );
            }
        }
        glLineWidth(1);
    }

    /*
    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::Select( unsigned int * index )
    {
        if( m_mapLoader==NULL ) return; //status error
        
        //unselect first
        if( m_SelectedID!=-1 ){
            SelectNode(false);
            m_SelectedID=-1;
        }
        
        if( index==NULL ) return;
        m_SelectedID = index[0];
        SelectNode(true);
    }

    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::SelectNode(bool bSel)
    {
        if( m_SelectedID<0 || m_SelectedID>=m_Nodes.size() ) return;
        m_Nodes[m_SelectedID].Select( bSel );
    }

    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::DumpSelected()
    {
        if( m_SelectedID<0 || m_SelectedID>=m_Nodes.size() ) return;
        DumpNode();
    }

    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::DumpNode()
    {
        if( m_SelectedID<0 || m_SelectedID>=m_Nodes.size() ) return;
        m_Nodes[m_SelectedID].Dump();
    }
    */

}//namespace plum

#endif // !defined(_MAPMODEL_H_)

