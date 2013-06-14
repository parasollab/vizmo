#ifndef PLUMSTATE_H_
#define PLUMSTATE_H_

namespace plum{

  class PlumState{
    public:
      //render mode
      static const int MV_WIRE_MODE;
      static const int MV_SOLID_MODE;
      static const int MV_INVISIBLE_MODE;

      static const int BUILD_MODEL_OK;
      static const int BUILD_ENV_MODEL_ERROR;
      static const int BUILD_MAP_MODEL_ERROR;
      static const int BUILD_CLIENT_MODEL_ERROR;
  };

}

#endif
