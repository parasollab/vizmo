// PathLoader.cpp: implementation of the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "PathLoader.h"
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPathLoader::CPathLoader()
{
    m_iCurrent_CfgIndex=0;
    notExists = false;  // used to know if there is .path file available -- 
}

CPathLoader::~CPathLoader()
{
    FreePathList();
}       

bool CPathLoader::ParseFile()
{
    //check input
    if( CheckCurrentStatus()==false ){
      notExists=true;
      return true;
    }

    ifstream fin(m_strFileName.c_str(), ios::in);

    //reset every thing
    FreePathList();
    m_iCurrent_CfgIndex=0;
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //Read file info
    char garbige[100];
    fin.getline(garbige,99);        //the garbege of path file
    fin.getline(garbige,99);
    
    unsigned int iPathSize=0;
    fin>>iPathSize;
    for( unsigned int iF=0;iF<iPathSize;iF++ )
    {
        double * dCfg=new double[6];
        fin>>dCfg[0]>>dCfg[1]>>dCfg[2]>>dCfg[3]>>dCfg[4]>>dCfg[5];
        m_pList.push_back(dCfg);
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    //bye bye!!
    fin.close();
    return true;
}

double * CPathLoader::GetNextConfigure( bool bOutputFrameNumber )
{
    double * currentCfg = GetConfigure(bOutputFrameNumber, m_iCurrent_CfgIndex);
    m_iCurrent_CfgIndex++;
    if( (unsigned int)m_iCurrent_CfgIndex>=m_pList.size() )
        m_iCurrent_CfgIndex=0;
    
    return currentCfg;
}

double * CPathLoader::GetPreviousConfigure( bool bOutputFrameNumber )
{
    double * currentCfg = GetConfigure(bOutputFrameNumber, m_iCurrent_CfgIndex);
    m_iCurrent_CfgIndex--;
    if( m_iCurrent_CfgIndex<0 )
        m_iCurrent_CfgIndex=m_pList.size()-1;
    
    return currentCfg;
}

double * CPathLoader::GetParticularConfiguration(int index)
{
  
  if((index>=m_pList.size() || index<0))
     return GetConfigure(false,index);

    m_iCurrent_CfgIndex=index;

    double * currentCfg = GetConfigure(true, m_iCurrent_CfgIndex);
   
    
    
    return currentCfg;
}

double * CPathLoader::GetConfigure( bool bOutputFrameNumber, int & index )
{
    double * currentCfg=new double[6];
    currentCfg[0]=m_pList[m_iCurrent_CfgIndex][0];
    currentCfg[1]=m_pList[m_iCurrent_CfgIndex][1];
    currentCfg[2]=m_pList[m_iCurrent_CfgIndex][2];
    currentCfg[3]=m_pList[m_iCurrent_CfgIndex][3]*6.2831852;
    currentCfg[4]=m_pList[m_iCurrent_CfgIndex][4]*6.2831852;
    currentCfg[5]=m_pList[m_iCurrent_CfgIndex][5]*6.2831852;
    
    if( bOutputFrameNumber )
        cout<< "- ChainMaiViewer Mag : Currnet Frame # = " << m_iCurrent_CfgIndex
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
