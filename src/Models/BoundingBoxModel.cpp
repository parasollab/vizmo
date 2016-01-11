#include "BoundingBoxModel.h"

#include <limits>
#include <sstream>

#include <glut.h>

#include "Environment/BoundingBox.h"

BoundingBoxModel::
BoundingBoxModel(shared_ptr<BoundingBox> _b) :
  BoundaryModel("Bounding Box", _b),
  m_boundingBox(_b) {
    Build();
  }

BoundingBoxModel::
BoundingBoxModel(const pair<double, double>& _x, const pair<double, double>& _y,
    const pair<double, double>& _z) :
  BoundaryModel("Bounding Box", NULL) {
    m_boundingBox = shared_ptr<BoundingBox>(new BoundingBox(_x, _y, _z));
    m_boundary = m_boundingBox;
    Build();
  }

void
BoundingBoxModel::
Build() {
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
  double zmin = bbx[2].second == numeric_limits<double>::max() ?
    -1 : bbx[2].first;
  double zmax = bbx[2].second == numeric_limits<double>::max() ?
    1 : bbx[2].second;

  // Compute center
  m_center[0] = (bbx[0].first + bbx[0].second) / 2.;
  m_center[1] = (bbx[1].first + bbx[1].second) / 2.;
  m_center[2] = (zmin + zmax) / 2.;

  GLdouble vertices[] = {
    bbx[0].first,  bbx[1].first,  zmin,
    bbx[0].second, bbx[1].first,  zmin,
    bbx[0].second, bbx[1].first,  zmax,
    bbx[0].first,  bbx[1].first,  zmax,
    bbx[0].first,  bbx[1].second, zmin,
    bbx[0].second, bbx[1].second, zmin,
    bbx[0].second, bbx[1].second, zmax,
    bbx[0].first,  bbx[1].second, zmax
  };

  //Face index
  GLubyte id1[] = { 3, 2, 1, 0 }; //buttom
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
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
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
  const pair<double, double>* const bbx = m_boundingBox->GetBox();
  return vector<pair<double, double>>(bbx, bbx+3);
}

double
BoundingBoxModel::
GetMaxDist() {
  return m_boundingBox->GetMaxDist();
}

size_t
BoundingBoxModel::
GetNumVertices() const {
  return 8;
}

size_t
BoundingBoxModel::
GetNumFacets() const {
  return 6;
}

void
BoundingBoxModel::
AddToTetGen(tetgenio* _tetModel) const {
  const pair<double, double>* const ranges = m_boundingBox->GetBox();
  _tetModel->pointlist[ 0] = ranges[0].first;
  _tetModel->pointlist[ 1] = ranges[1].first;
  _tetModel->pointlist[ 2] = ranges[2].first;
  _tetModel->pointlist[ 3] = ranges[0].first;
  _tetModel->pointlist[ 4] = ranges[1].first;
  _tetModel->pointlist[ 5] = ranges[2].second;
  _tetModel->pointlist[ 6] = ranges[0].first;
  _tetModel->pointlist[ 7] = ranges[1].second;
  _tetModel->pointlist[ 8] = ranges[2].first;
  _tetModel->pointlist[ 9] = ranges[0].first;
  _tetModel->pointlist[10] = ranges[1].second;
  _tetModel->pointlist[11] = ranges[2].second;
  _tetModel->pointlist[12] = ranges[0].second;
  _tetModel->pointlist[13] = ranges[1].first;
  _tetModel->pointlist[14] = ranges[2].first;
  _tetModel->pointlist[15] = ranges[0].second;
  _tetModel->pointlist[16] = ranges[1].first;
  _tetModel->pointlist[17] = ranges[2].second;
  _tetModel->pointlist[18] = ranges[0].second;
  _tetModel->pointlist[19] = ranges[1].second;
  _tetModel->pointlist[20] = ranges[2].first;
  _tetModel->pointlist[21] = ranges[0].second;
  _tetModel->pointlist[22] = ranges[1].second;
  _tetModel->pointlist[23] = ranges[2].second;

  tetgenio::facet* f;
  tetgenio::polygon* p;
  //side 1
  f = &_tetModel->facetlist[0];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 0;
  p->vertexlist[1] = 1;
  p->vertexlist[2] = 3;
  p->vertexlist[3] = 2;

  //side 2
  f = &_tetModel->facetlist[1];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 0;
  p->vertexlist[1] = 2;
  p->vertexlist[2] = 6;
  p->vertexlist[3] = 4;

  //side 3
  f = &_tetModel->facetlist[2];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 4;
  p->vertexlist[1] = 5;
  p->vertexlist[2] = 7;
  p->vertexlist[3] = 6;

  //side 4
  f = &_tetModel->facetlist[3];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 5;
  p->vertexlist[1] = 1;
  p->vertexlist[2] = 3;
  p->vertexlist[3] = 7;

  //side 5
  f = &_tetModel->facetlist[4];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 7;
  p->vertexlist[1] = 3;
  p->vertexlist[2] = 2;
  p->vertexlist[3] = 6;

  //side 6
  f = &_tetModel->facetlist[5];
  f->numberofpolygons = 1;
  f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
  f->numberofholes = 0;
  f->holelist = NULL;
  p = &f->polygonlist[0];
  p->numberofvertices = 4;
  p->vertexlist = new int[p->numberofvertices];
  p->vertexlist[0] = 0;
  p->vertexlist[1] = 4;
  p->vertexlist[2] = 5;
  p->vertexlist[3] = 1;

  for(size_t i = 0; i < 6; ++i)
    _tetModel->facetmarkerlist[i] = -1;
}

