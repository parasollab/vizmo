#include <limits>
#include <sstream>

#include "BoundingBoxModel.h"

#include <glut.h>

BoundingBoxModel::
BoundingBoxModel() : BoundaryModel("Bounding Box"), m_bbx(3,
    make_pair(-numeric_limits<double>::max(), numeric_limits<double>::max())) { }


BoundingBoxModel::
BoundingBoxModel(const pair<double, double>& _x, const pair<double, double>& _y,
    const pair<double, double>& _z) : BoundaryModel("Bounding Box") {
  m_bbx.push_back(_x);
  m_bbx.push_back(_y);
  m_bbx.push_back(_z);
  Build();
}


bool
BoundingBoxModel::
Parse(istream& _is) {
  //read next three tokens
  string line;
  getline(_is, line);
  istringstream iss(line);
  for(size_t i = 0; i < 3; ++i) {
    string tok;
    if(iss >> tok){
      size_t del = tok.find(":");
      if(del == string::npos) {
        cerr << "Error::Reading bounding box range " << i
             << ". Should be delimited by ':'." << endl;
        return false;
      }
      istringstream minVStr(tok.substr(0, del)),
                    maxVStr(tok.substr(del+1, tok.length()));
      double minV, maxV;
      if((minVStr >> minV && maxVStr >> maxV) == false) {
        cerr << "Error::Reading bounding box range " << i << "." << endl;
        return false;
      }
      m_bbx[i] = make_pair(minV, maxV);
    }
    else if(i < 2) { //error. only 1 token provided.
      cerr << "Error::Reading bounding box ranges. Only one provided." << endl;
      return false;
    }
  }
  return true;
}


void
BoundingBoxModel::
Build() {
  double zmin = m_bbx[2].second == numeric_limits<double>::max() ?
      -1 : m_bbx[2].first;
  double zmax = m_bbx[2].second == numeric_limits<double>::max() ?
       1 : m_bbx[2].second;

  // Compute center
  m_center[0] = (m_bbx[0].first + m_bbx[0].second) / 2.;
  m_center[1] = (m_bbx[1].first + m_bbx[1].second) / 2.;
  m_center[2] = (zmin + zmax) / 2.;

  GLdouble vertices[] = {
      m_bbx[0].first,  m_bbx[1].first,  zmin,
      m_bbx[0].second, m_bbx[1].first,  zmin,
      m_bbx[0].second, m_bbx[1].first,  zmax,
      m_bbx[0].first,  m_bbx[1].first,  zmax,
      m_bbx[0].first,  m_bbx[1].second, zmin,
      m_bbx[0].second, m_bbx[1].second, zmin,
      m_bbx[0].second, m_bbx[1].second, zmax,
      m_bbx[0].first,  m_bbx[1].second, zmax
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
  glPushMatrix();
  glTranslatef( (m_bbx[0].first+m_bbx[0].second) / 2.,
                (m_bbx[1].first+m_bbx[1].second) / 2.,
                (m_bbx[2].first+m_bbx[2].second) / 2.);
  glScalef( m_bbx[0].second - m_bbx[0].first,
            m_bbx[1].second - m_bbx[1].first,
            m_bbx[2].second - m_bbx[2].first);
  glutSolidCube(1);
  glPopMatrix();
}


void
BoundingBoxModel::
Print(ostream& _os) const {
  _os << Name() << endl << "[ ";
  for(int i = 0; i < 3; ++i)
    if(m_bbx[i].second != numeric_limits<double>::max())
      _os << m_bbx[i].first << ":" << m_bbx[i].second << " ";
  _os << "]" << endl;
}


void
BoundingBoxModel::
Write(ostream& _os) const {
  _os << "Box ";
  for(size_t i = 0; i < 3; ++i)
    if(m_bbx[i].second != numeric_limits<double>::max())
      _os << m_bbx[i].first << ":" << m_bbx[i].second << " ";
}
