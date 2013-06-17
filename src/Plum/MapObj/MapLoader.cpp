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

         ifstream fin(m_filename.c_str());
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
      m_strFileDir=GetPathName(m_filename);

      //Open file for reading data
      string s;

      //get version comment
      GoToNext(in);
      in >> s >> s >> s >> s; //Roadmap Version Number 061300
      m_strVersionNumber = s;

      //get preamble info
      GoToNext(in);
      getline(in, s);
      m_strPreamble = s;

      //get env file name info
      GoToNext(in);
      getline(in, s);
      //see if we need to add directory
      if(s[0] != '/')
         m_strEnvFileName = m_strFileDir + s;
      else
         m_strEnvFileName = s;

      unsigned int number=0;
      //get lp info
      GoToNext( in );
      in >> number; //# of lps
      for( unsigned int iLP=0; iLP<number; iLP++ ) {
         GoToNext(in);
         getline(in, s);
         if(m_strLPs.size() != number){
            m_strLPs.push_back(s);
         }
      }

      //get cd info
      GoToNext( in );
      in >> number; //# of lps
      for( unsigned int iCD=0; iCD<number; iCD++ ) {
         GoToNext(in);
         getline(in, s);
         if( m_strCDs.size() != number){
            m_strCDs.push_back(s);
         }
      }

      //get dm info
      GoToNext( in );
      in >> number;
      for( unsigned int iDM=0; iDM<number; iDM++ ) {
         GoToNext(in);
         getline(in, s);
         if( m_strDMs.size() != number){
            m_strDMs.push_back(s);   
         }
      }

      // June 17-05
      // Supports new version of map generation:
      // ask whether this has the RNGSEEDSTART tag:
      string s_ver = m_strVersionNumber;
      if( s_ver == "041805"){
         GoToNext(in);
         getline(in, s);
         m_seed = s;
      }

      return true;
   }

   //void  CMapHeaderLoader::WriteMapFile(const char *filename){

   bool CMapHeaderLoader::WriteMapFile(){


      //m_Graph->WriteGraph(filename);

      return true;
   }
  

}//end of namespace plum

