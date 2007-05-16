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
    int dof =CCfg::dof;
    for( unsigned int iF=0;iF<iFileSize;iF++ )
    {
      dCfg=new double[dof];
      for(int j=0; j<dof; j++){
        fin>>dCfg[j];
      }
      m_sgList.push_back(dCfg); //here is stored the query
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //bye bye!!
    fin.close();
    return true;
}

double * CQueryLoader::GetStartGoal(int Index){

  int dof =CCfg::dof;
  double * currentCfg=new double[dof]; 
  for(int i=0;i<dof;i++) currentCfg[i]=m_sgList[Index][i];
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

