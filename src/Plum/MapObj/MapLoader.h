// MapLoader.h: interface for the CMapLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAPLOADER_H_)
#define _MAPLOADER_H_

#include "Plum/Loadable.h"
#include <graph.h>
#include <algorithms/graph_algo_util.h>
#include <algorithms/graph_input_output.h>
#include <string>
using namespace std;
using namespace stapl;
namespace plum{

    //This class Responsilbe for load header part of map file
    class CMapHeaderLoader : public Loadable
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

	//void WriteMapFile(const char *filename);
	bool WriteMapFile();
        
        const list<string> & GetLPs() const { return m_strLPs; }
        const list<string> & GetCDs() const { return m_strCDs; }
        const list<string> & GetDMs() const { return m_strDMs; }
	const string GetSeed() const {return m_seed; }

        //////////////////////////////////////////////////////////////////////
        //      Protected Methods and data members
        //////////////////////////////////////////////////////////////////////
    protected:
        bool ParseHeader( istream & in );

        //////////////////////////////////////////////////////////////////////
        //      Private Methods and data members
        //////////////////////////////////////////////////////////////////////
	//private:
	public:
        string  m_strVersionNumber;
        string  m_strPreamble;
        string  m_strEnvFileName;
        string  m_strFileDir;
        list<string> m_strLPs;
        list<string> m_strCDs;
        list<string> m_strDMs;
	string m_seed;
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
        //WeightedMultiDiGraph<Cfg,WEIGHT> * GetGraph() { return m_Graph; }
        typedef graph<DIRECTED,MULTIEDGES,Cfg,WEIGHT> Wg;
        typedef typename Wg::vertex_descriptor VID;
        typedef typename Wg::vertex_iterator VI;
	
	
        Wg * GetGraph() { return m_Graph; }
        void InitGraph() {m_Graph = new Wg();}
        //void KillGraph(){ delete m_Graph; m_Graph=NULL; }
        VID Cfg2VID(Cfg target){
          VI vi;
          VID tvid = -1;
          for(vi=GetGraph()->begin();vi!=GetGraph()->end();vi++){
            if( target == (*vi).property() ){
              tvid=(*vi).descriptor();   
              break;
            }
         
          }
          return tvid;
        }

	//////////////////////////////////////////////////////////
	// Called from VizmoRoadmapGUI::handleAddNode()
	// when no roadmap exists....
	//////////////////////////////////////////////////////////
	void genGraph();

        //////////////////////////////////////////////////////////////////////
        //      Protected Methods and data members
        //////////////////////////////////////////////////////////////////////
    protected:        
        Wg * m_Graph;
                
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
        //KillGraph();
		delete m_Graph; m_Graph=NULL;
    }

    //////////////////////////////////////////////////////////////////////
    // Implemetation of core function
    //////////////////////////////////////////////////////////////////////
    template<class Cfg, class WEIGHT>
      bool CMapLoader<Cfg, WEIGHT>::ParseFile() {      
        if(!CheckCurrentStatus())
          return false;

        ifstream fin(m_filename.c_str());
        if(!CMapHeaderLoader::ParseHeader(fin)) 
          return false;

        //Get Graph Data
        string s;
        getline(fin, s);
        m_Graph =new Wg();
        if(m_Graph==NULL){ 
          cout<<"Graph null ..."<<endl;
          return false; 
        }
        read_graph(*m_Graph, fin);

        return true;
      }

    //called from VizmoRoadmapGUI::handleAddNode()
    template< class Cfg, class WEIGHT > void
      CMapLoader<Cfg, WEIGHT>::genGraph()
      {
	m_Graph =new Wg();
      }


}//namespace plum

#endif // !defined(_MAPLOADER_H_)
