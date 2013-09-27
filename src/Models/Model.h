#ifndef MODEL_H_
#define MODEL_H_

#include <list>
#include <string>
#include <sstream>
#include <cstring>

#include <QKeyEvent>

#include <gl.h>
#include "Utilities/GL/GLTransform.h"
#include "Utilities/Color.h"

enum RenderMode {INVISIBLE_MODE, WIRE_MODE, SOLID_MODE};

class Model : public GLTransform {
  public:
    Model(const string& _name) : m_name(_name), m_renderMode(SOLID_MODE) {}
    ~Model() {
      for(vector<Model*>::iterator cit = m_allChildren.begin();
          cit != m_allChildren.end(); ++cit)
        delete *cit;
    }

    const string& Name() const {return m_name;}

    RenderMode GetRenderMode() const {return m_renderMode;}
    virtual void SetRenderMode(RenderMode _mode) {m_renderMode = _mode;}

    const Color4& GetColor() const {return m_color;}
    virtual void SetColor(const Color4& _c) {m_color = _c;}

    //GetChildren for compatability until model constructors intantiate children
    virtual void GetChildren(list<Model*>& _models) {};
    const vector<Model*>& SelectableChildren() const {return m_selectableChildren;}
    const vector<Model*>& AllChildren() const {return m_allChildren;}

    //initialization of gl models
    virtual void BuildModels() = 0;
    //determing if _index is this GL model
    virtual void Select(GLuint* _index, vector<Model*>& _sel) = 0;
    //draw is called for the scene.
    virtual void Draw(GLenum _mode) = 0;
    //DrawSelect is only called if item is selected
    virtual void DrawSelect() = 0;
    //output model info
    virtual void Print(ostream& _os) const = 0;

  protected:
    string m_name;
    RenderMode m_renderMode;
    Color4 m_color;
    vector<Model*> m_selectableChildren, m_allChildren;
};

class LoadableModel : public Model {
  public:
    LoadableModel(const string& _name) : Model(_name) {}

    const string& GetFilename() const {return m_filename;}
    void SetFilename(const string& _filename) {m_filename = _filename;}

    //initialization of data from file
    virtual void ParseFile() = 0;

  protected:
    string m_filename;
};

#endif
