#ifndef EDGEMODEL_H_
#define EDGEMODEL_H_

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <GL/gl.h>
#include <GL/glut.h>

#include "MPProblem/Weight.h"

#include "CfgModel.h"
#include "Model.h"

class CfgModel;

class EdgeModel : public Model, public DefaultWeight<CfgModel> {

  public:
    EdgeModel(string _lpLabel = "", double _weight = LONG_MAX, const vector<CfgModel>& _intermediates = vector<CfgModel>());
    EdgeModel(const DefaultWeight<CfgModel>& _e);

    void SetName();
    size_t GetID() { return m_id; }

    CfgModel* GetStartCfg() { return m_startCfg; }
    CfgModel* GetEndCfg() { return m_endCfg; }
    void SetStartCfg(CfgModel* _s) { m_startCfg = _s; }
    void SetEndCfg(CfgModel* _e) { m_endCfg = _e; }

    void Set(size_t _id, CfgModel* _c1, CfgModel* _c2);

    void Build() {}
    void Select(GLuint* _index, vector<Model*>& _sel) {};
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

    void DrawRenderInCC();

    static double m_edgeThickness;

  private:
    CfgModel* m_startCfg, * m_endCfg;
    size_t m_id;
};

#endif


















