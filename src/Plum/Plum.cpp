#include "Plum.h"

#include <cstring>

#include <GL/glu.h>

namespace plum {

  void
    Plum::Clean() {
      m_glModels.clear();
      m_selectedItems.clear();
    }

  bool
    Plum::ParseFile() {
      typedef vector<GLModel*>::iterator MIT;
      for(MIT mit = m_glModels.begin(); mit != m_glModels.end(); ++mit)
        if(!(*mit)->ParseFile())
          return false;
      return true;
    }

  BuildState
    Plum::BuildModels() {
      typedef vector<GLModel*>::iterator MIT;
      for(MIT mit = m_glModels.begin(); mit != m_glModels.end(); ++mit) {
        if(!(*mit)->BuildModels()) {
          cerr << "Couldn't build model: " << (*mit)->GetName()<< endl;
          return CLIENT_MODEL_ERROR;
        }
      }
      return MODEL_OK;
    }

  void
    Plum::Draw(){

      typedef vector<GLModel*>::iterator MIT;
      for(MIT mit = m_glModels.begin(); mit!=m_glModels.end(); ++mit){
        if(!*mit)
          continue;
        glEnable(GL_LIGHTING);
        (*mit)->Draw(GL_RENDER);
      }

      for(MIT mit = m_selectedItems.begin(); mit != m_selectedItems.end(); ++mit)
        (*mit)->DrawSelect();
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
      int objSize=m_glModels.size();
      for( int iCM=0; iCM<objSize; iCM++ ){
        glPushName(iCM);
        GLModel* model = m_glModels[iCM];
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
          if(curName[0] > m_glModels.size())
            return;
          GLModel* selectModel = m_glModels[curName[0]];
          if(selectModel!=NULL)
            selectModel->Select(&curName[1], m_selectedItems);
        }

        delete [] curName;  //free preallocated mem
      }//end for each hit

      //only the closest
      if( !all ){ //
        // analyze selected item //not name which created in this lib
        if(selName[0] > m_glModels.size())
          return;
        GLModel* selectModel = m_glModels[selName[0]];
        if(selectModel != NULL){
          selectModel->Select(&selName[1], m_selectedItems);
        }
      }
      delete [] selName;
    }

}

