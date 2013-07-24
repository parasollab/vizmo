#include "PolyhedronModel.h"

#include <cmath>
#include <algorithm>
#include <numeric>

#include <RAPID.H>
#include <obb.H>

#include "ModelFactory.h"

#include "Utilities/Exceptions.h"
#include "ModelGraph/ModelGraph.h"
using namespace modelgraph;

PolyhedronModel::PolyhedronModel() : m_solidID(-1), m_wiredID(-1), m_rapidModel(NULL) {
}

PolyhedronModel::~PolyhedronModel(){
  delete m_rapidModel;
  glDeleteLists(m_wiredID,1);
  glDeleteLists(m_solidID,1);
}

const string
PolyhedronModel::GetName() const {
  string filename = m_bodyInfo.m_modelDataFileName;
  size_t pos = filename.find('/');
  return pos == string::npos ? filename : filename.substr(pos+1);
}

vector<string>
PolyhedronModel::GetInfo() const {
  vector<string> info;
  info.push_back(m_bodyInfo.m_modelDataFileName);
  return info;
}

void
PolyhedronModel::BuildModels() {

  string file = m_bodyInfo.m_modelDataFileName;
  IModel* imodel = CreateModelLoader(file, false);

  const PtVector& points=imodel->GetVertices();
  const TriVector& tris=imodel->GetTriP(); //GetTriangles();

  //compute center of mass and radius
  COM(points);
  Radius(points);

  //set point for opengl
  GLdouble vertice[3*points.size()];

  typedef PtVector::const_iterator PIT;
  for(PIT pit = points.begin(); pit!=points.end(); ++pit)
    copy(pit->begin(), pit->end(), vertice + (pit-points.begin())*3);

  vector<Vector3d> normals;
  ComputeNormals(points, tris, normals);

  BuildRapid(points, tris);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertice);

  BuildSolid(points, tris, normals);
  BuildWired(points, tris, normals);

  //setup rotation and translation
  if(m_bodyInfo.m_isFixed) {
    tx()=m_bodyInfo.m_x;
    ty()=m_bodyInfo.m_y;
    tz()=m_bodyInfo.m_z;

    rx() = m_bodyInfo.m_alpha;
    ry() = m_bodyInfo.m_beta;
    rz() = m_bodyInfo.m_gamma;

    double cx2 = cos(m_bodyInfo.m_alpha/2);
    double sx2 = sin(m_bodyInfo.m_alpha/2);
    double cy2 = cos(m_bodyInfo.m_beta/2);
    double sy2 = sin(m_bodyInfo.m_beta/2);
    double cz2 = cos(m_bodyInfo.m_gamma/2);
    double sz2 = sin(m_bodyInfo.m_gamma/2);

    Quaternion qx(cx2, Vector3d(sx2, 0, 0));
    Quaternion qy(cy2, Vector3d(0, sy2, 0));
    Quaternion qz(cz2, Vector3d(0, 0, sz2));
    Quaternion nq = qz * qy * qx;
    q(nq.normalize());
  }
}

void PolyhedronModel::Draw(GLenum _mode) {
  if(m_solidID == size_t(-1)) return;
  if(m_renderMode == INVISIBLE_MODE) return;

  float* color = &m_rgba[0];
  glColor4fv(color);
  glPushMatrix();
  glTransform();
  glScale();

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

  glPopMatrix();
}

void PolyhedronModel::DrawSelect() {
  glLineWidth(2);
  glPushMatrix();
  glTransform();
  glColor3d(1,1,0);
  glCallList(m_wiredID);
  glPopMatrix();
}

//build models, given points and triangles
void
PolyhedronModel::ComputeNormals(const PtVector& _points, const TriVector& _tris, vector<Vector3d>& _norms) {
  _norms.clear();
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = _tris.begin(); trit!=_tris.end(); ++trit){
    const Tri& tri= *trit;
    Vector3d v1 = _points[tri[1]] - _points[tri[0]];
    Vector3d v2 = _points[tri[2]] - _points[tri[0]];
    _norms.push_back((v1%v2).normalize());
  }
}

void
PolyhedronModel::BuildRapid(const PtVector& _points, const TriVector& _tris) {
  m_rapidModel = new RAPID_model;
  m_rapidModel->BeginModel();

  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = _tris.begin(); trit!=_tris.end(); ++trit){
    const Tri& tri = *trit;

    Vector3d p1 = _points[tri[0]] - m_com;
    Vector3d p2 = _points[tri[1]] - m_com;
    Vector3d p3 = _points[tri[2]] - m_com;

    m_rapidModel->AddTri(p1, p2, p3, trit - _tris.begin());
  }
  m_rapidModel->EndModel();
}

void
PolyhedronModel::BuildSolid(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms) {
  m_solidID = glGenLists(1);
  glNewList(m_solidID, GL_COMPILE);

  glEnable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(-m_com[0], -m_com[1], -m_com[2]);

  //draw
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = _tris.begin(); trit!=_tris.end(); ++trit){
    const Tri& tri = *trit;
    glNormal3dv(_norms[trit-_tris.begin()]);
    GLint id[3] = {tri[0], tri[1], tri[2]};
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, id);
  }

  glPopMatrix();
  glEndList();
}

//build wire frame
void
PolyhedronModel::BuildWired(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms) {
  CModelGraph mg;
  if(!mg.doInit(_points, _tris))
    throw BuildException(WHERE, "Cannot initialize ModelGraph");

  //build model
  m_wiredID=glGenLists(1);
  glNewList(m_wiredID,GL_COMPILE);

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(-m_com[0], -m_com[1], -m_com[2]);

  const CModelEdge* edge = mg.getEdges();
  while(edge != NULL){
    int tril = edge->getLeftTri();
    int trir = edge->getRightTri();

    if(tril == -1 || trir == -1 || 1-fabs(_norms[tril] * _norms[trir]) > 1e-3){
      GLint id[2]={edge->getStartPt(),edge->getEndPt()};
      glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, id);
    }
    edge = edge->getNext();
  }

  glPopMatrix();
  glEndList();
}

//set m_com to center of mass of _points
void
PolyhedronModel::COM(const PtVector& _points) {
  m_com = accumulate(_points.begin(), _points.end(), Point3d(0, 0, 0));
  m_com /= _points.size();
  if(m_bodyInfo.m_isSurface)
    m_com[1] = 0;
}

//set m_radius to distance furthest point in _points to m_com
void
PolyhedronModel::Radius(const PtVector& _points) {
  m_radius = 0;
  for(PtVector::const_iterator i = _points.begin(); i!=_points.end(); ++i) {
    double d = (*i - m_com).normsqr();
    if(d > m_radius)
      m_radius = d;
  }
  m_radius = sqrt(m_radius);
}

void
PolyhedronModel::CopyRapidModel(const PolyhedronModel& _source){
  m_rapidModel = new RAPID_model;

  m_rapidModel->num_tris = (_source.m_rapidModel)->num_tris;
  m_rapidModel->build_state = (_source.m_rapidModel)->build_state;
  m_rapidModel->num_tris_alloced = (_source.m_rapidModel)->num_tris_alloced;
  m_rapidModel->num_boxes_alloced = (_source.m_rapidModel)->num_boxes_alloced;

  m_rapidModel->b = new box[(_source.m_rapidModel)->num_boxes_alloced];
  box* boxBeginPtr = &((_source.m_rapidModel)->b)[0];
  box* boxEndPtr = &((_source.m_rapidModel)->b)[m_rapidModel->num_boxes_alloced];
  copy(boxBeginPtr, boxEndPtr, &(m_rapidModel->b)[0]);

  m_rapidModel->tris = new tri[(_source.m_rapidModel)->num_tris_alloced];
  tri* triBeginPtr = &((_source.m_rapidModel)->tris)[0];
  tri* triEndPtr = &((_source.m_rapidModel)->tris)[m_rapidModel->num_tris_alloced];
  copy(triBeginPtr, triEndPtr, &(m_rapidModel->tris)[0]);
}




