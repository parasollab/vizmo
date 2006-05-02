// PathModel.h: interface for the CPathModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PATHMODEL_H_)
#define _PATHMODEL_H_

//////////////////////////////////////////////////////////////////////
//std Headers
#include <math.h>

//////////////////////////////////////////////////////////////////////
//Plum Headers
#include <GLModel.h>
using namespace plum;

//////////////////////////////////////////////////////////////////////
//OBPRMView Headers
#include "PathLoader.h"
class OBPRMView_Robot;

class CPathModel : public CGLModel
{
public:
    
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    CPathModel();
    virtual ~CPathModel();
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetPathLoader( CPathLoader * pPathLoader ){ m_pPathLoader=pPathLoader; }
    void SetModel( OBPRMView_Robot * pRobot ){ m_pRobot=pRobot; }
    
    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    virtual bool BuildModels();
    virtual void Draw( GLenum mode );
    
    //output info to std ouput
    //virtual void DumpSelected();
    
    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////
private:
    int m_Index;
    int m_DLIndex;//Display list index
    CPathLoader * m_pPathLoader;
    OBPRMView_Robot * m_pRobot;
};

#endif // !defined(_PATHMODEL_H_)
