// GLModel.h: interface for the GLModel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef GLMODEL_H_
#define GLMODEL_H_

#include <list>
#include <string>
#include <sstream>
#include <cstring>

#include <QKeyEvent>

#include <gl.h>
#include "Utilities/GL/gliDataStructure.h"

namespace plum{
  
  enum RenderMode {WIRE_MODE, SOLID_MODE, INVISIBLE_MODE};
  enum BuildState {MODEL_OK, ENV_MODEL_ERROR, MAP_MODEL_ERROR, CLIENT_MODEL_ERROR};

   class GLModel : public gliTransform
   {
      public:
         GLModel()
         { 
            m_enableSelection=true; 
            m_renderMode = SOLID_MODE;
            m_RGBA.clear();
            for(int i = 0; i < 4; i++) {
              m_RGBA.push_back(0.0);
            }
         }

         GLModel(const GLModel& other) : gliTransform(other)
      {
         m_enableSelection=other.m_enableSelection;
         m_renderMode = other.m_renderMode;
         m_RGBA = other.m_RGBA; 
      }

         virtual ~GLModel(){/*do nothing*/}
         const string GetFilename() const { return m_filename; }
         void SetFilename(string _filename){ m_filename = _filename; }

         //////////////////////////////////////////////////////////////////////
         // Action functions
         //////////////////////////////////////////////////////////////////////
         virtual void EnableSelection( bool enable=true ){ 
            m_enableSelection=enable; 
         }

         virtual void Select( unsigned int * index, vector<gliObj>& sel ){/*nothing*/}
         virtual bool ParseFile(){ return true; }  //pure eventually 
         virtual bool BuildModels() =0;
         virtual void Draw( GLenum mode ) =0;

         //the scale may be difficult for some models....
         virtual void Scale( double x, double y, double z )
         { 
            m_Scale[0]=x; m_Scale[1]=y; m_Scale[2]=z; };

            //this function is called when this obj is selected
            virtual void DrawSelect(){/*nothing*/} 

            //set wire/solid/hide
            virtual void SetRenderMode(RenderMode mode){m_renderMode = mode;}

            //get/set color
            virtual void SetColor( float r, float g, float b, float a ){
              m_RGBA.clear(); 
              m_RGBA.push_back(r); m_RGBA.push_back(g); m_RGBA.push_back(b); m_RGBA.push_back(a);
            }
            
            vector<float> GetColor() const { return m_RGBA; } 

            //Get the name information
            virtual const string GetName() const =0;//{ return "unknow"; }

            //get the contained children if any
            virtual void GetChildren( list<GLModel*>& models )
            { /*do nothing as default*/ }

            //Get more detailde information
            virtual vector<string> GetInfo() const {return vector<string>();}

            virtual bool KP( QKeyEvent * e ) {return true;}

      public:
            bool  m_enableSelection;
            RenderMode   m_renderMode;     //wire or solid or hide
            vector<float> m_RGBA;  //Color 

      private:
            string m_filename; 
   };

}//namespace plum
#endif // !defined(_GLMODEL_H_)
