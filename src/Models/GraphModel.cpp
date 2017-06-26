#include "GraphModel.h"
#include "Utilities/Font.h"


void
GraphModel::
Build() {
  // Start the call list.
  m_callList = glGenLists(1);
  if(m_callList == 0) {
    cerr << "Error:\n\tIn GraphModel::Build(), cannot draw the model "
         << "because we could not allocate a GL call list." << endl;
    return;
  }
  glNewList(m_callList, GL_COMPILE);

  glDisable(GL_LIGHTING);

  glPointSize(9);
  glLineWidth(3);
  glColor3f(0, 1, 0);

  DrawGraph();

  glEnable(GL_LIGHTING);

  glEndList();
}


void
GraphModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}



void
GraphModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}



void
GraphModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}



void
GraphModel::
DrawSelected() {
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


void
GraphModel::
Print(ostream& _os) const {
  _os << "Graph" << endl
      << "\tNum vertices: " << m_graph.get_num_vertices() << endl
      << "\tNum edges: " << m_graph.get_num_edges() << endl;
}

void GraphModel::DrawGraph()	{
	// Draw graph vertices
	glBegin(GL_POINTS);
  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)
    glVertex3dv(v->property().GetPoint());
  glEnd();

  // Draw  graph edges.
  for(auto e = m_graph.edges_begin(); e != m_graph.edges_end(); ++e) {
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property().GetIntermediates())
      glVertex3dv(v.GetPoint());
    glEnd();
  }

	//Draw vertices label
	 for(auto v = m_graph.begin(); v != m_graph.end(); ++v)  {
    		Point3d pos = v->property().GetPoint();
    		glColor3d(0.1, 0.1, 0.1);
    		DrawStr(pos[0]-0.75, pos[1]-0.75, pos[2], to_string(v->descriptor()));
  	}

}


template <>
void
GraphModel::
BuildGraph<ReebGraphConstruction::ReebGraph>(const ReebGraphConstruction::ReebGraph& _g) {
	// Add graph vertices
	for(auto v = _g.begin(); v != _g.end(); ++v) {
		auto vd = (*v).descriptor();
		m_graph.add_vertex(vd, CfgModel((*v).property().m_vertex));
		m_graph.find_vertex(vd)->property().SetIndex(vd);
	}

	// Add graph edges 
	for(auto e = _g.edges_begin(); e != _g.edges_end(); ++e) {
		vector<CfgModel> intermediates;
		for(auto& v : e->property().m_path)
			intermediates.emplace_back(CfgModel(v));
		EdgeModel edge("",1, intermediates);
		auto src = m_graph.find_vertex(e->source());
		auto trgt = m_graph.find_vertex(e->target());
		edge.Set(&((*src).property()), &((*trgt).property()));
		m_graph.add_edge(e->source(), e->target(), edge);
	}
}


template <>
void
GraphModel::
BuildGraph<ReebGraphConstruction::FlowGraph>(const ReebGraphConstruction::FlowGraph& _g) {
	
	// Add graph vertices
	for(auto v = _g.begin(); v != _g.end(); ++v)	{
		auto vd = (*v).descriptor();
		m_graph.add_vertex(vd, CfgModel((*v).property()));
		m_graph.find_vertex(vd)->property().SetIndex(vd);

	}

	// Add graph edges 
	for(auto e = _g.edges_begin(); e != _g.edges_end(); ++e) {
		vector<CfgModel> intermediates;
		for(auto& v : e->property())
			intermediates.emplace_back(CfgModel(v));
		EdgeModel edge("",1, intermediates);
		auto src = m_graph.find_vertex(e->source());
		auto trgt = m_graph.find_vertex(e->target());
		edge.Set(&((*src).property()), &((*trgt).property()));
		m_graph.add_edge(e->source(), e->target(), edge);
	}

}

//revision
//This funtion will write to a file formating it in the way that info is
//extracted in our Add Skeleton function so that the graph can be reopened
void
GraphModel::
SaveSkeleton(ostream& _os) const {
  _os << m_graph.get_num_vertices() << " "
      << m_graph.get_num_edges() << endl;
  for(auto v = m_graph.begin(); v != m_graph.end(); ++v)
    _os << (*v).descriptor() << " " << (*v).property().GetPoint() << endl;
  for(auto e = m_graph.edges_begin(); e != m_graph.edges_end(); ++e)  {
    auto intermediates =  e->property().GetIntermediates();
    _os << e->source() << " " << e->target() << " "
        << intermediates.size() << " ";
    for(auto v : intermediates)
      _os << v.GetPoint();
    _os << endl;
  }
}

/*
template<>
void
GraphModel<ReebGraphConstruction::FlowGraph>::
HighlightVertices() const  {
}*/
