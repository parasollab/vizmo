#include "GraphModel.h"
#include "Utilities/Font.h"

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

  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)  {
    Point3d pos = v->property();
    glColor3d(0.1, 0.1, 0.1);
    DrawStr(pos[0]-0.75, pos[1]-0.75, pos[2], to_string(v->descriptor()));
  }
  glColor3d(0, 1, 0);
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
  //revisions
  //highlight vertices
  //double n = m_graph.get_num_vertices()-1;
 /* for(auto v = m_graph.begin(); v != m_graph.end(); ++v)  {
    v.SetRenderMode(WIRE_MODE);
    v.SetColor(Color4(1.0 - i/n, 0, i/n, 1));
    v.DrawRender();
    Point3d pos = v->property();
    glColor3d(0.1, 0.1, 0.1);
    DrawStr(pos[0]-0.5, pos[1]-0.5, pos[2], to_string(v->descriptor()));
  }*/

}

//revision
//This funtion will write to a file formating it in the way that info is
//extracted in our Add Skeleton function so that the graph can be reopened
template <>
void
GraphModel<ReebGraphConstruction::FlowGraph>::
Print(ostream& _os) const {
  _os << m_graph.get_num_vertices() << " "
      << m_graph.get_num_edges() << endl;
  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)
    _os << (*v).descriptor() << " " << (*v).property() << endl;
  for(auto e = m_graph.edges_begin(); e != m_graph.edges_end(); ++e)  {
    _os << e->source() << " " << e->target() << " "
        << e->property().size() << " ";
    for(auto v : e->property())
      _os << v;
    _os << endl;
  }
}

/*
template<>
void
GraphModel<ReebGraphConstruction::FlowGraph>::
HighlightVertices() const  {
}*/
