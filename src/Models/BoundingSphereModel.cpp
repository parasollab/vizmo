#include "BoundingSphereModel.h"

#include <limits>
#include <sstream>

#include "glut.h"

#include "Environment/BoundingSphere.h"

BoundingSphereModel::
BoundingSphereModel(shared_ptr<BoundingSphere> _b) :
    BoundaryModel("Bounding Sphere", _b), m_boundingSphere(_b) {
  m_center = m_boundingSphere->GetCenter();
  Build();
}

BoundingSphereModel::
BoundingSphereModel(const Point3d& _c, double _r) :
    BoundaryModel("Bounding Sphere", NULL) {
  m_boundingSphere = shared_ptr<BoundingSphere>(new BoundingSphere(_c, _r));
  m_boundary = m_boundingSphere;
  m_center = m_boundingSphere->GetCenter();
  Build();
}

double
BoundingSphereModel::
GetRadius() const {
  return m_boundingSphere->GetRadius();
}

vector<pair<double, double> >
BoundingSphereModel::
GetRanges() const {
  vector<pair<double, double> > ranges;
  const Vector3d& center = m_boundingSphere->GetCenter();
  double radius = m_boundingSphere->GetRadius();
  for(int i=0; i<3; i++)
    ranges.push_back(make_pair(center[i] - radius, center[i] + radius));
  return ranges;
}

void
BoundingSphereModel::
Build() {
  const Vector3d& center = m_boundingSphere->GetCenter();
  double radius = m_boundingSphere->GetRadius();

  GLUquadricObj* quad = gluNewQuadric();

  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glPolygonMode(GL_FRONT, GL_FILL);
  glColor3f(0.85, 0.85, 0.85);
  glPushMatrix();
  glTranslatef(center[0], center[1], center[2]);
  gluSphere(quad, radius, 20, 20);
  glPopMatrix();
  glDisable(GL_CULL_FACE);
  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glColor3f(1.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(center[0], center[1], center[2]);
  glutWireSphere(radius, 20, 20);
  glPopMatrix();
  glEndList();

  gluDeleteQuadric(quad);
}

void
BoundingSphereModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << *m_boundingSphere << endl;
}

double
BoundingSphereModel::
GetMaxDist() {
  return m_boundingSphere->GetMaxDist();
}
