#include "BoundingSphereModel.h"
#include "glut.h"
#include <limits>

BoundingSphereModel::BoundingSphereModel() : BoundaryModel(), m_center(0.0, 0.0, 0.0), m_radius(numeric_limits<double>::max()) {
}

bool BoundingSphereModel::BuildModels(){

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

  return true;
}

vector<string>
BoundingSphereModel::GetInfo() const {
  vector<string> info;
  info.push_back(GetName());
  info.push_back(" [ ");
  ostringstream tmp;
  tmp << m_center << " " << m_radius << endl;
  info.push_back(tmp.str());
  info.push_back(" ] ");
  return info;
}

bool
BoundingSphereModel::Parse(istream& _is){
  if(!(_is >> m_center >> m_radius)){
    cerr << "Error reading Bounding Sphere" << endl;
    return false;
  }
  return true;
}
