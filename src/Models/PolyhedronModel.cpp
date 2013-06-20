#include "PolyhedronModel.h"

#include <cmath>

#include <RAPID.H>

#include "ModelGraph/ModelGraph.h"
using namespace modelgraph;

#include "ModelFactory.h"

PolyhedronModel::PolyhedronModel(plum::CBodyInfo& _bodyInfo) : 
  m_solidID(-1), m_wiredID(-1), 
  m_bodyInfo(_bodyInfo), m_rapidModel(NULL) {
  }

PolyhedronModel::~PolyhedronModel() {
  delete m_rapidModel;
  glDeleteLists(m_wiredID,1);
  glDeleteLists(m_solidID,1);
}

const string
PolyhedronModel::GetName() const { 
  string filename = m_bodyInfo.m_strModelDataFileName;
  size_t pos = filename.find('/');
  return pos == string::npos ? filename : filename.substr(pos+1);
}

vector<string> 
PolyhedronModel::GetInfo() const { 
  vector<string> info;
  info.push_back(m_bodyInfo.m_strModelDataFileName);
  return info;
}

bool
PolyhedronModel::BuildModels() {

  string file = m_bodyInfo.m_strModelDataFileName;
  IModel* imodel = CreateModelLoader(file, false);

  const PtVector& points=imodel->GetVertices();
  const TriVector& tris=imodel->GetTriP(); //GetTriangles();
 
  //compute center of mass and radius
  COM(points);
  Radius(points);

  //set point for opengl
  GLdouble* vertice=new GLdouble[3*points.size()];
  if(vertice==NULL)
    return false;

  typedef PtVector::const_iterator PIT;
  for(PIT pit = points.begin(); pit!=points.end(); ++pit)
    pit->get(vertice + (pit-points.begin())*3);

  vector<Vector3d> normals;
  ComputeNormals(points, tris, normals);

  BuildRapid(points, tris);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertice);

  BuildSolid(points, tris, normals);

  bool wiredSuccess = BuildWired(points, tris, normals);
  delete [] vertice;

  if(!wiredSuccess) {
    cerr << "Build Model (" << m_bodyInfo.m_strModelDataFileName << ") Error" << endl;
    return false;
  }

  //setup rotation and translation
  if(m_bodyInfo.m_bIsFixed) {
    tx()=m_bodyInfo.m_X;
    ty()=m_bodyInfo.m_Y;
    tz()=m_bodyInfo.m_Z;

    rx() = m_bodyInfo.m_Alpha;
    ry() = m_bodyInfo.m_Beta;
    rz() = m_bodyInfo.m_Gamma;
    
    double cx2 = cos(m_bodyInfo.m_Alpha/2);
    double sx2 = sin(m_bodyInfo.m_Alpha/2);
    double cy2 = cos(m_bodyInfo.m_Beta/2);
    double sy2 = sin(m_bodyInfo.m_Beta/2);
    double cz2 = cos(m_bodyInfo.m_Gamma/2);
    double sz2 = sin(m_bodyInfo.m_Gamma/2);

    Quaternion qx(cx2, Vector3d(sx2, 0, 0));
    Quaternion qy(cy2, Vector3d(0, sy2, 0));
    Quaternion qz(cz2, Vector3d(0, 0, sz2));
    Quaternion nq = qz * qy * qx;
    q(nq.normalize());
  }	

  return true;
}

void PolyhedronModel::Draw(GLenum _mode) {
  if(m_solidID == -1) return;
  if(m_RenderMode == plum::INVISIBLE_MODE) return;

  float* color = &m_RGBA[0]; 
  glColor4fv(color); 
  glPushMatrix();
  glTransform();
  glScale();
  
  if(m_RenderMode == plum::SOLID_MODE){           
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
    const Tri& tri= *trit;

    double p1[3], p2[3], p3[3];
    (_points[tri[0]] - m_com).get(p1);
    (_points[tri[1]] - m_com).get(p2);
    (_points[tri[2]] - m_com).get(p3);

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
  double value[3];
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = _tris.begin(); trit!=_tris.end(); ++trit){
    const Tri& tri = *trit;
    _norms[trit-_tris.begin()].get(value);
    glNormal3dv(value);
    GLint id[3] = {tri[0], tri[1], tri[2]};
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, id);
  }

  glPopMatrix();
  glEndList();
}

//build wire frame
bool
PolyhedronModel::BuildWired(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms) {   
  CModelGraph mg;
  if(!mg.doInit(_points, _tris))
    return false;

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

  return true;
}

//set m_com to center of mass of _points
void
PolyhedronModel::COM(const PtVector& _points) {
  m_com = Point3d(0, 0, 0);
  for(PtVector::const_iterator i = _points.begin(); i!=_points.end(); ++i) {
    const Point3d tmp = *i;
    m_com[0]+=tmp[0];
    m_com[1]+=tmp[1];
    m_com[2]+=tmp[2];
  }
  double size = _points.size();
  m_com[0]/=size;
  m_com[1]/=size;
  m_com[2]/=size;

  if(m_bodyInfo.m_IsSurface)
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
