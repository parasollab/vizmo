#include "RegionBoxModel.h"

#include "glut.h"

#include <QtGui>

#include "MPProblem/BoundingBox.h"
#include "Utilities/GLUtils.h"

RegionBoxModel::RegionBoxModel() : RegionModel("Box Region"), m_lmb(false),
  m_firstClick(true), m_highlightedPart(NONE), m_boxVertices(4), m_prevPos(4) {}

shared_ptr<Boundary>
RegionBoxModel::GetBoundary() const {
  return shared_ptr<Boundary>(new BoundingBox(
        make_pair(m_boxVertices[0][0], m_boxVertices[3][0]),
        make_pair(m_boxVertices[1][1], m_boxVertices[0][1])));
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
  glTranslatef(0, 0, +0.01);
  //create model
  glBegin(GL_QUADS);
  for(int i = 0; i < 4; i++)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glEndList();

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
  for(int i = 0; i < 4; i++)
    glVertex3dv(m_boxVertices[i]);
  glEnd();
  glEndList();

  //reset gl modes to previous configuration
  glPopMatrix();
  glEnable(GL_LIGHTING);
}

//output model info
void
RegionBoxModel::Print(ostream& _os) const {
  _os << Name() << " ";
  for(int i = 0; i < 4; i++)
    _os << "(" << m_boxVertices[i] << ")";
  _os << endl;
}

bool
RegionBoxModel::MousePressed(QMouseEvent* _e) {
  if(_e->buttons() == Qt::LeftButton && (m_firstClick || m_highlightedPart)) {
    m_clicked = QPoint(_e->pos().x(), g_height - _e->pos().y());
    m_lmb = true;
    return true;
  }
  return false;
}

bool
RegionBoxModel::MouseReleased(QMouseEvent* _e) {
  if(m_lmb) {
    m_lmb = false;
    m_firstClick = false;
    for(int i = 0; i < 4; i++)
      m_prevPos[i] = m_boxVertices[i];

    //rebuild vertex list after moving/resizing
    if(m_highlightedPart > NONE) {
      //???
    }
    return true;
  }
  return false;
}

bool
RegionBoxModel::MouseMotion(QMouseEvent* _e) {
  if(m_lmb) {
    //get mouse position
    QPoint mousePos = QPoint(_e->pos().x(), g_height - _e->pos().y());
    Vector3d worldPrj = ProjectToWorld(mousePos.x(), mousePos.y(),
        Point3d(0, 0, 0), Vector3d(0, 0, 1));

    //handle creation
    if(m_firstClick) {
      //create box: start from top left and draw CCW about vector (0, 0, 1)
      int minX = min(m_clicked.x(), mousePos.x());
      int maxX = max(m_clicked.x(), mousePos.x());
      int minY = min(m_clicked.y(), mousePos.y());
      int maxY = max(m_clicked.y(), mousePos.y());
      m_boxVertices[0] = ProjectToWorld(minX, maxY, Point3d(0, 0, 0), Vector3d(0, 0, 1));
      m_boxVertices[1] = ProjectToWorld(minX, minY, Point3d(0, 0, 0), Vector3d(0, 0, 1));
      m_boxVertices[2] = ProjectToWorld(maxX, minY, Point3d(0, 0, 0), Vector3d(0, 0, 1));
      m_boxVertices[3] = ProjectToWorld(maxX, maxY, Point3d(0, 0, 0), Vector3d(0, 0, 1));
    }

    //handle resizing
    if(m_highlightedPart > NONE && m_highlightedPart < ALL) {
      if(m_highlightedPart & LEFT) {
        //set [0] [1] x = new x
        m_boxVertices[0][0] = worldPrj[0];
        m_boxVertices[1][0] = worldPrj[0];
      }
      if(m_highlightedPart & RIGHT) {
        //set [2] [3] x = new x
        m_boxVertices[2][0] = worldPrj[0];
        m_boxVertices[3][0] = worldPrj[0];
      }
      if(m_highlightedPart & TOP) {
        //set [0] [3] y = new y
        m_boxVertices[0][1] = worldPrj[1];
        m_boxVertices[3][1] = worldPrj[1];
      }
      if(m_highlightedPart & BOTTOM) {
        //set [1] [2] y = new y
        m_boxVertices[1][1] = worldPrj[1];
        m_boxVertices[2][1] = worldPrj[1];
      }
      //ensure that [0] is still top left and that drawing is CCW
      if(m_boxVertices[0][0] > m_boxVertices[3][0]) {
        swap(m_boxVertices[0], m_boxVertices[3]);
        swap(m_boxVertices[1], m_boxVertices[2]);
        m_highlightedPart = m_highlightedPart ^ LEFT ^ RIGHT;
      }
      if(m_boxVertices[0][1] < m_boxVertices[1][1]) {
        swap(m_boxVertices[0], m_boxVertices[1]);
        swap(m_boxVertices[2], m_boxVertices[3]);
        m_highlightedPart = m_highlightedPart ^ TOP ^ BOTTOM;
      }
    }

    //hanlde translating
    else if(m_highlightedPart == ALL) {
      Point3d oldPos = ProjectToWorld(m_clicked.x(), m_clicked.y(),
        Point3d(0, 0, 0), Vector3d(0, 0, 1));
      Vector3d delta = worldPrj - oldPos;
      for(int i = 0; i < 4; i++)
        m_boxVertices[i] = m_prevPos[i] + delta;
    }
    return true;
  }
  return false;
}

bool
RegionBoxModel::PassiveMouseMotion(QMouseEvent* _e) {
  //clear highlighted part
  m_highlightedPart = NONE;

  //Get mouse position and project to world
  QPoint mousePos = QPoint(_e->pos().x(), g_height - _e->pos().y());
  Vector3d worldPrj = ProjectToWorld(mousePos.x(), mousePos.y(),
      Point3d(0, 0, 0), Vector3d(0, 0, 1));

  double minY = m_boxVertices[2][1];
  double maxY = m_boxVertices[0][1];
  double minX = m_boxVertices[0][0];
  double maxX = m_boxVertices[2][0];

  if( worldPrj[0] > minX + 1 && worldPrj[0] < maxX - 1 &&
      worldPrj[1] > minY + 1 && worldPrj[1] < maxY - 1)
    m_highlightedPart = ALL;
  else {
    if(worldPrj[1] >= minY - .7 && worldPrj[1] <= maxY + .7) {
      if(abs(worldPrj[0] - minX) < .5)
        m_highlightedPart |= LEFT;
      else if(abs(worldPrj[0] - maxX) < .5)
        m_highlightedPart |= RIGHT;
    }
    if(worldPrj[0] >= minX - .7 && worldPrj[0] <= maxX + .7) {
      if(abs(worldPrj[1] - minY) < .5)
        m_highlightedPart |= BOTTOM;
      else if(abs(worldPrj[1] - maxY) < .5)
        m_highlightedPart |= TOP;
    }
  }

  if(!m_highlightedPart)
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  return m_highlightedPart;
}

double
RegionBoxModel::Density() const {
  double area = (m_boxVertices[3][0] - m_boxVertices[0][0]) *
    (m_boxVertices[0][1] - m_boxVertices[1][1]);
  return (m_numVertices + m_failedAttempts) / area;
}
