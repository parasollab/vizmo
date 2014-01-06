#include "RegionSphereModel.h"

#include "glut.h"

#include "MPProblem/BoundingSphere.h"
#include "Utilities/GLUtils.h"

RegionSphereModel::RegionSphereModel() : RegionModel("Sphere Region"), m_radius(-1), m_lmb(false), m_firstClick(true) {}

shared_ptr<Boundary>
RegionSphereModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere());
}

//initialization of gl models
void
RegionSphereModel::BuildModels() {
}

//determing if _index is this GL model
void
RegionSphereModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

//draw is called for the scene.
void
RegionSphereModel::Draw() {
  if(m_radius < 0)
    return;

  GLUquadricObj* quad =gluNewQuadric();
  glPolygonMode(GL_FRONT, GL_FILL);
  glColor4f(0.85, 0.85, 0.85, 0.5);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  gluSphere(quad, m_radius, 20, 20);
  glPopMatrix();
  gluDeleteQuadric(quad);
}

//DrawSelect is only called if item is selected
void
RegionSphereModel::DrawSelect() {
}

//output model info
void
RegionSphereModel::Print(ostream& _os) const {
}

bool
RegionSphereModel::MousePressed(QMouseEvent* _e) {
  if(_e->buttons() == Qt::LeftButton) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_lmb = true;
    if(m_firstClick) {
      //set center and click spot
      int x = m_clicked.x(), y = m_clicked.y();
      m_center = ProjectToWorld(x, y, Point3d(0, 0, 0), Vector3d(0, 0, 1));

      m_firstClick = false;
    }
    return true;
  }
  return false;
}

bool
RegionSphereModel::MouseReleased(QMouseEvent* _e) {
  if(m_lmb) {
    m_lmb = false;
    return true;
  }
  return false;
}

bool
RegionSphereModel::MouseMotion(QMouseEvent* _e) {
  if(m_lmb) {
    Point3d prj = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(), Point3d(0, 0, 0), Vector3d(0, 0, 1));
    m_radius = (m_center - prj).norm();
    return true;
  }
  return false;
}

bool
RegionSphereModel::PassiveMouseMotion(QMouseEvent* _e) {
  return false;
}

