#include "BoundingBox2DModel.h"

#include <limits>
#include <sstream>

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

//#include "Environment/WorkspaceBoundingBox.h"
#include "Geometry/Boundaries/WorkspaceBoundingBox.h"

BoundingBox2DModel::
BoundingBox2DModel(shared_ptr<WorkspaceBoundingBox> _b) :
  BoundaryModel("Bounding Box", _b),
  m_boundingBox(_b) {
    Build();
  }

BoundingBox2DModel::
BoundingBox2DModel(const pair<double, double>& _x,
    const pair<double, double>& _y) :
  BoundaryModel("Bounding Box", NULL) {
    std::vector<double> center = {0.,0.};
    std::vector<std::pair<double,double>> bbx = {_x,_y};
    m_boundingBox = shared_ptr<WorkspaceBoundingBox>(new WorkspaceBoundingBox(center));
    m_boundingBox->ResetBoundary(bbx,0.);
    m_boundary = m_boundingBox;
    Build();
  }

void
BoundingBox2DModel::
Build() {
  auto x = m_boundingBox->GetRange(0);
  auto y = m_boundingBox->GetRange(1);

  // Compute center
  m_center[0] = (x.min + x.max) / 2.;
  m_center[1] = (y.min + y.max) / 2.;
  m_center[2] = 0;

  GLdouble vertices[] = {
    x.min,  y.min, 0,
    x.min,  y.max, 0,
    x.max, y.min, 0,
    x.max, y.max, 0,
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
  auto x = m_boundingBox->GetRange(0);
  auto y = m_boundingBox->GetRange(1);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glScalef( x.max - x.min,
      y.max - y.min,
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
  auto x = m_boundingBox->GetRange(0);
  auto y = m_boundingBox->GetRange(1);
  vector<pair<double, double>> ranges;
  ranges.emplace_back(x.min,x.max);
  ranges.emplace_back(y.min,y.max);
  ranges.push_back(make_pair(
        -numeric_limits<double>::max(), numeric_limits<double>::max()));
  return ranges;
}

double
BoundingBox2DModel::
GetMaxDist() {
  return m_boundingBox->GetMaxDist();
}
