// PathLoader.cpp: implementation of the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "PathLoader.h"
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathLoader::CPathLoader()
{
}

CPathLoader::~CPathLoader()
{
    FreePathList();
}       

bool CPathLoader::ParseFile()
{
    //check input
    if( CheckCurrentStatus()==false ){
		return true;
    }

    ifstream fin(m_strFileName.c_str(), ios::in);

    //reset every thing
    FreePathList();
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //Read file info
    char garbige[100];
    fin.getline(garbige,99);        //the garbege of path file
    fin.getline(garbige,99);
    
    unsigned int iPathSize=0;
    fin>>iPathSize;
    
    int dof=CCfg::dof;

    for( unsigned int iF=0;iF<iPathSize;iF++ )
    {
      double * dCfg=new double[dof];

      for(int j=0; j<dof; j++){
	fin>>dCfg[j];
      }
      m_pList.push_back(dCfg);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //bye bye!!
    fin.close();
    return true;
}

double * CPathLoader::GetConfiguration(int frame)
{
	if(frame>=(int)m_pList.size()) frame=m_pList.size()-1;
	if(frame<0 ) frame=0;
	double * currentCfg = GetConfigure(false, frame);    
	return currentCfg;
}

double * CPathLoader::GetConfigure( bool bOutputFrameNumber, int & index )
{
  int dof=CCfg::dof;
  double * currentCfg=new double[dof];
  static double TwoPI=3.1415926535*2.0;
  for(int i=0;i<dof;i++){
    if((i==0) || (i==1)|| (i==2)){
      currentCfg[i]=m_pList[index][i];
    }
    else{
      currentCfg[i]=m_pList[index][i]*TwoPI;
    }
  }

  if( bOutputFrameNumber )
    cout<< "- ChainMaiViewer Mag : Current Frame # = " << index
        << "/"<< m_pList.size()-1 <<endl;
  
  return currentCfg;
}

void CPathLoader::FreePathList()
{
    //not thing I son't need to free~
    if( m_pList.empty() ) return;
    
    for( unsigned int iF=0;iF<m_pList.size();iF++ )
    {
        delete [] m_pList[iF];
    }
    
    m_pList.clear();
}
