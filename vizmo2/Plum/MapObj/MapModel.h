// MapModel.h: interface for the CMapModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#ifdef WIN32
#pragma warning( disable: 4018 )
#endif

#include "MapLoader.h"
#include "CCModel.h"
#include "PlumState.h"

#include <math.h>
#include "mathtool/Gauss.h"

#include "src/EnvObj/Robot.h"

namespace plum{
    
    template <class Cfg, class WEIGHT>
        class CMapModel : public CGLModel
    {
		typedef CCModel<Cfg,WEIGHT> myCCModel;
    public:

        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapModel();
        virtual ~CMapModel();
        
        //////////////////////////////////////////////////////////////////////
        // Access functions
        //////////////////////////////////////////////////////////////////////
        void SetMapLoader( CMapLoader<Cfg,WEIGHT>  * mapLoader ){ m_mapLoader=mapLoader; }
        

	void SetRobotModel( OBPRMView_Robot* pRobot ){ 
	  //cout<<"SetRobotMOdel in MapModel.h"<<endl;
	  m_pRobot = pRobot;//new OBPRMView_Robot(*pRobot); 
	}
	
	vector< myCCModel >& GetCCModels() { return m_CCModels; }

        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        
        virtual bool BuildModels();
        virtual void Draw( GLenum mode );
        //set wire/solid/hide
        virtual void SetRenderMode( int mode );

    private:

	OBPRMView_Robot* m_pRobot;
        vector< myCCModel > m_CCModels;
        CMapLoader<Cfg,WEIGHT> * m_mapLoader;

    protected:
        //virtual void DumpNode();
        //virtual void SelectNode( bool bSel );
    };
    
    /*********************************************************************
    *
    *      Implementation of CMapModel
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
		m_pRobot=NULL;
    }
    
    template <class Cfg, class WEIGHT>
        CMapModel<Cfg, WEIGHT>::~CMapModel()
    {
        //m_SelectedID=-1;
    }
    
    template <class Cfg, class WEIGHT>
        bool CMapModel<Cfg, WEIGHT>::BuildModels()
    {
        //get graph
        if( m_mapLoader==NULL ) return false;
        WeightedGraph<Cfg, WEIGHT> * graph = m_mapLoader->GetGraph();
        if( graph==NULL ) return false;

        //Get CCs
        typedef vector< pair<int,VID> >::iterator CIT;//CC iterator
        vector< pair<int,VID> > CCs = graph->GetCCStats();
        int CCSize=CCs.size();
        m_CCModels.reserve(CCSize);
        for( CIT ic=CCs.begin();ic!=CCs.end();ic++ ){
            myCCModel cc;

	    ///// new
	    cc.RobotModel(m_pRobot);
	    /////////

            cc.BuildModels(ic->second,graph);

            m_CCModels.push_back(cc);
        }

	

        //release graph, not used
        //m_mapLoader->KillGraph();

        return true;
    }
    
    template <class Cfg, class WEIGHT>
        void CMapModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_EnableSeletion ) return;
        //Draw each CC
        typedef vector< myCCModel >::iterator CIT;//CC iterator
        for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
            if( mode==GL_SELECT ) glPushName( ic-m_CCModels.begin() );
            ic->Draw(GL_RENDER); //not select node, just select CC first
            if( mode==GL_SELECT ) glPopName();
        }
	//m_pRobot->size = 1;
    }

    template <class Cfg, class WEIGHT>
    void CMapModel<Cfg, WEIGHT>::SetRenderMode( int mode ){ 
        m_RenderMode=mode;
        typedef vector< myCCModel >::iterator CIT;//CC iterator
        for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
            ic->SetRenderMode(mode);
        }
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

