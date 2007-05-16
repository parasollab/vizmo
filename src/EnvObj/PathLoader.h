// PathLoader.h: interface for the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PATH_LOADER_H_)
#define _PATH_LOADER_H_

#include <vector>
using namespace std;

#include "MapObj/Cfg.h"

#include <ILoadable.h>
using namespace plum;

/**
* This class reads path. OBPRM style path.
* Client how wants to retrive path could call LoadPath to
* read path form given file name and get individal cfg from 
* GetNextConfigure.
*/
class CPathLoader : public I_Loadable
{
public:
    
    //////////////////////////////////////////////////////////////////////
    //    Constructors and Destructor
    ///////////////////////////////////////////////////////////////////////
    CPathLoader();
    virtual ~CPathLoader();
    virtual bool ParseFile();
    
    /////////////////////////x/////////////////////////////////////////////
    //    Access Methods
    ///////////////////////////////////////////////////////////////////////
    //Get a particular configuration
    virtual double * GetConfiguration(int frame);
    virtual unsigned int GetPathSize(){ return m_pList.size(); }
    virtual unsigned int GetDOF(){ return CCfg::dof; }

    //////////////////////////////////////////////////////////////////////
    //    Protected Methods and Data
    ///////////////////////////////////////////////////////////////////////
protected:
    
    ///Free memory
    virtual void FreePathList();
    virtual double * GetConfigure(bool bOutputFrameNumber, int & index );

private:
    
    vector<double *> m_pList;       ///path storage
};

#endif // !defined(_PATH_LOADER_H_)


