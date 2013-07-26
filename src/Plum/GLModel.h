#ifndef GLMODEL_H_
#define GLMODEL_H_

#include <list>
#include <string>
#include <sstream>
#include <cstring>

#include <QKeyEvent>

#include <gl.h>
#include "Utilities/GL/gliDataStructure.h"
#include "Utilities/Color.h"

enum RenderMode {WIRE_MODE, SOLID_MODE, INVISIBLE_MODE};

class GLModel : public gliTransform
{
  public:
    GLModel() : m_color() {
      m_enableSelection=true;
      m_renderMode = SOLID_MODE;
    }

    GLModel(const GLModel& _other) : gliTransform(_other) {
      m_enableSelection = _other.m_enableSelection;
      m_renderMode = _other.m_renderMode;
      m_color = _other.m_color;
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
        m_scale[0]=x; m_scale[1]=y; m_scale[2]=z; };

    //this function is called when this obj is selected
    virtual void DrawSelect(){/*nothing*/}

    //set wire/solid/hide
    virtual void SetRenderMode(RenderMode mode){m_renderMode = mode;}

    //get/set color
    virtual void SetColor(const Color4& _c) {m_color = _c;}
    const Color4& GetColor() const {return m_color;}

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

  private:
    string m_filename;
    Color4 m_color;  //Color
};

#endif
