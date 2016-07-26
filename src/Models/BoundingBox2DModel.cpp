#include "BoundingBox2DModel.h"

#include <limits>
#include <sstream>

#include <glut.h>

#include "Environment/BoundingBox2D.h"

BoundingBox2DModel::
BoundingBox2DModel(shared_ptr<BoundingBox2D> _b) :
  BoundaryModel("Bounding Box", _b),
  m_boundingBox(_b) {
    Build();
  }

BoundingBox2DModel::
BoundingBox2DModel(const pair<double, double>& _x,
    const pair<double, double>& _y) :
  BoundaryModel("Bounding Box", NULL) {
    m_boundingBox = shared_ptr<BoundingBox2D>(new BoundingBox2D(_x, _y));
    m_boundary = m_boundingBox;
    Build();
  }

void
BoundingBox2DModel::
Build() {
  pair<double, double> x = m_boundingBox->GetRange(0);
  pair<double, double> y = m_boundingBox->GetRange(1);

  // Compute center
  m_center[0] = (x.first + x.second) / 2.;
  m_center[1] = (y.first + y.second) / 2.;
  m_center[2] = 0;

  GLdouble vertices[] = {
    x.first,  y.first, 0,
    x.first,  y.second, 0,
    x.second, y.first, 0,
    x.second, y.second, 0,
  };

  //line index
  GLubyte lineid[] = { 0, 1, 1, 3, 3, 2, 2, 0 };

  //set properties for this box
  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);

  //setup points
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);

  //Draw lines
  glLineWidth(2);
  glColor3d(0.0, 0.0, 0.0);
  glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, lineid);

  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);
  //Draw lines
  glDrawElements(GL_LINES, 8, GL_UNSIGNED_BYTE, lineid);
  glEndList();

  glDisableClientState(GL_VERTEX_ARRAY);
}


void
BoundingBox2DModel::
DrawHaptics() {
  pair<double, double> x = m_boundingBox->GetRange(0);
  pair<double, double> y = m_boundingBox->GetRange(1);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glScalef( x.second - x.first,
      y.second - y.first,
      1);
  glutSolidCube(1);
  glPopMatrix();
}


void
BoundingBox2DModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << *m_boundingBox << endl;
}

vector<pair<double, double> >
BoundingBox2DModel::
GetRanges() const {
  pair<double, double> x = m_boundingBox->GetRange(0);
  pair<double, double> y = m_boundingBox->GetRange(1);
  vector<pair<double, double>> ranges;
  ranges.push_back(x);
  ranges.push_back(y);
  ranges.push_back(make_pair(
        -numeric_limits<double>::max(), numeric_limits<double>::max()));
  return ranges;
}

double
BoundingBox2DModel::
GetMaxDist() {
  return m_boundingBox->GetMaxDist();
}
