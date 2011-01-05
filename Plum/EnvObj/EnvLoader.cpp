// EnvLoader.cpp: implementation of the CEnvLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "EnvLoader.h"
#include "MultiBodyInfo.h"
#include "MapObj/Cfg.h"

namespace plum{
    
  //////////////////////////////////////////////////////////////////////
  // Construction/Destruction
  //////////////////////////////////////////////////////////////////////
  
  CEnvLoader::CEnvLoader()
  {
    m_cNumberOfMultiBody = 0;
    m_pMBInfo=NULL;
  }
  
  CEnvLoader::~CEnvLoader()
  {
    Free_Memory();
  }
    
  //////////////////////////////////////////////////////////////////////
  // Implemetation of core function
  //////////////////////////////////////////////////////////////////////
  
  bool 
  CEnvLoader::ParseFile( )
  {
    if( CheckCurrentStatus()==false )
      return false;
    
    //Open file for reading datat
    ifstream fin(m_strFileName.c_str(), ios::in);
    
    if( ParseFileHeader(fin)==false ) 
      return false;
    
    if( ParseFileBody(fin)==false ) 
      return false;
    
    fin.close();
    
    return true;
  }
  
  //////////////////////////////////////////////////////////////////////
  //      Protected Member Functions
  //////////////////////////////////////////////////////////////////////
  
  void CEnvLoader::Free_Memory()
  {      
    delete [] m_pMBInfo;
    m_pMBInfo=NULL;
  }
  
  void CEnvLoader::SetModelDataDir( const string strModelDataDir )
  {
    m_strModelDataDir=strModelDataDir;
    cout<<"- Geo Dir   : "<<m_strModelDataDir<<endl;
  }
  
  bool CEnvLoader::ParseFileHeader( ifstream & fin )
  {
    char strLine[150]; //150 is max line length...
    
    //parse
    while( !fin.eof() )
      {
	char c=fin.peek();
	fin.getline(strLine, 149); //read a line

	if( !isCommentLine(c) ){ //if this line is not comment
	  m_cNumberOfMultiBody = atoi( strLine ); //total number of multiBodies
	  break;
	}       
      }
    
    return true;
  }
  
  bool CEnvLoader::ParseFileBody( ifstream & fin )
  {
    m_pMBInfo = new CMultiBodyInfo[m_cNumberOfMultiBody];
    if( m_pMBInfo==NULL ) return false;
    
    for( int iM=0; iM<m_cNumberOfMultiBody; iM++ )
      {
	if( ParseMultiBody(fin, m_pMBInfo[iM])==false )
	  return false;
      }
    
    return true;
  }
  
  bool CEnvLoader::ParseMultiBody( ifstream & fin, CMultiBodyInfo & MBInfo )
  {
    
    char strData[150];
    //int dof = 6;
    int dof = 0;
    bool bActive=false;
    
    GoToNext(fin);
    fin >> strData; /*Tag, "MultiBody"*/ 
    fin >> strData; //Tag, "Active/Passive"

    if( strData[0]=='A'||strData[0]=='a'){
      bActive=true;
      MBInfo.m_active = true;
      
    }

    fin >> MBInfo.m_cNumberOfBody;  /* number of body in this multibody */
     
    MBInfo.m_pBodyInfo = new CBodyInfo[MBInfo.m_cNumberOfBody];
    if( MBInfo.m_pBodyInfo==NULL ) return false;

    getColor(fin);
       
    for( int iB=0; iB<MBInfo.m_cNumberOfBody; iB++ )
      {
	if( ParseBody(fin, MBInfo.m_pBodyInfo[iB])==false )
	  return false;

	if((iB == 0)&&(!MBInfo.m_pBodyInfo[0].m_bIsFixed))
	  dof = 6;
      }
    
    
    ////////////////////////////////////////////////////////
    //get connection info
    ////////////////////////////////////////////////////////
           
    GoToNext(fin);
    fin.getline(strData, MAX_LINE_LENGTH ); //tag Connection
    int numberOfRobotConnections;
    fin >> numberOfRobotConnections; /* Number of Connections (links) */
    MBInfo.m_NumberOfConnections = numberOfRobotConnections;

    if( bActive ){ //if is robot
      dof+=numberOfRobotConnections;
      CCfg::dof=dof;
      cout<< "DOF's: "<<CCfg::dof<<endl; 
      DoF = dof;
   }

    //Each body can have more than one connection
    //Get connection info if numberOfRobotConnection != 0 

    int currentBody = 0; //index of current Body
    //int nextBody = 0; //indext of next Body

    //Do transformation for body0, the base. This is always needed.	 
    MBInfo.m_pBodyInfo[currentBody].doTransform();
 
    if( numberOfRobotConnections!=0 ){
      
      //initialize the MBInfo.m_pBodyInfo[i].m_pConnectionInfo for each body
      
      for(int i=0; i<MBInfo.m_cNumberOfBody; i++){
	MBInfo.m_pBodyInfo[i].m_pConnectionInfo = 
	  new CConnectionInfo[numberOfRobotConnections];
	
	if( MBInfo.m_pBodyInfo[i].m_pConnectionInfo==NULL ) { 
	  cout<<"COULDN'T CREATE CONNECTION INFO"<<endl; return false;}
      }
      for( int iB=0; iB<numberOfRobotConnections; iB++ ){
	if( ParseConnections(fin, MBInfo.m_pBodyInfo)==false )
	  return false;
	MBInfo.listConnections.push_back(pair<int,int>(preIndx,nextIndx));
      }
    }
    
    return true;
    
  }
  
  bool CEnvLoader::ParseBody( ifstream & fin, CBodyInfo & BodyInfo )
  {
    char strData[150];
    //char tmp[150];
    static double PI_180=3.1415926535/180;
    //static double PI_180 = 3.1415926535*2;
#ifdef WIN32
    string sep="\\"; //path seperator
#else
    string sep="/";
#endif

    GoToNext(fin);
    
    //Get Body Type
    fin>>strData;
    
    if( strcmp(strData, "FixedBody")==0 )
      BodyInfo.m_bIsFixed = true;
    else
      BodyInfo.m_bIsFixed = false;
    
    //get body index
    fin >> BodyInfo.m_Index;

    //set "m_IsBase" var. to true if m_Index==0
    if (BodyInfo.m_Index ==0)
      BodyInfo.m_IsBase = true;
    
    //get data file name
    fin >> strData;
    //store name of *.g file and its subdirectory (if exists)
    BodyInfo.m_strFileName = strData;
    
    if( !m_strModelDataDir.empty() ) {
      //store just the path of the current directory
      BodyInfo.m_strDirectory = m_strModelDataDir;

      BodyInfo.m_strModelDataFileName+=m_strModelDataDir;
      BodyInfo.m_strModelDataFileName+=sep;
    }
    BodyInfo.m_strModelDataFileName+=strData;
    
    //if Body0 then read orientation and position 
    //else put 0's for the rest of the Bodies if they exist
    if(BodyInfo.m_Index ==0){
    fin >> BodyInfo.m_X >> BodyInfo.m_Y >> BodyInfo.m_Z;
    fin >> BodyInfo.m_Alpha >> BodyInfo.m_Beta >> BodyInfo.m_Gamma;
    }
    else{
    BodyInfo.m_X = BodyInfo.m_Y = BodyInfo.m_Z = BodyInfo.m_Alpha = BodyInfo.m_Beta = BodyInfo.m_Gamma = 0;
    }
          
    //convert to radians	 
    BodyInfo.m_Alpha=BodyInfo.m_Alpha*PI_180;
    BodyInfo.m_Beta=BodyInfo.m_Beta*PI_180;
    BodyInfo.m_Gamma=BodyInfo.m_Gamma*PI_180;

    //set color information
    if(m_rgb[0] != -1){
      BodyInfo.rgb[0]=m_rgb[0];BodyInfo.rgb[1]=m_rgb[1];BodyInfo.rgb[2]=m_rgb[2];
    }
    else{
      if(BodyInfo.m_bIsFixed){
	BodyInfo.rgb[0]=0.5;BodyInfo.rgb[1]=0.5;BodyInfo.rgb[2]=0.5;
      }
      else{
	BodyInfo.rgb[0]=1;BodyInfo.rgb[1]=0;BodyInfo.rgb[2]=0;
      }
    }	
      
    GoToNext(fin);
    
    return true;
  }
  

  bool CEnvLoader::ParseConnections(ifstream & fin, CBodyInfo *BodyInfo )
  {
    char strData[150];
    static double PI_180=3.1415926535/180;
    preIndx=nextIndx=-1;
    int conn = 0;
    
    fin >> preIndx >> nextIndx;
 
    // Tag, "Actuated/NonActuated"
    fin >> strData; 

    //Increment m_cNumberOfConnection for each body
    
    BodyInfo[preIndx].m_cNumberOfConnection++;
    
    //Get connection index for this body
    conn = BodyInfo[preIndx].m_cNumberOfConnection - 1;
    
    //set next and pre index
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_preIndex = preIndx;
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_nextIndex = nextIndx;

    //set Actuated/NonActuated

    if( strcmp(strData, "Actuated")==0 )
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_actuated = true;
    else
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_actuated = false;
    
    //poisition
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].m_posX >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_posY >> 
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_posZ;
 
    
    //orientation
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientX >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientY >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientZ;
    
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientX = 
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientX*PI_180;
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientY = 
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientY*PI_180;
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientZ = 
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orientZ*PI_180;

    //DH parameters
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].alpha >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].a >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].d >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].theta;
    //save original theta   
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_theta =
      BodyInfo[preIndx].m_pConnectionInfo[conn].theta;

    // Tag, "Revolute" or "Prismatic"
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].m_articulation;
    
    //poisiton DH frame
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].m_pos2X >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_pos2Y >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_pos2Z;

    //orientation DH frame
    fin >> BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2X >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Y >>
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Z;
    
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2X = 
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2X*PI_180;
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Y =
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Y*PI_180;
    BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Z =
      BodyInfo[preIndx].m_pConnectionInfo[conn].m_orient2Z*PI_180;
    
    return true;
  }


  void CEnvLoader::SetNewMultiBodyInfo(CMultiBodyInfo * mbi){
    for(int i=0; i<m_cNumberOfMultiBody; i++){
     
      m_pMBInfo[i] = mbi[i];
    }
  
  }

}//namespace plum

