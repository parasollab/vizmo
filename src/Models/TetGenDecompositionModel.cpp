#include "TetGenDecompositionModel.h"

#include "Utilities/TetGenDecomposition.h"

TetGenDecompositionModel::
TetGenDecompositionModel(TetGenDecomposition* _tetgen) :
  Model("TetGen Decomposition"),
  m_tetgen(_tetgen),
  m_tetrahedronID(0), m_dualGraphID(0) {
    SetRenderMode(INVISIBLE_MODE);
  }

TetGenDecompositionModel::
~TetGenDecompositionModel() {
  glDeleteLists(m_tetrahedronID, 1);
  glDeleteLists(m_dualGraphID, 1);
}

void
TetGenDecompositionModel::
Build() {
  BuildTetrahedrons();
  BuildDualGraph();
}

void
TetGenDecompositionModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}

void
TetGenDecompositionModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  if(m_tetrahedronID == 0)
    Build();

  glDisable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);

  glColor4f(0.0, 1.0, 1.0, 0.01);
  glCallList(m_tetrahedronID);

  glColor4f(1.0, 0.0, 1.0, 1);
  glCallList(m_dualGraphID);

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
}

void
TetGenDecompositionModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  if(m_tetrahedronID == 0)
    Build();

  glCallList(m_dualGraphID);
}

void
TetGenDecompositionModel::
DrawSelected() {
  if(m_tetrahedronID == 0)
    Build();

  glDisable(GL_LIGHTING);

  glCallList(m_dualGraphID);

  glEnable(GL_LIGHTING);
}

void
TetGenDecompositionModel::
Print(ostream& _os) const {
  _os << "TetGen Decomposition" << endl
    << "Num points: " << m_tetgen->GetNumPoints() << endl
    << "Num tetrahedrons: " << m_tetgen->GetNumTetras() << endl;
}

void
TetGenDecompositionModel::
BuildTetrahedrons() {
  m_tetrahedronID = glGenLists(1);
  glNewList(m_tetrahedronID, GL_COMPILE);

  size_t numCorners = m_tetgen->GetNumCorners();
  size_t numTetras = m_tetgen->GetNumTetras();
  const double* const points = m_tetgen->GetPoints();
  const int* const tetra = m_tetgen->GetTetras();

  glBegin(GL_TRIANGLES);
  for(size_t i = 0; i < numTetras; ++i) {
    Vector3d vs[numCorners];
    for(size_t j = 0; j < numCorners; ++j)
      vs[j] = Vector3d(&points[3*tetra[i*numCorners + j]]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[1]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[3]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[1]);
    glVertex3dv(vs[3]);
    glVertex3dv(vs[1]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[3]);
  }
  glEnd();

  glBegin(GL_LINES);
  for(size_t i = 0; i < numTetras; ++i) {
    Vector3d vs[numCorners];
    for(size_t j = 0; j < numCorners; ++j)
      vs[j] = Vector3d(&points[3*tetra[i*numCorners + j]]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[1]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[0]);
    glVertex3dv(vs[3]);
    glVertex3dv(vs[1]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[2]);
    glVertex3dv(vs[3]);
    glVertex3dv(vs[3]);
    glVertex3dv(vs[1]);
  }
  glEnd();

  glEndList();
}

void
TetGenDecompositionModel::
BuildDualGraph() {
  m_dualGraphID = glGenLists(1);
  glNewList(m_dualGraphID, GL_COMPILE);

  TetGenDecomposition::DualGraph& dualGraph = m_tetgen->GetDualGraph();

  glBegin(GL_POINTS);
  for(auto v = dualGraph.begin(); v != dualGraph.end(); ++v)
    glVertex3dv(v->property());
  glEnd();

  glBegin(GL_LINES);
  for(auto e = dualGraph.edges_begin(); e != dualGraph.edges_end(); ++e) {
    glVertex3dv(dualGraph.find_vertex(e->source())->property());
    glVertex3dv(dualGraph.find_vertex(e->target())->property());
  }
  glEnd();

  glEndList();
}

