#include "BoundingSphere2DModel.h"

#include <limits>
#include <sstream>

//#include "Environment/WorkspaceBoundingSphere.h"
#include "Geometry/Boundaries/WorkspaceBoundingSphere.h"

#include "Utilities/GLUtils.h"

BoundingSphere2DModel::
BoundingSphere2DModel(shared_ptr<WorkspaceBoundingSphere> _b) :
    BoundaryModel("Bounding Sphere", _b), m_boundingSphere(_b) {
  m_center = Point2d(0,0); //m_boundingSphere->GetCenter();
  Build();
}

BoundingSphere2DModel::
BoundingSphere2DModel(const Point2d& _c, double _r) :
    BoundaryModel("Bounding Sphere", NULL) {
    Point3d c(_c[0],_c[1],0);
  m_boundingSphere = shared_ptr<WorkspaceBoundingSphere>(new WorkspaceBoundingSphere(c, _r));
  m_boundary = m_boundingSphere;
  m_center = _c; //m_boundingSphere->GetCenter();
  Build();
}

double
BoundingSphere2DModel::
GetRadius() const {
  return m_boundingSphere->GetRadius();
}

vector<pair<double, double> >
BoundingSphere2DModel::
GetRanges() const {
  vector<pair<double, double> > ranges;
  const auto& center = m_boundingSphere->GetCenter();
  double radius = m_boundingSphere->GetRadius();
  for(int i=0; i<2; i++)
    ranges.push_back(make_pair(center[i] - radius, center[i] + radius));
  ranges.push_back(make_pair(
        -numeric_limits<double>::max(), numeric_limits<double>::max()));
  return ranges;
}

void
BoundingSphere2DModel::
Build() {
  const auto& center = m_boundingSphere->GetCenter();
  double radius = m_boundingSphere->GetRadius();

  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);
  glLineWidth(2);
  glColor3d(0.0, 0.0, 0.0);
  glTranslatef(center[0], center[1], center[2]);
  GLUtils::DrawCircle(radius, false);
  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glTranslatef(center[0], center[1], center[2]);
  GLUtils::DrawCircle(radius, false);
  glEndList();
}

void
BoundingSphere2DModel::
Print(ostream& _os) const {
  _os << Name() << endl
    << *m_boundingSphere << endl;
}

double
BoundingSphere2DModel::
GetMaxDist() {
  return m_boundingSphere->GetMaxDist();
}
