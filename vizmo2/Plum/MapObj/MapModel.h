// MapModel.h: interface for the CMapModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#include <math.h>
#include "mathtool/Gauss.h"
#include "src/EnvObj/Robot.h"

#include "MapLoader.h"
#include "CCModel.h"
#include "PlumState.h"

namespace plum{
    
    template <class Cfg, class WEIGHT>
        class CMapModel : public CGLModel
    {
        typedef CCModel<Cfg,WEIGHT> myCCModel;
	typedef CMapLoader<Cfg,WEIGHT> Loader;
    public:
        
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapModel();
        virtual ~CMapModel();
        
        //////////////////////////////////////////////////////////////////////
        // Access functions
        //////////////////////////////////////////////////////////////////////
        void SetMapLoader( Loader * mapLoader ){ m_mapLoader=mapLoader; }
        void SetRobotModel( OBPRMView_Robot* pRobot ){ m_pRobot = pRobot; }     
        vector< myCCModel >& GetCCModels() { return m_CCModels; }
        
        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        
        virtual bool BuildModels();
        virtual void Draw( GLenum mode );
        //set wire/solid/hide
        virtual void SetRenderMode( int mode );
        virtual const string GetName() const { return "Map"; }
        virtual void GetChildren( list<CGLModel*>& models ){
            typedef typename vector< myCCModel >::iterator CIT;
            for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ )
                models.push_back(&(*ic));
        }
        virtual list<string> GetInfo() const;
        
    private:
        
        OBPRMView_Robot* m_pRobot;
        vector< myCCModel > m_CCModels;
        Loader * m_mapLoader;
        
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
		typename Loader::WG * graph = m_mapLoader->GetGraph();
        if( graph==NULL ) return false;
        
        //Get CCs
        typedef typename vector< pair<int,VID> >::iterator CIT;//CC iterator
        vector< pair<int,VID> > CCs;
		GetCCStats(*graph,CCs);
        int CCSize=CCs.size();
        m_CCModels.reserve(CCSize);
        for( CIT ic=CCs.begin();ic!=CCs.end();ic++ ){
            myCCModel cc(ic-CCs.begin());
            cc.RobotModel(m_pRobot);    
            cc.BuildModels(ic->second,graph);   
            m_CCModels.push_back(cc);
        }
        
        //release graph, not used
        m_mapLoader->KillGraph();
        
        return true;
    }
    
    template <class Cfg, class WEIGHT>
        void CMapModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_EnableSeletion ) return;
        //Draw each CC
        typedef typename vector< myCCModel >::iterator CIT;//CC iterator
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
        typedef typename vector< myCCModel >::iterator CIT;//CC iterator
        for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
            ic->SetRenderMode(mode);
        }
    }
    
    template <class Cfg, class WEIGHT>
        list<string> CMapModel<Cfg, WEIGHT>::GetInfo() const { 
        list<string> info; 
        info.push_back(string(m_mapLoader->GetFileName()));
        
        {
            ostringstream temp;
            temp<<"There are "<<m_CCModels.size()<<" connected components";
            info.push_back(temp.str());
        }
        return info;
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

