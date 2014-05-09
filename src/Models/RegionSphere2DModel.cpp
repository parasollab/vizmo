#include "RegionSphere2DModel.h"

#include <QtGui>

#include "Utilities/Camera.h"
#include "MPProblem/BoundingSphere.h"
#include "Utilities/GLUtils.h"

RegionSphere2DModel::RegionSphere2DModel(const Point3d& _center, double _radius, bool _firstClick) :
  RegionModel("Sphere Region 2D"),
  m_center(_center), m_centerOrig(_center), m_radius(_radius),
  m_lmb(false), m_firstClick(_firstClick), m_highlightedPart(NONE) {}

shared_ptr<Boundary>
RegionSphere2DModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere(m_center, m_radius));
}

//initialization of gl models
void
RegionSphere2DModel::Build() {
}

//determing if _index is this GL model
void
RegionSphere2DModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

//draw is called for the scene.
void
RegionSphere2DModel::DrawRender() {
  if(m_radius < 0)
    return;

  glColor4fv(m_color);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  DrawCircle(m_radius, true);
  glPopMatrix();

  //change cursor based on highlight
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == PERIMETER)
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
}

void
RegionSphere2DModel::DrawSelect() {
  if(m_radius < 0)
    return;

  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  DrawCircle(m_radius, true);
  glPopMatrix();
}

//DrawSelect is only called if item is selected
void
RegionSphere2DModel::DrawSelected() {
  if(m_radius < 0)
    return;

  glLineWidth(4);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  DrawCircle(m_radius, false);
  glPopMatrix();
}

//output model info
void
RegionSphere2DModel::Print(ostream& _os) const {
  _os << Name() << endl
    << "[ " << m_center << " " << m_radius << " ]" << endl;
}

bool
RegionSphere2DModel::MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

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
RegionSphere2DModel::MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    m_lmb = false;
    m_firstClick = false;
    m_centerOrig = m_center;
    return true;
  }
  return false;
}

bool
RegionSphere2DModel::MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    Point3d prj = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        Point3d(0, 0, 0), Vector3d(0, 0, 1));
    //resizing
    if(m_firstClick || m_highlightedPart == PERIMETER)
      m_radius = (m_center - prj).norm();
    //translation
    else if(m_highlightedPart == ALL) {
      Point3d prjClicked = ProjectToWorld(m_clicked.x(), m_clicked.y(),
          Point3d(0, 0, 0), Vector3d(0, 0, 1));
      Vector3d diff = prj - prjClicked;
      m_center = m_centerOrig + diff;
    }

    ClearNodeCount();
    ClearFACount();

    return true;
  }
  return false;
}

bool
RegionSphere2DModel::PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  m_highlightedPart = NONE;
  Point3d prj = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
      Point3d(0, 0, 0), Vector3d(0, 0, 1));
  double v = (prj - m_center).norm();
  if(v < m_radius - .5)
    m_highlightedPart = ALL;
  else if(v < m_radius + .5)
    m_highlightedPart = PERIMETER;

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  return m_highlightedPart;
}

double
RegionSphere2DModel::WSpaceArea() const {
  return PI * sqr(m_radius);
}