// PlumUtil.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(_PLUMUTIL_H_)
#define _PLUMUTIL_H_

#include "MapObj/MapModel.h"
#include "MapObj/MapLoader.h"
#include "EnvObj/CmdParser.h"
#include "EnvObj/EnvModel.h"
#include "EnvObj/EnvLoader.h"
#include "PlumObject.h"

namespace plum 
{

    /**
    * Creaete Map Object from given name
    */
    template<class Cfg, class Weight> PlumObject *
    createMapObj( const string filename )
    {
        //create loader and model
        CMapLoader<Cfg,Weight> * mapLoader=new CMapLoader<Cfg,Weight>();
        CMapModel<Cfg,Weight> * mapModel=new CMapModel<Cfg,Weight>();
        if( mapLoader==NULL || mapModel==NULL ) return NULL;
        mapLoader->SetDataFileName( filename );
        mapModel->SetMapLoader(mapLoader);
        PlumObject * object=new PlumObject(mapModel,mapLoader);
        return object;
    }

    template<class Cfg, class Weight> PlumObject * 
    createEnvObj( CMapLoader<Cfg,Weight> * mapLoader )
    {
        //create Env : guess file name from map file
        if( mapLoader->ParseHeader()==false ) return NULL;
        CCmdParser cmdParser; //parse the command line (Preamble) in map file
        if( !cmdParser.ParseCmd(mapLoader->GetFileDir(),mapLoader->GetPreamble()) )
            return NULL;

        return 
        createEnvObj(mapLoader->GetEnvFileName(), cmdParser.GetModelDataDir());
    }

    /**
    * Create Environment object from given file
    */
    PlumObject * createEnvObj( const string filename, const string modelDir );

}//end of plum

#endif //_PLUMUTIL_H_

