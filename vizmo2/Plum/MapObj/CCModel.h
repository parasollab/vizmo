#ifndef _PLUM_CCMODEL_H_
#define _PLUM_CCMODEL_H_

#include <Graph.h>
#include <iostream>
#include <vector> 
#include <string>
using namespace std;

#include "GLModel.h"
#include "MapLoader.h"
#include "src/EnvObj/Robot.h"


namespace plum{
    
    template <class Cfg, class WEIGHT> class CCModel : public CGLModel
    {
        typedef typename CMapLoader<Cfg,WEIGHT>::WG WG;

    public:
        
        //type for the shape of node representation
        enum Shape { Robot, Box, Point};

	// to know if change color of CC's
	bool newColor;
	int id;
	char m_shape; //'r' robot, 'b' box, 'p' point
        
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
        void Select( unsigned int * index );
        bool BuildModels(VID id,WG * g); //call this instread
        const string GetName() const;
        virtual list<string> GetInfo() const;
        
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

	/// Allow to change color of CC's
	void changeColor(double r, double g, double b, Shape s){
	  m_sNodeShape=s;
	  m_RGBA[0]=r;
	  m_RGBA[1]=g;
	  m_RGBA[2]=b;	  
	}
/*
	void DrawSelect()
	{
        glLineWidth(2);
        glPushMatrix();
        glTransform();
        glColor3d(1,1,0);
        glCallList(m_WiredID);
        glPopMatrix();
       }
*/   
        
    private:
        
        //CC ID
        int m_ID;
        
        void BuildNodeModels();
        void BuildRobotNodes();
        void BuildBoxNodes();
        void BuildPointNodes();
        void BuildEdges();

	void ChangeColor();
        
        vector< Cfg > m_Nodes;
        vector< WEIGHT> m_Edges;
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
        CCModel<Cfg, WEIGHT>::CCModel(unsigned int ID)
    {
        m_RenderMode = CPlumState::MV_INVISIBLE_MODE;
        m_ID=ID;

	id = ID;

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

	//initialize variable for color
	newColor = false;
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
    bool CCModel<Cfg, WEIGHT>::BuildModels(VID id,WG* g)
    {
        if( g==NULL ) return false;
        
        //Setup cc nodes
		vector<int> ccnid; //node id in this cc
        GetCC(*g,id,ccnid);
        int nSize=ccnid.size();
		m_Nodes.reserve(nSize);
        for( int iN=0; iN<nSize; iN++ ){
			int nid=ccnid[iN];
			m_Nodes.push_back(g->GetData(nid));
            m_Nodes[iN].Set(nid, m_pRobot);
		}

        //Setup edges
        //m_Edges = g->GetCCEdges(id);
		vector< pair<VID,VID> > ccedges;
		GetCCEdges(*g,ccedges,id);
        int eSize=ccedges.size();
		m_Edges.reserve(eSize);
        for( int iE=0; iE<eSize; iE++ ){
            Cfg cfg1=g->GetData(ccedges[iE].first);
            Cfg cfg2=g->GetData(ccedges[iE].second);
            Point3d p1(cfg1.GetX(),cfg1.GetY(),cfg1.GetZ());
            Point3d p2(cfg2.GetX(),cfg2.GetY(),cfg2.GetZ());
	    WEIGHT w=g->GetEdgeWeight(ccedges[iE].first,ccedges[iE].second);
	    w.Set(p1,p2);
	    m_Edges.push_back(w);
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
            glEnable(GL_LIGHTING);
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
            glEnable(GL_LIGHTING);
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
                m_Edges[iE].Draw();
        }   
        glEndList();
    }

    template <class Cfg, class WEIGHT>
      void CCModel<Cfg, WEIGHT>::ChangeColor(){

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
            glEnable(GL_LIGHTING);
            int nSize=m_Nodes.size();
	    //cout<<"SHAPE in ChangeColor():: "<<m_shape<<endl;
            for( int iN=0; iN<nSize; iN++ ){
	      if(m_shape == 'r')
                m_Nodes[iN].DrawRobot();//draw robot
	      else if(m_shape == 'b')
		m_Nodes[iN].DrawBox(m_fBoxScale);//draw box
	      else if(m_shape == 'p')
		m_Nodes[iN].DrawPoint();//draw point
	    }
        }
        glEndList();
        
        //restore values of robot color, size and position.
        m_pRobot->size=oldscale;
        m_pRobot->tx()=x;
        m_pRobot->ty()=y;
        m_pRobot->tz()=z;
        m_pRobot->q(q);
        m_pRobot->SetColor(R,G,B,m_pRobot->GetColor()[3]);
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
	    m_shape='r';
	    break;
        case Box: 
            if(m_DID_Box==-1) BuildNodeModels();
            list=m_DID_Box; 
	    m_shape = 'b';
	    break;
        case Point: 
            if(m_DID_PT==-1) BuildNodeModels();
            list=m_DID_PT; 
	    m_shape = 'p';
	    break;
        }

/*  	cout<<"**************"<<endl; */
/*  	cout<<"SHAPE::"<<m_sNodeShape<<endl; */
/*  	cout<<"m_shape in Draw():: "<<m_shape<<endl; */
/*  	cout<<"newColor value in draw():: "<<newColor<<endl; */
/*  	cout<<"**************"<<endl; */

	if(newColor) ChangeColor();
	//DrawSelect();
        
        glCallList(list);
        
        ////////////////////////////////////////////////////////////////////////////////
        // Draw edge
        if( mode==GL_SELECT ) return; //no selection for edge
        //glColor3f(0.1f,0.1f,0.1f);
	glColor3f(m_RGBA[0],m_RGBA[1],m_RGBA[2]);
        if( m_DID_Edges==-1 ) BuildEdges();
        glCallList(m_DID_Edges);
    }
    
    template <class Cfg, class WEIGHT>
        void CCModel<Cfg, WEIGHT>::Select( unsigned int * index )
    {
        //do notthing yet
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


