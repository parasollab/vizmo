#ifndef ROBOTMODEL_H_
#define ROBOTMODEL_H_

#include "GLModel.h"
#include "MultiBodyModel.h"

class EnvModel;

class RobotModel : public GLModel {
  public:
    RobotModel(EnvModel* _env);

    virtual void GetChildren(list<GLModel*>& _models) {_models.push_back(m_robotModel);}
    virtual const string GetName() const {return "Robot";}
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetColor(const Color4& _c);

    void SetInitialCfg(vector<double>& _cfg) {m_initCfg = _cfg;}
    const vector<double>& CurrentCfg() {return m_currCfg;}

    void BackUp();
    void Configure(vector<double>& _cfg);
    void Restore();

    virtual void BuildModels();
    virtual void Select(unsigned int* _index, vector<GLModel*>& _sel);
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();

  private:
    EnvModel* m_envModel;
    MultiBodyModel* m_robotModel;

    vector<double> m_initCfg, m_currCfg;

    //variables for back up/restore
    RenderMode m_renderModeBackUp;
    Color4 m_colorBackUp;
};

#endif

