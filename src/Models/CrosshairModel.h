#ifndef CROSSHAIR_MODEL_H_
#define CROSSHAIR_MODEL_H_

#include "Model.h"
//#include "Vector.h"
//
//using namespace mathtool;
//using namespace std;

class CrosshairModel : public Model {
  public:
    CrosshairModel(Point3d* _p = NULL);

    //access functions
    void SetPos(Point3d& _p) {m_worldPos = &_p;}
    Point3d* GetPos() const {return m_worldPos;}

    //control functions
    void Toggle() {m_enabled = !m_enabled;}

    //Model functions
    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {}
    void DrawRender();
    void DrawSelect() {}
    void DrawSelected() {}
    void Print(ostream& _os) const {}

  protected:
    bool IsInsideBBX() const;

  private:
    Point3d* m_worldPos;
    vector< pair<double, double> > m_worldRange;

    bool m_enabled;
};

#endif
