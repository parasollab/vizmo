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

namespace plum{
    
    class CGLModel : public gliTransform
    {
    public:
        CGLModel()
        { 
            m_EnableSeletion=true; 
            m_RenderMode=CPlumState::MV_SOLID_MODE; 
        }
        
        virtual ~CGLModel(){/*do nothing*/}
        
        //////////////////////////////////////////////////////////////////////
        // Action functions
        //////////////////////////////////////////////////////////////////////
        virtual void EnableSelection( bool enable=true ){ 
            m_EnableSeletion=enable; 
        }
        
        virtual void Select( unsigned int * index, vector<gliObj>& sel ){/*nothing*/}
        virtual bool BuildModels() =0;
        virtual void Draw( GLenum mode ) =0;
        
        //this function is called when this obj is selected
        virtual void DrawSelect(){/*nothing*/} 
        
        //set wire/solid/hide
        virtual void SetRenderMode( int mode ){ m_RenderMode=mode; }
        
        //get/set color
        virtual void SetColor( float r, float g, float b, float a ){
            m_RGBA[0]=r; m_RGBA[1]=g; m_RGBA[2]=b; m_RGBA[3]=a;
        }
        virtual const float * GetColor() const { return m_RGBA; }

        //Get the name information
        virtual const string GetName() const =0;//{ return "unknow"; }

        //get the contained children if any
        virtual void GetChildren( list<CGLModel*>& models )
        { /*do nothing as default*/ }

        //Get more detailde information
        virtual list<string> GetInfo() const { return list<string>(); }

    protected:
        bool  m_EnableSeletion;
        int   m_RenderMode;     //wire or solid or hide
        float m_RGBA[4];        //Color
    };
    
}//namespace plum
#endif // !defined(_GLMODEL_H_)
