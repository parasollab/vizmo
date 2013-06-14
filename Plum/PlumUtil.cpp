#include "PlumUtil.h"

namespace plum{

  PlumObject* createEnvObj( const string filename, const string modelDir )
  {
    //create loader and model
    CEnvLoader* envLoader=new CEnvLoader();
    CEnvModel* envModel=new CEnvModel();
    if( envLoader==NULL || envModel==NULL ) return NULL;

    envLoader->SetFilename(filename);
    envLoader->SetModelDataDir(modelDir);
    envModel->SetEnvLoader(envLoader);

    PlumObject* object=new PlumObject(envModel,envLoader);
    return object;
  }

}//end if namespace plum

