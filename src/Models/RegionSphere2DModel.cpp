#include "RegionSphere2DModel.h"

#include <QtGui>

#include "Environment/BoundingSphere2D.h"

#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"
#include "Utilities/IO.h"


RegionSphere2DModel::
RegionSphere2DModel(const Point3d& _center, double _radius, bool _firstClick) :
    RegionModel("Sphere Region 2D", SPHERE2D),
    m_centerOrig(_center), m_radius(_radius), m_radiusOrig(_radius),
    m_lmb(false), m_firstClick(_firstClick), m_highlightedPart(NONE) {
  m_center = _center;
}


shared_ptr<Boundary>
RegionSphere2DModel::
GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingSphere2D(
        Vector2d(m_center[0], m_center[1]), m_radius));
}


bool
RegionSphere2DModel::
operator==(const RegionModel& _other) const {
  if(_other.GetShape() == this->GetShape()) {
    const RegionSphere2DModel* myModel =
        static_cast<const RegionSphere2DModel*>(&_other);
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
RegionSphere2DModel::
ApplyOffset(const Vector3d& _v) {
    m_center += Vector3d(_v[0], _v[1], 0);
    m_centerOrig = m_center;
}


double
RegionSphere2DModel::
GetShortLength() const {
  return 2. * m_radius;
}


double
RegionSphere2DModel::
GetLongLength() const {
  return 2. * m_radius;
}


//initialization of gl models
void
RegionSphere2DModel::
Build() {
}


//determing if _index is this GL model
void
RegionSphere2DModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}


//draw is called for the scene.
void
RegionSphere2DModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE || m_radius < 0)
    return;

  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glColor4fv(m_color);
  GLUtils::DrawCircle(m_radius, true);
  glColor4f(.2, .2, .2, .5);
  GLUtils::DrawCircle(m_radius, false);
  glPopMatrix();

  //change cursor based on highlight
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == PERIMETER)
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
}


void
RegionSphere2DModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE || m_radius < 0)
    return;

  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  GLUtils::DrawCircle(m_radius, true);
  glPopMatrix();
}


//DrawSelect is only called if item is selected
void
RegionSphere2DModel::
DrawSelected() {
  if(m_renderMode == INVISIBLE_MODE || m_radius < 0)
    return;

  glLineWidth(4);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  GLUtils::DrawCircle(m_radius, false);
  glPopMatrix();
}


//output model info
void
RegionSphere2DModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << "[ " << m_center << " " << m_radius << " ]" << endl;
}


void
RegionSphere2DModel::
OutputDebugInfo(ostream& _os) const {
  _os << m_type << " SPHERE2D " << m_centerOrig << " " << m_radiusOrig << endl;
}


bool
RegionSphere2DModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;
  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), GLUtils::windowHeight - _e->pos().y());
    m_lmb = true;
    if(m_firstClick) {
      //set center and click spot
      int x = m_clicked.x(), y = m_clicked.y();
      m_center = GLUtils::ProjectToWorld(x, y);
    }
    return true;
  }
  return false;
}


bool
RegionSphere2DModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    if(!m_firstClick)
      VDRemoveRegion(this);
    m_lmb = false;
    m_firstClick = false;
    m_centerOrig = m_center;
    m_radiusOrig = m_radius;
    VDAddRegion(this);
    return true;
  }
  return false;
}


bool
RegionSphere2DModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    Point3d prj = GLUtils::ProjectToWorld(_e->pos().x(),
        GLUtils::windowHeight - _e->pos().y());
    //resizing
    if(m_firstClick || m_highlightedPart == PERIMETER)
      m_radius = (m_center - prj).norm();
    //translation
    else if(m_highlightedPart == ALL) {
      Point3d prjClicked = GLUtils::ProjectToWorld(m_clicked.x(), m_clicked.y(),
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
RegionSphere2DModel::
PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;
  m_highlightedPart = NONE;
  Point3d prj = GLUtils::ProjectToWorld(_e->pos().x(),
      GLUtils::windowHeight - _e->pos().y());
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
RegionSphere2DModel::
WSpaceArea() const {
  return PI * sqr(m_radius);
}
