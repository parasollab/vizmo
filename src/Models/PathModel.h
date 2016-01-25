#ifndef PATH_MODEL_H_
#define PATH_MODEL_H_

#include <Vector.h>
using namespace mathtool;

#include "CfgModel.h"
#include "Utilities/Color.h"

class PathModel : public LoadableModel {
  public:
    PathModel(const string& _filename);

    size_t GetSize() {return m_path.size();}
    vector<Color4>& GetGradientVector() {return m_stopColors;}

    void SetLineWidth(float _width) {m_lineWidth = _width;}
    void SetDisplayInterval(int _disp) {m_displayInterval = _disp;}
    const CfgModel& GetConfiguration(size_t _i) const {return m_path[_i];}

    float GetLineWidth() {return m_lineWidth;}
    size_t GetDisplayInterval() {return m_displayInterval;}

    void ConfigureFrame(size_t _frame);

    void ParseFile();
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect();
    void DrawSelected() {}
    void Print(ostream& _os) const;
    void SavePath(const string& _filename);

  private:
    Color4 Mix(Color4& _a, Color4& _b, float _percent);

    vector<CfgModel> m_path; //path storage
    size_t m_glPathIndex; //Display list index
    size_t m_frame;          ///< Frame for playing path

    //display options
    float m_lineWidth;
    size_t m_displayInterval;
    vector<Color4> m_stopColors; //gradient stops
};

#endif
