// QueryModel.h: interface for the CQueryModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_QUERY_MODEL_H_)
#define _QUERY_MODEL_H_

//////////////////////////////////////////////////////////////////////
//std Headers
#include <math.h>

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <GLModel.h>
using namespace plum;

//////////////////////////////////////////////////////////////////////
//OBPRMView Headers
#include "QueryLoader.h"
class OBPRMView_Robot;

class CQueryModel : public CGLModel
{
public:
    
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    CQueryModel();
    virtual ~CQueryModel();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetQueryLoader( CQueryLoader * pQueryLoader ){ m_pQueryLoader=pQueryLoader; }
    void SetModel( OBPRMView_Robot * pRobot ){ m_pRobot=pRobot; }
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    virtual void Draw( GLenum mode );
    
    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////

private:

    int m_Index;
    int m_DLIndex;//Display list index
    CQueryLoader * m_pQueryLoader;
    OBPRMView_Robot * m_pRobot;
};

#endif // !defined(_QUERY_MODEL_H_)
