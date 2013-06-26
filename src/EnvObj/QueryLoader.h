// StartGoalLoader.h: interface for the CStartGoalLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_QUERY_LOADER_H_)
#define _QUERY_LOADER_H_

#include <vector>
using namespace std;

#include "Plum/MapObj/CfgModel.h"

#include "Plum/Loadable.h"
using namespace plum;

#include "Plum/GLModel.h" 
using namespace plum;
/**
* This class reads .query file to visualize start and goal positions.
*/
class CQueryLoader : public Loadable
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
  
    virtual unsigned int GetQuerySize(){ return m_sgList.size(); }
    virtual vector<double> GetStartGoal( int Index);

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



