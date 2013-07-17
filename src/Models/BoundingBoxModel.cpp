#include "BoundingBoxModel.h"
#include <limits>

BoundingBoxModel::BoundingBoxModel() : BoundaryModel() {
  for(int i = 0; i<3; ++i){
    m_bbx[i].first = -numeric_limits<double>::max();
    m_bbx[i].second = numeric_limits<double>::max();
  }
}

bool BoundingBoxModel::BuildModels(){
  double zmin = m_bbx[2].second == numeric_limits<double>::max() ? -1 : m_bbx[2].first;
  double zmax = m_bbx[2].second == numeric_limits<double>::max() ? 1 : m_bbx[2].second;
  GLdouble vertices[]=
  { m_bbx[0].first, m_bbx[1].first, zmin,
    m_bbx[0].second, m_bbx[1].first, zmin,
    m_bbx[0].second, m_bbx[1].first, zmax,
    m_bbx[0].first, m_bbx[1].first, zmax,
    m_bbx[0].first, m_bbx[1].second, zmin,
    m_bbx[0].second, m_bbx[1].second, zmin,
    m_bbx[0].second, m_bbx[1].second, zmax,
    m_bbx[0].first, m_bbx[1].second, zmax};

  //Face index
  GLubyte id1[] = { 3, 2, 1, 0 }; //buttom
  GLubyte id2[] = { 4, 5, 6, 7 }; //top
  GLubyte id3[] = { 2, 6, 5, 1 }; //left
  GLubyte id4[] = { 0, 4, 7, 3 }; //right
  GLubyte id5[] = { 1, 5, 4, 0 }; //back
  GLubyte id6[] = { 7, 6, 2, 3 }; //front

  //line index
  GLubyte lineid[] = 
  { 0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7};

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

  return true;
}

const string BoundingBoxModel::GetName() const{
  return "Bounding Box";
}

vector<string> 
BoundingBoxModel::GetInfo() const { 
  vector<string> info; 
  info.push_back(GetName());
  info.push_back(" [ ");
  for(int i = 0; i < 3; ++i){
    if(m_bbx[i].second != numeric_limits<double>::max()){
      ostringstream tmp;
      tmp << m_bbx[i].first << ":" << m_bbx[i].second << " ";
      info.push_back(tmp.str());
    }
  }
  return info;
}

bool
BoundingBoxModel::Parse(istream& _is){
  //read next three tokens
  string line;
  getline(_is, line);
  istringstream iss(line);
  for(size_t i = 0; i < 3; ++i){
    string tok;
    if(iss >> tok){
      size_t del = tok.find(":");
      if(del == string::npos){
        cerr << "Error::Reading bounding box range " << i << ". Should be delimited by ':'." << endl;
        return false;
      }
      istringstream minv(tok.substr(0,del)), maxv(tok.substr(del+1, tok.length()));
      if(!(minv>>m_bbx[i].first && maxv>>m_bbx[i].second)){
        cerr << "Error::Reading bounding box range " << i << "." << endl;
        return false;
      }
    }
    else if(i < 2) { //error. only 1 token provided.
      cerr << "Error::Reading bounding box ranges. Only one provided." << endl;
      return false;
    }
  }
  return true;
}
