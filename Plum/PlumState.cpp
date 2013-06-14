// PlumState.cpp: implementation of the CPlumState class.
//
//////////////////////////////////////////////////////////////////////

#include "PlumState.h"

namespace plum{

    const int PlumState::MV_WIRE_MODE = 0;
    const int PlumState::MV_SOLID_MODE= 1;
    const int PlumState::MV_INVISIBLE_MODE=2;

    const int PlumState::BUILD_MODEL_OK            = 0;
    const int PlumState::BUILD_ENV_MODEL_ERROR     = 1;
    const int PlumState::BUILD_MAP_MODEL_ERROR     = 2;
    const int PlumState::BUILD_CLIENT_MODEL_ERROR  = 3;

}
