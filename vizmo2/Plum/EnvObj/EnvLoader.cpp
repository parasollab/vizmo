// EnvLoader.cpp: implementation of the CEnvLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "EnvLoader.h"
#include "MultiBodyInfo.h"

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
                m_cNumberOfMultiBody = atoi( strLine );
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
    
    bool
        CEnvLoader::ParseMultiBody( ifstream & fin, CMultiBodyInfo & MBInfo )
    {
        char strData[150];
        
        GoToNext(fin);
        fin >> strData; /*Tag, "MultiBody"*/ fin >> strData; //Tag, "Active/Passive"
        fin >> MBInfo.m_cNumberOfBody;  /* number of body in this multibody */
        
        MBInfo.m_pBodyInfo = new CBodyInfo[MBInfo.m_cNumberOfBody];
        if( MBInfo.m_pBodyInfo==NULL ) return false;
        
        for( int iB=0; iB<MBInfo.m_cNumberOfBody; iB++ )
        {
            if( ParseBody(fin, MBInfo.m_pBodyInfo[iB])==false )
                return false;
        }
        return true;
    }
    
    bool
        CEnvLoader::ParseBody( ifstream & fin, CBodyInfo & BodyInfo )
    {
        char strData[150];
		static double PI_180=3.1415926/180;
#ifdef WIN32
        string sep="\\"; //path seperator
#else
        string sep="/";
#endif
        
        //Get Body Type
        GoToNext(fin);
        fin>>strData;
        if( strcmp(strData, "FixedBody")==0 )
            BodyInfo.m_bIsFixed = true;
        else
            BodyInfo.m_bIsFixed = false;
        
        //get body index
        fin >> BodyInfo.m_Index;
        
        //get data file name
        fin >> strData;
        if( !m_strModelDataDir.empty() ) {
            BodyInfo.m_strModelDataFileName+=m_strModelDataDir;
            BodyInfo.m_strModelDataFileName+=sep;
        }
        BodyInfo.m_strModelDataFileName+=strData;
        
        //get orientation and position
        fin >> BodyInfo.m_X >> BodyInfo.m_Y >> BodyInfo.m_Z;
        fin >> BodyInfo.m_Alpha >> BodyInfo.m_Beta >> BodyInfo.m_Gamma;
		//convert to radian
        BodyInfo.m_Alpha=BodyInfo.m_Alpha*PI_180;
		BodyInfo.m_Beta=BodyInfo.m_Beta*PI_180;
		BodyInfo.m_Gamma=BodyInfo.m_Gamma*PI_180;

        //get connection info
        GoToNext(fin);
        fin.getline(strData, MAX_LINE_LENGTH ); //tag Connection
        fin >> BodyInfo.m_cNumberOfConnection;
        
        //Get connection info if BodyInfo.m_cNumberOfConnection != 0 
        if( BodyInfo.m_cNumberOfConnection!=0 ){
            BodyInfo.m_pConnectionInfo = 
                new CConnectionInfo[BodyInfo.m_cNumberOfConnection];
            if( BodyInfo.m_pConnectionInfo==NULL ) return false;
            
            for( int iC=0; iC<BodyInfo.m_cNumberOfConnection; iC++ )
            {
                if( ParseConnection(fin, BodyInfo.m_pConnectionInfo[iC])==false )
                    return false;
            }
        }
        
        return true;
    }
    
    bool
        CEnvLoader::ParseConnection(ifstream & fin, CConnectionInfo & ConnectionInfo )
    {
        char strData[150];
        
        fin >> ConnectionInfo.m_preIndex >> ConnectionInfo.m_nextIndex;
        fin >> strData;  // Tag, "Actuated/NonActuated"
        
        //following line just read data and throw it away
        fin >> strData >> strData >> strData  //poisiton
            >> strData >> strData >> strData  //orientation
            >> strData >> strData >> strData >> strData //DH parameter
            >> strData // Tag, "Revolute" or "Prismatic"
            >> strData >> strData >> strData  //poisiton
            >> strData >> strData >> strData; //orientation
        
        return true;
    }
    
}//namespace plum

