#ifndef _PLUM_CCMODEL_H_
#define _PLUM_CCMODEL_H_

#include <Graph.h>
#include <iostream>
#include <vector> 
#include <string>
using namespace std;

#include "GLModel.h"
#include "MapLoader.h"
#include "SimpleCfg.h"
#include "src/EnvObj/Robot.h"

namespace plum{
    
	class CCModelBase : public CGLModel
	{
	public:
        //type for the shape of node representation
        enum Shape { Robot, Box, Point};

		CCModelBase(unsigned int ID){ 
			m_ID=ID; 
		    m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
			//Set random Color
			m_RGBA[0]=((float)rand())/RAND_MAX;
			m_RGBA[1]=((float)rand())/RAND_MAX;
			m_RGBA[2]=((float)rand())/RAND_MAX;
			//size
			m_fRobotScale=0.5;
			m_fBoxScale = 0.5;
			m_sNodeShape = Point;
			//display id
		    m_DID_Edges=m_DID_ROBOT=m_DID_Box=m_DID_PT=-1;
		}

		virtual ~CCModelBase(){
			glDeleteLists(m_DID_Edges,1);
			glDeleteLists(m_DID_ROBOT,1);
			glDeleteLists(m_DID_Box,1);
			glDeleteLists(m_DID_PT,1);
		}

		void ReBuildAll(){
			glDeleteLists(m_DID_Edges,1);
			glDeleteLists(m_DID_ROBOT,1);
			glDeleteLists(m_DID_Box,1);
			glDeleteLists(m_DID_PT,1);
			m_DID_Edges=-1; 
			m_DID_ROBOT=-1; 
			m_DID_Box=-1;   
			m_DID_PT=-1;    		
		}

        //////////////////////////////////////////////////////////////////////
        // Accss
        //////////////////////////////////////////////////////////////////////      

        void scaleNode( float scale, Shape s ) { 
            m_sNodeShape=s;
            if(m_sNodeShape==Robot && m_fRobotScale!=scale){
                m_fRobotScale = scale;
                glDeleteLists(m_DID_ROBOT,1);
                m_DID_ROBOT=-1; //need new id
            }
            else if(m_sNodeShape==Box && m_fBoxScale!=scale){
                m_fBoxScale=scale;
                glDeleteLists(m_DID_Box,1);
                m_DID_Box=-1; //need new id
            }
        }
        
        void changeShape(Shape s) {m_sNodeShape=s;}
		
		/// Allow to change color of CC's
		void SetColor( float r, float g, float b, float a ){
			CGLModel::SetColor(r,g,b,a); 
			//force to rebuild robot
			glDeleteLists(m_DID_ROBOT,1);
			m_DID_ROBOT=-1; //need new id
		} 

		//get id
		int ID() const { return m_ID; }
	protected:
		//CC ID
        int m_ID;

	    //to store the node size of the current selection
        float m_fRobotScale;
        float m_fBoxScale;
        
        //to store the "name" of the item selected
        Shape m_sNodeShape;
        
        //display ids
        int m_DID_Edges; //id for edges
        int m_DID_ROBOT; //id for original robots
        int m_DID_Box;   //id for box
        int m_DID_PT;    //id for points
	};

    template <class Cfg, class WEIGHT> class CCModel : public CCModelBase
    {
        typedef typename CMapLoader<Cfg,WEIGHT>::WG WG;
		
    public:
        
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CCModel(unsigned int ID);
        virtual ~CCModel();
        
        //////////////////////////////////////////////////////////////////////
        // GL interface functions
        //////////////////////////////////////////////////////////////////////      
        bool BuildModels(); //not used, should not call this
        void Draw( GLenum mode );
		void DrawSelect();
        void Select( unsigned int * index, vector<gliObj>& sel );
        bool BuildModels(VID id,WG * g); //call this instread
        const string GetName() const;
        virtual list<string> GetInfo() const;
        
        void RobotModel(OBPRMView_Robot* pRobot){
            m_pRobot=pRobot;
        }
        
    private:
        
        void BuildNodeModels();
        void DrawRobotNodes(GLenum mode);
        void DrawBoxNodes(GLenum mode);
        void DrawPointNodes(GLenum mode);
        void BuildEdges();
		        
        vector<Cfg*> m_Nodes;
        vector< WEIGHT> m_Edges;
        OBPRMView_Robot* m_pRobot;
		WG * m_Graph;
    };
    
    
    /*********************************************************************
    *
    *      Implementation of CCModel
    *
    *********************************************************************/
    template <class Cfg, class WEIGHT>
        CCModel<Cfg, WEIGHT>::CCModel(unsigned int ID)
		:CCModelBase(ID){ m_Graph=NULL; }
    
    template <class Cfg, class WEIGHT>
        CCModel<Cfg, WEIGHT>::~CCModel()
    {m_Graph=NULL;}
    
    template <class Cfg, class WEIGHT>
        bool CCModel<Cfg, WEIGHT>::BuildModels()
    {
        //do not call this function
        cerr<<"CCModel<Cfg, WEIGHT>::BuildModels() : Do not call this function\n"
            <<"call CCModel<Cfg, WEIGHT>::BuildModel(VID id,WG* g)"
            <<" instead"<<endl;
        return false;
    }
    
    template <class Cfg, class WEIGHT>
		bool CCModel<Cfg, WEIGHT>::BuildModels(VID id,WG* g)
    {
        if( g==NULL ) return false;
        m_Graph=g;

        //Setup cc nodes
		vector<int> ccnid; //node id in this cc
        GetCC(*g,id,ccnid);
        int nSize=ccnid.size();
		m_Nodes.reserve(nSize);
        for( int iN=0; iN<nSize; iN++ ){
			int nid=ccnid[iN];
			CSimpleCfg* cfg=g->GetReferenceofData(nid);
		    cfg->Set(nid,m_pRobot,this);
			m_Nodes.push_back(cfg);
		}
		
        //Setup edges
        //m_Edges = g->GetCCEdges(id);
		vector< pair<VID,VID> > ccedges;
		GetCCEdges(*g,ccedges,id);
        int eSize=ccedges.size();
		m_Edges.reserve(eSize/2);
		int edge_index=0;
        for( int iE=0; iE<eSize; iE++ ){
			if( ccedges[iE].first<ccedges[iE].second ) continue;
            Cfg* cfg1=g->GetReferenceofData(ccedges[iE].first);
            Cfg* cfg2=g->GetReferenceofData(ccedges[iE].second);
			WEIGHT w=g->GetEdgeWeight(ccedges[iE].first,ccedges[iE].second);
			w.Set(edge_index,cfg1,cfg2);
			edge_index++;
			m_Edges.push_back(w);
        }
        
        return true;
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::BuildNodeModels()
    {
        switch( m_sNodeShape ){
        case Robot: 
			m_DID_ROBOT = glGenLists(1);
			glNewList( m_DID_ROBOT, GL_COMPILE );
			DrawRobotNodes(GL_RENDER); break;
        case Box: 
			m_DID_Box = glGenLists(1);
			glNewList( m_DID_Box, GL_COMPILE );
			DrawBoxNodes(GL_RENDER); break;
        case Point: 
			m_DID_PT = glGenLists(1);
			glNewList( m_DID_PT, GL_COMPILE );
			DrawPointNodes(GL_RENDER); break;
        }
		glEndList();
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::DrawRobotNodes(GLenum mode) {

		if( m_pRobot==NULL ) return; //no robot given
		if( mode==GL_RENDER ){
			m_pRobot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],m_pRobot->GetColor()[3]);
			glEnable(GL_LIGHTING);
		}

		int nSize=m_Nodes.size();
		for( int iN=0; iN<nSize; iN++ ){
			m_Nodes[iN]->Scale(m_fRobotScale,m_fRobotScale,m_fRobotScale);
			m_Nodes[iN]->SetShape(CSimpleCfg::Robot);
			m_Nodes[iN]->Draw(mode);//draw robot;
		}
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::DrawBoxNodes(GLenum mode) {
		
		glEnable(GL_LIGHTING);
		int nSize=m_Nodes.size();
		for( int iN=0; iN<nSize; iN++ ){
			m_Nodes[iN]->Scale(m_fBoxScale,m_fBoxScale,m_fBoxScale);
			m_Nodes[iN]->SetShape(CSimpleCfg::Box);
			m_Nodes[iN]->Draw(mode);//draw point;
		}
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::DrawPointNodes(GLenum mode){	
        glDisable(GL_LIGHTING);
		glPointSize(4);
		int nSize=m_Nodes.size();
		for( int iN=0; iN<nSize; iN++ ){
			m_Nodes[iN]->Scale(1,1,1);
			m_Nodes[iN]->SetShape(CSimpleCfg::Point);
			m_Nodes[iN]->Draw(mode);//draw point;
		}
        glEndList();
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::BuildEdges(){
        //build edges
        m_DID_Edges = glGenLists(1);
        glNewList( m_DID_Edges, GL_COMPILE );
        glDisable(GL_LIGHTING);
        {
            int eSize=m_Edges.size();
            for( int iE=0; iE<eSize; iE++ )
                m_Edges[iE].Draw(m_RenderMode);
        }   
        glEndList();
    }	
    
    ////////////////////////////////////////////////////////////////////////////////////
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_EnableSeletion ) return;
        
        ////////////////////////////////////////////////////////////////////////////////
        glColor4fv(m_RGBA); //Set Color
        
        ////////////////////////////////////////////////////////////////////////////////
        // Draw vertex
        //m_pRobot->size=m_fNodeScale;
        int list=-1;
        switch( m_sNodeShape ){
        case Robot: 
            if(m_DID_ROBOT==-1) BuildNodeModels();
            list=m_DID_ROBOT; 
			break;
        case Box: 
            if(m_DID_Box==-1) BuildNodeModels();
            list=m_DID_Box; 
			break;
        case Point: 
            if(m_DID_PT==-1) BuildNodeModels();
            list=m_DID_PT; 
			break;
        }
				
		if( mode==GL_SELECT ) glPushName(1); //1 means nodes
        glCallList(list);
		if( mode==GL_SELECT ) glPopName();
        
        ////////////////////////////////////////////////////////////////////////////////
        // Draw edge
        if( m_DID_Edges==-1 ) BuildEdges();
		glColor3f(0.2f,0.2f,0.2f);
		glLineWidth(1);

		if( mode==GL_SELECT ) glPushName(2); //1 means edge
        glCallList(m_DID_Edges);
		if( mode==GL_SELECT ) glPopName();
    }

	template <class Cfg, class WEIGHT>
		void CCModel<Cfg, WEIGHT>::DrawSelect()
	{
        if( m_DID_Edges==-1 ) BuildEdges();
		glColor3f(1,1,0);
		glLineWidth(3);
        glCallList(m_DID_Edges);
		glLineWidth(1);
	}
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::
		Select( unsigned int * index, vector<gliObj>& sel )
    {
		if( index==NULL || m_Graph==NULL ) return;
		if(index[0]==1){
			CSimpleCfg* cfg=m_Graph->GetReferenceofData(index[1]);
			cfg->SetShape((CSimpleCfg::Shape)m_sNodeShape);
			sel.push_back(cfg);
		}
		else sel.push_back(&m_Edges[index[1]]);
    }
    
    template <class Cfg, class WEIGHT>
        const string CCModel<Cfg, WEIGHT>::GetName() const 
    { 
        ostringstream temp;
        temp<<"CC"<<m_ID;
        return temp.str(); 
    }
    
    template <class Cfg, class WEIGHT>
        list<string> CCModel<Cfg, WEIGHT>::GetInfo() const 
    { 
		//changeStatus();
		
        list<string> info; 
        {
            ostringstream temp;
            temp<<"There are "<<m_Nodes.size()<<" nodes";
            info.push_back(temp.str());
        }
        {
            ostringstream temp;
            temp<<"There are "<<m_Edges.size()<<" edges";
            info.push_back(temp.str());
        }       
        return info;
    }
    
}//end of namespace plum

#endif //_PLUM_CCMODEL_H_


