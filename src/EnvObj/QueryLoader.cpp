// QueryLoader.cpp: implementation of the CQueryLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "QueryLoader.h"
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;


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
    
    ifstream fin(m_filename.c_str());
    
    //reset every thing
    FreePathList();
    m_iCurrent_CfgIndex=0;
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //Read file info
    unsigned int iFileSize=2;
    
    //Build  Model
    int dof  = CfgModel::GetDOF(); 
    for( unsigned int iF=0;iF<iFileSize;iF++ )
    {
      dCfg=new double[dof];

      //read in Robot Index and throw it away for now
      double robotIndex;
      fin >> robotIndex;

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

vector<double> CQueryLoader::GetStartGoal(int Index){

  int dof = CfgModel::GetDOF(); 
  vector<double> currentCfg(dof); 
  for(int i=0;i<dof;i++) 
    currentCfg[i]=m_sgList[Index][i];
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

