// MapModel.h: interface for the CMapModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPMODEL_H_)
#define _MAPMODEL_H_

#include <math.h>
#include "Gauss.h"
#include "src/EnvObj/Robot.h"

#include "MapLoader.h"
#include "CCModel.h"
#include "PlumState.h"

#include <algorithms/graph_algo_util.h>

using namespace stapl;
using namespace std;

namespace plum{
    
    template <class Cfg, class WEIGHT>
        class CMapModel : public CGLModel
    {
        typedef CCModel<Cfg,WEIGHT> myCCModel;
	typedef CMapLoader<Cfg,WEIGHT> Loader;
        typedef graph<DIRECTED,MULTIEDGES,Cfg,WEIGHT> Weg;
        typedef typename Weg::vertex_descriptor VID;
        typedef vector_property_map<Weg, size_t> color_map_t;
        color_map_t cmap;
    public:
        
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapModel();
        virtual ~CMapModel();// {
        
        //////////////////////////////////////////////////////////////////////
        // Access functions
        //////////////////////////////////////////////////////////////////////
        void SetMapLoader( Loader * mapLoader ){ m_mapLoader=mapLoader; }
        void SetRobotModel( OBPRMView_Robot* pRobot ){ m_pRobot = pRobot; }     
        vector<myCCModel*>& GetCCModels() { return m_CCModels; }
	
        myCCModel* GetCCModel(int id) {return m_CCModels[id]; }
	int number_of_CC(){return m_CCModels.size();}

	bool AddCC(int vid){
	  //get graph
	  if( m_mapLoader==NULL ) return false;
	  typename Loader::Wg * graph = m_mapLoader->GetGraph();
	  if( graph==NULL ) return false;
	  myCCModel * cc=new myCCModel(m_CCModels.size());
	  cc->RobotModel(m_pRobot);  
	  cc->BuildModels(vid,graph); 
	  float size;
          vector<float> color; 
	  if(m_CCModels[m_CCModels.size()-1]->getShape() == 0)
	    size = m_CCModels[m_CCModels.size()-1]->getRobotSize();
	  else if(m_CCModels[m_CCModels.size()-1]->getShape() == 1)
	    size = m_CCModels[m_CCModels.size()-1]->getBoxSize();
	  else
	    size = 0.0;
	  
	  color =  m_CCModels[m_CCModels.size()-1]->getColor();
	  
	  cc->change_properties(m_CCModels[m_CCModels.size()-1]->getShape(),size,
				color, true);
	  m_CCModels.push_back(cc);
	  
	  return true;
	}
	
        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        
        virtual bool BuildModels();
        virtual void Draw( GLenum mode );
	void Select( unsigned int * index, vector<gliObj>& sel );
        //set wire/solid/hide
        virtual void SetRenderMode( int mode );
        virtual const string GetName() const { return "Map"; }

        virtual void GetChildren( list<CGLModel*>& models ){
            typedef typename vector<myCCModel*>::iterator CIT;
            for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ )
                models.push_back(*ic);
        } 

	void SetProperties(typename myCCModel::Shape s, float size, 
			   vector<float> color, bool isNew){
	  typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
	  for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ )
	    (*ic)->change_properties(s, size, color, isNew);
	}
	
        virtual vector<string> GetInfo() const;
        
    private:
        
        OBPRMView_Robot* m_pRobot;
        vector< myCCModel* > m_CCModels;
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
        m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
        m_pRobot=NULL;
		m_enableSelection=true; //disable selection
    }
    
    template <class Cfg, class WEIGHT>
        CMapModel<Cfg, WEIGHT>::~CMapModel()
    {
      typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
      for(CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++){
        delete *ic;
      }//end for
    }
    
    template <class Cfg, class WEIGHT>
    bool CMapModel<Cfg, WEIGHT>::BuildModels()
    {
        typedef typename vector<myCCModel*>::iterator CCIT;//CC iterator
        //get graph
        if( m_mapLoader==NULL ) return false;
		typename Loader::Wg * graph = m_mapLoader->GetGraph();
        if( graph==NULL ) return false;
        for(CCIT ic = m_CCModels.begin(); ic != m_CCModels.end(); ic++)
          delete (*ic);
        m_CCModels.clear(); //new line Jul-01-05
        //Get CCs
        typedef typename vector< pair<size_t,VID> >::iterator CIT;//CC iterator
        vector<pair<size_t,VID> > ccs;
        cmap.reset();
        get_cc_stats(*graph,cmap,ccs);  
        

        int CCSize=ccs.size();
        m_CCModels.reserve(CCSize);
        for( CIT ic=ccs.begin();ic!=ccs.end();ic++ ){
            myCCModel* cc = new myCCModel(ic-ccs.begin());
            cc->RobotModel(m_pRobot);    
            cc->BuildModels(ic->second,graph);   
            m_CCModels.push_back(cc);
        }
        return true;
    }
    
    template <class Cfg, class WEIGHT>
        void CMapModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_enableSelection ) return;
        //Draw each CC
	int size = 0;
        typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
        for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
	  if( mode==GL_SELECT ) glPushName( (*ic)->ID() ); 
	  
	  (*ic)->Draw(mode);
	  
	  if( mode==GL_SELECT ) glPopName();
	  size++;
        }
        //m_pRobot->size = 1;
    }
    
    template <class Cfg, class WEIGHT>
        void CMapModel<Cfg, WEIGHT>::SetRenderMode( int mode ){ 
        m_RenderMode=mode;
        typedef typename vector<myCCModel*>::iterator CIT;//CC iterator
        for( CIT ic=m_CCModels.begin();ic!=m_CCModels.end();ic++ ){
            (*ic)->SetRenderMode(mode);
        }
    }
    
    template <class Cfg, class WEIGHT>
        vector<string> CMapModel<Cfg, WEIGHT>::GetInfo() const { 
        vector<string> info; 
        info.push_back(string(m_mapLoader->GetFileName()));
        
        {
            ostringstream temp;
            temp<<"There are "<<m_CCModels.size()<<" connected components";
            info.push_back(temp.str());
        }
        return info;
    }
    
    template <class Cfg, class WEIGHT>
		void CMapModel<Cfg, WEIGHT>::Select( unsigned int * index, vector<gliObj>& sel )
      {
      if( m_mapLoader==NULL ) return; //status error
      if( index==NULL ) return;
      m_CCModels[index[0]]->Select(&index[1],sel); 
    }

    
}//namespace plum

#endif // !defined(_MAPMODEL_H_)

