#ifndef _PLUM_CCMODEL_H_
#define _PLUM_CCMODEL_H_

#include <Graph.h>
#include "GLModel.h"
#include <iostream>
#include <vector> 
#include <string>
using namespace std;

#include "src/EnvObj/Robot.h"

namespace plum{
    
    template <class Cfg, class WEIGHT> class CCModel : public CGLModel
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CCModel();
        virtual ~CCModel();
        
        //////////////////////////////////////////////////////////////////////
        // GL interface functions
        //////////////////////////////////////////////////////////////////////
        
	OBPRMView_Robot* m_pRobot;

        bool BuildModels(); //not used, should not call this
        void Draw( GLenum mode );
        void Select( unsigned int * index );
        bool BuildModels(VID id,WeightedGraph<Cfg,WEIGHT>* g); //call this instread
        
		void RobotModel(OBPRMView_Robot* pRobot){
			m_pRobot=pRobot;
		}

		//void scaleNode( float scale ) { m_fNodeScale=scale;}
		void scaleNode( float scale, string s ) { 
		  m_fNodeScale=scale; 
		  shape=s;
		  if(shape == "Original"){
		    change = true;
		    m_fRobotNodeSize = scale;
		  }
		}
		void changeNode(string s) {m_sNodeShape = s;}

    private:
        vector< Cfg > m_Nodes;
        vector< pair< pair<VID,VID>, WEIGHT> > m_Edges;
	// variables used to manage the size of the robot
	float m_fRobotNodeSize;
	bool change;
	//to store the node size of the current selection
	float m_fNodeScale;
	//to store the "name" of the item selected
	string m_sNodeShape;
	string shape;

    };


     /*********************************************************************
     *
     *      Implementation of CCModel
     *
     *********************************************************************/
    template <class Cfg, class WEIGHT>
    CCModel<Cfg, WEIGHT>::CCModel()
    {
        m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
        //Set random Color
        m_RGBA[0]=((float)rand())/RAND_MAX;
        m_RGBA[1]=((float)rand())/RAND_MAX;
        m_RGBA[2]=((float)rand())/RAND_MAX;
	//size
	m_fNodeScale=0.5;
	m_fRobotNodeSize = 0.5;
	change = false;
    }

    template <class Cfg, class WEIGHT>
    CCModel<Cfg, WEIGHT>::~CCModel()
    {
        //nothing yet
    }

    template <class Cfg, class WEIGHT>
    bool CCModel<Cfg, WEIGHT>::BuildModels()
    {
        //do not call this function
        cerr<<"CCModel<Cfg, WEIGHT>::BuildModels() : Do not call this function\n"
            <<"call CCModel<Cfg, WEIGHT>::BuildModel(VID id,WeightedGraph<Cfg,WEIGHT>* g)"
            <<" instead"<<endl;
        return false;
    }

    template <class Cfg, class WEIGHT>
    bool CCModel<Cfg, WEIGHT>::BuildModels(VID id,WeightedGraph<Cfg,WEIGHT>* g)
    {
        if( g==NULL ) return false;
		
        //get cc nodes
	Cfg cfg = g->GetData(id);      
        m_Nodes = g->GetCC(cfg);
        int nSize=m_Nodes.size();
        for( int iN=0; iN<nSize; iN++ ){
	  if( m_Nodes[iN].BuildModel( g->GetVID(m_Nodes[iN]), m_pRobot )==false )
	    return false;
        }
		
	//get cc edges
        m_Edges = g->GetCCEdges(id);
        int eSize=m_Edges.size();
        for( int iE=0; iE<eSize; iE++ ){
            //if( m_Edges[iE].first.first > m_Edges[iE].first.second ){
            Cfg start = g->GetData(m_Edges[iE].first.first);
            Cfg end = g->GetData(m_Edges[iE].first.second);
            if( m_Edges[iE].second.BuildModel(start, end)==false )
                return false;
            // }
        }
        
        return true;
    }


    template <class Cfg, class WEIGHT>
      void CCModel<Cfg, WEIGHT>::Draw( GLenum mode ) {
        
        if( m_RenderMode == CPlumState::MV_INVISIBLE_MODE ) return;
        if( mode==GL_SELECT && !m_EnableSeletion ) return;
        
        ////////////////////////////////////////////////////////////////////////////////
        
	glColor4fv(m_RGBA); //Set Color
	
	////////////////////////////////////////////////////////////////////////////////
	//backup
	double oldscale=m_pRobot->size;
	Quaternion q=m_pRobot->q();
	double x=m_pRobot->tx();
	double y=m_pRobot->ty();
	double z=m_pRobot->tz();
        
	////////////////////////////////////////////////////////////////////////////////
        // Draw vertex
	//m_pRobot->size=m_fNodeScale;

/*  	if((shape != "Box") && (shape != "Point")&& (shape != "Original")){ */
/*  	  m_pRobot->size=0.5; */
/*  	} */

	if (change){
	  m_pRobot->size=m_fNodeScale;
	}
	else
	  m_pRobot->size=m_fRobotNodeSize;

       
	for( int iN=0; iN<m_Nodes.size(); iN++ ){
	  m_Nodes[iN].m_NodeShape = m_sNodeShape;
	  if(shape == "Box"){
	    m_Nodes[iN].scaleBox = m_fNodeScale;
	  }
	  else if(shape == "Point"){
	    m_Nodes[iN].scalePoint = m_fNodeScale;
	  }


	  if( mode==GL_SELECT ) glPushName( iN );
	  m_Nodes[iN].Draw( mode );			
	  if( mode==GL_SELECT ) glPopName();
        }
	////////////////////////////////////////////////////////////////////////////////
	//restore
        m_pRobot->size=oldscale;
	m_pRobot->tx()=x;
	m_pRobot->ty()=y;
	m_pRobot->tz()=z;
	m_pRobot->q(q);

	change = false;

        ////////////////////////////////////////////////////////////////////////////////
        // Draw edge
        if( mode==GL_SELECT ) return; //no selection for edge
        glLineWidth(2);
        for( int iE=0; iE<m_Edges.size(); iE++ ){
	  m_Edges[iE].second.Draw( mode );
        }
        glLineWidth(1);
    }
    
    template <class Cfg, class WEIGHT>
      void CCModel<Cfg, WEIGHT>::Select( unsigned int * index )
      {
        //do notthing yet
      }
    
}//end of namespace plum

#endif //_PLUM_CCMODEL_H_
