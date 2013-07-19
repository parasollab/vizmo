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

  class GLModel : public gliTransform
  {
    public:
      GLModel()
      {
        m_enableSelection=true;
        m_renderMode = SOLID_MODE;
        m_rGBA.clear();
        for(int i = 0; i < 4; i++) {
          m_rGBA.push_back(0.0);
        }
      }

      GLModel(const GLModel& other) : gliTransform(other)
    {
      m_enableSelection=other.m_enableSelection;
      m_renderMode = other.m_renderMode;
      m_rGBA = other.m_rGBA;
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

      virtual void Select( unsigned int * index, vector<GLModel*>& sel ){/*nothing*/}
      virtual void ParseFile() {}
      virtual void BuildModels() {};
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
        m_rGBA.clear();
        m_rGBA.push_back(r); m_rGBA.push_back(g); m_rGBA.push_back(b); m_rGBA.push_back(a);
      }

      vector<float> GetColor() const { return m_rGBA; }

      //Get the name information
      virtual const string GetName() const =0;

      //get the contained children if any
      virtual void GetChildren( list<GLModel*>& models )
      { /*do nothing as default*/ }

      //Get more detailde information
      virtual vector<string> GetInfo() const {return vector<string>();}

      virtual bool KP( QKeyEvent * e ) {return true;}

    public:
      bool  m_enableSelection;
      RenderMode   m_renderMode;     //wire or solid or hide
      vector<float> m_rGBA;  //Color

    private:
      string m_filename;
  };

}//namespace plum
#endif // !defined(_GLMODEL_H_)
