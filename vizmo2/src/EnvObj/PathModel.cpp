// PathModel.cpp: implementation of the CPathModel class.
//
//////////////////////////////////////////////////////////////////////

#include "PathModel.h"
#include "Robot.h"

//////////////////////////////////////////////////////////////////////
// Include Plum headers
#include <Plum.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathModel::CPathModel()
{
    m_Index=-1;
    m_DLIndex=-1;
    m_pPathLoader=NULL;
    m_pRobot=NULL;
        m_RenderMode=CPlumState::MV_INVISIBLE_MODE;
}

CPathModel::~CPathModel()
{
    m_Index=-1;
    m_DLIndex=-1;
    m_pPathLoader=NULL;
    m_pRobot=NULL;
}

//////////////////////////////////////////////////////////////////////
// Action Methods
//////////////////////////////////////////////////////////////////////
bool CPathModel::BuildModels(){

    //can't build model without model loader
    if( m_pRobot==NULL || m_pPathLoader==NULL ) return false;

    ///////////////////////////////////////////////////////
    //Build Path Model
    unsigned int iPathSize = m_pPathLoader->GetPathSize();
    m_pRobot->SetRenderMode(CPlumState::MV_WIRE_MODE);
    const float * col=m_pRobot->GetColor(); //old color
    float oldcol[4]; memcpy(oldcol,col,4*sizeof(float));
    
    glMatrixMode( GL_MODELVIEW );
    m_DLIndex=glGenLists(1);
    glNewList( m_DLIndex, GL_COMPILE );
      for( unsigned int iP=0; iP<iPathSize; iP++ ){
	  double percent=((double)iP)/iPathSize;
	  m_pRobot->SetColor(percent,0.8f,1-percent,1.0);
          double * Cfg = m_pPathLoader->GetConfiguration(iP);
          if( iP%3==0 ){
              m_pRobot->Configure(Cfg);
              m_pRobot->Draw(GL_RENDER);
          }
	      delete [] Cfg;
      }
    glEndList();
    
    //set back
    m_pRobot->SetRenderMode(CPlumState::MV_SOLID_MODE);
    m_pRobot->SetColor(oldcol[0],oldcol[1],oldcol[2],oldcol[3]);
    return true;
}

void CPathModel::Draw( GLenum mode ){

    if( mode==GL_SELECT ) return; //not draw any thing
        if( m_RenderMode==CPlumState::MV_INVISIBLE_MODE ) return;

    if( m_DLIndex==-1 )
        return;

    //set to line represnet
	glLineWidth(0.5);
    glColor3f( 0.3f, 0.2f, 0.7f );
    glCallList( m_DLIndex );
}
