#ifndef GLUTILS_H_
#define GLUTILS_H_

#include "Vector.h"
using namespace mathtool;

extern int g_width, g_height; //global m_width/m_height variables

//project a single 3d point to the window
Point3d ProjectToWindow(const Point3d& _pt);
//project a set of 3d points to the window
void ProjectToWindow(Point3d* _pts, size_t _size);
//Project a window coordinate (_x, _y) to the plane defined by _ref, _n
//x,y is window coord
//a reference point, ref, and a nornmal, n, defines a plane
//the unprojected point is intersetion of shooting ray from (x,y)
//and the plane defined by n and ref
Point3d ProjectToWorld(double _x, double _y, const Point3d& _ref, const Vector3d& _n);
//draw with OpenGL a circle of radius r
void DrawCircle(double _r, bool _fill);
//Draw arc from radians s to e with radius r on plane made by v1, v2
void DrawArc(double _r, double _s, double _e, const Vector3d& _v1, const Vector3d& _v2);

#endif
