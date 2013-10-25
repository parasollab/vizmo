#include "TransformTool.h"

#include <Quaternion.h>
using namespace mathtool;

#include <glut.h>

#include <QMouseEvent>
#include <QKeyEvent>

#include "Camera.h"
#include "PickBox.h"
#include "Models/Model.h"

////////////////////////////////////////////////////////////////////////////////
//Helper functions for Transform Tools
////////////////////////////////////////////////////////////////////////////////

inline Point3d
ProjectToWindow(const Point3d& _pt) {
  //Get matrix info
  int viewPort[4];
  double modelViewM[16], projM[16];

  glGetIntegerv(GL_VIEWPORT, viewPort );
  glGetDoublev(GL_MODELVIEW_MATRIX, modelViewM);
  glGetDoublev(GL_PROJECTION_MATRIX, projM);

  Point3d proj;
  gluProject(_pt[0], _pt[1], _pt[2],
      modelViewM, projM, viewPort,
      &proj[0], &proj[1], &proj[2]);
  return proj;
}

inline void
ProjectToWindow(Point3d* _pts, size_t _size) {
  //Get matrix info
  int viewPort[4];
  double modelViewM[16], projM[16];

  glGetIntegerv(GL_VIEWPORT, viewPort);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelViewM);
  glGetDoublev(GL_PROJECTION_MATRIX, projM);

  for(size_t i=0; i < _size; ++i) {
    Point3d proj;
    gluProject(_pts[i][0], _pts[i][1], _pts[i][2],
        modelViewM, projM, viewPort,
        &proj[0], &proj[1], &proj[2]);
    _pts[i] = proj;
  }
}

//x,y is window coord
//a reference point, ref, and a nornmal, n, defines a plane
//the unprojected point is intersetion of shooting ray from (x,y)
//and the plane defined by n and ref
inline Point3d
ProjectToWorld(double _x, double _y, const Point3d& _ref, const Vector3d& _n) {
  //Get matrix info
  int viewPort[4];
  double modelViewM[16], projM[16];

  glGetIntegerv(GL_VIEWPORT, viewPort);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelViewM);
  glGetDoublev(GL_PROJECTION_MATRIX, projM);

  Vector3d s,e; //start and end of ray

  //unproject to plane defined by current x and y direction
  gluUnProject(_x, _y, 0,
      modelViewM, projM, viewPort,
      &s[0], &s[1], &s[2]);
  gluUnProject(_x, _y, 1.0,
      modelViewM, projM, viewPort,
      &e[0], &e[1], &e[2]);

  double t=0;
  double base = (e-s)*_n;

  if(fabs(base)/(s-e).norm() < 1e-2) {
    Vector3d se = (s-e).normalize();
    Vector3d v = _n%se;
    Point3d proj = ProjectToWindow(_ref);

    //find d1 and d2
    double d1 = sqrt((proj[0]-_x)*(proj[0]-_x)+(proj[1]-_y)*(proj[1]-_y));
    Vector3d g = _ref + 0.1*v;
    Point3d projg = ProjectToWindow(g);
    Point3d xy(_x, _y, 0);

    if((projg - proj) * (xy - proj) < 0)
      d1 = -d1;

    d1 = fmod(d1, 200);
    if(d1 > 100)
      d1 = d1 - 200;
    else if(d1 < -100)
      d1 = 200 + d1;

    double d2;
    if(d1 > 50) {
      d1 = 100 - d1;
      d2 = -sqrt(2500 - d1*d1);
    }
    else if(d1<-50 ) {
      d1 = -100 - d1;
      d2 = -sqrt(2500 - d1*d1);
    }
    else
      d2 = sqrt(2500 - d1*d1);

    return _ref + d1*v + d2*se;
  }
  else {
    t = (_ref*_n - s*_n)/base;
    return (1-t)*s + t*e;
  }
}

inline Point3d
ProjectToWorld(double _x, double _y, const Point3d& _ref) {
  Camera* cam = GetCameraFactory().GetCurrentCamera();
  Vector3d n = cam->GetWindowZ();
  return ProjectToWorld(_x, _y, _ref, n);
}

inline void
DrawCircle(double _r) {
  glBegin(GL_LINE_LOOP);
  for(double t = 0; t < TWOPI; t += 0.2)
    glVertex2f(_r*cos(t), _r*sin(t));
  glEnd();
}

//Draw arc from radians s to e with radius r on plane made by v1, v2
inline void
DrawArc(double _r, double _s, double _e, const Vector3d& _v1, const Vector3d& _v2) {
  glBegin(GL_LINE_STRIP);
  for(float t = _s; t < _e; t += 0.2)
    glVertex3dv(_r*cos(t)*_v1 + _r*sin(t)*_v2);
  glVertex3dv(_r*cos(_e)*_v1 + _r*sin(_e)*_v2);
  glEnd();
}

///////////////////////////////////////////////////////////////////////////////
// TransformToolBase
///////////////////////////////////////////////////////////////////////////////

TransformableModel* TransformToolBase::m_obj = NULL;
Point3d TransformToolBase::m_objPosPrj;
Point3d TransformToolBase::m_xPrj;
Point3d TransformToolBase::m_yPrj;
Point3d TransformToolBase::m_zPrj;
int TransformToolBase::m_h=0;
int TransformToolBase::m_w=0;
int TransformToolBase::m_hitX=0;
int TransformToolBase::m_hitY=0;

void
TransformToolBase::SetSelectedObj(TransformableModel* _obj) {
  m_obj = _obj;
  ProjectToWindow();
}

void
TransformToolBase::SetWindowSize(int _w, int _h) {
  m_w = _w; m_h = _h;
  ProjectToWindow();
}

void
TransformToolBase::ProjectToWindow() {
  if(m_obj) {
    const Vector3d& pos = m_obj->Translation();

    Point3d pts[4] = {pos, pos + Point3d(0.1, 0, 0), pos + Point3d(0, 0.1, 0), pos + Point3d(0, 0, 0.1)};
    ::ProjectToWindow(pts, 4);
    m_objPosPrj = pts[0];
    m_xPrj = pts[1];
    m_yPrj = pts[2];
    m_zPrj = pts[3];
  }
}

void
TransformToolBase::Draw() {
  if(!m_obj) return;

  glMatrixMode(GL_PROJECTION); //change to Ortho view
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, m_w, 0, m_h, -100, 100);

  Draw(false);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

///////////////////////////////////////////////////////////////////////////////
// TranslationTool
///////////////////////////////////////////////////////////////////////////////

bool
TranslationTool::MousePressed(QMouseEvent* _e) {
  m_movementType = NON;
  //if object exists and left mouse button is pressed and selected
  if(m_obj && _e->button() == Qt::LeftButton &&
      Select(_e->pos().x(), _e->pos().y())) {
    m_objPosCatch = m_obj->Translation(); //store old pos
    m_objPosCatchPrj = m_objPosPrj;
    m_hitX = _e->pos().x();
    m_hitY = m_h - _e->pos().y();
    m_hitUnPrj = ProjectToWorld(m_hitX, m_hitY, m_objPosCatch);
    return true;
  }
  return false;
}

bool
TranslationTool::MouseReleased(QMouseEvent* _e) {
  if(m_movementType == NON)
    return false; //nothing selected
  m_movementType = NON;
  return true;
}

bool
TranslationTool::MouseMotion(QMouseEvent* _e) {
  if(m_movementType == NON)
    return false; //nothing selected

  int x = _e->pos().x();
  int y = m_h - _e->pos().y();

  Point3d curPos = ProjectToWorld(x, y, m_objPosCatch);
  Vector3d v = curPos - m_hitUnPrj;

  switch(m_movementType) {
    case X_AXIS:
      m_obj->Translation()[0] = m_objPosCatch[0] + v[0];
      break;
    case Y_AXIS:
      m_obj->Translation()[1] = m_objPosCatch[1] + v[1];
      break;
    case Z_AXIS:
      m_obj->Translation()[2] = m_objPosCatch[2] + v[2];
      break;
    case VIEW_PLANE:
      m_obj->Translation() = m_objPosCatch + v;
  }

  ProjectToWindow();

  return true;
}

void
TranslationTool::Draw(bool _selected) {

  glDisable(GL_LIGHTING);

  //draw reference axis
  double x = m_objPosPrj[0], y = m_objPosPrj[1];
  Point3d xdir = m_objPosPrj + (m_xPrj - m_objPosPrj).normalize()*50;
  Point3d ydir = m_objPosPrj + (m_yPrj - m_objPosPrj).normalize()*50;
  Point3d zdir = m_objPosPrj + (m_zPrj - m_objPosPrj).normalize()*50;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if(m_movementType!=NON) {
    glPointSize(3);
    glBegin(GL_POINTS);
    glVertex2d(m_objPosCatchPrj[0], m_objPosCatchPrj[1]);
    glEnd();

    glBegin(GL_LINES);
    glColor3d(0.6, 0.6, 0.6);
    glVertex3d(m_objPosCatchPrj[0], m_objPosCatchPrj[1], -5);
    glVertex3d(x, y, -5);
    glEnd();
  }

  //draw x axis
  if(_selected)
    glLoadName(X_AXIS);
  glBegin(GL_LINES);
  if(!_selected)
    glColor3f(1, 0, 0);
  glVertex2d(x, y);
  glVertex2d(xdir[0], xdir[1]);
  glEnd();

  //draw y axis
  if(_selected)
    glLoadName(Y_AXIS);
  glBegin(GL_LINES);
  if(!_selected)
    glColor3f(0, 1, 0);
  glVertex2d(x, y);
  glVertex2d(ydir[0], ydir[1]);
  glEnd();

  //draw z axis
  if(_selected)
    glLoadName(Z_AXIS);
  glBegin(GL_LINES);
  if(!_selected)
    glColor3f(0, 0, 1);
  glVertex2d(x, y);
  glVertex2d(zdir[0], zdir[1]);
  glEnd();

  //draw center
  if(!_selected) {
    glBegin(GL_LINE_LOOP);
    glColor3f(1, 1, 0);
    glVertex2d(x-5, y+5);
    glVertex2d(x+5, y+5);
    glVertex2d(x+5, y-5);
    glVertex2d(x-5, y-5);
    glEnd();
  }

  glPopMatrix();
}

bool
TranslationTool::Select(int _x, int _y) {
  _y = m_h - _y;

  //clicking on center box
  if(fabs(_x - m_objPosPrj[0]) < 10 &&
      fabs(_y - m_objPosPrj[1]) < 10) {
    m_movementType = VIEW_PLANE;
    return true;
  }

  // get view port
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // prepare for selection mode
  GLuint hitBuffer[1000];
  glSelectBuffer(1000, hitBuffer);
  glRenderMode(GL_SELECT);

  // name stack
  glInitNames();
  glPushName(0);

  // change view and draw
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(_x, _y, 10, 10, viewport);
  glOrtho(0, m_w, 0, m_h, -100, 100);

  Draw(true);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();

  //check result - 0 implies nothing selected
  if(glRenderMode(GL_RENDER) == 0)
    return false;

  m_movementType = (MovementType)hitBuffer[3];

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// RotationTool
///////////////////////////////////////////////////////////////////////////////

//angle of _s about the normal to the plane _v1, _v2. That is to say the
//angle between _s projected onto the plane of _v1, _v2 and _v2.
inline double
ComputeAngle(const Vector3d& _s, const Vector3d& _v1, const Vector3d& _v2){
  return atan2(_s*_v2, _s*_v1);
}

void
RotationTool::SetSelectedObj(TransformableModel* _obj) {
  TransformToolBase::SetSelectedObj(_obj);
  ComputeLocalAxis();
  ComputeArcs();
}

void
RotationTool::ComputeArcs() {
  Camera* cam = GetCameraFactory().GetCurrentCamera();
  Vector3d v = cam->GetWindowZ();
  ComputeArcs(m_arcs[0], m_localAxis[0], m_localAxis[1], m_localAxis[2], v);
  ComputeArcs(m_arcs[1], m_localAxis[1], m_localAxis[2], m_localAxis[0], v);
  ComputeArcs(m_arcs[2], m_localAxis[2], m_localAxis[0], m_localAxis[1], v);
}

bool
RotationTool::MousePressed(QMouseEvent* _e) {

  int x = _e->pos().x(), y = m_h - _e->pos().y();
  if(!m_obj || _e->button() != Qt::LeftButton || !Select(x, y))
    return false;

  m_objPosCatch = m_obj->Translation();
  m_objQuatCatch = m_obj->RotationQ();

  copy(m_localAxis, m_localAxis + 3, m_localAxisCatch);

  Vector3d axis, v1, v2; //rotation axis
  switch(m_movementType) {
    case X_AXIS:
      axis = m_localAxisCatch[0];
      v1 = m_localAxisCatch[1];
      v2 = m_localAxisCatch[2];
      break;
    case Y_AXIS:
      axis = m_localAxisCatch[1];
      v1 = m_localAxisCatch[2];
      v2 = m_localAxisCatch[0];
      break;
    case Z_AXIS:
      axis = m_localAxisCatch[2];
      v1 = m_localAxisCatch[0];
      v2 = m_localAxisCatch[1];
      break;
    case VIEW_PLANE:
      Camera* cam = GetCameraFactory().GetCurrentCamera();
      axis = cam->GetWindowZ();
      v1 = cam->GetWindowX();
      v2 = cam->GetWindowY();
      break;
  }

  Point3d prj = ProjectToWorld(x, y, m_objPosCatch, axis);
  m_curAngle = m_hitAngle = ComputeAngle(prj - m_objPosCatch, v1, v2);

  return true;
}

bool
RotationTool::MouseReleased(QMouseEvent* _e) {
  if(m_movementType == NON)
    return false; //nothing selected
  m_movementType = NON;
  return true;
}

bool
RotationTool::MouseMotion(QMouseEvent* _e) {

  if(m_movementType==NON)
    return false; //nothing selected

  int x = _e->pos().x();
  int y = m_h - _e->pos().y();

  Vector3d axis, v1, v2; //rotation axis
  switch(m_movementType) {
    case X_AXIS:
      axis = m_localAxisCatch[0];
      v1 = m_localAxisCatch[1];
      v2 = m_localAxisCatch[2];
      break;
    case Y_AXIS:
      axis = m_localAxisCatch[1];
      v1 = m_localAxisCatch[2];
      v2 = m_localAxisCatch[0];
      break;
    case Z_AXIS:
      axis = m_localAxisCatch[2];
      v1 = m_localAxisCatch[0];
      v2 = m_localAxisCatch[1];
      break;
    case VIEW_PLANE:
      Camera* cam = GetCameraFactory().GetCurrentCamera();
      axis = cam->GetWindowZ();
      v1 = cam->GetWindowX();
      v2 = cam->GetWindowY();
      break;
  }

  Point3d prj = ProjectToWorld(x, y, m_objPosCatch, axis);
  m_curAngle = ComputeAngle(prj - m_objPosCatch, v1, v2);

  double a = m_curAngle - m_hitAngle; //displacement angle
  //clamp between PI and -PI
  if(a > PI)
    a -= TWOPI;
  else if(a < -PI)
    a += TWOPI;

  //compute new q
  Quaternion q(cos(a/2), sin(a/2) * axis);
  m_obj->RotationQ() = (q*m_objQuatCatch).normalized();

  ComputeLocalAxis();
  ComputeArcs();

  return true;
}

void
RotationTool::Enable(){
  ComputeLocalAxis();
  ComputeArcs();
}

void
RotationTool::Draw(bool _selected) {

  //draw reference axis
  Camera* cam = GetCameraFactory().GetCurrentCamera();
  double x = m_objPosPrj[0], y = m_objPosPrj[1];

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glTranslatef(x, y, 0);

  glPushMatrix();
  glRotated(cam->GetCameraElev(), 1.0, 0.0, 0.0);
  glRotated(cam->GetCameraAzim(), 0.0, 1.0, 0.0);

  if(m_movementType != NON) {
    Vector3d v1, v2;
    switch(m_movementType) {
      case X_AXIS:
        v1 = m_localAxisCatch[1];
        v2 = m_localAxisCatch[2];
        break;
      case Y_AXIS:
        v1 = m_localAxisCatch[2];
        v2 = m_localAxisCatch[1];
        break;
      case Z_AXIS:
        v1 = m_localAxisCatch[0];
        v2 = m_localAxisCatch[1];
        break;
      case VIEW_PLANE:
        v1 = cam->GetWindowX();
        v2 = cam->GetWindowY();
        break;
    }

    Vector3d s = cos(m_hitAngle)*v1*m_radius + sin(m_hitAngle)*v2*m_radius;
    Vector3d e = cos(m_curAngle)*v1*m_radius + sin(m_curAngle)*v2*m_radius;

    //draw 2 points
    glPointSize(4);
    glBegin(GL_POINTS);
    glColor4f(0.6f, 0.6f, 0.6f, 0.6f);
    glVertex3d(s[0], s[1], s[2]);
    glVertex3d(e[0], e[1], e[2]);
    glEnd();
  }

  //draw circle around x axis
  if(_selected)
    glLoadName(X_AXIS);
  if(!_selected)
    glColor3f(1, 0, 0);
  DrawArc(m_radius, m_arcs[0][0], m_arcs[0][1], m_localAxis[1], m_localAxis[2]);

  //draw y axis
  if(_selected)
    glLoadName(Y_AXIS);
  if(!_selected)
    glColor3f(0, 1, 0);
  DrawArc(m_radius, m_arcs[1][0], m_arcs[1][1], m_localAxis[2], m_localAxis[0]);

  //draw z axis
  if(_selected)
    glLoadName(Z_AXIS);
  if(!_selected)
    glColor3f(0, 0, 1);
  DrawArc(m_radius, m_arcs[2][0], m_arcs[2][1], m_localAxis[0], m_localAxis[1]);

  glPopMatrix(); //pop camera rotation

  //draw center
  if(!_selected) {
    glColor3f(0.9f, 0.9f, 0);
    DrawCircle(m_radius + 5);
    glColor3f(0.6f, 0.6f, 0.6f);
    DrawCircle(m_radius);
  }

  glPopMatrix();
}

bool
RotationTool::Select(int _x, int _y) {

  // get view port
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport);

  // prepare for selection mode
  GLuint hitBuffer[1000];
  glSelectBuffer(1000, hitBuffer);
  glRenderMode(GL_SELECT);

  // number stack
  glInitNames();
  glPushName(0);

  // change view volume and draw
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(_x, _y, 10, 10, viewport);
  glOrtho(0, m_w, 0, m_h, -100, 100);

  Draw(true);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();

  //check result
  if(glRenderMode(GL_RENDER) > 0) {
    m_movementType = (MovementType)hitBuffer[3];
    return true;
  }

  double dx = m_objPosPrj[0] - _x, dy = m_objPosPrj[1] - _y;
  double dist = sqrt(dx*dx + dy*dy);
  if(dist < m_radius + 10 && dist > m_radius) {
    m_movementType = VIEW_PLANE;
    return true;
  }

  return false;
}

void
RotationTool::ComputeArcs(double angle[2], Vector3d& n, Vector3d& v1, Vector3d& v2, Vector3d& view) {
  Vector3d s = view % n;
  Vector3d e = n % view;
  angle[0] = ComputeAngle(s, v1, v2);
  angle[1] = ComputeAngle(e, v1, v2);
  if(angle[1] < angle[0])
    angle[1] += TWOPI;
}

void
RotationTool::ComputeLocalAxis() {
  if(!m_obj)
    return;

  static Vector3d x(1,0,0);
  static Vector3d y(0,1,0);
  static Vector3d z(0,0,1);

  const Quaternion& q = m_obj->RotationQ();
  m_localAxis[0] = (q*x*(-q)).imaginary();
  m_localAxis[1] = (q*y*(-q)).imaginary();
  m_localAxis[2] = (q*z*(-q)).imaginary();
}

Point3d
RotationTool::UnPrjToWorld(const Point3d& ref, const Vector3d& n, int _x, int _y) {
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport);

  // change view volume
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, m_w, 0, m_h, -100, 100);
  Point3d p = ProjectToWorld(_x, _y, ref, n);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  return p;
}

///////////////////////////////////////////////////////////////////////////////
// ScaleTool
///////////////////////////////////////////////////////////////////////////////

bool
ScaleTool::MousePressed(QMouseEvent* _e) {
  m_movementType = NON;
  int x = _e->pos().x(), y = m_h - _e->pos().y();
  if(m_obj && _e->button() == Qt::LeftButton && Select(x, y)) {
    m_objPosCatch = m_obj->Translation(); //store old pos
    m_objPosCatchPrj = m_objPosPrj;
    m_hitX = x;
    m_hitY = y;
    m_hitUnPrj = ProjectToWorld(m_hitX, m_hitY, m_objPosCatch);
    m_origScale = m_obj->Scale();
    return true;
  }
  return false;
}

bool
ScaleTool::MouseReleased(QMouseEvent* _e) {
  if(m_movementType == NON)
    return false; //nothing selected
  m_movementType = NON;
  return true;
}

bool
ScaleTool::MouseMotion(QMouseEvent* _e) {

  if(m_movementType == NON)
    return false; //nothing selected

  int x = _e->pos().x();
  int y = m_h - _e->pos().y();

  Point3d curPos = ProjectToWorld(x, y, m_objPosCatch);
  Vector3d v = (curPos-m_hitUnPrj)/10;

  switch(m_movementType) {
    case X_AXIS:
      if(m_origScale[0] + v[0] > 0)
        m_obj->Scale()[0] = m_origScale[0] + v[0];
      break;

    case Y_AXIS:
      if(m_origScale[1] + v[1] > 0)
        m_obj->Scale()[1] = m_origScale[1] + v[1];
      break;

    case Z_AXIS:
      if(m_origScale[2] + v[2] > 0)
        m_obj->Scale()[2] = m_origScale[2] + v[2];
      break;

    case VIEW_PLANE:
      if((m_origScale+v).norm() > 0)
        m_obj->Scale() = m_origScale + v;
      break;
  }

  ProjectToWindow();

  return true;
}

void
ScaleTool::Draw(bool _selected) {
  glDisable(GL_LIGHTING);
  //draw reference axis
  double x = m_objPosPrj[0], y = m_objPosPrj[1];
  Point3d xdir= m_objPosPrj + (m_xPrj - m_objPosPrj).normalize()*50;
  Point3d ydir= m_objPosPrj + (m_yPrj - m_objPosPrj).normalize()*50;
  Point3d zdir= m_objPosPrj + (m_zPrj - m_objPosPrj).normalize()*50;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  //draw x axis
  if(_selected)
    glLoadName(X_AXIS);
  glBegin(GL_LINE_LOOP);
  glColor3f(1, 0, 0);
  glVertex2d(xdir[0]-3, xdir[1]-3);
  glVertex2d(xdir[0]+3, xdir[1]-3);
  glVertex2d(xdir[0]+3, xdir[1]+3);
  glVertex2d(xdir[0]-3, xdir[1]+3);
  glEnd();

  //draw y axis
  if(_selected)
    glLoadName(Y_AXIS);
  glBegin(GL_LINE_LOOP);
  glColor3f(0, 1, 0);
  glVertex2d(ydir[0]-3, ydir[1]-3);
  glVertex2d(ydir[0]+3, ydir[1]-3);
  glVertex2d(ydir[0]+3, ydir[1]+3);
  glVertex2d(ydir[0]-3, ydir[1]+3);    //glVertex2d(m_yPrj[0],m_yPrj[1]);
  glEnd();

  //draw z axis
  if(_selected)
    glLoadName(Z_AXIS);
  glBegin(GL_LINE_LOOP);
  glColor3f(0, 0, 1);
  glVertex2d(zdir[0]-3, zdir[1]-3);
  glVertex2d(zdir[0]+3, zdir[1]-3);
  glVertex2d(zdir[0]+3, zdir[1]+3);
  glVertex2d(zdir[0]-3, zdir[1]+3);
  glEnd();

  //draw axis and center
  if(!_selected) {

    glBegin(GL_LINES);
    //x
    glColor3f(1, 0, 0);
    glVertex2d(x, y);
    glVertex2d(xdir[0], xdir[1]);
    //y
    glColor3f(0,1,0);
    glVertex2d(x,y);
    glVertex2d(ydir[0], ydir[1]);
    //z
    glColor3f(0,0,1);
    glVertex2d(x,y);
    glVertex2d(zdir[0], zdir[1]);
    glEnd();
    //center
    glBegin(GL_LINE_LOOP);
    glColor3f(0.9f, 0.9f, 0);
    glVertex2d(x-5, y+5);
    glVertex2d(x+5, y+5);
    glVertex2d(x+5, y-5);
    glVertex2d(x-5, y-5);
    glEnd();
  }

  glPopMatrix();
}

bool
ScaleTool::Select(int _x, int _y) {

  if(fabs(_x - m_objPosPrj[0]) < 10 && fabs(_y - m_objPosPrj[1]) < 10) {
    m_movementType = VIEW_PLANE;
    return true;
  }

  // get view port
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // prepare for selection mode
  GLuint hitBuffer[1000];
  glSelectBuffer(1000, hitBuffer);
  glRenderMode(GL_SELECT);

  // number stact
  glInitNames();
  glPushName(0);

  // change view volume and draw
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(_x, _y, 10, 10, viewport);
  glOrtho(0, m_w, 0, m_h, -100, 100);

  Draw(true);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();

  //check result. 0 implies nothing selected
  if(glRenderMode(GL_RENDER) == 0)
    return false;

  m_movementType = (MovementType)hitBuffer[3];
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// class TransformTool
///////////////////////////////////////////////////////////////////////////////

void
TransformTool::SetWindowSize(int _w, int _h) {
  m_translationTool.SetWindowSize(_w, _h);
  m_scaleTool.SetWindowSize(_w, _h);
}

void
TransformTool::CheckSelectObject() {
  m_rotationTool.ResetSelectedObj();

  //get selected objects
  const vector<Model*>& objs = GetPickedSceneObjs();
  if(!objs.empty())
    m_rotationTool.SetSelectedObj((TransformableModel*)objs.front());
}

void
TransformTool::Draw() {
  if(m_tool)
    m_tool->Draw();
}

bool
TransformTool::MousePressed(QMouseEvent* _e) {
  if(m_tool)
    return m_tool->MousePressed(_e);
  return false;
}

bool
TransformTool::MouseReleased(QMouseEvent* _e) {
  if(m_tool)
    return m_tool->MouseReleased(_e);
  return false;
}

bool
TransformTool::MouseMotion(QMouseEvent* _e) {
  if(m_tool)
    return m_tool->MouseMotion(_e);
  return false;
}

bool
TransformTool::KeyPressed(QKeyEvent* _e) {
  switch(_e->key()) {
    case 'q': case 'Q':
      if(m_tool)
        m_tool->Disable();
      m_tool = NULL;
      return true;

    case 'w': case 'W':
      if(m_tool)
        m_tool->Disable();
      m_tool = &m_translationTool;
      m_tool->Enable();
      return true;

    case 'e': case 'E':
      if(m_tool)
        m_tool->Disable();
      m_tool = &m_rotationTool;
      m_tool->Enable();
      return true;

    case 'r': case 'R':
      if(m_tool)
        m_tool->Disable();
      m_tool = &m_scaleTool;
      m_tool->Enable();
      return true;

    default:
      return false; //not handled
  }
}

void
TransformTool::CameraMotion() {
  m_translationTool.ProjectToWindow();
  m_scaleTool.ProjectToWindow();
  if(m_tool == &m_rotationTool)
    m_rotationTool.ComputeArcs(); //view angle changed...
}

