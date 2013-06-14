#include "MovieBYULoader.h"
#include <iostream>

namespace plum{
    
    //////////////////////////////////////////////////////////////////////////////////////
    // Implemetation of ILoadable interface
    //////////////////////////////////////////////////////////////////////////////////////
    bool 
        CMovieBYULoader::ParseFile()
    {
        if( CheckCurrentStatus()==false )
            return false;
        
        //Open file for reading datat
        ifstream fin(m_filename.c_str());
        
        if( ParseSection1(fin)==false )
            return false;
        if( ParseSection2(fin)==false )
            return false;
        if( ParseSection3(fin)==false )
            return false;
        
        fin.close();
        return true;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  Private Methods
    //
    ////////////////////////////////////////////////////////////////////////////////////////////
    bool CMovieBYULoader::ParseSection1(ifstream & in)
    {
        in>>m_PartsSize>>m_VertexSize>>m_PolygonSize>>m_EdgeSize;
        parts.reserve(m_PartsSize);
        
        for(int i=0;i<m_PartsSize;i++)
        {
            pair<int,int> range;
            in>>range.first>>range.second;
            parts.push_back(range);
        }
        return true;
    }
    
    bool CMovieBYULoader::ParseSection2(ifstream & in)
    {
        points.reserve(m_VertexSize);
        
        for(int i=0;i<m_VertexSize;i++)
        {
            Point3d pt;
            in>>pt;
            points.push_back(pt);
        }
        return true;
    }
    
    bool CMovieBYULoader::ParseSection3(ifstream & in)
    {
        
        /////////////////////////////////////////////////////////////////////////////////////
        for(int i=0;i<m_PolygonSize;i++)
        {
            int id=0;
            vector<int> index;
            do{
                in>>id; index.push_back(id);
            }while(id>=0);
            if( in.eof() ){
                cerr<<"! Error "<<m_filename<<" : Not enough triangle defined."<<endl;
                cerr<<i-1<<" triangles found instead of "<<m_PolygonSize<<" triangles"<<endl;
                exit(1);
            }
            
            int size=index.size();
            if( size!=3 ){
                cerr<<"! Error "<<m_filename<<" : Please trianglate model first"<<endl;
                exit(1);
            }
            
            index[2]=-index[2];
            Tri tri(index[0]-1,index[1]-1,index[2]-1);
            triangles.push_back(tri);
        }
        
        return true;
    }
    
}// namespace plum

