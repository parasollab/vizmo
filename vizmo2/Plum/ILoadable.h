///////////////////////////////////////////////////////////////////////////////////////////
// I_Loadable is an interface file for all model loader

#ifndef _I_LOADABLE_H_
#define _I_LOADABLE_H_

#ifdef WIN32
#pragma warning( disable : 4244 4786 )
#endif 

#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#define MAX_LINE_LENGTH 500

namespace plum{
    
    class
        I_Loadable
    {
    public:
        
    /**
    * Constructor/Destructor
        */
        I_Loadable(){ m_strFileName=""; }
        virtual ~I_Loadable(){}
        
        /**
        * The filename that is going to be load in
        */
        virtual void SetDataFileName( const string szFileName )
        {           
            m_strFileName=szFileName;
        }
        
        /**
        * Parse given file.
        */
        virtual bool ParseFile()=0;
        
        /**
        * Check if given file name is valid
        */
        virtual bool CheckCurrentStatus()
        {           
            //Check if file exist
            ifstream fin(m_strFileName.c_str(), ios::in);
            if( !fin.good() )
            {   //not good. file not found
                cerr<<"File ("<< m_strFileName <<") not found";
                return false;
            }
            
            fin.close();
            
            return true;
        }
        
        string GetFileName() const { return m_strFileName; }
        
    protected:
        
    /**
    * discard all commanded lines util the umcommanded line found
        */
        void GoToNext(istream & in)
        {
            char line[400];
            while( !in.eof() )
            {
                char c=in.peek(); //read first character
                if( !isCommentLine(c) ){ //if this line is not comment
                    return;
                }
                in.getline(line,399);
            }
        }
        
        /**
        * is given char a start of command line?
        */
        bool isCommentLine( const char c) const
        {           
            //a line starts with #
            if( c=='#' ) return true;
            
            //a line has whitespace only
            if( isspace(c) ) return true;
            
            //none of above
            return false;
        }
        
        /**
        * getPathName from given filename
        */
        string getPathName( const string filename )
        {
            char sep='/';
            unsigned int pos=filename.rfind(sep);
            if( pos==string::npos ) return string(); //empty
            string pathname(filename.begin(),filename.begin()+pos+1);
            
            return pathname;
        }
        
        
        //the file name that is going to be load
        string m_strFileName;
    };
    
}//namespace plum

#endif //_I_LOADABLE_H_
