#ifndef COLOR_H_
#define COLOR_H_

#include <Vector.h>

#include <gl.h>

typedef mathtool::Vector<float, 3> Color3;
typedef mathtool::Vector<float, 4> Color4;

inline void SetGLColor(const Color3& _c) {glColor3fv(_c);}
inline void SetGLColor(const Color4& _c) {glColor4fv(_c);}

#endif
