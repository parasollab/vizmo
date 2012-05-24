// EnvLoader.h: interface for the CEnvLoader class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ENVLOADER_H_
#define ENVLOADER_H_

#include "ILoadable.h"
#include "RobotInfo.h"
#include <graph.h>
#include <algorithms/graph_input_output.h>
#include <algorithms/connected_components.h>

namespace plum{

  class CMultiBodyInfo;
  class CBodyInfo;
  class CConnectionInfo;
  
  /**
   * This class load env file.
   */
  class CEnvLoader : public I_Loadable
    {
      //////////////////////////////////////////////////////////////////////
      //
      // Constructor/Destructor
      //
      //////////////////////////////////////////////////////////////////////
    public:
      CEnvLoader();
      virtual ~CEnvLoader();
      
      //////////////////////////////////////////////////////////////////////
      // Core function
      //////////////////////////////////////////////////////////////////////
      virtual bool ParseFile();
      virtual void SetModelDataDir( const string strModelDataDir );
      
      //////////////////////////////////////////////////////////////////////
      // Access function
      //////////////////////////////////////////////////////////////////////
      virtual int GetNumberOfMultiBody() const{ return m_cNumberOfMultiBody; }
      virtual const CMultiBodyInfo * GetMultiBodyInfo() const { return m_pMBInfo; }
      
      
      // function for the tree control... returns the CBodyInfo for the ith body
      
      string getModelDirString(){return  m_strModelDataDir;}
      //used to set correct directory of the *.g file added
      void newModelDir();

      //To get access to DOF var. from Robot
      int DoF;
      int getDOF(){return DoF;}

      vector<Robot>& GetRobots(){return robotVec;}
      
      ////////////////////////////////
      //Objects Deleted from Scene
      ///////////////////////////////

      void DecreaseNumMB(){m_cNumberOfMultiBody = m_cNumberOfMultiBody - 1;}
      void IncreaseNumMB(){m_cNumberOfMultiBody = m_cNumberOfMultiBody + 1;}
      void SetNewMultiBodyInfo(CMultiBodyInfo * mbi);
      
      ////////////////////////////////////////////////////////////////////////////
      //
      //      Protected Methods and data members
      //
      ////////////////////////////////////////////////////////////////////////////
    protected:
      void Free_Memory();
      
      virtual bool ParseFileHeader(ifstream & ifs);
      virtual bool ParseFileBody(ifstream & ifs);
      virtual bool ParseMultiBody(ifstream & ifs, CMultiBodyInfo & MBInfo);
      virtual bool ParseActiveBody(ifstream & ifs, CBodyInfo & BodyInfo);
      virtual bool ParseOtherBody(ifstream & ifs, CBodyInfo & BodyInfo);
      virtual bool ParseConnections(ifstream & ifs, CBodyInfo *BodyInfo);
      
      void BuildRobotStructure(); 

      ////////////////////////////////////////////////////////////////////////////
      //
      //      Private Methods and data members
      //
      ////////////////////////////////////////////////////////////////////////////
      
      
    private:
      
      string m_strModelDataDir;
      int m_cNumberOfMultiBody;
      CMultiBodyInfo * m_pMBInfo;
      int previousBodyIndex, nextBodyIndex;
      Robot::JointType jointType;

      bool m_ContainsSurfaces;
    
      typedef stapl::graph<stapl::UNDIRECTED, stapl::NONMULTIEDGES, size_t> RobotGraph;
      RobotGraph m_robotGraph;
      vector<Robot> robotVec;
    };
  
}//namespace plum

#endif // !defined(_ENVLOADER_H_)
