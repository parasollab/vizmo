#ifndef SURFACE_MULTI_BODY_MODEL_H_
#define SURFACE_MULTI_BODY_MODEL_H_

#include "StaticMultiBodyModel.h"

class SurfaceMultiBody;

class SurfaceMultiBodyModel : public StaticMultiBodyModel {
  public:
    SurfaceMultiBodyModel(shared_ptr<SurfaceMultiBody> _s);

  private:
    shared_ptr<SurfaceMultiBody> m_surfaceMultiBody;
};

#endif

