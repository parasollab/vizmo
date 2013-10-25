#ifndef CFGMODEL_H_
#define CFGMODEL_H_

#include <iostream>
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "RobotModel.h"

template <typename, typename>
class CCModel;
class EdgeModel;

class CfgModel : public Model {

  friend ostream& operator<<(ostream& _out, const CfgModel& _cfg);
  friend istream& operator>>(istream& _in, CfgModel& _cfg);

  public:
    enum Shape { Robot, Box, Point }; //Node representation

    CfgModel();
    CfgModel(const CfgModel& _cfg);

    void SetName();
    int GetIndex() const { return m_index; }
    int GetCCID();
    const vector<double>& GetDataCfg() { return m_dofs; }
    Point3d GetPoint() const {return Point3d(m_dofs[0], m_dofs[1], m_isVolumetricRobot ? m_dofs[2] : 0); }

    static bool GetIsVolumetricRobot() { return m_isVolumetricRobot; } //For Translation() call in NodeEditBox...for now
    CCModel<CfgModel, EdgeModel>* GetCC() const { return m_cc; }
    RobotModel* GetRobot() const { return m_robot; }
    bool IsInCollision() { return m_inColl; }
    static int GetDOF() { return m_dof; }
    static Shape GetShape() { return m_shape; }
    static float GetPointSize() { return m_pointScale; }

    static void SetDOF(int _d) { m_dof = _d; }
    static void SetIsPlanarRobot(bool _b) { m_isPlanarRobot = _b; }
    static void SetIsVolumetricRobot(bool _b) { m_isVolumetricRobot = _b; }
    static void SetIsRotationalRobot(bool _b) { m_isRotationalRobot = _b; }
    void SetInCollision(bool _isColl) { m_inColl = _isColl; }
    static void SetShape(Shape _shape){ m_shape =_shape; }
    //Set new values to dofs vector
    void SetCfg(const vector<double>& _newCfg);
    void SetIndex(int _i) { m_index = _i; SetName();}
    void SetCCModel(CCModel<CfgModel, EdgeModel>* _cc) { m_cc = _cc; }
    void SetRobot(RobotModel* _r) { m_robot = _r; }

    bool operator==(const CfgModel& _other) const;
    double& operator[](size_t _i) { return m_dofs[_i]; }
    const double& operator[](size_t _i) const { return m_dofs[_i]; }
    void Set(int _index, RobotModel* _robot, CCModel<CfgModel, EdgeModel>* _cc);
    static void Scale(float _scale);

    void BuildModels() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void Draw(GLenum _mode);
    void DrawRobot();
    void DrawBox();
    void DrawPoint();
    void DrawSelect();
    void Print(ostream& _os) const;

  private:
    static CfgModel m_invalidCfg;
    static int m_dof;
    static double m_defaultDOF;
    static bool m_isPlanarRobot;
    static bool m_isVolumetricRobot;
    static bool m_isRotationalRobot;
    static Shape m_shape;
    static float m_boxScale;
    static float m_pointScale;

    bool m_inColl; //For collision detection
    RobotModel* m_robot; //Testing
    vector<double> m_dofs;
    int m_index;
    CCModel<CfgModel, EdgeModel>* m_cc;
};

#endif

