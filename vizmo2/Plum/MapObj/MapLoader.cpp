// MapLoader.cpp: implementation of the CMapLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "MapLoader.h"

namespace plum {

    /*********************************************************************
    *
    *      Implementation of CMapHeaderLoader
    *
    *********************************************************************/
    CMapHeaderLoader::CMapHeaderLoader()
    {
        //do nothing yet
    }


    bool
    CMapHeaderLoader::ParseHeader()
    {
        if( CheckCurrentStatus()==false )
            return false;

        ifstream fin(m_strFileName.c_str(), ios::in);
        bool result = ParseHeader( fin );
        fin.close();
        return result;
    }

    //////////////////////////////////////////////////////////////////////
    //      Protected Member Functions
    //////////////////////////////////////////////////////////////////////

    bool 
    CMapHeaderLoader::ParseHeader( istream & in )
    {
        //get path name
        m_strFileDir=getPathName(m_strFileName);

        //Open file for reading datat
        char strData[MAX_LINE_LENGTH+1]="";

        //get version comment
        GoToNext( in );
        in >> strData >> strData >> strData >> strData; //Roadmap Version Number 061300
        m_strVersionNumber=strData;
        
        //get preamble info
        GoToNext( in );
        in.getline(strData, MAX_LINE_LENGTH);
        m_strPreamble=strData;
        
        //get env file name info
        GoToNext( in );
        in.getline(strData, MAX_LINE_LENGTH);
        m_strEnvFileName = m_strFileDir+strData;
        
        int number=0;
        //get lp info
        GoToNext( in );
        in >> number; //# of lps
        for( int iLP=0; iLP<number; iLP++ ) {
            GoToNext( in );
            in.getline(strData, MAX_LINE_LENGTH);
            m_strLPs.push_back(strData);   
        }
        
        //get cd info
        GoToNext( in );
        in >> number; //# of lps
        for( int iCD=0; iCD<number; iCD++ ) {
            GoToNext( in );
            in.getline(strData, MAX_LINE_LENGTH);
            m_strCDs.push_back(strData);
        }
        
        //get dm info
        GoToNext( in );
        in >> number;
        for( int iDM=0; iDM<number; iDM++ ) {
            GoToNext( in );
            in.getline(strData, MAX_LINE_LENGTH);
            m_strDMs.push_back(strData);   
        }

        return true;
    }

}//end of namespace plum

