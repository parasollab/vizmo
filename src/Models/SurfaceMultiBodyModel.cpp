#include "SurfaceMultiBodyModel.h"

#include "Geometry/Bodies/MultiBody.h"

SurfaceMultiBodyModel::
SurfaceMultiBodyModel(MultiBody* _s) :
  StaticMultiBodyModel("SurfaceMultiBody", _s), m_surfaceMultiBody(_s) {
  }
