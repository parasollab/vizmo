#include "RegionBoxModel.h"

#include <QtGui>

#include "Models/EnvModel.h"
#include "Models/Vizmo.h"
#include "MPProblem/BoundingBox.h"
#include "Utilities/Camera.h"
#include "Utilities/GLUtils.h"
#include "Utilities/IO.h"

RegionBoxModel::
RegionBoxModel() : RegionModel("Box Region", BOX),
    m_lmb(false), m_rmb(false), m_firstClick(true), m_highlightedPart(NONE),
    m_boxVertices(8), m_prevPos(8), m_winVertices(8),
    m_min(0, 0, 0), m_max(0, 0, 0), m_crosshair() {}

RegionBoxModel::
RegionBoxModel(pair<double, double> _xRange, pair<double, double> _yRange,
    pair<double, double> _zRange) : RegionModel("Box Region", BOX),
    m_lmb(false), m_rmb(false), m_firstClick(false), m_highlightedPart(NONE),
    m_boxVertices(8), m_prevPos(8), m_winVertices(8),
    m_min(_xRange.first, _yRange.first, _zRange.first),
    m_max(_xRange.second, _yRange.second, _zRange.second),
    m_crosshair() {
  m_boxVertices[0] = Point3d(m_min[0], m_max[1], m_max[2]);
  m_boxVertices[1] = Point3d(m_min[0], m_min[1], m_max[2]);
  m_boxVertices[2] = Point3d(m_max[0], m_min[1], m_max[2]);
  m_boxVertices[3] = Point3d(m_max[0], m_max[1], m_max[2]);
  m_boxVertices[4] = Point3d(m_min[0], m_max[1], m_min[2]);
  m_boxVertices[5] = Point3d(m_min[0], m_min[1], m_min[2]);
  m_boxVertices[6] = Point3d(m_max[0], m_min[1], m_min[2]);
  m_boxVertices[7] = Point3d(m_max[0], m_max[1], m_min[2]);

  m_prevPos = m_boxVertices;
}

shared_ptr<Boundary>
RegionBoxModel::
GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingBox(
      make_pair(m_boxVertices[0][0], m_boxVertices[3][0]),
      make_pair(m_boxVertices[1][1], m_boxVertices[0][1]),
      make_pair(m_boxVertices[4][2], m_boxVertices[0][2])));
}

//initialization of gl models
void
RegionBoxModel::
Build() {
}

//determing if _index is this GL model
void
RegionBoxModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

bool
RegionBoxModel::
operator==(const RegionModel& _other) const {
  if(_other.GetShape() == this->GetShape()) {
    const RegionBoxModel* myModel = static_cast<const RegionBoxModel*>(&_other);
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

double
RegionBoxModel::
GetShortLength() const {
  double xlen, ylen, zlen, len;
  xlen = m_boxVertices[3][0] - m_boxVertices[0][0];
  ylen = m_boxVertices[0][1] - m_boxVertices[1][1];
  zlen = m_boxVertices[0][2] - m_boxVertices[4][2];

  len = min(xlen, ylen);
  len = min(len, zlen);

  return len;
}

double
RegionBoxModel::
GetLongLength() const {
  double xlen, ylen, zlen, len;
  xlen = m_boxVertices[3][0] - m_boxVertices[0][0];
  ylen = m_boxVertices[0][1] - m_boxVertices[1][1];
  zlen = m_boxVertices[0][2] - m_boxVertices[4][2];

  len = max(xlen, ylen);
  len = max(len, zlen);

  return len;
}

void
RegionBoxModel::
ApplyOffset(const Vector3d& _v) {
  m_highlightedPart = ALL;
  ApplyTransform(_v);
  m_prevPos = m_boxVertices;
  m_highlightedPart = NONE;
}

//draw is called for the scene.
void
RegionBoxModel::
DrawRender() {
  //configure gl modes
  glColor4fv(m_color);

  //create model
  glBegin(GL_QUADS);

  //front face
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);

  //back face
  for(int i = 7; i > 3; --i)
    glVertex3dv(m_boxVertices[i]);

  //bottom
  glVertex3dv(m_boxVertices[1]);
  glVertex3dv(m_boxVertices[5]);
  glVertex3dv(m_boxVertices[6]);
  glVertex3dv(m_boxVertices[2]);

  //right
  glVertex3dv(m_boxVertices[2]);
  glVertex3dv(m_boxVertices[6]);
  glVertex3dv(m_boxVertices[7]);
  glVertex3dv(m_boxVertices[3]);

  //top
  glVertex3dv(m_boxVertices[3]);
  glVertex3dv(m_boxVertices[7]);
  glVertex3dv(m_boxVertices[4]);
  glVertex3dv(m_boxVertices[0]);

  //left
  glVertex3dv(m_boxVertices[0]);
  glVertex3dv(m_boxVertices[4]);
  glVertex3dv(m_boxVertices[5]);
  glVertex3dv(m_boxVertices[1]);
  glEnd();

  //create outline
  glLineWidth(2);
  glColor3f(.9, .9, .9);
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  for(int i = 7; i > 3; --i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3dv(m_boxVertices[0]);
  glVertex3dv(m_boxVertices[4]);

  glVertex3dv(m_boxVertices[1]);
  glVertex3dv(m_boxVertices[5]);

  glVertex3dv(m_boxVertices[2]);
  glVertex3dv(m_boxVertices[6]);

  glVertex3dv(m_boxVertices[3]);
  glVertex3dv(m_boxVertices[7]);
  glEnd();

  //change mouse cursor if highlighted
  if(m_highlightedPart == ALL)
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
  else if(m_highlightedPart == (LEFT | TOP) ||
      m_highlightedPart == (RIGHT | BOTTOM))
    QApplication::setOverrideCursor(Qt::SizeFDiagCursor);
  else if(m_highlightedPart == (LEFT | BOTTOM) ||
      m_highlightedPart == (RIGHT | TOP))
    QApplication::setOverrideCursor(Qt::SizeBDiagCursor);
  else if(m_highlightedPart & (LEFT | RIGHT))
    QApplication::setOverrideCursor(Qt::SizeHorCursor);
  else if(m_highlightedPart & (TOP | BOTTOM))
    QApplication::setOverrideCursor(Qt::SizeVerCursor);
  else
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  //draw crosshair during manipulation
  if(m_lmb || m_rmb)
    m_crosshair.DrawRender();
}

void
RegionBoxModel::
DrawSelect() {
  //create model
  glBegin(GL_QUADS);

  //front face
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);

  //back face
  for(int i = 7; i > 3; --i)
    glVertex3dv(m_boxVertices[i]);

  //bottom
  glVertex3dv(m_boxVertices[1]);
  glVertex3dv(m_boxVertices[5]);
  glVertex3dv(m_boxVertices[6]);
  glVertex3dv(m_boxVertices[2]);

  //right
  glVertex3dv(m_boxVertices[2]);
  glVertex3dv(m_boxVertices[6]);
  glVertex3dv(m_boxVertices[7]);
  glVertex3dv(m_boxVertices[3]);

  //top
  glVertex3dv(m_boxVertices[3]);
  glVertex3dv(m_boxVertices[7]);
  glVertex3dv(m_boxVertices[4]);
  glVertex3dv(m_boxVertices[0]);

  //left
  glVertex3dv(m_boxVertices[0]);
  glVertex3dv(m_boxVertices[4]);
  glVertex3dv(m_boxVertices[5]);
  glVertex3dv(m_boxVertices[1]);
  glEnd();

  //create outline
  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  for(int i = 7; i > 3; --i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3dv(m_boxVertices[0]);
  glVertex3dv(m_boxVertices[4]);

  glVertex3dv(m_boxVertices[1]);
  glVertex3dv(m_boxVertices[5]);

  glVertex3dv(m_boxVertices[2]);
  glVertex3dv(m_boxVertices[6]);

  glVertex3dv(m_boxVertices[3]);
  glVertex3dv(m_boxVertices[7]);
  glEnd();
}

//DrawSelect is only called if item is selected
void
RegionBoxModel::
DrawSelected() {
  //configure gl modes
  glLineWidth(4);
  glColor3f(.9, .9, 0.);

  //create model
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < 4; ++i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  for(int i = 7; i > 3; --i)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3dv(m_boxVertices[0]);
  glVertex3dv(m_boxVertices[4]);

  glVertex3dv(m_boxVertices[1]);
  glVertex3dv(m_boxVertices[5]);

  glVertex3dv(m_boxVertices[2]);
  glVertex3dv(m_boxVertices[6]);

  glVertex3dv(m_boxVertices[3]);
  glVertex3dv(m_boxVertices[7]);
  glEnd();
}

//output model info
void
RegionBoxModel::
Print(ostream& _os) const {
  _os << Name() << " ";
  for(size_t i = 0; i < m_boxVertices.size(); i++)
    _os << "(" << m_boxVertices[i] << ")";
  _os << endl;
}

// output debug info
void
RegionBoxModel::
OutputDebugInfo(ostream& _os) const {
  _os << m_type << " BOX" << " "
      << m_prevPos[0][0] << " "
      << m_prevPos[1][1] << " "
      << m_prevPos[4][2] << " "
      << m_prevPos[2][0] << " "
      << m_prevPos[0][1] << " "
      << m_prevPos[0][2] << endl;
}

bool
RegionBoxModel::
MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    //if first click, set m_center for crosshair
    if(m_firstClick) {
      double disp = GetVizmo().GetMaxEnvDist() / 3.;
      m_center = ProjectToWorld(m_clicked.x(), m_clicked.y(),
          _c->GetEye() + disp * (_c->GetDir()), -_c->GetDir());
      m_crosshair.SetPos(m_center);
    }
    m_lmb = true;
    return true;
  }
  if(_e->buttons() == Qt::RightButton && !m_firstClick &&
      m_highlightedPart > NONE) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_rmb = true;
    return true;
  }
  return false;
}

bool
RegionBoxModel::
MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;
  if(m_lmb || m_rmb) {
    if(!m_firstClick)
      VDRemoveRegion(this);
    m_lmb = false;
    m_rmb = false;
    m_firstClick = false;
    m_prevPos = m_boxVertices;
    VDAddRegion(this);
    return true;
  }
  return false;
}

bool
RegionBoxModel::
MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;
  //get mouse position
  QPoint mousePos = QPoint(_e->pos().x(), g_height - _e->pos().y());
  if(m_lmb) {
    Point3d c; //click point
    Point3d m; //mouse point

    //handle creation
    if(m_firstClick) {
      //create box: start from top left and draw CCW about vector (0, 0, 1)
      double disp = GetVizmo().GetMaxEnvDist() / 3.;
      c = ProjectToWorld(m_clicked.x(), m_clicked.y(),
          _c->GetEye() + disp * (_c->GetDir()), -_c->GetDir());
      m = ProjectToWorld(mousePos.x(), mousePos.y(),
          _c->GetEye() + disp * (_c->GetDir()), -_c->GetDir());
      m_min[0] = min(c[0], m[0]), m_max[0] = max(c[0], m[0]);
      m_min[1] = min(c[1], m[1]), m_max[1] = max(c[1], m[1]);
      m_min[2] = min(c[2], m[2]), m_max[2] = max(c[2], m[2]);
      m_boxVertices[0] = Point3d(m_min[0], m_max[1], m_max[2]);
      m_boxVertices[1] = Point3d(m_min[0], m_min[1], m_max[2]);
      m_boxVertices[2] = Point3d(m_max[0], m_min[1], m_max[2]);
      m_boxVertices[3] = Point3d(m_max[0], m_max[1], m_max[2]);
      m_boxVertices[4] = Point3d(m_min[0], m_max[1], m_min[2]);
      m_boxVertices[5] = Point3d(m_min[0], m_min[1], m_min[2]);
      m_boxVertices[6] = Point3d(m_max[0], m_min[1], m_min[2]);
      m_boxVertices[7] = Point3d(m_max[0], m_max[1], m_min[2]);
    }
    //handle translating and resizing
    else if(m_highlightedPart > NONE) {
      //get click pos, mouse pos, and their difference
      c = ProjectToWorld(m_clicked.x(), m_clicked.y(), m_center, -_c->GetDir());
      m = ProjectToWorld(mousePos.x(), mousePos.y(), m_center, -_c->GetDir());
      Vector3d deltaMouse = m - c;
      //resizing
      if(m_highlightedPart < ALL) {
        //axisCtrlDir[q] vector tracks which camera direction is
        //controlling world direction q - needed to reassign an unused
        //mouse x or y control to the cameraZ direction
        Vector3d deltaWorld;
        vector<Vector3d> axisCtrlDir(3);

        MapControls(_c, deltaMouse, deltaWorld, axisCtrlDir);
        ApplyTransform(deltaWorld);
      }
      //translation in camera x/y
      else if(m_highlightedPart == ALL)
        ApplyTransform(deltaMouse);
    }
    //reset feedback values
    ClearNodeCount();
    ClearFACount();
    FindCenter();
    return true;
  }
  //translation in ~camera z
  if(m_rmb) {
    //get screen-y mouse and click positions
    Point3d m = ProjectToWorld(0, mousePos.y(), m_center, -_c->GetDir());
    Point3d c = ProjectToWorld(0, m_clicked.y(), m_center, -_c->GetDir());
    //find the difference between m and c in the screen-y direction,
    //then multiply by the unit vector pointing from the camera to the
    //center to get delta
    Vector3d delta = (m_center - _c->GetEye()).normalize() *
      ((m - c) * _c->GetWindowY());
    //translate region according to delta
    ApplyTransform(delta);

    //reset feedback values
    ClearNodeCount();
    ClearFACount();
    FindCenter();
    return true;
  }

  return false;
}

bool
RegionBoxModel::
PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  //clear highlighted part
  m_highlightedPart = NONE;

  //Get mouse position, store as vector
  Vector2d m(_e->pos().x(), g_height - _e->pos().y());

  //Project vertices to viewscreen
  for(size_t i = 0; i < m_winVertices.size(); ++i) {
    Point3d p = ProjectToWindow(m_boxVertices[i]);
    m_winVertices[i][0] = p[0];
    m_winVertices[i][1] = p[1];
  }

  //r is a vector pointing from a box corner c to m
  //e is a vector pointing from a box corner c to its neighbor c+1
  Vector2d e, r;
  //check front edges
  for(int i = 0; i < 4; ++i) {
    if(!m_highlightedPart) {
      e = m_winVertices[(i + 1) % 4] - m_winVertices[i];
      r = m - m_winVertices[i];
      if(r.comp(e) > 0 && r.comp(e) < e.norm() &&
          (r.orth(e)).norm() < 3) {
        switch(i) {
          case 0:
            m_highlightedPart |= LEFT + FRONT;
            break;
          case 1:
            m_highlightedPart |= BOTTOM + FRONT;
            break;
          case 2:
            m_highlightedPart |= RIGHT + FRONT;
            break;
          case 3:
            m_highlightedPart |= TOP + FRONT;
            break;
        }
      }
    }
  }
  //check back edges
  for(int i = 0; i < 4; ++i) {
    if(!m_highlightedPart) {
      e = m_winVertices[(i + 1) % 4 + 4] - m_winVertices[i + 4];
      r = m - m_winVertices[i + 4];
      if(r.comp(e) > 0 && r.comp(e) < e.norm() &&
          (r.orth(e)).norm() < 3) {
        switch(i) {
          case 0:
            m_highlightedPart |= LEFT + BACK;
            break;
          case 1:
            m_highlightedPart |= BOTTOM + BACK;
            break;
          case 2:
            m_highlightedPart |= RIGHT + BACK;
            break;
          case 3:
            m_highlightedPart |= TOP + BACK;
            break;
        }
      }
    }
  }
  //check side edges
  for(int i = 0; i < 4; ++i) {
    if(!m_highlightedPart) {
      e = m_winVertices[i] - m_winVertices[i + 4];
      r = m - m_winVertices[i + 4];
      if(r.comp(e) > 0 && r.comp(e) < e.norm() &&
          (r.orth(e)).norm() < 3) {
        switch(i) {
          case 0:
            m_highlightedPart |= TOP + LEFT;
            break;
          case 1:
            m_highlightedPart |= LEFT + BOTTOM;
            break;
          case 2:
            m_highlightedPart |= BOTTOM + RIGHT;
            break;
          case 3:
            m_highlightedPart |= RIGHT + TOP;
            break;
        }
      }
    }
  }

  //check for select all
  if(!m_highlightedPart) {
    //q is a vector pointing from box center to m
    //d is a vector pointing from box center to a corner
    Vector3d center = ProjectToWindow(m_center);
    Vector2d q(m[0] - center[0], m[1] - center[1]);
    Vector2d d(m_winVertices[0][0] - center[0], m_winVertices[0][1] - center[1]);
    if(q.norm() < d.norm() * .5)
      m_highlightedPart = ALL;
  }

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  return m_highlightedPart;
}

double
RegionBoxModel::
WSpaceArea() const {
  return (m_boxVertices[3][0] - m_boxVertices[0][0]) *
    (m_boxVertices[0][1] - m_boxVertices[1][1]) *
    (m_boxVertices[0][2] - m_boxVertices[4][2]);
}

void
RegionBoxModel::
FindCenter() {
  //find midpoint of the diagonal from top-left-front to bottom-right-back
  m_center = (m_boxVertices[0] + m_boxVertices[6])/2.;
}


void
RegionBoxModel::
MapControls(Camera* _c, const Vector3d& _deltaMouse,
    Vector3d& _deltaWorld, vector<Vector3d>& _axisCtrlDir) {
  double score, xScore, yScore, zScore;
  //define unit vectors of the world coordinate system
  Vector3d xHat(1, 0, 0);
  Vector3d yHat(0, 1, 0);
  Vector3d zHat(0, 0, 1);

  //define unit vectors in the camera's coordinate system
  Vector3d cameraX = -_c->GetWindowX();
  Vector3d cameraY = _c->GetWindowY();
  Vector3d cameraZ = -_c->GetWindowZ();

  //find the world axis Q closest to cameraX and
  //set _delta[Q] = change in cameraX
  xScore = fabs(cameraX * xHat);
  yScore = fabs(cameraX * yHat);
  zScore = fabs(cameraX * zHat);
  score = max(xScore, yScore);
  score = max(score, zScore);
  if(score == xScore) {
    //cameraX is nearest the X direction
    _deltaWorld[0] = _deltaMouse.proj(cameraX) * xHat;
    _axisCtrlDir[0] = cameraX;
  }
  else if(score == yScore) {
    //cameraX is nearest the Y direction
    _deltaWorld[1] = _deltaMouse.proj(cameraX) * yHat;
    _axisCtrlDir[1] = cameraX;
  }
  else if(score == zScore) {
    //cameraX is nearest the Z direction
    _deltaWorld[2] = _deltaMouse.proj(cameraX) * zHat;
    _axisCtrlDir[2] = cameraX;
  }

  //find the world axis Q closest to cameraY and
  //set _delta[Q] = change in cameraY. Store -cameraY
  //as controller for correct remapping to cameraZ
  xScore = fabs(cameraY * xHat);
  yScore = fabs(cameraY * yHat);
  zScore = fabs(cameraY * zHat);
  score = max(xScore, yScore);
  score = max(score, zScore);
  if(score == xScore) {
    //cameraY is nearest the X direction
    _deltaWorld[0] = _deltaMouse.proj(cameraY) * xHat;
    _axisCtrlDir[0] = -cameraY;
  }
  else if(score == yScore) {
    //cameraY _camDir is nearest the Y direction
    _deltaWorld[1] = _deltaMouse.proj(cameraY) * yHat;
    _axisCtrlDir[1] = -cameraY;
  }
  else if(score == zScore) {
    //cameraY _camDir is nearest the Z direction
    _deltaWorld[2] = _deltaMouse.proj(cameraY) * zHat;
    _axisCtrlDir[2] = -cameraY;
  }

  //if mouse x or y is not used for resizing the highlighted part,
  //assign it to control the unused (cameraZ) axis.
  if((m_highlightedPart & (TOP + BOTTOM)) &&
      (m_highlightedPart & (RIGHT + LEFT)) && _deltaWorld[2]) {
    if(!_deltaWorld[0])
      _deltaWorld[0] = _deltaMouse.comp(_axisCtrlDir[2]) * (cameraZ * xHat);
    if(!_deltaWorld[1])
      _deltaWorld[1] = _deltaMouse.comp(_axisCtrlDir[2]) * (cameraZ * yHat);
  }
  if((m_highlightedPart & (TOP + BOTTOM)) &&
      (m_highlightedPart & (FRONT + BACK)) && _deltaWorld[0]) {
    if(!_deltaWorld[1])
      _deltaWorld[1] = _deltaMouse.comp(_axisCtrlDir[0]) * (cameraZ * yHat);
    if(!_deltaWorld[2])
      _deltaWorld[2] = _deltaMouse.comp(_axisCtrlDir[0]) * (cameraZ * zHat);
  }
  if((m_highlightedPart & (RIGHT + LEFT)) &&
      (m_highlightedPart & (FRONT + BACK)) && _deltaWorld[1]) {
    if(!_deltaWorld[2])
      _deltaWorld[2] = _deltaMouse.comp(_axisCtrlDir[1]) * (cameraZ * zHat);
    if(!_deltaWorld[0])
      _deltaWorld[0] = _deltaMouse.comp(_axisCtrlDir[1]) * (cameraZ * xHat);
  }
}

void
RegionBoxModel::
ApplyTransform(const Vector3d& _delta) {
  if(m_highlightedPart & LEFT) {
    m_boxVertices[0][0] = m_prevPos[0][0] + _delta[0];
    m_boxVertices[1][0] = m_prevPos[1][0] + _delta[0];
    m_boxVertices[4][0] = m_prevPos[4][0] + _delta[0];
    m_boxVertices[5][0] = m_prevPos[5][0] + _delta[0];
  }
  if(m_highlightedPart & RIGHT) {
    m_boxVertices[2][0] = m_prevPos[2][0] + _delta[0];
    m_boxVertices[3][0] = m_prevPos[3][0] + _delta[0];
    m_boxVertices[6][0] = m_prevPos[6][0] + _delta[0];
    m_boxVertices[7][0] = m_prevPos[7][0] + _delta[0];
  }
  if(m_highlightedPart & TOP) {
    m_boxVertices[0][1] = m_prevPos[0][1] + _delta[1];
    m_boxVertices[3][1] = m_prevPos[3][1] + _delta[1];
    m_boxVertices[4][1] = m_prevPos[4][1] + _delta[1];
    m_boxVertices[7][1] = m_prevPos[7][1] + _delta[1];
  }
  if(m_highlightedPart & BOTTOM) {
    m_boxVertices[1][1] = m_prevPos[1][1] + _delta[1];
    m_boxVertices[2][1] = m_prevPos[2][1] + _delta[1];
    m_boxVertices[5][1] = m_prevPos[5][1] + _delta[1];
    m_boxVertices[6][1] = m_prevPos[6][1] + _delta[1];
  }
  if(m_highlightedPart & FRONT) {
    m_boxVertices[0][2] = m_prevPos[0][2] + _delta[2];
    m_boxVertices[1][2] = m_prevPos[1][2] + _delta[2];
    m_boxVertices[2][2] = m_prevPos[2][2] + _delta[2];
    m_boxVertices[3][2] = m_prevPos[3][2] + _delta[2];
  }
  if(m_highlightedPart & BACK) {
    m_boxVertices[4][2] = m_prevPos[4][2] + _delta[2];
    m_boxVertices[5][2] = m_prevPos[5][2] + _delta[2];
    m_boxVertices[6][2] = m_prevPos[6][2] + _delta[2];
    m_boxVertices[7][2] = m_prevPos[7][2] + _delta[2];
  }

  //ensure that [0] is still top-left-front corner and that drawing is CCW
  if(m_boxVertices[0][0] > m_boxVertices[3][0]) {
    swap(m_boxVertices[0], m_boxVertices[3]);
    swap(m_boxVertices[1], m_boxVertices[2]);
    swap(m_boxVertices[4], m_boxVertices[7]);
    swap(m_boxVertices[5], m_boxVertices[6]);
    swap(m_prevPos[0], m_prevPos[3]);
    swap(m_prevPos[1], m_prevPos[2]);
    swap(m_prevPos[4], m_prevPos[7]);
    swap(m_prevPos[5], m_prevPos[6]);
    m_highlightedPart = m_highlightedPart ^ LEFT ^ RIGHT;
  }
  if(m_boxVertices[0][1] < m_boxVertices[1][1]) {
    swap(m_boxVertices[0], m_boxVertices[1]);
    swap(m_boxVertices[2], m_boxVertices[3]);
    swap(m_boxVertices[4], m_boxVertices[5]);
    swap(m_boxVertices[6], m_boxVertices[7]);
    swap(m_prevPos[0], m_prevPos[1]);
    swap(m_prevPos[2], m_prevPos[3]);
    swap(m_prevPos[4], m_prevPos[5]);
    swap(m_prevPos[6], m_prevPos[7]);
    m_highlightedPart = m_highlightedPart ^ TOP ^ BOTTOM;
  }
  if(m_boxVertices[0][2] < m_boxVertices[4][2]) {
    swap(m_boxVertices[0], m_boxVertices[4]);
    swap(m_boxVertices[1], m_boxVertices[5]);
    swap(m_boxVertices[2], m_boxVertices[6]);
    swap(m_boxVertices[3], m_boxVertices[7]);
    swap(m_prevPos[0], m_prevPos[4]);
    swap(m_prevPos[1], m_prevPos[5]);
    swap(m_prevPos[2], m_prevPos[6]);
    swap(m_prevPos[3], m_prevPos[7]);
    m_highlightedPart = m_highlightedPart ^ FRONT ^ BACK;
  }
}
