// PathLoader.h: interface for the CPathLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PATH_LOADER_H_)
#define _PATH_LOADER_H_

#include <vector>
using namespace std;

#include <ILoadable.h>
using namespace plum;

/**
* This class read path. OBPRM style path.
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
    
    //////////////////////////////////////////////////////////////////////
    //    Access Methods
    ///////////////////////////////////////////////////////////////////////
    virtual double * GetNextConfigure(bool bOutputFrameNumber=false );
    virtual double * GetPreviousConfigure(bool bOutputFrameNumber=false );
    
    virtual unsigned int GetPathSize(){ return m_pList.size(); }
    virtual bool IsFirstStep(){ return (m_iCurrent_CfgIndex==0); }
    

    bool notExists;  // to know if there is .path file

    //////////////////////////////////////////////////////////////////////
    //    Protected Methods and Data
    ///////////////////////////////////////////////////////////////////////
protected:
    
    ///Free memory
    virtual void FreePathList();
    virtual double * GetConfigure(bool bOutputFrameNumber, int & index );

private:
    
    int m_iCurrent_CfgIndex;        ///current index for path retrival
    vector<double *> m_pList;       ///path storage
};

#endif // !defined(_PATH_LOADER_H_)
