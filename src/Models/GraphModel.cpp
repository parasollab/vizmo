#include "GraphModel.h"

template <>
void
GraphModel<ReebGraphConstruction::ReebGraph>::
BuildGraph() {
  // Add  graph vertices.
  glBegin(GL_POINTS);
  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)
    glVertex3dv(v->property().m_vertex);
  glEnd();

  // Add  graph edges.
  for(auto e = m_graph.edges_begin(); e != m_graph.edges_end(); ++e) {
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property().m_path)
      glVertex3dv(v);
    glEnd();
  }
}


template <>
void
GraphModel<ReebGraphConstruction::FlowGraph>::
BuildGraph() {
  // Add  graph vertices.
  glBegin(GL_POINTS);
  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)
    glVertex3dv(v->property());
  glEnd();

  // Add  graph edges.
  for(auto e = m_graph.edges_begin(); e != m_graph.edges_end(); ++e) {
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property())
      glVertex3dv(v);
    glEnd();
  }
}
