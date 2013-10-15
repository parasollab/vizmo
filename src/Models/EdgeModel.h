#ifndef EDGE_MODEL_H_
#define EDGE_MODEL_H_

#include <iostream>
#include <string>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "CfgModel.h"

class EdgeModel : public Model {

  friend ostream& operator<<(ostream& _out, const EdgeModel& _edge);
  friend istream& operator>>(istream& _in, EdgeModel& _edge);

  public:
    EdgeModel();
    EdgeModel(double _weight);

    void SetName();
    vector<int> GetEdgeNodes();
    int& GetLP(){ return m_lp; }
    double& GetWeight(){ return m_weight; }
    double& Weight(){ return m_weight; }
    int GetID() { return m_id; }
    const CfgModel& GetStartCfg() { return m_startCfg; }
    const CfgModel& GetEndCfg() { return m_endCfg; }
    void SetStartCfg(CfgModel _s) { m_startCfg = _s; }
    void SetEndCfg(CfgModel _e) { m_endCfg = _e; }
    void SetCfgShape(CfgModel::Shape _s) { m_cfgShape = _s; }
    //Want m_edgeThickness to be private, but cannot figure out how to compile
    //with this declaration
    //template<class CfgModel, class EdgeModel> friend void MapModel<CfgModel, EdgeModel>::SetEdgeThickness(double _thickness);

    bool operator==(const EdgeModel& _other);
    void Set(int _id, CfgModel* _c1, CfgModel* _c2, RobotModel* _robot = NULL);

    void BuildModels() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void Draw(GLenum _mode);
    void DrawSelect();
    void Print(ostream& _os) const;

    static double m_edgeThickness;

  private:
    CfgModel m_startCfg, m_endCfg;
    int m_lp;
    double m_weight;
    int m_id;
    CfgModel::Shape m_cfgShape;
    vector <CfgModel> m_intermediateCfgs;

    friend class CfgModel;
};

#endif


















