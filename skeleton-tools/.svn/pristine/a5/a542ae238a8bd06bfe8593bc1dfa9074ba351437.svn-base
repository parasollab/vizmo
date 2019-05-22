#include "ThreadSafeSphereModel.h"
#include "Utilities/GLUtils.h"

void
ThreadSafeSphereModel::
MoveTo(const Point3d& _p) {
  m_lock.lock();
  m_center = _p;
  m_lock.unlock();
}


void
ThreadSafeSphereModel::
DrawRender() {
  glPushMatrix();

  m_lock.lock();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  m_lock.unlock();

  glColor4f(0, .5, 0, .3);
  GLUtils::DrawWireSphere(m_radius);

  glPopMatrix();
}
