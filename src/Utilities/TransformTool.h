#ifndef TRANSFORMTOOL_H_
#define TRANSFORMTOOL_H_

#include<Vector.h>
#include<Quaternion.h>
using namespace mathtool;

#include <qgl.h>

class Camera;
class TransformableModel;

///////////////////////////////////////////////////////////////////////////////
// TransformToolBase
///////////////////////////////////////////////////////////////////////////////

class TransformToolBase {
  public:
    TransformToolBase() {}

    static void SetCurrentCamera(Camera* _camera) {m_currentCamera = _camera;}

    void ResetSelectedObj() {m_obj = NULL;}
    virtual void SetSelectedObj(TransformableModel* _obj);
    void ProjectToWindow();

    void Draw();
    virtual bool MousePressed(QMouseEvent* _e) = 0;
    virtual bool MouseReleased(QMouseEvent* _e) = 0;
    virtual bool MouseMotion(QMouseEvent* _e) = 0;
    virtual void Enable() {};   //called when this tool is activated
    virtual void Disable() {};  //called when this tool is unactivated

  protected:
    enum MovementType{NON, X_AXIS, Y_AXIS, Z_AXIS, VIEW_PLANE}; //move where?

    virtual void Draw(bool _selected) = 0;

    static TransformableModel* m_obj;    //selected Object
    static Camera* m_currentCamera;
    static Point3d m_objPosPrj; //project(m_obj.pos and 3 axis), (win coord)
    static Point3d m_xPrj, m_yPrj, m_zPrj;
    static int m_hitX, m_hitY;    //mouse hit on m_hitX, m_hitY (win coord)
};

///////////////////////////////////////////////////////////////////////////////
// TranslationTool
///////////////////////////////////////////////////////////////////////////////

class TranslationTool : public TransformToolBase {
  public:

    TranslationTool():TransformToolBase() {m_movementType = NON;}

    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);

  protected:
    void Draw(bool _selected);
    bool Select(int _x, int _y);

  private:
    MovementType m_movementType; //which axis is selected
    Vector3d m_deltaDis;         //displacement caused by user
    Point3d m_hitUnPrj;          //unproject(m_w, m_h)
    Point3d m_objPosCatch;       //catch for m_obj->pos
    Point3d m_objPosCatchPrj;    //catch for m_objPosPrj
};

///////////////////////////////////////////////////////////////////////////////
// RotationTool
///////////////////////////////////////////////////////////////////////////////

class RotationTool : public TransformToolBase {
  public:

    RotationTool() :
      TransformToolBase(), m_movementType(NON),
      m_hitAngle(0), m_curAngle(0), m_radius(50) {}

    void SetSelectedObj(TransformableModel* _obj);
    void ComputeArcs(); //compute values for m_arcs, called when view changed

    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    void Enable();

  protected:

    void Draw(bool _selected);
    bool Select(int _x, int _y);

  private:

    void ComputeArcs(double _angle[2], Vector3d& _n, Vector3d& _v1, Vector3d& _v2, Vector3d& _view);
    void ComputeLocalAxis();
    Point3d UnPrjToWorld(const Point3d& _ref, const Vector3d& _n, int _x, int _y);

    MovementType m_movementType;  //which axis is selected
    Vector3d m_localAxis[3];      //axis of object
    Vector3d m_localAxisCatch[3]; //catch for m_localAxis[3]
    double m_hitAngle;            //the angle when mouse clikced.
    double m_curAngle;            //current angle of mouse point
    Point3d m_objPosCatch;        //catch for m_obj->pos
    Quaternion m_objQuatCatch;    //catch for m_obj->q
    double m_radius;              //Radius of tool
    double m_arcs[3][2];          //start/end of each arc
};

///////////////////////////////////////////////////////////////////////////////
// ScaleTool
///////////////////////////////////////////////////////////////////////////////

class ScaleTool : public TransformToolBase {
  public:

    ScaleTool() : TransformToolBase(), m_movementType(NON) {}

    bool MousePressed(QMouseEvent * _e);
    bool MouseReleased(QMouseEvent * _e);
    bool MouseMotion(QMouseEvent * _e);

  protected:

    void Draw(bool _selected);
    bool Select(int _x, int _y);

  private:

    MovementType  m_movementType; //which axis is selected
    Vector3d m_deltaDis;          //displacement caused by user
    Point3d  m_hitUnPrj;          //unproject(m_w, m_h)
    Point3d  m_objPosCatch;       //catch for m_obj->pos
    Point3d  m_objPosCatchPrj;    //catch for m_objPosPrj
    Vector3d m_origScale;         //old scale
};

///////////////////////////////////////////////////////////////////////////////
// TransformTool
///////////////////////////////////////////////////////////////////////////////

class TransformTool {
  public:
    TransformTool(Camera* _camera) : m_tool(NULL) {
      TransformToolBase::SetCurrentCamera(_camera);
    }

    void CheckSelectObject();
    void ResetSelectedObj() {m_tool->ResetSelectedObj();}
    void ProjectToWindow() {m_translationTool.ProjectToWindow(); m_scaleTool.ProjectToWindow();}

    void Draw();
    bool MousePressed(QMouseEvent* _e);
    bool MouseReleased(QMouseEvent* _e);
    bool MouseMotion(QMouseEvent* _e);
    bool KeyPressed(QKeyEvent* _e);
    void CameraMotion();

  private:
    TransformToolBase* m_tool; //current tool or NULL if none selected
    TranslationTool m_translationTool;
    RotationTool m_rotationTool;
    ScaleTool m_scaleTool;
};

#endif
