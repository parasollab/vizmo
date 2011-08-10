// PathLoader.cpp: implementation of the CDebugLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "DebugLoader.h"
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugLoader::CDebugLoader()
{
  //m_mapLoader = new CMapLoader<CCfg, CSimpleEdge>();
}

CDebugLoader::~CDebugLoader()
{
    FreeDebugList();
}       

bool CDebugLoader::ParseFile()
{
    //check input
    if( CheckCurrentStatus()==false ){
		return true;
    }

    ifstream fin(m_strFileName.c_str());

    //reset every thing
    FreeDebugList();
   
    m_iList.push_back(new Default());
    /////////////////////////////////////////////////////////////////////////////////////////
    //Read file info
    string line;
    do{
      line = "";
      getline(fin,line);
      istringstream iss(line);
      string instructionName;
      iss>>instructionName;
      if(instructionName=="AddNode"){
        CCfg c;
        iss>>c;
        c.SetColor(((float)rand())/RAND_MAX,((float)rand())/RAND_MAX, ((float)rand())/RAND_MAX,1);
        m_iList.push_back(new AddNode(c));
      }
      else if(instructionName=="AddEdge"){
        CCfg s, t;
        iss>>s>>t;
        m_iList.push_back(new AddEdge(s,t));
      }
      else if(instructionName=="AddTempCfg"){
        CCfg c;
        bool valid;
        iss>>c>>valid;
        if(valid) c.SetColor(0,0,0,0.25);
        else c.SetColor(1,0,0,0.25);
        m_iList.push_back(new AddTempCfg(c, valid));
      }
      else if(instructionName=="AddTempRay"){
        CCfg c;
        iss>>c;
        m_iList.push_back(new AddTempRay(c));
      }
      else if(instructionName=="AddTempEdge"){
        CCfg s, t;
        iss>>s>>t;
        m_iList.push_back(new AddTempEdge(s,t));
      }
      else if(instructionName=="ClearAll"){
        m_iList.push_back(new ClearAll());
      }
      else if(instructionName=="ClearLastTemp"){
        m_iList.push_back(new ClearLastTemp());
      }
      else if(instructionName=="ClearComments"){
        m_iList.push_back(new ClearComments());
      }
      else if(instructionName=="RemoveNode"){
        CCfg c;
        iss>>c;
        m_iList.push_back(new RemoveNode(c));
      }
      else if(instructionName=="RemoveEdge"){
        CCfg s, t;
        iss>>s>>t;
        m_iList.push_back(new RemoveEdge(s,t));
      }
      else if(instructionName=="Comment"){
        m_iList.push_back(new Comment(iss.str().substr(8,iss.str().length())));
      }
      else if(instructionName=="Query"){
        CCfg s, t;
        iss>>s>>t;
        m_iList.push_back(new Query(s,t));
      }
    }while(line!="");
   
    cout<<"Done loading debug file"<<endl;

    /*for(size_t i = 0; i!=m_iList.size(); i++){
      m_iList[i].first->Print(cout);
    }*/

    /////////////////////////////////////////////////////////////////////////////////////////
    //bye bye!!
    fin.close();
    return true;
}

FRAME CDebugLoader::ConfigureFrame(int frame)
{
	if(frame>=(int)m_iList.size()) frame=m_iList.size()-1;
	if(frame<0 ) frame=0;
	return GetFrame(false, frame);    
}

FRAME CDebugLoader::GetFrame( bool bOutputFrameNumber, int index )
{
  if( bOutputFrameNumber )
    cout<< "- ChainMaiViewer Mag : Current Frame # = " << index
        << "/"<< m_iList.size()-1 <<endl;

  return m_iList[index];
}

void CDebugLoader::FreeDebugList()
{
    //not thing I son't need to free~
    if( m_iList.empty() ) return;
    
    for( unsigned int iF=0;iF<m_iList.size();iF++ )
    {
        delete m_iList[iF];
    }
    
    m_iList.clear();
}
