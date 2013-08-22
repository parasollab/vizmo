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

class GLModel : public GLTransform {
  public:
    GLModel() : m_renderMode(SOLID_MODE), m_color() {}
    GLModel(const GLModel& _other) :
      GLTransform(_other),
      m_renderMode(_other.m_renderMode),
      m_color(_other.m_color) {}

    virtual void GetChildren(list<GLModel*>& _models) {}

    virtual const string GetName() const = 0;
    virtual vector<string> GetInfo() const {return vector<string>();}

    RenderMode GetRenderMode() const {return m_renderMode;}
    virtual void SetRenderMode(RenderMode _mode) {m_renderMode = _mode;}

    const Color4& GetColor() const {return m_color;}
    virtual void SetColor(const Color4& _c) {m_color = _c;}

    const string& GetFilename() const {return m_filename;}
    void SetFilename(const string& _filename) {m_filename = _filename;}

    //determing if _index is this GL model
    virtual void Select(unsigned int* _index, vector<GLModel*>& _sel) {}

    //initialization of data from file
    virtual void ParseFile() {}
    //initialization of gl models
    virtual void BuildModels() {};

    //draw is called for the scene.
    virtual void Draw(GLenum _mode) = 0;
    //DrawSelect is only called if item is selected
    virtual void DrawSelect(){}

  protected:
    RenderMode m_renderMode;     //wire or solid or hide
    Color4 m_color;
    string m_filename;
};

#endif
