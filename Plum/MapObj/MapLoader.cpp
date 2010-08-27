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

   bool CMapHeaderLoader::ParseHeader( istream & in )
   {
      //get path name
      m_strFileDir=getPathName(m_strFileName);

      //Open file for reading data
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
      //see if we need to add directory
      if( strData[0] != '/')
         m_strEnvFileName = m_strFileDir+strData;
      else
         m_strEnvFileName = strData;

      int number=0;
      //get lp info
      GoToNext( in );
      in >> number; //# of lps
      for( int iLP=0; iLP<number; iLP++ ) {
         GoToNext( in );
         in.getline(strData, MAX_LINE_LENGTH);
         if( m_strLPs.size() != number){
            m_strLPs.push_back(strData);   
         }
      }

      //get cd info
      GoToNext( in );
      in >> number; //# of lps
      for( int iCD=0; iCD<number; iCD++ ) {
         GoToNext( in );
         in.getline(strData, MAX_LINE_LENGTH);
         if( m_strCDs.size() != number){
            m_strCDs.push_back(strData);
         }
      }

      //get dm info
      GoToNext( in );
      in >> number;
      for( int iDM=0; iDM<number; iDM++ ) {
         GoToNext( in );
         in.getline(strData, MAX_LINE_LENGTH);
         if( m_strDMs.size() != number){
            m_strDMs.push_back(strData);   
         }
      }

      // June 17-05
      // Supports new version of map generation:
      // ask whether this has the RNGSEEDSTART tag:
      string s_ver = m_strVersionNumber;
      if( s_ver == "041805"){
         GoToNext( in );
         in.getline(strData, MAX_LINE_LENGTH);
         m_seed = strData;
      }

      return true;
   }

   //void  CMapHeaderLoader::WriteMapFile(const char *filename){

   bool CMapHeaderLoader::WriteMapFile(){


      //m_Graph->WriteGraph(filename);

      return true;
   }

}//end of namespace plum

