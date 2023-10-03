#ifndef ACTIVE_MULTIBODY_MODEL_H_
#define ACTIVE_MULTIBODY_MODEL_H_

#include "Geometry/Bodies/MultiBody.h"
#include "MPProblem/Environment/Environment.h"

#include "MultiBodyModel.h"

class ActiveMultiBodyModel : public MultiBodyModel {
  public:

    ActiveMultiBodyModel(MultiBody* _a);

    size_t Dofs() const;
    bool IsPlanar() const;

    void SetInitialCfg(const vector<double>& _cfg) {m_initCfg = _cfg;}
    const vector<double>& CurrentCfg() {return m_currCfg;}

    void BackUp();
    void ConfigureRender(const vector<double>& _cfg);
    bool InCSpace(const vector<double>& _cfg);
    void RestoreColor();
    void Restore();

    void Print(ostream& _os) const;

    virtual void Build();
    virtual void DrawSelected();
    void DrawSelectedImpl();

    const vector<DofInfo>& GetDOFInfo() const;

  private:
    MultiBody* m_activeMultiBody;

    vector<double> m_initCfg, m_currCfg;

    //variables for back up/restore
    RenderMode m_renderModeBackUp;
    vector<Color4> m_colorBackUp;
};

#endif

