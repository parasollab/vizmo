#include "RegionBoxModel.h"

#include "glut.h"

#include <QtGui>

#include "MPProblem/BoundingBox.h"
#include "Utilities/GLUtils.h"
#include "Utilities/Camera.h"

RegionBoxModel::RegionBoxModel() : RegionModel("Box Region"), m_lmb(false), m_rmb(false),
  m_firstClick(true), m_highlightedPart(NONE), m_boxVertices(8), m_prevPos(8), m_winVertices(8) {}

shared_ptr<Boundary>
RegionBoxModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingBox(
        make_pair(m_boxVertices[0][0], m_boxVertices[3][0]),
        make_pair(m_boxVertices[1][1], m_boxVertices[0][1]),
        make_pair(m_boxVertices[4][2], m_boxVertices[0][2])));
}

//initialization of gl models
void
RegionBoxModel::BuildModels() {
}

//determing if _index is this GL model
void
RegionBoxModel::Select(GLuint* _index, vector<Model*>& _sel) {
  if(_index)
    _sel.push_back(this);
}

//draw is called for the scene.
void
RegionBoxModel::Draw() {
  //configure gl modes
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
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

  //reset gl modes to previous configuration
  glPopMatrix();

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
  else
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

//DrawSelect is only called if item is selected
void
RegionBoxModel::DrawSelect() {
  //configure gl modes
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glLineWidth(4);
  glPushMatrix();
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

  //reset gl modes to previous configuration
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

//output model info
void
RegionBoxModel::Print(ostream& _os) const {
  _os << Name() << " ";
  for(size_t i = 0; i < m_boxVertices.size(); i++)
    _os << "(" << m_boxVertices[i] << ")";
  _os << endl;
}

bool
RegionBoxModel::MousePressed(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_lmb = true;
    GetCameraVectors(_c);
    return true;
  }
  if(_e->buttons() == Qt::RightButton && !m_firstClick && m_highlightedPart > NONE) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_rmb = true;
    GetCameraVectors(_c);
    return true;
  }
  return false;
}

bool
RegionBoxModel::MouseReleased(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb || m_rmb) {
    m_lmb = false;
    m_rmb = false;
    m_firstClick = false;
    m_prevPos = m_boxVertices;
    FindCenter();
    return true;
  }
  return false;
}

bool
RegionBoxModel::MouseMotion(QMouseEvent* _e, Camera* _c) {
  if(m_type == AVOID)
    return false;

  if(m_lmb) {
    //get mouse position
    QPoint mousePos = QPoint(_e->pos().x(), g_height - _e->pos().y());
    Point3d c;
    Point3d m;

    //handle creation
    if(m_firstClick) {
      //create box: start from top left and draw CCW about vector (0, 0, 1)
      c = ProjectToWorld(m_clicked.x(), m_clicked.y(), Point3d(), -_c->GetDir());
      m = ProjectToWorld(mousePos.x(), mousePos.y(), Point3d(), -_c->GetDir());
      double minx = min(c[0], m[0]), maxx = max(c[0], m[0]);
      double miny = min(c[1], m[1]), maxy = max(c[1], m[1]);
      double minz = min(c[2], m[2]), maxz = max(c[2], m[2]);
      m_boxVertices[0] = Point3d(minx, maxy, maxz);
      m_boxVertices[1] = Point3d(minx, miny, maxz);
      m_boxVertices[2] = Point3d(maxx, miny, maxz);
      m_boxVertices[3] = Point3d(maxx, maxy, maxz);
      m_boxVertices[4] = Point3d(minx, maxy, minz);
      m_boxVertices[5] = Point3d(minx, miny, minz);
      m_boxVertices[6] = Point3d(maxx, miny, minz);
      m_boxVertices[7] = Point3d(maxx, maxy, minz);
    }
    //handle resizing
    else if(m_highlightedPart > NONE && m_highlightedPart < ALL) {
      c = ProjectToWorld(m_clicked.x(), m_clicked.y(), m_center, -_c->GetDir());
      m = ProjectToWorld(mousePos.x(), mousePos.y(), m_center, -_c->GetDir());
      Vector3d xHat(1, 0, 0);
      Vector3d yHat(0, 1, 0);
      Vector3d zHat(0, 0, 1);
      double xScore, yScore, zScore, score;
      double deltaX = 0;
      double deltaY = 0;
      double deltaZ = 0;

      //calculate x motion
      xScore = fabs(m_cameraX * xHat);
      yScore = fabs(m_cameraX * yHat);
      zScore = fabs(m_cameraX * zHat);
      score = max(xScore, yScore);
      score = max(score, fabs(zScore));

      if(score == xScore) {
        //cameraX is nearest the X direction
        deltaX = ((m - c) * m_cameraX) * (m_cameraX * xHat);
      }
      else if(score == yScore) {
        //cameraX is nearest the Y direction
        deltaY = ((m - c) * m_cameraX) * (m_cameraX * yHat);
      }
      else if(score == zScore) {
        //cameraX is nearest the Z direction
        deltaZ = ((m - c) * m_cameraX) * (m_cameraX * zHat);
      }

      //calculate y motion
      xScore = fabs(m_cameraY * xHat);
      yScore = fabs(m_cameraY * yHat);
      zScore = fabs(m_cameraY * zHat);
      score = max(xScore, yScore);
      score = max(score, fabs(zScore));

      if(score == xScore) {
        //cameraY is nearest the X direction
        deltaX = ((m - c) * m_cameraY) * (m_cameraY * xHat);
      }
      else if(score == yScore) {
        //cameraY is nearest the Y direction
        deltaY = ((m - c) * m_cameraY) * (m_cameraY * yHat);
      }
      else if(score == zScore) {
        //cameraY is nearest the Z direction
        deltaZ = ((m - c) * m_cameraY) * (m_cameraY * zHat);
      }

      //if a mouse axis is unused, assign it to the appropriate box axis
      if((m_highlightedPart & (TOP + BOTTOM)) &&
          (m_highlightedPart & (RIGHT + LEFT)) && deltaZ) {
        if(!deltaX) deltaX = deltaZ;
        if(!deltaY) deltaY = deltaZ;
      }
      if((m_highlightedPart & (TOP + BOTTOM)) &&
          (m_highlightedPart & (FRONT + BACK)) && deltaX) {
        if(!deltaZ) deltaZ = -deltaX;
        if(!deltaY) deltaY = deltaX;
      }
      if((m_highlightedPart & (RIGHT + LEFT)) &&
          (m_highlightedPart & (FRONT + BACK)) && deltaY) {
        if(!deltaZ) {
          deltaZ = -deltaY;
          if(m_cameraZ * zHat < 0) deltaZ = deltaY;
        }
        if(!deltaX) {
          deltaX = deltaY;
          if(m_cameraZ * xHat > 0) deltaX = -deltaY;
        }
      }

      //apply delta to vertices
      if(m_highlightedPart & LEFT) {
        m_boxVertices[0][0] = m_prevPos[0][0] + deltaX;
        m_boxVertices[1][0] = m_prevPos[1][0] + deltaX;
        m_boxVertices[4][0] = m_prevPos[4][0] + deltaX;
        m_boxVertices[5][0] = m_prevPos[5][0] + deltaX;
      }
      if(m_highlightedPart & RIGHT) {
        m_boxVertices[2][0] = m_prevPos[2][0] + deltaX;
        m_boxVertices[3][0] = m_prevPos[3][0] + deltaX;
        m_boxVertices[6][0] = m_prevPos[6][0] + deltaX;
        m_boxVertices[7][0] = m_prevPos[7][0] + deltaX;
      }
      if(m_highlightedPart & TOP) {
        m_boxVertices[0][1] = m_prevPos[0][1] + deltaY;
        m_boxVertices[3][1] = m_prevPos[3][1] + deltaY;
        m_boxVertices[4][1] = m_prevPos[4][1] + deltaY;
        m_boxVertices[7][1] = m_prevPos[7][1] + deltaY;
      }
      if(m_highlightedPart & BOTTOM) {
        m_boxVertices[1][1] = m_prevPos[1][1] + deltaY;
        m_boxVertices[2][1] = m_prevPos[2][1] + deltaY;
        m_boxVertices[5][1] = m_prevPos[5][1] + deltaY;
        m_boxVertices[6][1] = m_prevPos[6][1] + deltaY;
      }
      if(m_highlightedPart & FRONT) {
        m_boxVertices[0][2] = m_prevPos[0][2] + deltaZ;
        m_boxVertices[1][2] = m_prevPos[1][2] + deltaZ;
        m_boxVertices[2][2] = m_prevPos[2][2] + deltaZ;
        m_boxVertices[3][2] = m_prevPos[3][2] + deltaZ;
      }
      if(m_highlightedPart & BACK) {
        m_boxVertices[4][2] = m_prevPos[4][2] + deltaZ;
        m_boxVertices[5][2] = m_prevPos[5][2] + deltaZ;
        m_boxVertices[6][2] = m_prevPos[6][2] + deltaZ;
        m_boxVertices[7][2] = m_prevPos[7][2] + deltaZ;
      }
      //ensure that [0] is still top left and that drawing is CCW
      if(m_boxVertices[0][0] > m_boxVertices[3][0]) {
        swap(m_boxVertices[0], m_boxVertices[3]);
        swap(m_boxVertices[1], m_boxVertices[2]);
        swap(m_boxVertices[4], m_boxVertices[7]);
        swap(m_boxVertices[5], m_boxVertices[6]);
        m_highlightedPart = m_highlightedPart ^ LEFT ^ RIGHT;
      }
      if(m_boxVertices[0][1] < m_boxVertices[1][1]) {
        swap(m_boxVertices[0], m_boxVertices[1]);
        swap(m_boxVertices[2], m_boxVertices[3]);
        swap(m_boxVertices[4], m_boxVertices[5]);
        swap(m_boxVertices[6], m_boxVertices[7]);
        m_highlightedPart = m_highlightedPart ^ TOP ^ BOTTOM;
      }
      if(m_boxVertices[0][2] < m_boxVertices[4][2]) {
        swap(m_boxVertices[0], m_boxVertices[4]);
        swap(m_boxVertices[1], m_boxVertices[5]);
        swap(m_boxVertices[2], m_boxVertices[6]);
        swap(m_boxVertices[3], m_boxVertices[7]);
        m_highlightedPart = m_highlightedPart ^ FRONT ^ BACK;
      }
    }

    //translation in camera x/y
    else if(m_highlightedPart == ALL) {
      Vector3d worldPrj = ProjectToWorld(mousePos.x(), mousePos.y(),
        m_center, -_c->GetDir());
      Point3d oldPos = ProjectToWorld(m_clicked.x(), m_clicked.y(),
          m_center, -_c->GetDir());
      Vector3d delta = worldPrj - oldPos;
      for(size_t i = 0; i < m_boxVertices.size(); ++i)
        m_boxVertices[i] = m_prevPos[i] + delta;
    }

    ClearNodeCount();
    ClearFACount();

    return true;
  }

  //translation in camera z
  if(m_rmb) {
    //get mouse position
    QPoint mousePos = QPoint(_e->pos().x(), g_height - _e->pos().y());
    Vector3d delta = (m_center - _c->GetEye()).normalize() *
      (mousePos.y() - m_clicked.y());
    for(size_t i = 0; i < m_boxVertices.size(); ++i)
      m_boxVertices[i] = m_prevPos[i] + delta;
    return true;
  }

  return false;
}

bool
RegionBoxModel::PassiveMouseMotion(QMouseEvent* _e, Camera* _c) {
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
  if(!m_highlightedPart) {
    for(int i = 0; i < 4; ++i) {
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
  if(!m_highlightedPart) {
    for(int i = 0; i < 4; ++i) {
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
  if(!m_highlightedPart) {
    for(int i = 0; i < 4; ++i) {
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
RegionBoxModel::WSpaceArea() const {
  return (m_boxVertices[3][0] - m_boxVertices[0][0]) *
    (m_boxVertices[0][1] - m_boxVertices[1][1]) *
    (m_boxVertices[0][2] - m_boxVertices[4][2]);
}

void
RegionBoxModel::FindCenter() {
  m_center = (m_boxVertices[0] + m_boxVertices[6])/2.;
}

void
RegionBoxModel::GetCameraVectors(Camera* _c) {
  Vector3d s = ProjectToWorld(0, 0, Point3d(0, 0, 0), -_c->GetDir());
  Vector3d e = ProjectToWorld(1, 0, Point3d(0, 0, 0), -_c->GetDir());
  m_cameraX = (e - s).normalize();

  s = ProjectToWorld(0, 0, Point3d(0, 0, 0), -_c->GetDir());
  e = ProjectToWorld(0, 1, Point3d(0, 0, 0), -_c->GetDir());
  m_cameraY = (e - s).normalize();

  m_cameraZ = (-_c->GetDir()).normalize();
}
