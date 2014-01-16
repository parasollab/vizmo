#include "RegionSphereModel.h"

#include "glut.h"

#include <QtGui>

#include "MPProblem/BoundingSphere.h"
#include "Utilities/GLUtils.h"

RegionSphereModel::RegionSphereModel() : RegionModel("Sphere Region"),
  m_radius(-1), m_lmb(false), m_firstClick(true), m_highlightedPart(NONE) {}

shared_ptr<Boundary>
RegionSphereModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere(m_center, m_radius));
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

  glColor4f(0.85, 0.85, 0.85, 0.5);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glTranslatef(0, 0, -0.01);
  DrawCircle(m_radius, true);
  glPopMatrix();

  //change cursor based on highlight
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == PERIMETER)
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
}

//DrawSelect is only called if item is selected
void
RegionSphereModel::DrawSelect() {
  if(m_radius < 0)
    return;

  glDisable(GL_LIGHTING);
  glLineWidth(4);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  DrawCircle(m_radius, false);
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

//output model info
void
RegionSphereModel::Print(ostream& _os) const {
  _os << Name() << endl
    << "[ " << m_center << " " << m_radius << " ]" << endl;
}

bool
RegionSphereModel::MousePressed(QMouseEvent* _e) {
  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_lmb = true;
    if(m_firstClick) {
      //set center and click spot
      int x = m_clicked.x(), y = m_clicked.y();
      m_center = ProjectToWorld(x, y, Point3d(0, 0, 0), Vector3d(0, 0, 1));
    }
    return true;
  }
  return false;
}

bool
RegionSphereModel::MouseReleased(QMouseEvent* _e) {
  if(m_lmb) {
    m_lmb = false;
    m_firstClick = false;
    m_centerOrig = m_center;
    return true;
  }
  return false;
}

bool
RegionSphereModel::MouseMotion(QMouseEvent* _e) {
  if(m_lmb) {
    Point3d prj = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(), Point3d(0, 0, 0), Vector3d(0, 0, 1));
    if(m_firstClick || m_highlightedPart == PERIMETER)
      m_radius = (m_center - prj).norm();
    else if(m_highlightedPart == ALL) {
      Point3d prjClicked = ProjectToWorld(m_clicked.x(), m_clicked.y(), Point3d(0, 0, 0), Vector3d(0, 0, 1));
      Vector3d diff = prj - prjClicked;
      m_center = m_centerOrig + diff;
    }
    return true;
  }
  return false;
}

bool
RegionSphereModel::PassiveMouseMotion(QMouseEvent* _e) {
  m_highlightedPart = NONE;
  Point3d prj = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(), Point3d(0, 0, 0), Vector3d(0, 0, 1));
  double v = sqrt(sqr(prj[0] - m_center[0]) + sqr(prj[1] - m_center[1]) + sqr(prj[2] - m_center[2]));
  if(v < m_radius - 0.5)
    m_highlightedPart = ALL;
  else if(v < m_radius + 0.5)
    m_highlightedPart = PERIMETER;

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  return m_highlightedPart;
}

