// QueryModel.cpp: implementation of the CQueryModel class.
//
//////////////////////////////////////////////////////////////////////

#include "QueryModel.h"
#include "Robot.h"

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <Plum.h>
#include<GL/glut.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQueryModel::CQueryModel()
{
    m_Index=-1;
    m_DLIndex=-1;
    m_pQueryLoader=NULL;
    m_pRobot=NULL;
    m_RenderMode=CPlumState::MV_INVISIBLE_MODE;
}

CQueryModel::~CQueryModel()
{
    m_Index=-1;
    m_DLIndex=-1;
    m_pQueryLoader=NULL;
    m_pRobot=NULL;
}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////
bool CQueryModel::BuildModels(){
	
    //can't build model without model loader
	
	if( m_pRobot==NULL || m_pQueryLoader==NULL )
		
		return false;
	
	///////////////////////////////////////////////////////
	//Build Model
    unsigned int iPathSize = m_pQueryLoader->GetPathSize();
	
    glMatrixMode( GL_MODELVIEW );
    m_DLIndex=glGenLists(1);
    m_pRobot->SetRenderMode(CPlumState::MV_WIRE_MODE);
    //remeber
    const float * col=m_pRobot->GetColor(); //remember old color
    float oldcol[4]; memcpy(oldcol,col,4*sizeof(float));
    m_pRobot->SetColor(0,1,0,0);
    //create list
    glNewList( m_DLIndex, GL_COMPILE );
    for( unsigned int iQ=0; iQ<iPathSize; iQ++ ){
		double * Cfg = m_pQueryLoader->GetStartGoal(iQ);  
		m_pRobot->Configure(Cfg);
		if(iQ==1) m_pRobot->SetColor(1,0,0,0);
		m_pRobot->Draw(GL_RENDER);
		delete [] Cfg;
    }
    glEndList();
	
    //set back
    m_pRobot->SetColor(oldcol[0],oldcol[1],oldcol[2],oldcol[3]);
    m_pRobot->SetRenderMode(CPlumState::MV_SOLID_MODE);
    return true;
}

void CQueryModel::Draw( GLenum mode ){

    if( mode==GL_SELECT ) return; //not draw anything
    if( m_RenderMode==CPlumState::MV_INVISIBLE_MODE ) return;

    if( m_DLIndex==-1 )
        return;

    glLineWidth(2.0);
    glCallList( m_DLIndex );
}
