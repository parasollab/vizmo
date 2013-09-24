#ifndef ROBOTMODEL_H_
#define ROBOTMODEL_H_

#include <QKeyEvent>

#include <Vector.h>
#include <Matrix.h>
#include <Transformation.h>

using namespace mathtool;

#include "Plum/EnvObj/MultiBodyModel.h"
#include "Models/EnvModel.h"
#include "Plum/GLModel.h"
#include "Utilities/Color.h"

class RobotModel : public GLModel {

  public:

    RobotModel(EnvModel* _env);
    ~RobotModel();

    virtual void GetChildren(list<GLModel*>& _models) const {_models.push_back(m_robotModel);}
    virtual const string GetName() const {return "Robot";}
    virtual void SetRenderMode(RenderMode _mode);
    virtual void SetColor(const Color4& _c);

    virtual void BuildModels();
    virtual void Draw(GLenum _mode);
    virtual void DrawSelect();
    virtual void Select(unsigned int* _index, vector<GLModel*>& _sel);

    void SaveQry(vector<vector<double> >& cfg, char ch);
    void Configure(vector<double>& _cfg);

    //return current configuration
    vector<double> getFinalCfg();

    MultiBodyModel* getRobotModel() const;
    void BackUp();
    void Restore();
    //Keep initial Cfg.
    void InitialCfg(vector<double>& cfg);
    void RestoreInitCfg();

    vector<vector<double> > getNewStartAndGoal();

    const vector<double>& CurrentCfg() {return m_currCfg;}

  private:
    //////////////////////////////////////////////
    // storeCfg::
    //    cfg the cfg. to store
    //    c tells if cfg. is start or goal
    //////////////////////////////////////////////
    void storeCfg(vector<vector<double> >& cfg, char c, int dof);

    EnvModel* m_envModel;
    MultiBodyModel* m_robotModel;

    //////////////////////////////////////////////////////
    //To store start/goal cfgs for NEW Query
    /////////////////////////////////////////////////////
    vector<vector<double> > StartCfg;
    vector<vector<double> > GoalCfg;
    vector<double> m_currCfg;

    //variables for back up/restore
    Vector3d m_translationBackUp;
    Quaternion m_quaternionBackUp;
    Color4 m_colorBackUp;
    RenderMode m_renderModeBackUp;

    //to store the START cfg. It comes from Query or Path files
    vector<double> StCfg;
};

#endif

