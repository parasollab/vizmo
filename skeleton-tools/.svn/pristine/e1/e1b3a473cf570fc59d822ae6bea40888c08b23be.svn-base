#include "WorkspaceDecompositionModel.h"

#include "Workspace/WorkspaceDecomposition.h"

/*------------------------------- Construction -------------------------------*/

WorkspaceDecompositionModel::
WorkspaceDecompositionModel(const WorkspaceDecomposition* _wd) :
    Model("Workspace Decomposition"), m_decomposition(_wd) {
  SetRenderMode(INVISIBLE_MODE);
}


WorkspaceDecompositionModel::
~WorkspaceDecompositionModel() {
  glDeleteLists(m_regionList, 1);
  glDeleteLists(m_dualGraphList, 1);
}

/*------------------------------- Construction -------------------------------*/

void
WorkspaceDecompositionModel::
Build() {
  BuildRegions();
  BuildDualGraph();
}


void
WorkspaceDecompositionModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}


void
WorkspaceDecompositionModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_regionList == 0)
    Build();
  glCallList(m_regionList);
  glCallList(m_dualGraphList);
}


void
WorkspaceDecompositionModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_regionList == 0)
    Build();
  glCallList(m_dualGraphList);
}


void
WorkspaceDecompositionModel::
DrawSelected() {
  if(m_regionList == 0)
    Build();
  glCallList(m_regionList);
  glCallList(m_dualGraphList);
}


void
WorkspaceDecompositionModel::
Print(ostream& _os) const {
  _os << "Workspace Decomposition" << endl
      << "\tNum points: " << m_decomposition->GetNumPoints() << endl
      << "\tNum regions: " << m_decomposition->GetNumRegions() << endl;
}

/*--------------------------------- Helpers ----------------------------------*/

void
WorkspaceDecompositionModel::
BuildRegions() {
  m_regionList = glGenLists(1);
  glNewList(m_regionList, GL_COMPILE);

  glDisable(GL_LIGHTING);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glColor4f(0, 1, 1, .04);

  // Draw faces of the tetrahedra.
  for(auto iter = m_decomposition->begin(); iter != m_decomposition->end();
      ++iter) {
    for(const auto& facet : iter->property().GetFacets()) {
      glBegin(GL_POLYGON);
      for(size_t i = 0; i < facet.GetNumVertices(); ++i)
        glVertex3dv(facet.GetPoint(i));
      glEnd();
    }
  }

  // Draw edges to make them more visible.
  for(auto iter = m_decomposition->begin(); iter != m_decomposition->end();
      ++iter) {
    for(const auto& facet : iter->property().GetFacets()) {
      glBegin(GL_LINE_LOOP);
      for(size_t i = 0; i < facet.GetNumVertices(); ++i)
        glVertex3dv(facet.GetPoint(i));
      glEnd();
    }
  }

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);

  glEndList();
}


void
WorkspaceDecompositionModel::
BuildDualGraph() {
  const WorkspaceDecomposition::DualGraph& dualGraph = m_decomposition->
      GetDualGraph();

  m_dualGraphList = glGenLists(1);
  glNewList(m_dualGraphList, GL_COMPILE);

  glDisable(GL_LIGHTING);
  glColor4f(1, 0, 1, 1);

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

  glEnable(GL_LIGHTING);

  glEndList();
}

/*----------------------------------------------------------------------------*/
