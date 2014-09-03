#include "RegionSphereModel.h"

#include "glut.h"

#include <QtGui>

#include "Models/Vizmo.h"
#include "MPProblem/BoundingSphere.h"
#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"
#include "Utilities/IO.h"

RegionSphereModel::
RegionSphereModel(const Point3d& _center, double _radius, bool _firstClick) :
    RegionModel("Sphere Region", SPHERE), m_centerOrig(_center),
    m_radius(_radius), m_radiusOrig(_radius), m_lmb(false), m_rmb(false),
    m_firstClick(_firstClick), m_highlightedPart(NONE) {
  m_center = _center;
  m_crosshair.SetPos(m_center);
}

shared_ptr<Boundary>
RegionSphereModel::
GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere(m_center, m_radius));
}

//initialization of gl models
void
RegionSphereModel::
Build() {
}

//determing if _index is this GL model
void
RegionSphereModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

bool
RegionSphereModel::
operator==(const RegionModel& _other) const {
  if(_other.GetShape() == this->GetShape()) {
    const RegionSphereModel* myModel = static_cast<const RegionSphereModel*>(&_other);
    if(GetType() == myModel->GetType()) {
      bool result = true;
      result &= (m_centerOrig == myModel->m_centerOrig);
      result &= (fabs(m_radiusOrig - myModel->m_radiusOrig) <
        numeric_limits<double>::epsilon());
      return result;
    }
  }

  return false;
}

void
RegionSphereModel::
ApplyOffset(const Vector3d& _v) {
  m_center += _v;
  m_centerOrig = m_center;
}

double
RegionSphereModel::
GetShortLength() const {
  return 2. * m_radius;
}

double
RegionSphereModel::
GetLongLength() const {
  return 2. * m_radius;
}

//draw is called for the scene.
void
RegionSphereModel::
DrawRender() {
  if(m_radius < 0)
    return;
  glColor4fv(m_color);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glutSolidSphere(m_radius, 20, 20);
  glPopMatrix();

  //change cursor based on highlight
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == PERIMETER)
    QApplication::setOverrideCursor(Qt::SizeHorCursor);

  //draw crosshair during manipulation
  if(m_lmb || m_rmb)
    m_crosshair.DrawRender();
}

void
RegionSphereModel::
DrawSelect() {
  if(m_radius < 0)
    return;
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glutSolidSphere(m_radius, 20, 20);
  glPopMatrix();
}

//DrawSelect is only called if item is selected
void
RegionSphereModel::
DrawSelected() {
  if(m_radius < 0)
    return;
  glLineWidth(4);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glutWireSphere(m_radius, 20, 20);
  glPopMatrix();
}

//output model info
void
RegionSphereModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << "[ " << m_center << " " << m_radius << " ]" << endl;
}

void
RegionSphereModel::
OutputDebugInfo(ostream& _os) const {
  _os << m_type << " SPHERE " << m_centerOrig << " " << m_radiusOrig << endl;
}

bool
RegionSphereModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;


  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_lmb = true;
    if(m_firstClick) {
      //set center and click spot
      int x = m_clicked.x(), y = m_clicked.y();
      //set forward displacement of region from camera
      double disp = GetVizmo().GetMaxEnvDist() / 3.;
      m_center = ProjectToWorld(x, y, _c->GetEye() + disp * (_c->GetDir()),
          -_c->GetDir());
    }
    return true;
  }
  if(_e->buttons() == Qt::RightButton && (!m_firstClick)) {
    m_rmb = true;
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    return true;
  }
  return false;
}

bool
RegionSphereModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb || m_rmb) {
    if(!m_firstClick)
      VDRemoveRegion(this);
    m_lmb = false;
    m_rmb = false;
    m_firstClick = false;
    m_centerOrig = m_center;
    m_radiusOrig = m_radius;
    VDAddRegion(this);
    return true;
  }
  return false;
}

bool
RegionSphereModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    Point3d m = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
        m_center, -_c->GetDir());
    if(m_firstClick || m_highlightedPart == PERIMETER)
      m_radius = (m_center - m).norm();
    else if(m_highlightedPart == ALL) {
      Point3d c = ProjectToWorld(m_clicked.x(), m_clicked.y(),
          m_center, -_c->GetDir());
      Vector3d delta = m - c;
      m_center = m_centerOrig + delta;
    }
    ClearNodeCount();
    ClearFACount();
    return true;
  }
  else if(m_rmb && m_highlightedPart) {
    Point3d m = ProjectToWorld(0, g_height - _e->pos().y(),
        m_center, -_c->GetDir());
    Point3d c = ProjectToWorld(0, m_clicked.y(),
        m_center, -_c->GetDir());
    Vector3d delta = (m_centerOrig - _c->GetEye()).normalize() *
      ((m - c) * _c->GetWindowY());
    m_center = m_centerOrig + delta;
    return true;
  }
  return false;
}

bool
RegionSphereModel::
PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  m_highlightedPart = NONE;
  Point3d m = ProjectToWorld(_e->pos().x(), g_height - _e->pos().y(),
      m_center, -_c->GetDir());
  double v = (m - m_center).norm();
  if(v < m_radius - .5)
    m_highlightedPart = ALL;
  else if(v < m_radius + .5)
    m_highlightedPart = PERIMETER;

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);
  return m_highlightedPart;
}

double
RegionSphereModel::
WSpaceArea() const {
  return PI * sqr(m_radius);
}

