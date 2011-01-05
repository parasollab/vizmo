// BoundingBoxParser.cpp: implementation of the CBoundingBoxParser class.
//
//////////////////////////////////////////////////////////////////////

#include "BoundingBoxParser.h"
#include <stdlib.h>
#include <string>
#include <cstring>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBoundingBoxParser::CBoundingBoxParser()
{
    memset(m_BBXValue,0,sizeof(double)*6);
    m_Scale=1;
}

//////////////////////////////////////////////////////////////////////
// Core
//////////////////////////////////////////////////////////////////////
bool CBoundingBoxParser::ParseFile(){

    char * command = (char*)(m_strFileName.c_str());
    if( command==NULL ) return false;

    //get scale
    char * start=strstr(command, "-bbox_scale");
    if( start!=NULL ){
        start+=(strlen("-bbox_scale")+1);
        m_Scale=atof(start);
    }

    //find start and end of "possible" bbx in command
    command=strstr(command, "-bbox");
    if( command==NULL ) return true; //not found
    start=strchr(command, '[');
    if( start==NULL ) return true; //not found
    char * end=strchr(start, ']');
    if( end==NULL ) return true; //not found
    //copy start to end
    char * tmp = new char[end-start];
    if( tmp==NULL ) return true; //not enough memory
    for( int i=0;i<end-start-1;i++ ) tmp[i]=*(start+1+i);

    //get values from string and scale accroding to scale value
    const char del[] = ", \t\n"; int count=0;
    char * token = strtok(tmp,del);
    while( token!=NULL ){
        m_BBXValue[count++]=atof(token)*m_Scale;
        token = strtok(NULL,del);
        if( count==6 ) break;
    }

    if( token!=NULL || count!=6 ) return true; //not correct

    //free
    delete [] tmp;
    return true;
}
