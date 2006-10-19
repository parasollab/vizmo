// Plum.cpp
//
//////////////////////////////////////////////////////////////////////

#include "Plum.h"
#include <GL/glu.h>

namespace plum{

    /*********************************************************************
    *
    *      Implementation of CPlum
    *
    *********************************************************************/

    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CPlum::CPlum()
    {
        //m_SelectedSize = 0;
        //m_SelectedName = NULL;
    }

    CPlum::~CPlum()
    {
    }

    void CPlum::Clean()
    {
        m_ObjList.clear();
        m_SelectedItem.clear();
    }

    void CPlum::CleanSelectedItem()
    {
        m_SelectedItem.clear();
    }

    int 
    CPlum::ParseFile()
    {
        int objSize=m_ObjList.size();
        for( int iCM=0; iCM<objSize; iCM++ ){
            I_Loadable * loader=m_ObjList[iCM]->getLoader();
            if( loader==NULL ) continue;
            if( loader->ParseFile()==false ) return CPlumState::PARSE_ERROR;
        }

        return CPlumState::PARSE_OK;
    }


    int CPlum::BuildModels(){
            
        /////////////////////////////////////////////////////
        int objSize=m_ObjList.size();
        for( int iCM=0; iCM<objSize; iCM++ ){

            CGLModel * model=m_ObjList[iCM]->getModel();
            if( model==NULL ) continue;
            if( model->BuildModels()==false ){
	      cout<<"Couldn't build model..."<<endl;
	      return CPlumState::BUILD_CLIENT_MODEL_ERROR;
	    }
       }
        
        return CPlumState::BUILD_MODEL_OK;
    }

    void CPlum::Draw()
    {
        int objSize=m_ObjList.size();
        for( int iCM=0; iCM<objSize; iCM++ ){
            CGLModel * model=m_ObjList[iCM]->getModel();
            if( model==NULL ) continue;
            glEnable(GL_LIGHTING);
            model->Draw( GL_RENDER );
        }
        typedef vector<gliObj>::iterator GIT;
        for(GIT ig=m_SelectedItem.begin();ig!=m_SelectedItem.end();ig++){
            CGLModel * model=(CGLModel*)(*ig);
            model->DrawSelect();
        }
    }

    #define BUFFER_SIZE 1024

    void CPlum::Select(const gliBox& box)
    { 
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
        int objSize=m_ObjList.size();
        for( int iCM=0; iCM<objSize; iCM++ ){
            glPushName(iCM);
            CGLModel * model=m_ObjList[iCM]->getModel();
            if( model==NULL ) continue;
            model->Draw( GL_SELECT );
            glPopName();
        }
        
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        
        hits = glRenderMode( GL_RENDER );
        SearchSelectedItem(hits, hitBuffer,(w*h)>100);
    }

    //Parse the hitbuffer
    void CPlum::SearchSelectedItem(int hits, void * buffer, bool all)
    {
        /////////////////////////////////////////////////////
        // unselect everything first
        m_SelectedItem.clear();

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
                if( curName[0]>m_ObjList.size() ) return;
                CGLModel * selectModel=m_ObjList[curName[0]]->getModel();
                if( selectModel!=NULL ) 
                    selectModel->Select( &curName[1], m_SelectedItem );
            }

            delete [] curName;  //free preallocated mem
        }//end for each hit

        //only the closest
        if( !all ){ //
            // analyze selected item //not name which created in this lib
            if( selName[0]>m_ObjList.size() ) return;
            CGLModel * selectModel=m_ObjList[selName[0]]->getModel();
            if( selectModel!=NULL ){ 
                selectModel->Select( &selName[1], m_SelectedItem );
	    }
        }
        delete [] selName;
    }

}//namespace plum

