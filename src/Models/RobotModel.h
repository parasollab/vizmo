#ifndef ROBOTMODEL_H_
#define ROBOTMODEL_H_

#include "Model.h"
#include "MultiBodyModel.h"

class EnvModel;

class RobotModel : public Model {
  public:
    RobotModel(EnvModel* _env);

    void GetChildren(list<Model*>& _models) {_models.push_back(m_robotModel);}
    void SetRenderMode(RenderMode _mode);
    void SetColor(const Color4& _c);

    void SetInitialCfg(vector<double>& _cfg) {m_initCfg = _cfg;}
    const vector<double>& CurrentCfg() {return m_currCfg;}

    void BackUp();
    void Configure(const vector<double>& _cfg);
    void Restore();

    void BuildModels();
    virtual void SetSelectable(bool _s); //propagate selectability to children
    void Select(GLuint* _index, vector<Model*>& _sel);
    void Draw();
    void DrawSelect();
    void Print(ostream& _os) const;

  private:
    EnvModel* m_envModel;
    MultiBodyModel* m_robotModel;

    vector<double> m_initCfg, m_currCfg;

    //variables for back up/restore
    RenderMode m_renderModeBackUp;
    Color4 m_colorBackUp;
};

#endif

