#ifndef STATIC_MULTI_BODY_MODEL_H_
#define STATIC_MULTI_BODY_MODEL_H_

#include "MultiBodyModel.h"

//class StaticMultiBody;
class MultiBody;

class StaticMultiBodyModel : public MultiBodyModel {
  public:
    StaticMultiBodyModel(MultiBody* _s);

    virtual void Build();

    ////////////////////////////////////////////////////////////////////////////
    /// @return Corresponding PMPL StaticMultiBody object
    MultiBody* GetStaticMultiBody() {return m_staticMultiBody;}

    void SetTransform(Transformation& _t);

  protected:
    StaticMultiBodyModel(string _name, MultiBody* _s);

  private:
    MultiBody* m_staticMultiBody;
};

#endif
