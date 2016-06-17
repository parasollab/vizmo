#include "PolyhedronModel.h"

#include <cmath>
#include <algorithm>
#include <numeric>

#include "IModel.h"
#include "ModelFactory.h"

#include "Utilities/VizmoExceptions.h"

PolyhedronModel::PolyhedronModel(const string& _filename,
    GMSPolyhedron::COMAdjust _comAdjust, double _mass)
  : Model(_filename), m_filename(_filename),
  m_numVerts(0), m_solidID(0), m_wiredID(0), m_normalsID(0),
  m_comAdjust(_comAdjust), m_mass(_mass) {
    Build();
  }

PolyhedronModel::PolyhedronModel(const PolyhedronModel& _p) : Model(_p),
  m_filename(_p.m_filename), m_comAdjust(_p.m_comAdjust) {
    Build();
  }

PolyhedronModel::~PolyhedronModel(){
  glDeleteLists(m_wiredID, 1);
  glDeleteLists(m_solidID, 1);
  glDeleteLists(m_normalsID, 1);
}

void
PolyhedronModel::Build() {

  IModel* imodel = CreateModelLoader(m_filename, false);

  bool isObj = m_filename.substr(m_filename.rfind('.'),
      m_filename.length()) == ".obj" && !imodel->GetNormals().empty();

  if(!imodel)
    throw BuildException(WHERE, "File '" + m_filename + "' does not exist.");

  const PtVector& points = imodel->GetVertices();
  m_numVerts = points.size();

  //compute center of mass and radius
  COM(points);
  Radius(points);

  //build all rendering call lists
  vector<Vector3d> normals;
  ComputeNormals(imodel, normals);
  if(isObj) {
    BuildSolidObj(imodel);
    BuildNormalsObj(imodel);
  }
  else {
    BuildSolidBYU(imodel, normals);
    BuildNormalsBYU(imodel, normals);
  }
  BuildWired(imodel, normals);

  delete imodel;
}

void
PolyhedronModel::
DrawRender() {
  if(m_solidID == 0 || m_renderMode == INVISIBLE_MODE)
    return;

  if(m_renderMode == SOLID_MODE){
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0, 2.0);
    glEnable(GL_NORMALIZE);
    glCallList(m_solidID);
    glDisable(GL_NORMALIZE);
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
  else
    glCallList(m_wiredID);
}

void
PolyhedronModel::
DrawSelect() {
  if(m_solidID == 0 || m_renderMode == INVISIBLE_MODE)
    return;

  glCallList(m_solidID);
}

void
PolyhedronModel::
DrawSelected() {
  glCallList(m_wiredID);
}

void
PolyhedronModel::
DrawHaptics() {
  glCallList(m_solidID);
}

void
PolyhedronModel::
Print(ostream& _os) const {
  _os << m_filename << endl;
}

void
PolyhedronModel::
DrawNormals() {
  if(m_showNormals)
    glCallList(m_normalsID);
}

void
PolyhedronModel::
BuildSolidObj(IModel* _model) {
  m_solidID = glGenLists(1);
  glNewList(m_solidID, GL_COMPILE);

  glEnable(GL_LIGHTING);
  glPushMatrix();

  switch(m_comAdjust) {
    case GMSPolyhedron::COMAdjust::COM:
      glTranslated(-m_com[0], -m_com[1], -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::Surface:
      glTranslated(-m_com[0], 0, -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::None:
    default:
      break;
  }

  PtVector& points = _model->GetVertices();
  vector<Vector3d>& normals = _model->GetNormals();
  vector<Vector2d>& textures = _model->GetTextureCoords();
  const TriVector& triP = _model->GetTriP();
  const TriVector& triN = _model->GetTriN();
  const TriVector& triT = _model->GetTriT();

  //draw
  glBegin(GL_TRIANGLES);
  for(size_t i = 0; i < triP.size(); ++i) {
    for(size_t j = 0; j < 3; j++) {
      if(!textures.empty())
        glTexCoord2dv(textures[triT[i][j]]);
      glNormal3dv(normals[triN[i][j]]);
      glVertex3dv(points[triP[i][j]]);
    }
  }
  glEnd();

  glPopMatrix();
  glEndList();
}

//build models, given points and triangles
void
PolyhedronModel::
ComputeNormals(IModel* _model, vector<Vector3d>& _norms) {

  const PtVector& points = _model->GetVertices();
  const TriVector& tris = _model->GetTriP();

  _norms.clear();
  _norms.reserve(tris.size());
  for(auto& tri : tris) {
    Vector3d v1 = points[tri[1]] - points[tri[0]];
    Vector3d v2 = points[tri[2]] - points[tri[0]];
    _norms.emplace_back((v1%v2).normalize());
  }
}

void
PolyhedronModel::
BuildSolidBYU(IModel* _model, const vector<Vector3d>& _norms) {

  m_solidID = glGenLists(1);
  glNewList(m_solidID, GL_COMPILE);

  glEnable(GL_LIGHTING);
  glPushMatrix();

  switch(m_comAdjust) {
    case GMSPolyhedron::COMAdjust::COM:
      glTranslated(-m_com[0], -m_com[1], -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::Surface:
      glTranslated(-m_com[0], 0, -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::None:
    default:
      break;
  }

  const PtVector& points = _model->GetVertices();
  const TriVector& tris = _model->GetTriP();

  //draw
  glBegin(GL_TRIANGLES);
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = tris.begin(); trit != tris.end(); ++trit) {
    const Tri& tri = *trit;
    glNormal3dv(_norms[trit - tris.begin()]);
    for(int i=0; i < 3; i++)
      glVertex3dv(points[tri[i]]);
  }
  glEnd();

  glPopMatrix();
  glEndList();
}

void
PolyhedronModel::
BuildNormalsObj(IModel* _model) {
  m_normalsID = glGenLists(1);
  glNewList(m_normalsID, GL_COMPILE);

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  switch(m_comAdjust) {
    case GMSPolyhedron::COMAdjust::COM:
      glTranslated(-m_com[0], -m_com[1], -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::Surface:
      glTranslated(-m_com[0], 0, -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::None:
    default:
      break;
  }

  PtVector& points = _model->GetVertices();
  vector<Vector3d>& normals = _model->GetNormals();
  const TriVector& triP = _model->GetTriP();
  const TriVector& triN = _model->GetTriN();

  //draw
  glColor3f(0, 1, 0);
  glLineWidth(2);
  glBegin(GL_LINES);
  for(size_t i = 0; i < triP.size(); ++i) {
    for(size_t j = 0; j < 3; ++j) {
      const Vector3d& norm = normals[triN[i][j]];
      Point3d start = points[triP[i][j]];
      Point3d end = start + norm.normalize();

      glVertex3dv(start);
      glVertex3dv(end);
    }
  }
  glEnd();

  glPointSize(4);
  glBegin(GL_POINTS);
  for(size_t i = 0; i < triP.size(); ++i) {
    for(size_t j = 0; j < 3; ++j) {
      const Vector3d& norm = normals[triN[i][j]];
      Point3d start = points[triP[i][j]];
      Point3d end = start + norm.normalize();

      glVertex3dv(end);
    }
  }
  glEnd();


  glPopMatrix();
  glEndList();
}

void
PolyhedronModel::
BuildNormalsBYU(IModel* _model, const vector<Vector3d>& _norms) {
  m_normalsID = glGenLists(1);
  glNewList(m_normalsID, GL_COMPILE);

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  switch(m_comAdjust) {
    case GMSPolyhedron::COMAdjust::COM:
      glTranslated(-m_com[0], -m_com[1], -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::Surface:
      glTranslated(-m_com[0], 0, -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::None:
    default:
      break;
  }

  const PtVector& points = _model->GetVertices();
  const TriVector& tris = _model->GetTriP();

  //draw
  glColor3f(0, 1, 0);
  glLineWidth(2);
  glBegin(GL_LINES);
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = tris.begin(); trit != tris.end(); ++trit) {
    const Tri& tri = *trit;
    const Vector3d& norm = _norms[distance(tris.begin(), trit)];
    Point3d center = (points[tri[0]] + points[tri[1]] + points[tri[2]])/3;
    Point3d endp = center + norm.normalize();

    glVertex3dv(center);
    glVertex3dv(endp);
  }
  glEnd();

  glPointSize(4);
  glBegin(GL_POINTS);
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = tris.begin(); trit != tris.end(); ++trit) {
    const Tri& tri = *trit;
    const Vector3d& norm = _norms[distance(tris.begin(), trit)];
    Point3d center = (points[tri[0]] + points[tri[1]] + points[tri[2]])/3;
    Point3d endp = center + norm.normalize();

    glVertex3dv(endp);
  }
  glEnd();

  glPopMatrix();
  glEndList();
}

//compute the model graph for wire frame
void
PolyhedronModel::
BuildModelGraph(IModel* _model) {
  const PtVector& points = _model->GetVertices();
  const TriVector& tris = _model->GetTriP();

  //create nodes
  for(size_t i = 0; i < points.size(); ++i)
    m_modelGraph.add_vertex(i, i);

  //create edge from triangles
  for(size_t i = 0; i < tris.size(); ++i) {
    for(size_t j = 0; j < 3; ++j) {
      int a = tris[i][j];
      int b = tris[i][(j+1)%3];
      //see if edge (a, b) exists
      ModelGraph::vertex_iterator vit;
      ModelGraph::adj_edge_iterator eit;
      ModelGraph::edge_descriptor eid(a, b);
      bool found = m_modelGraph.find_edge(eid, vit, eit);

      if(found) //set the right triangle
        (*eit).property()[1] = i;
      else //add new edge and set left triangle
        m_modelGraph.add_edge(a, b, Vector<int, 2>(i, -1));
    }
  }
}

void
PolyhedronModel::
BuildWired(IModel* _model, const vector<Vector3d>& _norms) {

  //create model graph
  BuildModelGraph(_model);

  //build model
  m_wiredID = glGenLists(1);
  glNewList(m_wiredID, GL_COMPILE);

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  switch(m_comAdjust) {
    case GMSPolyhedron::COMAdjust::COM:
      glTranslated(-m_com[0], -m_com[1], -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::Surface:
      glTranslated(-m_com[0], 0, -m_com[2]);
      break;
    case GMSPolyhedron::COMAdjust::None:
    default:
      break;
  }

  const PtVector& points = _model->GetVertices();

  glBegin(GL_LINES);
  typedef ModelGraph::edge_iterator EIT;
  for(EIT eit = m_modelGraph.edges_begin();
      eit != m_modelGraph.edges_end(); ++eit) {

    int tril = (*eit).property()[0];
    int trir = (*eit).property()[1];

    if(tril == -1 || trir == -1 || 1-fabs(_norms[tril] * _norms[trir]) > 1e-3) {
      glVertex3dv(points[(*eit).source()]);
      glVertex3dv(points[(*eit).target()]);
    }
  }
  glEnd();

  glPopMatrix();
  glEndList();
}

void
PolyhedronModel::
COM(const PtVector& _points) {
  m_com = accumulate(_points.begin(), _points.end(), Point3d(0, 0, 0));
  m_com /= _points.size();
}

void
PolyhedronModel::
Radius(const PtVector& _points) {
  m_radius = 0;
  for(PtVector::const_iterator i = _points.begin(); i!=_points.end(); ++i) {
    double d = (*i - m_com).normsqr();
    if(d > m_radius)
      m_radius = d;
  }
  m_radius = sqrt(m_radius);
}

