// GLModel.h: interface for the CGLModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_GLMODEL_H_)
#define _GLMODEL_H_

#if defined(WIN32)
#include <windows.h>
#pragma warning( disable : 4244 4786 )
#endif

#include <GL/gl.h>
#include <GL/gliDataStructure.h>

#include "PlumState.h"
#include <list>
#include <string>
#include <sstream>
#include <cstring>

#include <QKeyEvent>

namespace plum{

   class CGLModel : public gliTransform
   {
      public:
         CGLModel()
         { 
            m_enableSelection=true; 
            m_RenderMode=CPlumState::MV_SOLID_MODE;
            m_RGBA.clear();
            for(int i = 0; i < 4; i++) {
              m_RGBA.push_back(0.0);
            }
         }

         CGLModel(const CGLModel& other) : gliTransform(other)
      {
         m_enableSelection=other.m_enableSelection;
         m_RenderMode=other.m_RenderMode;
         m_RGBA = other.m_RGBA; 
      }

         virtual ~CGLModel(){/*do nothing*/}

         //////////////////////////////////////////////////////////////////////
         // Action functions
         //////////////////////////////////////////////////////////////////////
         virtual void EnableSelection( bool enable=true ){ 
            m_enableSelection=enable; 
         }

         virtual void Select( unsigned int * index, vector<gliObj>& sel ){/*nothing*/}
         virtual bool BuildModels() =0;
         virtual void Draw( GLenum mode ) =0;

         //the scale may be difficult for some models....
         virtual void Scale( double x, double y, double z )
         { 
            m_Scale[0]=x; m_Scale[1]=y; m_Scale[2]=z; };

            //this function is called when this obj is selected
            virtual void DrawSelect(){/*nothing*/} 

            //set wire/solid/hide
            virtual void SetRenderMode( int mode ){ m_RenderMode=mode; }

            //get/set color
            virtual void SetColor( float r, float g, float b, float a ){
              m_RGBA.clear(); 
              m_RGBA.push_back(r); m_RGBA.push_back(g); m_RGBA.push_back(b); m_RGBA.push_back(a);
            }
            //virtual const float * GetColor() const { return m_RGBA; }
            vector<float> GetColor() const { return m_RGBA; } 

            //Get the name information
            virtual const string GetName() const =0;//{ return "unknow"; }

            //get the contained children if any
            virtual void GetChildren( list<CGLModel*>& models )
            { /*do nothing as default*/ }

            //Get more detailde information
            virtual list<string> GetInfo() const { return list<string>(); }

            virtual bool KP( QKeyEvent * e ) {return true;}

      public:
            bool  m_enableSelection;
            int   m_RenderMode;     //wire or solid or hide

            vector<float> m_RGBA;  //Color  
   };

}//namespace plum
#endif // !defined(_GLMODEL_H_)
