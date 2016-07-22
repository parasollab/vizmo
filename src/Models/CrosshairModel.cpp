#include "CrosshairModel.h"

#include "GUI/MainWindow.h"

#include "Models/BoundaryModel.h"
#include "Models/EnvModel.h"
#include "Models/Vizmo.h"

void
CrosshairModel::
DrawRender() {
  EnvModel* env = GetVizmo().GetEnv();
  if(env && m_enable && env->InBoundary(*m_pos)) {
    const vector<pair<double, double>>& range = env->GetBoundary()->GetRanges();
    glPushMatrix();
    glTranslatef((*m_pos)[0], (*m_pos)[1], (*m_pos)[2]);
    glColor4f(.9, .9, .9, .2);
    glLineWidth(4);
    glBegin(GL_LINES);
      glVertex3f(range[0].first  - (*m_pos)[0], 0., 0.);
      glVertex3f(range[0].second - (*m_pos)[0], 0., 0.);
      glVertex3f(0., range[1].first  - (*m_pos)[1], 0.);
      glVertex3f(0., range[1].second - (*m_pos)[1], 0.);
      glVertex3f(0., 0., range[2].first  - (*m_pos)[2]);
      glVertex3f(0., 0., range[2].second - (*m_pos)[2]);
    glEnd();
    glPopMatrix();
  }
}
