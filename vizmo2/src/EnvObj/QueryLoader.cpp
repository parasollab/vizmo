// QueryLoader.cpp: implementation of the CQueryLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "QueryLoader.h"
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;

#include <Plum.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQueryLoader::CQueryLoader()
{
    m_iCurrent_CfgIndex=0;
    notExists = false;  // used to know if there is .path file available
    m_RenderMode=CPlumState::MV_INVISIBLE_MODE;
    m_DLIndex=-1;
}

CQueryLoader::~CQueryLoader()
{
    FreePathList();
}       

bool CQueryLoader::ParseFile()
{
    //check input
  if( CheckCurrentStatus()==false ) {
    notExists=true;
    return true;
  }

  ifstream fin(m_strFileName.c_str(), ios::in);

  //reset every thing
    FreePathList();
    m_iCurrent_CfgIndex=0;
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //Read file info

    unsigned int iFileSize=2;

    //Build  Model
    double rate = 360/(3.1415926*2);
    glMatrixMode( GL_MODELVIEW );
    m_DLIndex=glGenLists(1);
    glNewList( m_DLIndex, GL_COMPILE );
    for( unsigned int iF=0;iF<iFileSize;iF++ )
    {

    dCfg=new double[6];
        fin>>dCfg[0]>>dCfg[1]>>dCfg[2]>>dCfg[3]>>dCfg[4]>>dCfg[5];
    if (iF == 0)
      cout << "* START position: "<<dCfg[0]<<" "<<dCfg[1]<<" "<<dCfg[2]<<" "<<dCfg[3]<<" "<<dCfg[4]<<" "<<dCfg[5]<<endl;
    if (iF == 1)
          cout<< "* END position: "<<dCfg[0]<<" "<<dCfg[1]<<" "<<dCfg[2]<<" "<<dCfg[3]<<" "<<dCfg[4]<<" "<<dCfg[5]<<endl;
    
        m_sgList.push_back(dCfg); //here is stored the query

    
    //delete [] dCfg;
    }
    glEndList();
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //bye bye!!
    fin.close();
    return true;
}

double * CQueryLoader::GetStartGoal( int Index){

  double * currentCfg=new double[6];

  currentCfg[0]=m_sgList[Index][0];
  currentCfg[1]=m_sgList[Index][1];
  currentCfg[2]=m_sgList[Index][2];
  currentCfg[3]=m_sgList[Index][3]*6.2831852;
  currentCfg[4]=m_sgList[Index][4]*6.2831852;
  currentCfg[5]=m_sgList[Index][5]*6.2831852;

  return currentCfg;
}

void CQueryLoader::FreePathList()
{
    //not thing I don't need to free~
    if( m_sgList.empty() ) return;
    
    for( unsigned int iF=0;iF<m_sgList.size();iF++ )
    {
        delete [] m_sgList[iF];
    }
    
    m_sgList.clear();
}

