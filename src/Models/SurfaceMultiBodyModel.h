#ifndef SURFACE_MULTI_BODY_MODEL_H_
#define SURFACE_MULTI_BODY_MODEL_H_

#include "StaticMultiBodyModel.h"

class MultiBody;

class SurfaceMultiBodyModel : public StaticMultiBodyModel {
  public:
    SurfaceMultiBodyModel(MultiBody* _s);

  private:
    MultiBody* m_surfaceMultiBody;
};

#endif

