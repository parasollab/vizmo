#include "CrosshairModel.h"

#include "Models/BoundaryModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

CrosshairModel::
CrosshairModel(Point3d* _p) : Model("Crosshair"), m_worldPos(_p),
    m_enabled(false) {
  m_worldRange = GetVizmo().GetEnv()->GetBoundary()->GetRanges();
}


void
CrosshairModel::
DrawRender() {
  if(m_enabled && IsInsideBBX()) {
    glPushMatrix();
    glTranslatef((*m_worldPos)[0], (*m_worldPos)[1], (*m_worldPos)[2]);
    glColor4f(.9, .9, .9, .2);
    glLineWidth(4);
    glBegin(GL_LINES);
      glVertex3f(m_worldRange[0].first  - (*m_worldPos)[0], 0., 0.);
      glVertex3f(m_worldRange[0].second - (*m_worldPos)[0], 0., 0.);
      glVertex3f(0., m_worldRange[1].first  - (*m_worldPos)[1], 0.);
      glVertex3f(0., m_worldRange[1].second - (*m_worldPos)[1], 0.);
      glVertex3f(0., 0., m_worldRange[2].first  - (*m_worldPos)[2]);
      glVertex3f(0., 0., m_worldRange[2].second - (*m_worldPos)[2]);
    glEnd();
    glPopMatrix();
  }
}


bool
CrosshairModel::
IsInsideBBX() const {
  for(int i = 0; i < 3; ++i) {
    if((*m_worldPos)[i] < m_worldRange[i].first ||
        (*m_worldPos)[i] > m_worldRange[i].second)
      return false;
  }
  return true;
}
