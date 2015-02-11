#ifndef MODEL_H_
#define MODEL_H_

#include <list>
#include <vector>
#include <string>
using namespace std;

#include "Quaternion.h"
using namespace mathtool;

#include <QMouseEvent>
#include <QKeyEvent>

#include <gl.h>
#include "Utilities/Color.h"
#include "Utilities/Camera.h"

enum RenderMode {INVISIBLE_MODE, WIRE_MODE, SOLID_MODE};

class Model {

  public:

    //construction
    Model(const string& _name) : m_name(_name), m_renderMode(SOLID_MODE),
        m_selectable(true), m_showNormals(false) {}
    virtual ~Model() {
      for(vector<Model*>::iterator cit = m_allChildren.begin();
          cit != m_allChildren.end(); ++cit)
        delete *cit;
    }

    const string& Name() const {return m_name;}

    RenderMode GetRenderMode() const {return m_renderMode;}
    virtual void SetRenderMode(RenderMode _mode) {m_renderMode = _mode;}

    bool IsSelectable() {return m_selectable;}
    virtual void SetSelectable(bool _s) {m_selectable = _s;}

    const Color4& GetColor() const {return m_color;}
    virtual void SetColor(const Color4& _c) {m_color = _c;}

    virtual void ToggleNormals() {m_showNormals = !m_showNormals;}

    //GetChildren for compatability until model constructors intantiate children
    virtual void GetChildren(list<Model*>& _models) {};
    const vector<Model*>& SelectableChildren() const
        {return m_selectableChildren;}
    const vector<Model*>& AllChildren() const {return m_allChildren;}

    //initialization of model
    virtual void Build() = 0;
    //determing if _index is this GL model
    virtual void Select(GLuint* _index, vector<Model*>& _sel) = 0;
    //draw under GL_RENDER mode
    virtual void DrawRender() = 0;
    //draw under GL_SELECT mode
    virtual void DrawSelect() = 0;
    //DrawSelected is only called when model is selected
    virtual void DrawSelected() = 0;
    //Draw Haptics for PHANToM device
    virtual void DrawHaptics() {};
    //output model info
    virtual void Print(ostream& _os) const = 0;

    //mouse/keyboard event handling
    virtual bool MousePressed(QMouseEvent* _e, Camera* _c = NULL) {return false;}
    virtual bool MouseReleased(QMouseEvent* _e, Camera* _c = NULL) {return false;}
    virtual bool MouseMotion(QMouseEvent* _e, Camera* _c = NULL) {return false;}
    virtual bool PassiveMouseMotion(QMouseEvent* _e, Camera* _c = NULL)
        {return false;}
    virtual bool KeyPressed(QKeyEvent* _e) {return false;}

  protected:

    string m_name;
    RenderMode m_renderMode;
    Color4 m_color;
    vector<Model*> m_selectableChildren, m_allChildren;
    bool m_selectable;
    bool m_showNormals;
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


class TransformableModel : public Model {

  public:

    TransformableModel(const string& _name) : Model(_name), m_scale(1, 1, 1) {}

    void Transform() {
      //translation applied last
      glTranslated(m_pos[0], m_pos[1], m_pos[2]);

      //rotation applied second.
      //Need to convert Quaternion to Angle-axis for OpenGL
      const Vector3d& v = m_rotQ.imaginary();
      double t = atan2d(v.norm(), m_rotQ.real())*2;
      glRotated(t, v[0], v[1], v[2]);

      //scaling applied first
      glScaled(m_scale[0], m_scale[1], m_scale[2]);
    }

    //Translation
    Vector3d& Translation() {return m_pos;}
    const Vector3d& Translation() const {return m_pos;}

    //Rotation
    Vector3d& Rotation() {return m_rot;}
    const Vector3d& Rotation() const {return m_rot;}
    Quaternion& RotationQ() {return m_rotQ;}
    const Quaternion& RotationQ() const {return m_rotQ;}

    //Scale
    Vector3d& Scale() {return m_scale;}
    const Vector3d& Scale() const {return m_scale;}

  protected:

    Vector3d m_pos, m_scale;
    Vector3d m_rot;
    Quaternion m_rotQ; //Rotation
};

#endif
