#ifndef EDGE_MODEL_H_
#define EDGE_MODEL_H_

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "MPProblem/Weight.h"

#include "CfgModel.h"
#include "RobotModel.h"

class CfgModel;

class EdgeModel : public Model, public DefaultWeight<CfgModel> {

  public:
    EdgeModel(double _weight = LONG_MAX);

    void SetName();
    int GetID() { return m_id; }

    CfgModel* GetStartCfg() { return m_startCfg; }
    CfgModel* GetEndCfg() { return m_endCfg; }
    void SetStartCfg(CfgModel* _s) { m_startCfg = _s; }
    void SetEndCfg(CfgModel* _e) { m_endCfg = _e; }

    void Set(int _id, CfgModel* _c1, CfgModel* _c2, RobotModel* _robot = NULL);

    void BuildModels() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void Draw(GLenum _mode);
    void DrawSelect();
    void Print(ostream& _os) const;

    static double m_edgeThickness;

  private:
    CfgModel* m_startCfg, * m_endCfg;
    int m_id;
};

#endif


















