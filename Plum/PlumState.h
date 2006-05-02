#ifndef _PlumSTATE_H_
#define _PlumSTATE_H_

namespace plum{

    class CPlumState{
    public:
            //render mode
            static const int MV_WIRE_MODE;
            static const int MV_SOLID_MODE;
            static const int MV_INVISIBLE_MODE;

            //Parse meg
            static const int PARSE_OK;
            static const int PARSE_ERROR;
            static const int PARSE_UNDEFINED_STATE;

            static const int BUILD_MODEL_OK;
            static const int BUILD_ENV_MODEL_ERROR;
            static const int BUILD_MAP_MODEL_ERROR;
            static const int BUILD_CLIENT_MODEL_ERROR;
    };

}//namespace plum

#endif
