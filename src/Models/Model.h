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

enum RenderMode {WIRE_MODE, SOLID_MODE, INVISIBLE_MODE};

class Model : public GLTransform {
  public:
    Model() : m_renderMode(SOLID_MODE) {}
    ~Model() {
      for(vector<Model*>::iterator cit = m_children.begin(); cit != m_children.end(); ++cit)
        delete *cit;
    }

    virtual const string GetName() const = 0;
    //virtual void Print(ostream& _os) const = 0;
    virtual vector<string> GetInfo() const {return vector<string>();};

    //const vector<Model*>& GetChildren() {return m_children;}
    virtual void GetChildren(list<Model*>& _children) {};

    RenderMode GetRenderMode() const {return m_renderMode;}
    virtual void SetRenderMode(RenderMode _mode) {m_renderMode = _mode;}

    const Color4& GetColor() const {return m_color;}
    virtual void SetColor(const Color4& _c) {m_color = _c;}

    const string& GetFilename() const {return m_filename;}
    void SetFilename(const string& _filename) {m_filename = _filename;}

    //initialization of data from file
    virtual void ParseFile() {}
    //initialization of gl models
    virtual void BuildModels() {};

    //determing if _index is this GL model
    virtual void Select(unsigned int* _index, vector<Model*>& _sel) {}

    //draw is called for the scene.
    virtual void Draw(GLenum _mode) = 0;
    //DrawSelect is only called if item is selected
    virtual void DrawSelect(){}

  protected:
    RenderMode m_renderMode;     //wire or solid or hide
    Color4 m_color;
    string m_filename;
    vector<Model*> m_children;
};

#endif
