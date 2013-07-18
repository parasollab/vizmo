#ifndef CFG_MODEL_H_
#define CFG_MODEL_H_

#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>

#include "EnvObj/Robot.h"

using namespace std;

template <typename, typename>
class CCModel;

class EdgeModel;

class CfgModel : public plum::GLModel{

  friend ostream& operator<<(ostream& _out, const CfgModel& _cfg);
  friend istream& operator>>(istream& _in, CfgModel& _cfg);

  public:
    enum Shape { Robot, Box, Point }; //Node representation

    CfgModel();
    CfgModel(const CfgModel& _cfg);
    ~CfgModel();

    const string GetName() const;
    vector<string> GetInfo() const;
    vector<string> GetNodeInfo() const;
    static CfgModel& GetInvalidData(){ return m_invalidCfg; }
    int GetIndex() const { return m_index; }
    int GetCCID();
    vector<double>& GetDataCfg() { return m_dofs; }
    vector<double> GetUnknowns();
    static int GetDOF() { return m_dof; }
    CCModel<CfgModel, EdgeModel>* GetCC() const { return m_cc; }
    OBPRMView_Robot* GetRobot() const { return m_robot; }
    virtual void SetColor(float _r, float _g, float _b, float _a);
    static void SetDOF(int _d) { m_dof = _d; }
    void SetInCollision(bool _isColl) { m_coll = _isColl; }
    static void SetShape(Shape _shape){ m_shape =_shape; }
    //Set new values to dofs vector
    void SetCfg(vector<double> _newCfg);
    void SetIndex(int _i) { m_index = _i; }
    void SetCCModel(CCModel<CfgModel, EdgeModel>* _cc) { m_cc = _cc; }
    void SetRobot(OBPRMView_Robot* _r) { m_robot = _r; }

    bool operator==(const CfgModel& _other) const;
    void Set(int _index, OBPRMView_Robot* _robot, CCModel<CfgModel, EdgeModel>* _cc);
    bool BuildModels(){ return true; }
    void Draw(GLenum _mode);
    void DrawRobot();
    void DrawBox();
    void DrawPoint();
    void DrawSelect();
    virtual void Dump();
    //Function accessed from gliDataStructure
    void CopyCfg();
    //Translation
    double& tx();
    double& ty();
    double& tz();
    const double& tx() const;
    const double& ty() const;
    const double& tz() const;

  private:
    static CfgModel m_invalidCfg;
    static int m_dof;
    static double m_defaultDOF;
    static bool m_isPlanarRobot;
    static bool m_isVolumetricRobot;
    static bool m_isRotationalRobot;
    static Shape m_shape;

    bool m_coll; //For collision detection
    OBPRMView_Robot* m_robot; //Testing
    vector<double> m_dofs;
    int m_index;
    double m_unknown1, m_unknown2, m_unknown3;
    CCModel<CfgModel, EdgeModel>* m_cc;
};

#endif

