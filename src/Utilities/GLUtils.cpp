#include "GLUtils.h"

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
    if(fabs(ray * norm) < std::numeric_limits<double>::epsilon() )
      throw DrawException(WHERE,
          "\nCannot unproject: the plane is perpendicular to the ray.\n");
    double rayFactor = (_ref - s) * norm / (ray * norm);
    return rayFactor * ray + s;
  }


  void
  DrawCircle(double _r, bool _fill, unsigned short _segments) {
    GLfloat incr = TWOPI / _segments;
    glBegin(_fill ? GL_POLYGON : GL_LINE_LOOP);
    for(unsigned short t = 0; t < _segments; ++t)
      glVertex2f(_r * cos(t * incr), _r * sin(t * incr));
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


  void
  DrawSphere(const double _radius, const unsigned short _segments) {
    GLfloat oIncr = 2 * PI / _segments; // Angle increment for x,y coords.
    GLfloat zIncr = PI / _segments;     // Angle increment for z coords.
    GLfloat x, y, z, r;

    // Draw +zHat cap.
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, _radius); // The +zHat pole.
    z = _radius * cos(zIncr);
    r = _radius * sin(zIncr);
    for(short i = 0; i <= _segments; ++i) {
      x = r * cos(oIncr * i);
      y = r * sin(oIncr * i);
      glVertex3f(x, y, z);
    }
    glEnd();

    // Draw main surface.
    GLfloat z2, r2;
    for(short j = 1; j < _segments; ++j) {
      glBegin(GL_TRIANGLE_STRIP);
      z  = _radius * cos(zIncr * j);
      r  = _radius * sin(zIncr * j);
      z2 = _radius * cos(zIncr * (j + 1));
      r2 = _radius * sin(zIncr * (j + 1));
      for(short i = 0; i <= _segments; ++i) {
        x = cos(oIncr * i);
        y = sin(oIncr * i);
        glVertex3f(x * r , y * r ,  z);
        glVertex3f(x * r2, y * r2, z2);
      }
      glEnd();
    }

    // Draw -zHat cap.
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, -_radius);
    z = _radius * cos(zIncr * (_segments - 1));
    r = _radius * sin(zIncr * (_segments - 1));
    for(short i = _segments; i >= 0; --i) {
      x = r * cos(oIncr * i);
      y = r * sin(oIncr * i);
      glVertex3f(x, y, z);
    }
    glEnd();
  }


  void
  DrawWireSphere(const double _radius, const unsigned short _segments) {
    GLfloat oIncr = 2 * PI / _segments; // Angle increment for x,y coords.
    GLfloat zIncr = PI / _segments;     // Angle increment for z coords.
    GLfloat x, y, z, r;

    // Draw latitude lines.
    glBegin(GL_LINES);

    // Draw +zHat cap.
    z = _radius * cos(zIncr);
    r = _radius * sin(zIncr);
    for(short i = 0; i < _segments; ++i) {
      glVertex3f(0, 0, _radius);
      x = r * cos(oIncr * i);
      y = r * sin(oIncr * i);
      glVertex3f(x, y, z);
    }

    // Draw main surface.
    GLfloat z2, r2;
    for(short j = 1; j < _segments; ++j) {
      z  = _radius * cos(zIncr * j);
      r  = _radius * sin(zIncr * j);
      z2 = _radius * cos(zIncr * (j + 1));
      r2 = _radius * sin(zIncr * (j + 1));
      for(short i = 0; i <= _segments; ++i) {
        x = cos(oIncr * i);
        y = sin(oIncr * i);
        glVertex3f(x * r , y * r , z);
        glVertex3f(x * r2, y * r2, z2);
      }
    }

    // Draw -zHat cap.
    z = _radius * cos(zIncr * (_segments - 1));
    r = _radius * sin(zIncr * (_segments - 1));
    for(short i = _segments; i > 0; --i) {
      x = r * cos(oIncr * i);
      y = r * sin(oIncr * i);
      glVertex3f(x, y, z);
      glVertex3f(0, 0, -_radius);
    }
    glEnd();

    // Draw longitude lines.
    for(short i = 1; i < _segments; ++i) {
      glPushMatrix();
      z = _radius * cos(zIncr * i);
      r = _radius * sin(zIncr * i);

      glTranslatef(0, 0, z);
      DrawCircle(r, false, _segments);
      glPopMatrix();
    }
  }
}
