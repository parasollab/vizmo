// CmdParser.cpp: implementation of the CCmdParser class.
//
//////////////////////////////////////////////////////////////////////

#include "CmdParser.h"
#include <stdlib.h>
#include <string>
#include <strstream>
#include <iostream>
namespace plum{
    
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    
    CCmdParser::CCmdParser()
    {
        m_strModelDataDir="";
    }
    
    bool 
        CCmdParser::ParseCmd( const string & dir, const string & cmd )
    {
        m_strModelDataDir=dir;
        //retrun true if found
        if( FindModelDataDir( cmd )==false ) return true;
        
        //combine map dir and model dir to a full path name
        m_strModelDataDir=dir+m_strModelDataDir;
        
        return true;
    }
    
    bool
        CCmdParser::FindModelDataDir( string cmd )
    {
        unsigned int pos = cmd.find("-descDir");
        if( pos==string::npos ) return false;
        string sub=cmd.substr(pos+8);
        istrstream is(sub.c_str());
        is>>m_strModelDataDir;
        return true;
    }
    
}//namespace plum{

