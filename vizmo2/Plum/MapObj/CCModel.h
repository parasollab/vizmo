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

        //type for the shape of node representation
        enum Shape { Robot, Box, Point };

        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CCModel();
        virtual ~CCModel();
        
        //////////////////////////////////////////////////////////////////////
        // GL interface functions
        //////////////////////////////////////////////////////////////////////      
        bool BuildModels(); //not used, should not call this
        void Draw( GLenum mode );
        void Select( unsigned int * index );
        bool BuildModels(VID id,WeightedGraph<Cfg,WEIGHT>* g); //call this instread
        const string GetName() const { return "CC"; }

		virtual list<string> GetInfo() const { 
			list<string> info; 
			char strsize[256]=""; 
			sprintf(strsize,"There are %d nodes",m_Nodes.size());
			info.push_back(string(strsize));
			
			sprintf(strsize,"There are %d edges",m_Edges.size());
			info.push_back(string(strsize));

			return info;
		}

		////////////////////////////////////////////////////////////////////// 
        void RobotModel(OBPRMView_Robot* pRobot){
            m_pRobot=pRobot;
        }
        
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

        void changeNode(Shape s) {m_sNodeShape=s;}
        
    private:
    
        void BuildNodeModels();
        void BuildRobotNodes();
        void BuildBoxNodes();
        void BuildPointNodes();
        void BuildEdges();

        vector< Cfg > m_Nodes;
        vector< pair< pair<VID,VID>, WEIGHT> > m_Edges;

		OBPRMView_Robot* m_pRobot;

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
        m_fRobotScale=0.5;
        m_fBoxScale = 0.5;
        m_sNodeShape = Point;
        //display id
        m_DID_Edges=m_DID_ROBOT=m_DID_Box=m_DID_PT=-1;
    }
    
    template <class Cfg, class WEIGHT>
        CCModel<Cfg, WEIGHT>::~CCModel()
    {
        glDeleteLists(m_DID_Edges,1);
        glDeleteLists(m_DID_ROBOT,1);
        glDeleteLists(m_DID_Box,1);
        glDeleteLists(m_DID_PT,1);
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
        
        //Setup cc nodes
        Cfg cfg = g->GetData(id);      
        m_Nodes = g->GetCC(cfg);
        int nSize=m_Nodes.size();
        for( int iN=0; iN<nSize; iN++ )
            m_Nodes[iN].Set(g->GetVID(m_Nodes[iN]), m_pRobot);
        
        //Setup edges
        m_Edges = g->GetCCEdges(id);
        int eSize=m_Edges.size();
        for( int iE=0; iE<eSize; iE++ ){
			Cfg cfg1=g->GetData(m_Edges[iE].first.first);
			Cfg cfg2=g->GetData(m_Edges[iE].first.second);
			Point3d p1(cfg1.GetX(),cfg1.GetY(),cfg1.GetZ());
			Point3d p2(cfg2.GetX(),cfg2.GetY(),cfg2.GetZ());
            m_Edges[iE].second.Set(p1,p2);
		}

        return true;
    }

    template <class Cfg, class WEIGHT>
    void CCModel<Cfg, WEIGHT>::BuildNodeModels()
    {
        switch( m_sNodeShape ){
            case Robot: BuildRobotNodes(); break;
            case Box: BuildBoxNodes(); break;
            case Point: BuildPointNodes(); break;
        }
    }

    template <class Cfg, class WEIGHT>
    void CCModel<Cfg, WEIGHT>::BuildRobotNodes() {
        ////////////////////////////////////////////////////////////////////////////////
        //backup
        double oldscale=m_pRobot->size;
        Quaternion q=m_pRobot->q();
        double x=m_pRobot->tx();
        double y=m_pRobot->ty();
        double z=m_pRobot->tz();
        m_pRobot->size=m_fRobotScale;
		double R=m_pRobot->GetColor()[0];
		double G=m_pRobot->GetColor()[1];
		double B=m_pRobot->GetColor()[2];

        m_DID_ROBOT = glGenLists(1);
		m_pRobot->SetColor(m_RGBA[0],m_RGBA[1],m_RGBA[2],m_pRobot->GetColor()[3]);
        glNewList( m_DID_ROBOT, GL_COMPILE );
        {
            int nSize=m_Nodes.size();
            for( int iN=0; iN<nSize; iN++ )
                m_Nodes[iN].DrawRobot();//draw robot;
        }
        glEndList();

        ////////////////////////////////////////////////////////////////////////////////
        //restore
        m_pRobot->size=oldscale;
        m_pRobot->tx()=x;
        m_pRobot->ty()=y;
        m_pRobot->tz()=z;
        m_pRobot->q(q);
		m_pRobot->SetColor(R,G,B,m_pRobot->GetColor()[3]);
    }

    template <class Cfg, class WEIGHT>
    void CCModel<Cfg, WEIGHT>::BuildBoxNodes() {
        m_DID_Box = glGenLists(1);
        glNewList( m_DID_Box, GL_COMPILE );
        {
            int nSize=m_Nodes.size();
            for( int iN=0; iN<nSize; iN++ )
                m_Nodes[iN].DrawBox(m_fBoxScale);//draw point;
        }
        glEndList();    
    }

    template <class Cfg, class WEIGHT>
    void CCModel<Cfg, WEIGHT>::BuildPointNodes(){
        if( m_DID_PT!=-1 ) return; //build already, wont change..
        m_DID_PT = glGenLists(1);
        glNewList( m_DID_PT, GL_COMPILE );
        glDisable(GL_LIGHTING);
        {
            glPointSize(4);
            int nSize=m_Nodes.size();
            for( int iN=0; iN<nSize; iN++ )
                m_Nodes[iN].DrawPoint();//draw point;
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
                m_Edges[iE].second.Draw();
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
                list=m_DID_ROBOT; break;
            case Box: 
                if(m_DID_Box==-1) BuildNodeModels();
                list=m_DID_Box; break;
            case Point: 
                if(m_DID_PT==-1) BuildNodeModels();
                list=m_DID_PT; break;
        }

        glCallList(list);
        
        ////////////////////////////////////////////////////////////////////////////////
        // Draw edge
        if( mode==GL_SELECT ) return; //no selection for edge
        glColor3f(0.1f,0.1f,0.1f);
        if( m_DID_Edges==-1 ) BuildEdges();
        glCallList(m_DID_Edges);
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::Select( unsigned int * index )
    {
        //do notthing yet
    }
    

}//end of namespace plum

#endif //_PLUM_CCMODEL_H_
