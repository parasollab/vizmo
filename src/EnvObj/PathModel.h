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

class CPathModel : public CGLModel {
  public:
    typedef vector<float> RGBAcolor; 

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    CPathModel();
    virtual ~CPathModel();

    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetPathLoader(CPathLoader * pPathLoader){ m_pPathLoader=pPathLoader; }
    void SetModel(OBPRMView_Robot * pRobot){ m_pRobot=pRobot; }

    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetLineWidth(float _width) {m_lineWidth = _width;} 
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;} 
    virtual bool BuildModels();
    virtual void Draw(GLenum mode);
    virtual const string GetName() const { return "Path"; }
    virtual vector<string> GetInfo() const;
    size_t GetPathSize() {return m_pPathLoader->GetPathSize();}
    vector<RGBAcolor>& GetGradientVector() {return m_stopColors;} 

    //output info to std ouput
    //virtual void DumpSelected();

    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////
  private:
    RGBAcolor Mix(RGBAcolor& _a, RGBAcolor& _b, float _percent);

    int m_Index;
    int m_DLIndex;//Display list index
    float m_lineWidth; 
    int m_displayInterval; 
    CPathLoader * m_pPathLoader;
    OBPRMView_Robot * m_pRobot;

    vector<RGBAcolor> m_stopColors; //gradient stops 
};

#endif // !defined(_PATHMODEL_H_)
