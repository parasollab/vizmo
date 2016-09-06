#include "ReebGraphModel.h"
#include "Models/Vizmo.h"
#include "Utilities/ReebGraphConstruction.h"

/*------------------------------- Construction -------------------------------*/

ReebGraphModel::
ReebGraphModel(ReebGraphConstruction* _rg) : Model("Reeb Graph"),
    m_construction(_rg) {
  SetRenderMode(INVISIBLE_MODE);
}


ReebGraphModel::
~ReebGraphModel() {
  glDeleteLists(m_callList, 1);
}

/*----------------------------------------------------------------------------*/

void
ReebGraphModel::
Build() {
  ReebGraphConstruction::ReebGraph& reebGraph = m_construction->GetReebGraph();
  m_numVertices = reebGraph.get_num_vertices();
  m_numEdges = reebGraph.get_num_edges();

  // Start the call list.
  m_callList = glGenLists(1);
  if(m_callList == 0) {
    cerr << "Error:\n\tIn ReebGraphModel::Build(), cannot draw the model "
         << "because we could not allocate a GL call list." << endl;
    return;
  }
  glNewList(m_callList, GL_COMPILE);

  glDisable(GL_LIGHTING);

  glPointSize(9);
  glLineWidth(3);
  glColor3f(0, 1, 0);

  // Add Reeb graph vertices.
  glBegin(GL_POINTS);
  for(auto v = reebGraph.begin(); v != reebGraph.end(); ++v)
    glVertex3dv(v->property().m_vertex);
  glEnd();

  // Add Reeb graph edges.
  for(auto e = reebGraph.edges_begin(); e != reebGraph.edges_end(); ++e) {
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property().m_path)
      glVertex3dv(v);
    glEnd();
  }

  glEnable(GL_LIGHTING);

  glEndList();
}


void
ReebGraphModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}


void
ReebGraphModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


void
ReebGraphModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


void
ReebGraphModel::
DrawSelected() {
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


void
ReebGraphModel::
Print(ostream& _os) const {
  _os << "Reeb Graph" << endl
      << "\tNum vertices: " << m_numVertices << endl
      << "\tNum edges: " << m_numEdges << endl;
}

/*----------------------------------------------------------------------------*/
