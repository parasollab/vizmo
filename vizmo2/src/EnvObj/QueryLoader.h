// StartGoalLoader.h: interface for the CStartGoalLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_QUERY_LOADER_H_)
#define _QUERY_LOADER_H_

#include <vector>
using namespace std;

#include <ILoadable.h>
using namespace plum;

#include <GLModel.h>
using namespace plum;
/**
* This class reads .query file to visualize start and goal positions.
*/
class CQueryLoader : public I_Loadable
{
public:
    
    //////////////////////////////////////////////////////////////////////
    //    Constructors and Destructor
    ///////////////////////////////////////////////////////////////////////
    CQueryLoader();
    virtual ~CQueryLoader();
    virtual bool ParseFile();
    
    //////////////////////////////////////////////////////////////////////
    //    Access Methods
    ///////////////////////////////////////////////////////////////////////
  
    virtual unsigned int GetPathSize(){ return m_sgList.size(); }
    virtual double * GetStartGoal( int Index);

    /******** NEW Variables ********/

    bool notExists;
    
    //////////////////////////////////////////////////////////////////////
    //    Protected Methods and Data
    ///////////////////////////////////////////////////////////////////////
protected:
    
    ///Free memory
    virtual void FreePathList();

private:
    
    int m_iCurrent_CfgIndex;        ///current index for path retrival
    vector<double *> m_sgList;       ///path storage
    double * dCfg;
};

#endif // !defined(_QUERY_LOADER_H_)



