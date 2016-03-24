#include "GLUtils.h"

#include "glut.h"
#include "VizmoExceptions.h"


namespace GLUtils {

  int windowWidth  = 0;
  int windowHeight = 0;


  Point3d
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


  void
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


  Point3d
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

    Vector3d ray = (e - s).normalize();
    Vector3d norm = _n.normalize();
    if(fabs(ray * norm) < numeric_limits<double>::epsilon() )
      throw DrawException(WHERE,
          "\nCannot unproject: the plane is perpendicular to the ray.\n");
    double rayFactor = (_ref - s) * norm / (ray * norm);
    return rayFactor * ray + s;
  }


  void
  DrawCircle(double _r, bool _fill) {
    glBegin(_fill ? GL_POLYGON : GL_LINE_LOOP);
    for(double t = 0; t < TWOPI; t += 0.2)
      glVertex2f(_r*cos(t), _r*sin(t));
    glEnd();
  }


  void
  DrawArc(double _r, double _s, double _e, const Vector3d& _v1,
      const Vector3d& _v2) {
    glBegin(GL_LINE_STRIP);
    for(float t = _s; t < _e; t += 0.2)
      glVertex3dv(_r*cos(t)*_v1 + _r*sin(t)*_v2);
    glVertex3dv(_r*cos(_e)*_v1 + _r*sin(_e)*_v2);
    glEnd();
  }

  void
  DrawHollowCylinder(double _ir, double _or, double _h, double _s) {
    //inside verts top
    //for(size_t i = 0; i < _s; ++i)
    //  glVertex3d(_ir * cos(TWOPI/i), _ir * sin(TWOPI/i), _h/2);
    //inside verts bottom
    //for(size_t i = 0; i < _s; ++i)
    //  glVertex3d(_ir * cos(TWOPI/i), _ir * sin(TWOPI/i), -_h/2);
    //outside verts top
    //for(size_t i = 0; i < _s; ++i)
    //  glVertex3d(_or * cos(TWOPI/i), _or * sin(TWOPI/i), _h/2);
    //outside verts bottom
    //for(size_t i = 0; i < _s; ++i)
    //  glVertex3d(_or * cos(TWOPI/i), _or * sin(TWOPI/i), -_h/2);

    //triangle strip inner loop
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3d(-1, 0, 0);
    glVertex3d(_ir, 0, _h/2);
    glNormal3d(-1, 0, 0);
    glVertex3d(_ir, 0, -_h/2);
    for(size_t i = 1; i <= _s; ++i) {
      double frac = TWOPI * (double)i/_s;
      glNormal3d(-cos(frac), -sin(frac), 0);
      glVertex3d(_ir * cos(frac), _ir * sin(frac), _h/2);
      glNormal3d(-cos(frac), -sin(frac), 0);
      glVertex3d(_ir * cos(frac), _ir * sin(frac), -_h/2);
    }
    glEnd();

    //triangle strip inner loop
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3d(1, 0, 0);
    glVertex3d(_or, 0, _h/2);
    glNormal3d(1, 0, 0);
    glVertex3d(_or, 0, -_h/2);
    for(size_t i = 1; i <= _s; ++i) {
      double frac = TWOPI * (double)i/_s;
      glNormal3d(cos(frac), sin(frac), 0);
      glVertex3d(_or * cos(frac), _or * sin(frac), _h/2);
      glNormal3d(cos(frac), sin(frac), 0);
      glVertex3d(_or * cos(frac), _or * sin(frac), -_h/2);
    }
    glEnd();

    //triangle strip top
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3d(0, 0, 1);
    glVertex3d(_ir, 0, _h/2);
    glNormal3d(0, 0, 1);
    glVertex3d(_or, 0, _h/2);
    for(size_t i = 1; i <= _s; ++i) {
      double frac = TWOPI * (double)i/_s;
      glNormal3d(0, 0, 1);
      glVertex3d(_ir * cos(frac), _ir * sin(frac), _h/2);
      glNormal3d(0, 0, 1);
      glVertex3d(_or * cos(frac), _or * sin(frac), _h/2);
    }
    glEnd();

    //triangle strip bottom
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3d(0, 0, -1);
    glVertex3d(_ir, 0, -_h/2);
    glNormal3d(0, 0, -1);
    glVertex3d(_or, 0, -_h/2);
    for(size_t i = 1; i <= _s; ++i) {
      double frac = TWOPI * (double)i/_s;
      glNormal3d(0, 0, -1);
      glVertex3d(_ir * cos(frac), _ir * sin(frac), -_h/2);
      glNormal3d(0, 0, -1);
      glVertex3d(_or * cos(frac), _or * sin(frac), -_h/2);
    }
    glEnd();
  }
}
