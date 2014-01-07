#include "GLUtils.h"

#include "glut.h"

int g_width = 0, g_height = 0; //global m_width/m_height variables

//project a single 3d point to the window
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

//project a set of 3d points to the window
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

//x,y is window coord
//a reference point, ref, and a nornmal, n, defines a plane
//the unprojected point is intersetion of shooting ray from (x,y)
//and the plane defined by n and ref
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

//draw with OpenGL a circle of radius r
void
DrawCircle(double _r, bool _fill) {
  glBegin(_fill ? GL_POLYGON : GL_LINE_LOOP);
  for(double t = 0; t < TWOPI; t += 0.2)
    glVertex2f(_r*cos(t), _r*sin(t));
  glEnd();
}

//Draw arc from radians s to e with radius r on plane made by v1, v2
void
DrawArc(double _r, double _s, double _e, const Vector3d& _v1, const Vector3d& _v2) {
  glBegin(GL_LINE_STRIP);
  for(float t = _s; t < _e; t += 0.2)
    glVertex3dv(_r*cos(t)*_v1 + _r*sin(t)*_v2);
  glVertex3dv(_r*cos(_e)*_v1 + _r*sin(_e)*_v2);
  glEnd();
}

