#ifndef CFGMODEL_H_
#define CFGMODEL_H_

#include <iostream>
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "Cfg/Cfg.h"

#include "EdgeModel.h"
#include "Model.h"

template<typename, typename> class CCModel;
class EdgeModel;

class CfgModel : public Model, public Cfg {

  public:
    enum Shape { Robot, Box, Point }; //Node representation

    CfgModel();

    void SetName();
    size_t GetIndex() const { return m_index; }
    vector<double> GetDataCfg() { return m_v; }
    Point3d GetPoint() const {return Point3d(m_v[0], m_v[1], m_isVolumetricRobot ? m_v[2] : 0); }
    void SetIsQuery(){ m_isQuery=true;}
    bool IsQuery(){ return m_isQuery;}

    static bool GetIsVolumetricRobot() { return m_isVolumetricRobot; } //For Translation() call in NodeEditBox...for now
    CCModel<CfgModel, EdgeModel>* GetCC() const { return m_cc; }
    bool IsValid() { return m_isValid; }
    static Shape GetShape() { return m_shape; }
    static float GetPointSize() { return m_pointScale; }

    static void SetDOF(size_t _d) { m_dof = _d; }
    static void SetIsPlanarRobot(bool _b) { m_isPlanarRobot = _b; }
    static void SetIsVolumetricRobot(bool _b) { m_isVolumetricRobot = _b; }
    static void SetIsRotationalRobot(bool _b) { m_isRotationalRobot = _b; }
    void SetValidity(bool _validity) { m_isValid = _validity; }
    static void SetShape(Shape _shape){ m_shape =_shape; }
    //Set new values to dofs vector
    void SetCfg(const vector<double>& _newCfg);
    void SetIndex(size_t _i) { m_index = _i; SetName();}
    void SetCCModel(CCModel<CfgModel, EdgeModel>* _cc) { m_cc = _cc; }

    void Set(size_t _index, CCModel<CfgModel, EdgeModel>* _cc);
    static void Scale(float _scale);

    void BuildModels() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void Draw();
    void DrawRobot();
    void DrawBox();
    void DrawPoint();
    void DrawSelect();
    void Print(ostream& _os) const;

  private:
    void PerformBoxTranslation();
    static double m_defaultDOF;
    static bool m_isPlanarRobot;
    static bool m_isVolumetricRobot;
    static bool m_isRotationalRobot;
    static Shape m_shape;
    static float m_boxScale;
    static float m_pointScale;

    bool m_isValid; //For collision detection
    bool m_isQuery;
    size_t m_index;
    CCModel<CfgModel, EdgeModel>* m_cc;
};

#endif

