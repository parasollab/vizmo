#include "GLTransform.h"

#include <gl.h>

void GLTransform::Transform() {
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

