#include <cstring>

#include "Plum.h"
#include <GL/glu.h>
#include "Loadable.h"
#include "PlumObject.h"

namespace plum {

  void
  Plum::Clean() {

    m_plumObjects.clear();
    m_selectedItems.clear();
  }

  bool
  Plum::ParseFile(){

    typedef typename vector<PlumObject*>::iterator PIT;
    for(PIT pit = m_plumObjects.begin(); pit != m_plumObjects.end(); ++pit){
      Loadable* loader = (*pit)->GetLoader();
      //When all loaders are gone, virtual function ParseFile will be called
      //on all MODELS instead and this can be default rather than checked.
      //Or, ParseFile() can happen in model constructors if dependencies
      //allow
      if(!loader){
        //cout<<"No loader for " <<(*pit)->GetModel()->GetName()<<"; using model's ParseFile()"<<endl;
        ((*pit)->GetModel())->ParseFile();
        continue;
      }

      if(!loader->ParseFile()) //current default that will go away...
        return false;
    }
    return true;
  }

  BuildState
  Plum::BuildModels(){

    typedef typename vector<PlumObject*>::iterator PIT;
    for(PIT pit = m_plumObjects.begin(); pit != m_plumObjects.end(); ++pit){
      GLModel* model = (*pit)->GetModel();
      if(!model)
        continue;
      if(!model->BuildModels()){
        cerr << "Couldn't build model: " << model->GetName()<< endl;
        return CLIENT_MODEL_ERROR;
      }
    }
    return MODEL_OK;
  }

  void
  Plum::Draw(){

    typedef typename vector<PlumObject*>::iterator PIT;
    for(PIT pit = m_plumObjects.begin(); pit!=m_plumObjects.end(); ++pit){
      GLModel* model = (*pit)->GetModel();
      if(!model)
        continue;
      glEnable(GL_LIGHTING);
      model->Draw(GL_RENDER);
    }

    typedef vector<gliObj>::iterator GIT;
    for(GIT ig = m_selectedItems.begin(); ig != m_selectedItems.end(); ig++){
      GLModel* model = (GLModel*)(*ig);
      if(model != NULL)
        model->DrawSelect();
    }
  }

#define BUFFER_SIZE 1024

  void
    Plum::Select(const gliBox& box) {
      GLuint hitBuffer[BUFFER_SIZE];
      GLint viewport[4];
      GLuint hits;

      // prepare for selection mode
      glSelectBuffer( BUFFER_SIZE, hitBuffer);
      glRenderMode( GL_SELECT );

      // get view port
      glGetIntegerv( GL_VIEWPORT, viewport);

      // initialize stack
      glInitNames();

      // change view volum
      glMatrixMode( GL_PROJECTION );
      double pm[16]; //current projection matrix
      glGetDoublev(GL_PROJECTION_MATRIX,pm);

      glPushMatrix();
      glLoadIdentity();
      double x=(box.l+box.r)/2;
      double y=(box.t+box.b)/2;
      double w=fabs(box.r-box.l); if( w<5 ) w=5;
      double h=fabs(box.t-box.b); if( h<5 ) h=5;
      gluPickMatrix( x, y, w, h, viewport);
      glMultMatrixd(pm); //apply current proj matrix

      //draw
      glMatrixMode( GL_MODELVIEW );
      int objSize=m_plumObjects.size();
      for( int iCM=0; iCM<objSize; iCM++ ){
        glPushName(iCM);
        GLModel* model = m_plumObjects[iCM]->GetModel();
        if( model==NULL ) continue;
        model->Draw( GL_SELECT );
        glPopName();
      }

      glMatrixMode( GL_PROJECTION );
      glPopMatrix();

      hits = glRenderMode( GL_RENDER );
      SearchSelectedItems(hits, hitBuffer,(w*h)>100);
    }

  //Parse the hitbuffer
  void
    Plum::SearchSelectedItems(int hits, void * buffer, bool all) {
      /////////////////////////////////////////////////////
      // unselect everything first
      m_selectedItems.clear();

      //init local data
      GLuint * hitBuffer = (GLuint *)buffer;
      unsigned int * selName=NULL;

      //input error
      if( hitBuffer==NULL || hits==0 ) return;

      GLuint * ptr=hitBuffer;
      double z1; //near z for hit object
      double closeDistance = 1e3;

      for( int i=0; i<hits; i++ ) {
        unsigned int * curName=NULL;
        GLuint NameSize = *ptr; ptr++;
        z1 = ((double)*ptr)/0x7fffffff; ptr++; //near z
        ptr++; //far z, we don't use this info

        if((curName=new unsigned int[NameSize])==NULL) return;
        for( unsigned int iN=0; iN<NameSize; iN++ ){
          curName[iN] = (int)(*ptr);
          ptr++;
        }
        if(!all) {//not all
          if( z1<closeDistance ) {
            closeDistance = z1;     // set current nearset to z1
            delete [] selName;      //free preallocated mem
            if((selName=new unsigned int[NameSize])==NULL) return;
            memcpy(selName,curName,sizeof(unsigned int)*NameSize);
          }
        }
        else{ //select all
          if(curName[0] > m_plumObjects.size())
            return;
          GLModel* selectModel = m_plumObjects[curName[0]]->GetModel();
          if(selectModel!=NULL)
            selectModel->Select(&curName[1], m_selectedItems);
        }

        delete [] curName;  //free preallocated mem
      }//end for each hit

      //only the closest
      if( !all ){ //
        // analyze selected item //not name which created in this lib
        if(selName[0] > m_plumObjects.size())
          return;
        GLModel* selectModel = m_plumObjects[selName[0]]->GetModel();
        if(selectModel != NULL){
          selectModel->Select(&selName[1], m_selectedItems);
        }
      }
      delete [] selName;
    }

}

