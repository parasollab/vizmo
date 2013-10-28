#include "BoundingSphereModel.h"
#include "glut.h"
#include <limits>
#include <sstream>

BoundingSphereModel::BoundingSphereModel() : BoundaryModel("Bounding Sphere"), m_radius(numeric_limits<double>::max()) {
}

vector<pair<double, double> >
BoundingSphereModel::GetRanges(){

  vector<pair<double, double> > ranges;
  ranges.push_back(make_pair(m_center[0] - m_radius, m_center[0] + m_radius));
  ranges.push_back(make_pair(m_center[1] - m_radius, m_center[1] + m_radius));
  ranges.push_back(make_pair(m_center[2] - m_radius, m_center[2] + m_radius));
  return ranges;
}

bool
BoundingSphereModel::Parse(istream& _is) {
  if(!(_is >> m_center >> m_radius)){
    cerr << "Error reading Bounding Sphere" << endl;
    return false;
  }
  return true;
}

void
BoundingSphereModel::BuildModels() {
  GLUquadricObj* quad =gluNewQuadric();

  m_displayID = glGenLists(1);
  glNewList(m_displayID, GL_COMPILE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  glPolygonMode(GL_FRONT, GL_FILL);
  glColor3f(0.85, 0.85, 0.85);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  gluSphere(quad, m_radius, 20, 20);
  glPopMatrix();
  glDisable(GL_CULL_FACE);
  glEndList();

  m_linesID = glGenLists(1);
  glNewList(m_linesID, GL_COMPILE);
  glColor3f(1.0, 1.0, 0.0);
  glPushMatrix();
  glTranslatef(m_center[0], m_center[1], m_center[2]);
  glutWireSphere(m_radius, 20, 20);
  glPopMatrix();
  glEndList();

  gluDeleteQuadric(quad);
}

void
BoundingSphereModel::Print(ostream& _os) const {
  _os << Name() << endl
    << "[ " << m_center << " " << m_radius << " ]" << endl;
}

string
BoundingSphereModel::GetCoord(){
  ostringstream coord;
  coord<<m_center<<" "<<m_radius;
  return coord.str();
}
