#ifndef PATHMODEL_H_
#define PATHMODEL_H_

#include <Vector.h>
using namespace mathtool;

#include "Plum/GLModel.h"
using namespace plum;

class RobotModel;

class PathModel : public GLModel {
  public:
    typedef Vector<float, 4> Color4;

    PathModel(const string& _filename, RobotModel* _robotModel);

    virtual const string GetName() const {return "Path";}
    virtual vector<string> GetInfo() const;
    size_t GetPathSize() {return m_path.size();}
    vector<Color4>& GetGradientVector() {return m_stopColors;}
    const vector<double>& GetConfiguration(size_t _i) const {return m_path[_i];}

    void SetLineWidth(float _width) {m_lineWidth = _width;}
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;}

    virtual void ParseFile();
    virtual void BuildModels();
    virtual void Draw(GLenum _mode);

  private:
    Color4 Mix(Color4& _a, Color4& _b, float _percent);

    vector<vector<double> > m_path; //path storage
    size_t m_glPathIndex; //Display list index
    RobotModel * m_robotModel; //robot model

    //display options
    float m_lineWidth;
    size_t m_displayInterval;
    vector<Color4> m_stopColors; //gradient stops
};

#endif
