#include "RegionBox2DModel.h"

#include <QtGui>
#include <QApplication>

#include "Environment/BoundingBox2D.h"

#include "Utilities/GLUtils.h"
#include "Utilities/Camera.h"
#include "Utilities/IO.h"


RegionBox2DModel::
RegionBox2DModel() : RegionModel("Box Region 2D", BOX2D), m_lmb(false),
    m_firstClick(true), m_highlightedPart(NONE), m_boxVertices(4), m_prevPos(4),
    m_winVertices(4), m_min(0, 0), m_max(0, 0) { }


RegionBox2DModel::
RegionBox2DModel(pair<double, double> _xRange, pair<double, double> _yRange) :
    RegionModel("Box Region 2D", BOX2D), m_lmb(false), m_firstClick(false),
    m_highlightedPart(NONE), m_boxVertices(4), m_prevPos(4), m_winVertices(4),
    m_min(_xRange.first, _yRange.first), m_max(_xRange.second, _yRange.second) {
  m_boxVertices[0] = Point3d(m_min[0], m_max[1], 0.);
  m_boxVertices[1] = Point3d(m_min[0], m_min[1], 0.);
  m_boxVertices[2] = Point3d(m_max[0], m_min[1], 0.);
  m_boxVertices[3] = Point3d(m_max[0], m_max[1], 0.);

  m_prevPos = m_boxVertices;
}


shared_ptr<Boundary>
RegionBox2DModel::
GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingBox2D(
        make_pair(m_boxVertices[0][0], m_boxVertices[3][0]),
        make_pair(m_boxVertices[1][1], m_boxVertices[0][1]) ));
}


//initialization of gl models
void
RegionBox2DModel::
Build() {
}


//determing if _index is this GL model
void
RegionBox2DModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}


bool
RegionBox2DModel::
operator==(const RegionModel& _other) const {
  if(_other.GetShape() == this->GetShape()) {
    const RegionBox2DModel* myModel = static_cast<const RegionBox2DModel*>(&_other);
    if(GetType() == myModel->GetType()) {
      bool result = true;
      for(unsigned int i = 0; i < myModel->m_boxVertices.size(); i++) {
        result &= (m_boxVertices[i] == myModel->m_boxVertices[i]);
      }
      return result;
    }
  }
  return false;
}


void
RegionBox2DModel::
ApplyOffset(const Vector3d& _v) {
  m_highlightedPart = ALL;
  ApplyTransform(Vector2d(_v[0], _v[1]));
  m_prevPos = m_boxVertices;
  m_highlightedPart = NONE;
}


double
RegionBox2DModel::
GetShortLength() const {
  double xlen, ylen, len;
  xlen = m_boxVertices[3][0] - m_boxVertices[0][0];
  ylen = m_boxVertices[0][1] - m_boxVertices[1][1];

  len = min(xlen, ylen);

  return len;
}


double
RegionBox2DModel::
GetLongLength() const {
  double xlen, ylen, len;
  xlen = m_boxVertices[3][0] - m_boxVertices[0][0];
  ylen = m_boxVertices[0][1] - m_boxVertices[1][1];

  len = max(xlen, ylen);

  return len;
}


//draw is called for the scene.
void
RegionBox2DModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //configure gl modes
  glColor4fv(m_color);

  //draw box
  glBegin(GL_QUADS);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();

  //draw outline
  glLineWidth(2);
  glColor4f(.2, .2, .2, .5);
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();

  //change mouse cursor if highlighted
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == (LEFT | TOP) || m_highlightedPart == (RIGHT | BOTTOM))
    QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
  else if(m_highlightedPart == (LEFT | BOTTOM) || m_highlightedPart == (RIGHT | TOP))
    QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
  else if(m_highlightedPart & (LEFT | RIGHT))
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
  else if(m_highlightedPart & (TOP | BOTTOM))
    QApplication::setOverrideCursor(Qt::SizeVerCursor);
}


void
RegionBox2DModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //draw box
  glBegin(GL_QUADS);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();

  //draw outline
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
}


//DrawSelect is only called if item is selected
void
RegionBox2DModel::
DrawSelected() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  //configure gl modes
  glLineWidth(4);
  glColor3f(.9, .9, 0.);

  //draw select outline
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
}


//output model info
void
RegionBox2DModel::
Print(ostream& _os) const {
  _os << Name() << " ";
  for(size_t i = 0; i < m_boxVertices.size(); i++)
    _os << "(" << m_boxVertices[i] << ")";
  _os << endl;
}


// output debug info
void
RegionBox2DModel::
OutputDebugInfo(ostream& _os) const {
  _os << m_type << " BOX2D"
      << " " << m_prevPos[0][0]
      << " " << m_prevPos[1][1]
      << " " << m_prevPos[2][0]
      << " " << m_prevPos[3][1] << endl;
}


bool
RegionBox2DModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), GLUtils::windowHeight - _e->pos().y());
    m_lmb = true;
    return true;
  }
  return false;
}


bool
RegionBox2DModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  VDAddRegion(this);
  if(m_type == AVOID)
    return false;
  if(m_lmb) {
    if(!m_firstClick)
      VDRemoveRegion(this);
    m_lmb = false;
    m_firstClick = false;
    m_prevPos = m_boxVertices;
    FindCenter();
    VDAddRegion(this);
    return true;
  }
  return false;
}


bool
RegionBox2DModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    //get mouse position
    QPoint mousePos = QPoint(_e->pos().x(), GLUtils::windowHeight - _e->pos().y());

    Point3d c; //world click point
    Point3d m; //world mouse point

    //handle creation
    if(m_firstClick) {
      //create box: start from top left and draw CCW about vector (0, 0, 1)
      c = GLUtils::ProjectToWorld(m_clicked.x(), m_clicked.y());
      m = GLUtils::ProjectToWorld(mousePos.x(), mousePos.y());
      m_min[0] = min(c[0], m[0]), m_max[0] = max(c[0], m[0]);
      m_min[1] = min(c[1], m[1]), m_max[1] = max(c[1], m[1]);
      m_boxVertices[0] = Point3d(m_min[0], m_max[1], 0.);
      m_boxVertices[1] = Point3d(m_min[0], m_min[1], 0.);
      m_boxVertices[2] = Point3d(m_max[0], m_min[1], 0.);
      m_boxVertices[3] = Point3d(m_max[0], m_max[1], 0.);
    }
    //handle resizing & translating
    else if(m_highlightedPart > NONE) {
      c = GLUtils::ProjectToWorld(m_clicked.x(), m_clicked.y());
      m = GLUtils::ProjectToWorld(mousePos.x(), mousePos.y());
      Vector3d deltaMouse = m - c;
      Vector2d deltaWorld(deltaMouse * Vector3d(1, 0, 0),
          deltaMouse * Vector3d(0, 1, 0));

      ApplyTransform(deltaWorld);
    }
    ClearNodeCount();
    ClearFACount();

    return true;
  }

  return false;
}


bool
RegionBox2DModel::
PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  //clear highlighted part
  m_highlightedPart = NONE;

  //Get mouse position, store as vector
  Vector2d m(_e->pos().x(), GLUtils::windowHeight - _e->pos().y());

  //Project vertices to viewscreen
  for(size_t i = 0; i < m_winVertices.size(); ++i) {
    Point3d p = GLUtils::ProjectToWindow(m_boxVertices[i]);
    m_winVertices[i][0] = p[0];
    m_winVertices[i][1] = p[1];
  }

  //r is a vector pointing from a box corner c to m
  //e is a vector pointing from a box corner c to its neighbor c+1
  Vector2d e, r;
  //check front edges
  for(int i = 0; i < 4; ++i) {
    e = m_winVertices[(i + 1) % 4] - m_winVertices[i];
    r = m - m_winVertices[i];
    if(r.comp(e) > -6 && r.comp(e) < e.norm() &&
        (r.orth(e)).norm() < 3) {
      switch(i) {
        case 0:
          m_highlightedPart |= LEFT;
          break;
        case 1:
          m_highlightedPart |= BOTTOM;
          break;
        case 2:
          m_highlightedPart |= RIGHT;
          break;
        case 3:
          m_highlightedPart |= TOP;
          break;
      }
    }
  }

  //check for select all
  if(!m_highlightedPart) {
    if(m[0] >= m_winVertices[0][0] + 3 && m[0] <= m_winVertices[2][0] - 3 &&
        m[1] >= m_winVertices[1][1] + 3 && m[1] <= m_winVertices[0][1])
      m_highlightedPart = ALL;
  }

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  return m_highlightedPart;
}


double
RegionBox2DModel::
WSpaceArea() const {
  return (m_boxVertices[3][0] - m_boxVertices[0][0]) *
    (m_boxVertices[0][1] - m_boxVertices[1][1]);
}


void
RegionBox2DModel::
FindCenter() {
  m_center = (m_boxVertices[0] + m_boxVertices[2])/2.;
}


void
RegionBox2DModel::
ApplyTransform(const Vector2d& _delta) {
  if(m_highlightedPart & LEFT) {
    m_boxVertices[0][0] = m_prevPos[0][0] + _delta[0];
    m_boxVertices[1][0] = m_prevPos[1][0] + _delta[0];
  }
  if(m_highlightedPart & RIGHT) {
    m_boxVertices[2][0] = m_prevPos[2][0] + _delta[0];
    m_boxVertices[3][0] = m_prevPos[3][0] + _delta[0];
  }
  if(m_highlightedPart & TOP) {
    m_boxVertices[0][1] = m_prevPos[0][1] + _delta[1];
    m_boxVertices[3][1] = m_prevPos[3][1] + _delta[1];
  }
  if(m_highlightedPart & BOTTOM) {
    m_boxVertices[1][1] = m_prevPos[1][1] + _delta[1];
    m_boxVertices[2][1] = m_prevPos[2][1] + _delta[1];
  }
  //ensure that [0] is still top left and that drawing is CCW
  if(m_boxVertices[0][0] > m_boxVertices[3][0]) {
    swap(m_boxVertices[0], m_boxVertices[3]);
    swap(m_boxVertices[1], m_boxVertices[2]);
    swap(m_prevPos[0], m_prevPos[3]);
    swap(m_prevPos[1], m_prevPos[2]);
    m_highlightedPart = m_highlightedPart ^ LEFT ^ RIGHT;
  }
  if(m_boxVertices[0][1] < m_boxVertices[1][1]) {
    swap(m_boxVertices[0], m_boxVertices[1]);
    swap(m_boxVertices[2], m_boxVertices[3]);
    swap(m_prevPos[0], m_prevPos[1]);
    swap(m_prevPos[2], m_prevPos[3]);
    m_highlightedPart = m_highlightedPart ^ TOP ^ BOTTOM;
  }
}
