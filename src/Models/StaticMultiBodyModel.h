#ifndef STATIC_MULTI_BODY_MODEL_H_
#define STATIC_MULTI_BODY_MODEL_H_

#include "MultiBodyModel.h"

class StaticMultiBody;

class StaticMultiBodyModel : public MultiBodyModel {
  public:
    StaticMultiBodyModel(shared_ptr<StaticMultiBody> _s);

    virtual void Build();

    ////////////////////////////////////////////////////////////////////////////
    /// @return Corresponding PMPL StaticMultiBody object
    shared_ptr<StaticMultiBody> GetStaticMultiBody() {return m_staticMultiBody;}

    void SetTransform(Transformation& _t);
    virtual void DrawRender();

  protected:
    StaticMultiBodyModel(string _name, shared_ptr<StaticMultiBody> _s);

  private:
    shared_ptr<StaticMultiBody> m_staticMultiBody;
};

#endif
