#ifndef EDGE_MODEL_H_
#define EDGE_MODEL_H_

#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>

#include "CfgModel.h"
#include "EnvObj/RobotModel.h"

using namespace std;

class CfgModel;

class EdgeModel : public plum::GLModel{

  friend ostream& operator<<(ostream& _out, const EdgeModel& _edge);
  friend istream& operator>>(istream& _in, EdgeModel& _edge);

  public:
    EdgeModel();
    EdgeModel(double _weight);
    ~EdgeModel();

    const string GetName() const;
    vector<string> GetInfo() const;
    vector<int> GetEdgeNodes();
    int& GetLP(){ return m_lp; }
    double& GetWeight(){ return m_weight; }
    double& Weight(){ return m_weight; }
    int GetID() { return m_id; }
    const CfgModel& GetStartCfg() { return m_startCfg; }
    void SetThickness(size_t _thickness);
    void SetCfgShape(CfgModel::Shape _s) { m_cfgShape = _s; }

    bool operator==(const EdgeModel& _other);
    void Set(int _id, CfgModel* _c1, CfgModel* _c2, RobotModel* _robot = NULL);
    bool BuildModels(){ return true; }
    void Draw(GLenum _mode);
    void DrawSelect();

  private:
    CfgModel m_startCfg, m_endCfg;
    int m_lp;
    double m_weight;
    int m_id;
    size_t m_edgeThickness;
    CfgModel::Shape m_cfgShape;
    vector <CfgModel> m_intermediateCfgs;

    friend class CfgModel;
};

#endif


















