// EnvLoader.h: interface for the CEnvLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ENVLOADER_H_)
#define _ENVLOADER_H_

#include "ILoadable.h"

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
      
      ////////////////////////////////////////////////////////////////////////////
      //
      //      Protected Methods and data members
      //
      ////////////////////////////////////////////////////////////////////////////
    protected:
      void Free_Memory();
      
      virtual bool ParseFileHeader( ifstream & fin );
      virtual bool ParseFileBody( ifstream & fin );
      virtual bool ParseMultiBody( ifstream & fin, CMultiBodyInfo & MBInfo );
      virtual bool ParseBody( ifstream & fin, CBodyInfo & BodyInfo );
      virtual bool ParseConnections(ifstream & fin, CBodyInfo *BodyInfo );
      
      ////////////////////////////////////////////////////////////////////////////
      //
      //      Private Methods and data members
      //
      ////////////////////////////////////////////////////////////////////////////
      
      
    private:
      
      string m_strModelDataDir;
      int m_cNumberOfMultiBody;
      CMultiBodyInfo * m_pMBInfo;
      int preIndx, nextIndx;
    };
  
}//namespace plum

#endif // !defined(_ENVLOADER_H_)
