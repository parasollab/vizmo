#include "BoundingBoxModel.h"

#include <limits>
#include <sstream>

#ifdef __APPLE__
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

#include "Geometry/Boundaries/WorkspaceBoundingBox.h"

BoundingBoxModel::
BoundingBoxModel(shared_ptr<WorkspaceBoundingBox> _b) :
  BoundaryModel("Bounding Box", _b),
  m_boundingBox(_b) {
    Build();
  }

BoundingBoxModel::
BoundingBoxModel(const pair<double, double>& _x, const pair<double, double>& _y,
    const pair<double, double>& _z) :
  BoundaryModel("Bounding Box", NULL) {
    std::vector<double> center = {0.0,0.0,0.0};
    std::vector<std::pair<double,double>> bbx = {_x,_y,_z};
    m_boundingBox = shared_ptr<WorkspaceBoundingBox>(new WorkspaceBoundingBox(center));
    m_boundingBox->ResetBoundary(bbx,0.);
    m_boundary = m_boundingBox;
    Build();
  }

void
BoundingBoxModel::
Build() {
  //const pair<double, double>* const bbx = m_boundingBox->GetBox();
  const auto bbx = this->GetRanges();

  // Compute center
  m_center[0] = (bbx[0].first + bbx[0].second) / 2.;
  m_center[1] = (bbx[1].first + bbx[1].second) / 2.;
  m_center[2] = (bbx[2].first + bbx[2].second) / 2.;

  GLdouble vertices[] = {
    bbx[0].first,  bbx[1].first, bbx[2].first,
    bbx[0].second, bbx[1].first, bbx[2].first,
    bbx[0].second, bbx[1].first, bbx[2].second,
    bbx[0].first,  bbx[1].first, bbx[2].second,
    bbx[0].first,  bbx[1].second, bbx[2].first,
    bbx[0].second, bbx[1].second, bbx[2].first,
    bbx[0].second, bbx[1].second, bbx[2].second,
    bbx[0].first,  bbx[1].second, bbx[2].second
  };

  //Face index
  GLubyte id1[] = { 3, 2, 1, 0 }; //bottom
  GLubyte id2[] = { 4, 5, 6, 7 }; //top
  GLubyte id3[] = { 2, 6, 5, 1 }; //left
  GLubyte id4[] = { 0, 4, 7, 3 }; //right
  GLubyte id5[] = { 1, 5, 4, 0 }; //back
  GLubyte id6[] = { 7, 6, 2, 3 }; //front

  //line index
  GLubyte lineid[] = {
    0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7
  };

  //set properties for this box
  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glPolygonMode(GL_FRONT, GL_FILL);

  //setup points
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);

  glColor3d(0.95, 0.95, 0.95);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id1);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id2);

  glColor3d(0.85, 0.85, 0.85);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id3);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id4);

  glColor3d(0.75, 0.75, 0.75);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id5);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, id6);
  glDisable(GL_POLYGON_OFFSET_FILL);

  //Draw lines
  glLineWidth(1);
  glColor3d(0.5, 0.5, 0.5);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, lineid);

  glDisable(GL_CULL_FACE);
  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glVertexPointer(3, GL_DOUBLE, 0, vertices);
  //Draw lines
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, lineid);
  glEndList();

  glDisableClientState(GL_VERTEX_ARRAY);
}


void
BoundingBoxModel::
DrawHaptics() {
  //const pair<double, double>* const bbx = m_boundingBox->GetBox();
  const auto bbx = this->GetRanges();
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glScalef( bbx[0].second - bbx[0].first,
      bbx[1].second - bbx[1].first,
      bbx[2].second - bbx[2].first);
  glutSolidCube(1);
  glPopMatrix();
}


void
BoundingBoxModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << *m_boundingBox << endl;
}

vector<pair<double, double> >
BoundingBoxModel::
GetRanges() const {
  // const pair<double, double>* const bbx = m_boundingBox->GetBox();
  // return vector<pair<double, double>>(bbx, bbx+3);

  vector<pair<double,double>> ranges;
  const size_t dim = m_boundingBox->GetDimension();

  for(size_t i = 0; i < dim; i++) {
    auto range = m_boundingBox->GetRange(i);
    ranges.emplace_back(range.min,range.max);
  }

  return ranges;
}

double
BoundingBoxModel::
GetMaxDist() {
  return m_boundingBox->GetMaxDist();
}
