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

double * CPathLoader::GetConfiguration(int frame)
{
	if(frame>=(int)m_pList.size()) frame=m_pList.size()-1;
	if(frame<0 ) frame=0;
		
    double * currentCfg = GetConfigure(false, frame);    
    return currentCfg;
}

double * CPathLoader::GetConfigure( bool bOutputFrameNumber, int & index )
{
    double * currentCfg=new double[6];
    currentCfg[0]=m_pList[index][0];
    currentCfg[1]=m_pList[index][1];
    currentCfg[2]=m_pList[index][2];
    currentCfg[3]=m_pList[index][3]*6.2831852;
    currentCfg[4]=m_pList[index][4]*6.2831852;
    currentCfg[5]=m_pList[index][5]*6.2831852;
    
    if( bOutputFrameNumber )
        cout<< "- ChainMaiViewer Mag : Currnet Frame # = " << index
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
