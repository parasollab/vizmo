// MapLoader.h: interface for the CMapLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPLOADER_H_)
#define _MAPLOADER_H_

#include "ILoadable.h"
#include <Graph.h>
#include <string>
using namespace std;

namespace plum{

    //This class Responsilbe for load header part of map file
    class CMapHeaderLoader : public I_Loadable
    {
    public:
        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapHeaderLoader();

        //////////////////////////////////////////////////////////////////////
        // Core function
        //////////////////////////////////////////////////////////////////////
        virtual bool ParseFile(){ return ParseHeader(); }
        bool ParseHeader(); //Read the header part of map file only

        //////////////////////////////////////////////////////////////////////
        // Access function
        //////////////////////////////////////////////////////////////////////
        const string  GetVersionNumber() const { return m_strVersionNumber; }
        const string  GetPreamble()      const { return m_strPreamble; }
        const string  GetEnvFileName()   const { return m_strEnvFileName; }
        const string  GetFileDir()  const { return m_strFileDir; }
        
        const list<string> & GetLPs() const { return m_strLPs; }
        const list<string> & GetCDs() const { return m_strCDs; }
        const list<string> & GetDMs() const { return m_strDMs; }

        //////////////////////////////////////////////////////////////////////
        //      Protected Methods and data members
        //////////////////////////////////////////////////////////////////////
    protected:
        bool ParseHeader( istream & in );

        //////////////////////////////////////////////////////////////////////
        //      Private Methods and data members
        //////////////////////////////////////////////////////////////////////
    private:
        string  m_strVersionNumber;
        string  m_strPreamble;
        string  m_strEnvFileName;
        string  m_strFileDir;
        list<string> m_strLPs;
        list<string> m_strCDs;
        list<string> m_strDMs;
    };

    template< class Cfg, class WEIGHT >
    class CMapLoader : public CMapHeaderLoader 
    {
    public:

        //////////////////////////////////////////////////////////////////////
        // Constructor/Destructor
        //////////////////////////////////////////////////////////////////////
        CMapLoader();
        virtual ~CMapLoader();
        
        //////////////////////////////////////////////////////////////////////
        // Core function
        //////////////////////////////////////////////////////////////////////
        virtual bool ParseFile();   //read in map file
        
        //////////////////////////////////////////////////////////////////////
        // Access function
        //////////////////////////////////////////////////////////////////////
        //these two functions are only accessed from CMapModeler
        WeightedMultiDiGraph<Cfg,WEIGHT> * GetGraph() { return m_Graph; }
        void KillGraph(){ delete m_Graph; m_Graph=NULL; }
        
        //////////////////////////////////////////////////////////////////////
        //      Protected Methods and data members
        //////////////////////////////////////////////////////////////////////
    protected:        
        WeightedMultiDiGraph< Cfg, WEIGHT> * m_Graph;
    };

    /*********************************************************************
    *
    *      Implementation of CMapLoader
    *
    *********************************************************************/

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    template< class Cfg, class WEIGHT >
    CMapLoader<Cfg, WEIGHT>::CMapLoader()
    {
        m_Graph=NULL;
    }

    template< class Cfg, class WEIGHT >
    CMapLoader<Cfg, WEIGHT>::~CMapLoader()
    {
        KillGraph();
    }

    //////////////////////////////////////////////////////////////////////
    // Implemetation of core function
    //////////////////////////////////////////////////////////////////////
    template< class Cfg, class WEIGHT > bool 
    CMapLoader<Cfg, WEIGHT>::ParseFile()
    {        
        if( CheckCurrentStatus()==false )
            return false;

        ifstream fin(m_strFileName.c_str(), ios::in);
        if( CMapHeaderLoader::ParseHeader( fin )==false ) 
            return false;

        //Get Graph Data
        char strData[MAX_LINE_LENGTH];
        fin.getline(strData, MAX_LINE_LENGTH);
        m_Graph =new WeightedMultiDiGraph< Cfg, WEIGHT>();
        if( m_Graph==NULL ) return false;
        m_Graph->ReadGraph( fin );
        
        fin.close();        
        return true;
    }

}//namespace plum

#endif // !defined(_MAPLOADER_H_)
