#ifndef PATHMODEL_H_
#define PATHMODEL_H_

#include <math.h>

#include "Plum/GLModel.h" 
using namespace plum;

#include "PathLoader.h"

class OBPRMView_Robot;

class PathModel : public GLModel {
  public:
    typedef vector<float> RGBAcolor; 

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////
    PathModel();
    virtual ~PathModel();

    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetPathLoader(CPathLoader * _pathLoader){ m_pathLoader = _pathLoader; }
    void SetModel(OBPRMView_Robot * _robot){ m_robot = _robot; }

    //////////////////////////////////////////////////////////////////////
    // Action functions
    //////////////////////////////////////////////////////////////////////
    void SetLineWidth(float _width) {m_lineWidth = _width;} 
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;} 
    virtual bool BuildModels();
    virtual void Draw(GLenum mode);
    virtual const string GetName() const { return "Path"; }
    virtual vector<string> GetInfo() const;
    size_t GetPathSize() {return m_pathLoader->GetPathSize();}
    vector<RGBAcolor>& GetGradientVector() {return m_stopColors;} 

    //////////////////////////////////////////////////////////////////////
    // Private functions and data
    //////////////////////////////////////////////////////////////////////
  private:
    RGBAcolor Mix(RGBAcolor& _a, RGBAcolor& _b, float _percent);

    size_t m_glListIndex;//Display list index
    float m_lineWidth; 
    size_t m_displayInterval; 
    CPathLoader * m_pathLoader;
    OBPRMView_Robot * m_robot;

    vector<RGBAcolor> m_stopColors; //gradient stops 
};

#endif // !defined(_PATHMODEL_H_)
