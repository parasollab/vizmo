#ifndef STATIC_MULTI_BODY_MODEL_H_
#define STATIC_MULTI_BODY_MODEL_H_

#include "MultiBodyModel.h"

class StaticMultiBody;

class StaticMultiBodyModel : public MultiBodyModel {
  public:
    StaticMultiBodyModel(shared_ptr<StaticMultiBody> _s);
    StaticMultiBodyModel(string _name, shared_ptr<StaticMultiBody> _s);

    virtual void Build();

  private:
    shared_ptr<StaticMultiBody> m_staticMultiBody;
};

#endif
