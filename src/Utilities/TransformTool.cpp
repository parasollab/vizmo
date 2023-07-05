#include "TransformTool.h"

#ifdef __APPLE__
  #include <OpenGL/glu.h>
#else
  #include <GL/glu.h>
#endif

#include <QMouseEvent>
#include <QKeyEvent>

#include "Camera.h"
#include "GLUtils.h"
#include "GUI/GLWidget.h"
#include "GUI/MainWindow.h"
#include "Models/Vizmo.h"
#include "Models/EnvModel.h"

TransformTool::
TransformTool(const Transformation& _t) : m_rotationsOn(true) {
  SetTransform(_t);
}

void
TransformTool::
SetTransform(const Transformation& _t) {
  m_translation = _t.translation();
  convertFromMatrix(m_rotation, _t.rotation().matrix());
}

inline void
DrawArrow(double _r) {
  GLUquadric* quad = gluNewQuadric();
  gluQuadricDrawStyle(quad, GLU_FILL);
  gluQuadricNormals(quad, GLU_SMOOTH);

  glPushMatrix();
  glTranslated(0, 0, _r * 1.6);

  gluCylinder(quad, 0.1*_r, 0, _r*0.4, 30, 1);

  glRotated(180, 0, 1, 0);
  gluDisk(quad, 0.05*_r, 0.1*_r, 30, 1);

  glPopMatrix();
}

inline void
DrawAxis(double _r) {
  GLUquadric* quad = gluNewQuadric();
  gluQuadricDrawStyle(quad, GLU_FILL);
  gluQuadricNormals(quad, GLU_SMOOTH);

  glPushMatrix();
  glTranslated(0, 0, _r);

  gluCylinder(quad, 0.1*_r, 0, _r*0.4, 30, 1);

  glRotated(180, 0, 1, 0);
  gluDisk(quad, 0.05*_r, 0.1*_r, 30, 1);

  glPopMatrix();

  glPushMatrix();
  glTranslated(0, 0, -_r);

  gluCylinder(quad, 0.05*_r, 0.05*_r, 2*_r, 30, 1);
  gluDisk(quad, 0.05*_r, 0.1*_r, 30, 1);

  glRotated(180, 0, 1, 0);
  gluCylinder(quad, 0.1*_r, 0, _r*0.4, 30, 1);

  glPopMatrix();

  gluDeleteQuadric(quad);
}

void
TransformTool::
Draw() {
  glEnable(GL_LIGHTING);

  double r = GetVizmo().GetEnv()->GetRadius() * 0.2;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(m_translation[0], m_translation[1], m_translation[2]);

  //////////////////////////////////////////////////////////////////////////////
  // draw reference axis

  //draw x axis
  glColor4f(1, 0, 0, 0.5);
  glPushMatrix();
  glRotated(90, 0, 1, 0);
  DrawAxis(r);
  glPopMatrix();

  //draw y axis
  glColor4f(0, 1, 0, 0.5);
  glPushMatrix();
  glRotated(-90, 1, 0, 0);
  DrawAxis(r);
  glPopMatrix();

  //draw z axis
  glColor4f(0, 0, 1, 0.5);
  DrawAxis(r);

  //////////////////////////////////////////////////////////////////////////////
  // draw rotation axis

  if(m_rotationsOn) {
    const Vector3d& v = m_rotation.imaginary();
    double t = atan2d(v.norm(), m_rotation.real())*2;
    Vector3d axis = t == 0 ? Vector3d() : v / sind(t / 2.0);
    glRotated(t, axis[0], axis[1], axis[2]);

    //Axis of rotation
    glColor4f(0.5, 0.5, 0.5, 0.5);
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    DrawArrow(r);
    glPopMatrix();

    //x axis
    glColor4f(1, 0, 0, 0.5);
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    GLUtils::DrawHollowCylinder(r*1.45, r*1.5, 0.2*r, 30);
    glPopMatrix();

    //draw y axis
    glColor4f(0, 1, 0, 0.5);
    glPushMatrix();
    glRotated(-90, 1, 0, 0);
    GLUtils::DrawHollowCylinder(r*1.5, r*1.55, 0.2*r, 30);
    glPopMatrix();

    //draw z axis
    glColor4f(0, 0, 1, 0.5);
    GLUtils::DrawHollowCylinder(r*1.55, r*1.6, 0.2*r, 30);
  }

  glPopMatrix();
}

void
TransformTool::
DrawSelect() {
  //draw reference axis
  double r = GetVizmo().GetEnv()->GetRadius() * 0.2;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(m_translation[0], m_translation[1], m_translation[2]);

  //////////////////////////////////////////////////////////////////////////////
  // draw reference axis

  //draw x axis
  glPushName(1);
  glPushMatrix();
  glRotated(90, 0, 1, 0);
  DrawAxis(r);
  glPopMatrix();
  glPopName();

  //draw y axis
  glPushName(2);
  glPushMatrix();
  glRotated(-90, 1, 0, 0);
  DrawAxis(r);
  glPopMatrix();
  glPopName();

  //draw z axis
  glPushName(3);
  DrawAxis(r);
  glPopName();

  //////////////////////////////////////////////////////////////////////////////
  // draw rotation axis

  if(m_rotationsOn) {
    const Vector3d& v = m_rotation.imaginary();
    double t = atan2d(v.norm(), m_rotation.real())*2;
    Vector3d axis = t == 0 ? Vector3d() : v / sind(t / 2.0);
    glRotated(t, axis[0], axis[1], axis[2]);

    //x axis
    glPushName(4);
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    GLUtils::DrawHollowCylinder(r*1.45, r*1.5, 0.2*r, 30);
    glPopMatrix();
    glPopName();

    //draw y axis
    glPushName(5);
    glPushMatrix();
    glRotated(-90, 1, 0, 0);
    GLUtils::DrawHollowCylinder(r*1.5, r*1.55, 0.2*r, 30);
    glPopMatrix();
    glPopName();

    //draw z axis
    glPushName(6);
    GLUtils::DrawHollowCylinder(r*1.55, r*1.6, 0.2*r, 30);
    glPopName();
  }

  glPopMatrix();
}

bool
TransformTool::
Select(int _x, int _y) {
  _y = GLUtils::windowHeight - _y;

  Point3d prj = GLUtils::ProjectToWindow(m_translation);
  //clicking on center box
  if(fabs(_x - prj[0]) < 10 && fabs(_y - prj[1]) < 10) {
    m_movementType = MovementType::ViewPlane;
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

  // change view and draw
  glMatrixMode(GL_PROJECTION);
  double pm[16]; //current projection matrix
  glGetDoublev(GL_PROJECTION_MATRIX, pm);

  glPushMatrix();
  glLoadIdentity();

  gluPickMatrix(_x, _y, 1, 1, viewport);
  glMultMatrixd(pm); //apply current proj matrix

  glMatrixMode(GL_MODELVIEW);
  DrawSelect();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();

  //check result - 0 implies nothing selected
  GLint hit = glRenderMode(GL_RENDER);
  if(hit == 0)
    return false;

  //////////////////////////////////////////////////////////////////////////////
  // Search selected items

  GLuint* ptr = hitBuffer;

  //init local data
  unique_ptr<GLuint[]> selName(nullptr);

  double z1; //near z for hit object
  double closeDistance = numeric_limits<double>::infinity();

  for(GLint i = 0; i < hit; ++i) {
    GLuint nameSize = ptr[0];
    z1 = (double)ptr[1]/numeric_limits<GLuint>::max(); //near z
    ptr += 3; //skip far z, we don't use this info

    if(z1 < closeDistance) {
      closeDistance = z1;     // set current nearset to z1
      selName = unique_ptr<GLuint[]>(new GLuint[nameSize]);
      copy(ptr, ptr + nameSize, selName.get());
    }

    ptr += nameSize;
  }

  //////////////////////////////////////////////////////////////////////////////
  // grab movement type
  m_movementType = (MovementType)selName[0];

  return true;
}

bool
TransformTool::
MousePressed(QMouseEvent* _e) {
  //if left mouse button is pressed and selected
  if(_e->button() == Qt::LeftButton &&
      Select(_e->pos().x(), _e->pos().y())) {
    //m_objPosCatch = m_obj->Translation(); //store old pos
    m_oldTranslation = m_translation;
    m_oldRotation = m_rotation;
    m_hitX = _e->pos().x();
    m_hitY = GLUtils::windowHeight - _e->pos().y();
    m_hitWorld = GLUtils::ProjectToWorld(m_hitX, m_hitY, m_translation,
        GetMainWindow()->GetGLWidget()->GetCurrentCamera()->GetDir());
    return true;
  }
  return false;
}

bool
TransformTool::
MouseReleased(QMouseEvent* _e) {
  if(m_movementType == MovementType::None)
    return false;
  m_movementType = MovementType::None;
  return true;
}

bool
TransformTool::
MouseMotion(QMouseEvent* _e) {
  if(m_movementType == MovementType::None)
    return false; //nothing selected

  int x = _e->pos().x();
  int y = GLUtils::windowHeight - _e->pos().y();

  Point3d curPos = GLUtils::ProjectToWorld(x, y, m_oldTranslation,
      GetMainWindow()->GetGLWidget()->GetCurrentCamera()->GetDir());
  Vector3d v = curPos - m_hitWorld;

  auto calcRotQuat = [&](const Vector3d& _n) {
    Vector3d n = (m_oldRotation * _n * (-m_oldRotation)).imaginary();
    Point3d hit = GLUtils::ProjectToWorld(m_hitX, m_hitY, m_translation, n);
    Point3d motion = GLUtils::ProjectToWorld(x, y, m_translation, n);
    Vector3d hp = hit - m_translation;
    Vector3d mp = motion - m_translation;
    Vector3d cross = hp%mp;
    double angle = atan2(cross.norm(), hp * mp);
    if(cross * n < 0)
      angle = -angle;

    return (m_oldRotation * Quaternion(cos(angle/2.0), _n * sin(angle/2.0))).normalized();
  };

  switch(m_movementType) {
    case MovementType::XAxis:
      m_translation[0] = m_oldTranslation[0] + v[0];
      break;
    case MovementType::YAxis:
      m_translation[1] = m_oldTranslation[1] + v[1];
      break;
    case MovementType::ZAxis:
      m_translation[2] = m_oldTranslation[2] + v[2];
      break;
    case MovementType::ViewPlane:
      m_translation = m_oldTranslation + v;
      break;
    case MovementType::XAxisRot:
      m_rotation = calcRotQuat(Vector3d(1, 0, 0));
      break;
    case MovementType::YAxisRot:
      m_rotation = calcRotQuat(Vector3d(0, 1, 0));
      break;
    case MovementType::ZAxisRot:
      m_rotation = calcRotQuat(Vector3d(0, 0, 1));
      break;
    default:
      break;
  }

  switch(m_movementType) {
    case MovementType::XAxis:
    case MovementType::YAxis:
    case MovementType::ZAxis:
    case MovementType::ViewPlane:
      emit TranslationChanged(m_translation);
      break;
    case MovementType::XAxisRot:
    case MovementType::YAxisRot:
    case MovementType::ZAxisRot:
      emit RotationChanged(m_rotation);
      break;
    default:
      break;
  }


  return true;
}

bool
TransformTool::
KeyPressed(QKeyEvent* _e) {
  return false;
}

void
TransformTool::
ChangeTranslation(const Vector3d& _t) {
  m_translation = _t;
}

void
TransformTool::
ChangeRotation(const Quaternion& _r) {
  m_rotation = _r;
}
