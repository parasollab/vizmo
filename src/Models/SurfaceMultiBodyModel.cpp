#include "SurfaceMultiBodyModel.h"

#include "Environment/SurfaceMultiBody.h"

SurfaceMultiBodyModel::
SurfaceMultiBodyModel(shared_ptr<SurfaceMultiBody> _s) :
  StaticMultiBodyModel("SurfaceMultiBody", _s), m_surfaceMultiBody(_s) {
  }
