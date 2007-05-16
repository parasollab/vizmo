// PlumState.cpp: implementation of the CPlumState class.
//
//////////////////////////////////////////////////////////////////////

#include "PlumState.h"

namespace plum{

    const int CPlumState::MV_WIRE_MODE = 0;
    const int CPlumState::MV_SOLID_MODE= 1;
    const int CPlumState::MV_INVISIBLE_MODE=2;

    const int CPlumState::PARSE_OK               = 0;
    const int CPlumState::PARSE_ERROR            = 1;
    const int CPlumState::PARSE_UNDEFINED_STATE  = 2;

    const int CPlumState::BUILD_MODEL_OK            = 0;
    const int CPlumState::BUILD_ENV_MODEL_ERROR     = 1;
    const int CPlumState::BUILD_MAP_MODEL_ERROR     = 2;
    const int CPlumState::BUILD_CLIENT_MODEL_ERROR  = 3;

}
