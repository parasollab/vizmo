#include "PolyhedronModel.h"

#include <cmath>
#include <algorithm>
#include <numeric>

#include "ModelFactory.h"

#include "BodyModel.h"
#include "Utilities/VizmoExceptions.h"

PolyhedronModel::PolyhedronModel(BodyModel* _bodyModel)
  : Model(_bodyModel->GetFilename()), m_solidID(-1), m_wiredID(-1), m_bodyModel(_bodyModel) {
    BuildModels();
    SetColor(m_bodyModel->GetColor());
  }

PolyhedronModel::~PolyhedronModel(){
  glDeleteLists(m_wiredID,1);
  glDeleteLists(m_solidID,1);
}

void
PolyhedronModel::BuildModels() {

  string file = m_bodyModel->GetModelFilename();
  IModel* imodel = CreateModelLoader(file, false);

  if(!imodel)
    throw BuildException(WHERE, "File '" + file + "' does not exist.");

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

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, vertice);

  BuildSolid(points, tris, normals);
  BuildWired(points, tris, normals);

  //setup rotation and translation
  if(m_bodyModel->IsFixed()) {
    const Transformation& t = m_bodyModel->GetTransform();
    const Vector3d& p = t.translation();
    const Orientation& o = t.rotation();
    EulerAngle e;
    convertFromMatrix(e, o.matrix());
    Quaternion qua;
    convertFromMatrix(qua, o.matrix());

    m_bodyModel->Translation() = p;

    m_bodyModel->Rotation()[0] = e.alpha();
    m_bodyModel->Rotation()[1] = e.beta();
    m_bodyModel->Rotation()[2] = e.gamma();

    m_bodyModel->RotationQ() = qua;
  }
  delete imodel;
}

void PolyhedronModel::Draw() {
  if(m_solidID == GLuint(-1) || m_renderMode == INVISIBLE_MODE)
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

void PolyhedronModel::DrawSelect() {
  glCallList(m_wiredID);
}

void
PolyhedronModel::Print(ostream& _os) const {
  _os << m_bodyModel->GetModelFilename() << endl;
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
PolyhedronModel::BuildSolid(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms) {
  m_solidID = glGenLists(1);
  glNewList(m_solidID, GL_COMPILE);

  glEnable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(-m_com[0], -m_com[1], -m_com[2]);

  //draw
  glBegin(GL_TRIANGLES);
  typedef TriVector::const_iterator TRIT;
  for(TRIT trit = _tris.begin(); trit!=_tris.end(); ++trit){
    const Tri& tri = *trit;
    glNormal3dv(_norms[trit-_tris.begin()]);
    for(int i=0; i<3; i++)
      glVertex3dv(_points[tri[i]]);
    //GLint id[3] = {tri[0], tri[1], tri[2]};
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, id);
  }
  glEnd();

  glPopMatrix();
  glEndList();
}

//build wire frame
void
PolyhedronModel::BuildWired(const PtVector& _points, const TriVector& _tris, const vector<Vector3d>& _norms) {

  //create model graph
  BuildModelGraph(_points, _tris);

  //build model
  m_wiredID=glGenLists(1);
  glNewList(m_wiredID,GL_COMPILE);

  glDisable(GL_LIGHTING);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslated(-m_com[0], -m_com[1], -m_com[2]);

  typedef ModelGraph::edge_iterator EIT;
  for(EIT eit = m_modelGraph.edges_begin(); eit != m_modelGraph.edges_end(); ++eit) {
    int tril = (*eit).property()[0];
    int trir = (*eit).property()[1];

    if(tril == -1 || trir == -1 || 1-fabs(_norms[tril] * _norms[trir]) > 1e-3) {
      GLint id[2] = {(*eit).source(), (*eit).target()};
      glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, id);
    }
  }

  glPopMatrix();
  glEndList();
}

//compute the model graph for wire frame
void
PolyhedronModel::BuildModelGraph(const PtVector& _points, const TriVector& _tris) {
  //create nodes
  for(size_t i = 0; i < _points.size(); ++i)
    m_modelGraph.add_vertex(i, i);

  //create edge from triangles
  for(size_t i = 0; i < _tris.size(); ++i) {
    for(size_t j = 0; j < 3; ++j) {
      int a = _tris[i][j];
      int b = _tris[i][(j+1)%3];
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

//set m_com to center of mass of _points
void
PolyhedronModel::COM(const PtVector& _points) {
  m_com = accumulate(_points.begin(), _points.end(), Point3d(0, 0, 0));
  m_com /= _points.size();
  if(m_bodyModel->IsSurface())
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

