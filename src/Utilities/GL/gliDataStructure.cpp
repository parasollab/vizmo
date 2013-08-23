#include "gliDataStructure.h"

#include <qgl.h>

void GLTransform::glTransform() {
  //translation applied last
  glTranslated(m_pos[0], m_pos[1], m_pos[2]);

  //rotation applied second.
  //Need to convert Quaternion to Angle-axis for OpenGL
  const Vector3d& v = m_q.imaginary();
  double t = radToDeg(atan2(v.norm(), m_q.real())*2);
  glRotated(t, v[0], v[1], v[2]);

  //scaling applied first
  glScaled(m_scale[0], m_scale[1], m_scale[2]);
}

