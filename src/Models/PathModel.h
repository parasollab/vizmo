#ifndef PATHMODEL_H_
#define PATHMODEL_H_

#include <Vector.h>
using namespace mathtool;

#include "Model.h"
#include "Utilities/Color.h"

class RobotModel;

class PathModel : public LoadableModel {
  public:
    PathModel(const string& _filename, RobotModel* _robotModel);

    size_t GetSize() {return m_path.size();}
    vector<Color4>& GetGradientVector() {return m_stopColors;}
    const vector<double>& GetConfiguration(size_t _i) const {return m_path[_i];}

    void SetLineWidth(float _width) {m_lineWidth = _width;}
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;}

    virtual void ParseFile();
    virtual void BuildModels();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    virtual void Draw();
    void DrawSelect() {}
    void Print(ostream& _os) const;

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
