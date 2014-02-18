#ifndef PATHMODEL_H_
#define PATHMODEL_H_

#include <Vector.h>
using namespace mathtool;

#include "CfgModel.h"
#include "Utilities/Color.h"

class RobotModel;

class PathModel : public LoadableModel {
  public:
    PathModel(const string& _filename);

    size_t GetSize() {return m_path.size();}
    vector<Color4>& GetGradientVector() {return m_stopColors;}
    const CfgModel& GetConfiguration(size_t _i) const {return m_path[_i];}

    void SetLineWidth(float _width) {m_lineWidth = _width;}
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;}

    void ParseFile();
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;

  private:
    Color4 Mix(Color4& _a, Color4& _b, float _percent);

    vector<CfgModel> m_path; //path storage
    size_t m_glPathIndex; //Display list index

    //display options
    float m_lineWidth;
    size_t m_displayInterval;
    vector<Color4> m_stopColors; //gradient stops
};

#endif
