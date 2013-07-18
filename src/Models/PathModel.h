#ifndef PATHMODEL_H_
#define PATHMODEL_H_

#include <Vector.h>
using namespace mathtool;

#include "Plum/GLModel.h"
using namespace plum;

class OBPRMView_Robot;

class PathModel : public GLModel {
  public:
    typedef Vector<float, 4> Color4;

    PathModel(string _filename);

    virtual const string GetName() const {return "Path";}
    virtual vector<string> GetInfo() const;
    size_t GetPathSize() {return m_path.size();}
    vector<Color4>& GetGradientVector() {return m_stopColors;}
    const vector<double>& GetConfiguration(size_t _i) const {return m_path[_i];}

    void SetModel(OBPRMView_Robot * _robot) {m_robot = _robot;}
    void SetLineWidth(float _width) {m_lineWidth = _width;}
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;}

    virtual bool ParseFile();

    virtual bool BuildModels();
    virtual void Draw(GLenum _mode);

  private:
    Color4 Mix(Color4& _a, Color4& _b, float _percent);

    vector<vector<double> > m_path; //path storage
    size_t m_glPathIndex; //Display list index
    OBPRMView_Robot * m_robot; //robot model

    //display options
    float m_lineWidth;
    size_t m_displayInterval;
    vector<Color4> m_stopColors; //gradient stops
};

#endif // !defined(_PATHMODEL_H_)
