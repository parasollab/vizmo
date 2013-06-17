// PathLoader.h: interface for the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PATH_LOADER_H_)
#define _PATH_LOADER_H_

#include <vector>
using namespace std;

#include "Plum/MapObj/Cfg.h"

#include "Plum/Loadable.h"
using namespace plum;

/**
* This class reads path. OBPRM style path.
* Client how wants to retrive path could call LoadPath to
* read path form given file name and get individal cfg from 
* GetNextConfigure.
*/
class CPathLoader : public Loadable
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
    virtual vector<double> GetConfiguration(int frame);
    virtual unsigned int GetPathSize(){ return m_pList.size(); }
    virtual unsigned int GetDOF(){ return CCfg::m_dof; }

    //////////////////////////////////////////////////////////////////////
    //    Protected Methods and Data
    ///////////////////////////////////////////////////////////////////////
protected:
    
    ///Free memory
    virtual void FreePathList();
    virtual vector<double> GetConfigure(bool bOutputFrameNumber, int & index );

private:
    
    vector<vector<double> > m_pList;       ///path storage
};

#endif // !defined(_PATH_LOADER_H_)


