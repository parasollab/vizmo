#include "GraphModel.h"
#include "Utilities/Font.h"


/*********************** Model Functions *************************************/

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
}


void
GraphModel::
Select(GLuint* _index, vector<Model*>& _sel) {
  /*if(m_selectable && _index)
    _sel.push_back(this);*/
	if(!m_selectable || _index == NULL)
    return;

  if(_index[0] == 1)
    _sel.push_back(&(m_graph->find_vertex(_index[1])->property()));
  else {
    SkeletonGraphType::vertex_iterator vi;
    SkeletonGraphType:: adj_edge_iterator ei;
		m_graph->find_edge(SkeletonGraphType::edge_descriptor(_index[1], _index[2], _index[3]), vi, ei);
		_sel.push_back(&(*ei).property());
  }
}



void
GraphModel::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
	m_selected = false;
	DrawGraph();
  glCallList(m_callList);
}



void
GraphModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
	m_selected = true;
	DrawGraph();
  glCallList(m_callList);
}



void
GraphModel::
DrawSelected() {
}


void
GraphModel::
Print(ostream& _os) const {
  _os << "Graph" << endl
      << "\tNum vertices: " << m_graph->get_num_vertices() << endl
      << "\tNum edges: " << m_graph->get_num_edges() << endl;
}

void GraphModel::DrawGraph()	{
	glDisable(GL_LIGHTING);

	glPointSize(9);
  glLineWidth(3);
  //glColor3f(0, 1, 0);

	// Draw graph vertices
	if(m_selected)
		glPushName(1);
	for(auto v = m_graph->begin(); v != m_graph->end(); ++v)	{
		if(m_selected)
			glPushName((*v).descriptor());
		glColor4fv(v->property().GetColor());
		glBegin(GL_POINTS);
    glVertex3dv(v->property().GetPoint());
		glEnd();
		if(m_selected)
			glPopName();
	}
  if(m_selected)
		glPopName();
  /*
	//Draw vertices label
	for(auto v = m_graph->begin(); v != m_graph->end(); ++v)  {
  	Point3d pos = v->property().GetPoint();
  	glColor3d(0.1, 0.1, 0.1);
  	DrawStr(pos[0]-0.75, pos[1]-0.75, pos[2], to_string(v->descriptor()));
  }*/

  // Draw  graph edges.
	//glColor3f(0, 1, 0);
	if(m_selected)
		glPushName(3);
  for(auto e = m_graph->edges_begin(); e != m_graph->edges_end(); ++e) {
		if(m_selected)	{
			glPushName(e->source());
			glPushName(e->target());
			glPushName(e->id());
		}
		glColor4fv(e->property().GetColor());
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property().GetIntermediates())
      glVertex3dv(v.GetPoint());
    glEnd();
		if(m_selected)	{
			glPopName();
			glPopName();
			glPopName();
		}
  }
	if(m_selected)
		glPopName();


	glEnable(GL_LIGHTING);

  glEndList();
}

void
GraphModel::
GetChildren(list<Model*>& _models) {
	for(auto v = m_graph->begin(); v != m_graph->end(); ++v)
    _models.push_back(&(*v).property());
  for(auto e = m_graph->edges_begin(); e != m_graph->edges_end(); ++e)
		_models.push_back(&(*e).property());
}

void
GraphModel::
SetColor(const Color4& _c)	{
	for(auto v = m_graph->begin(); v != m_graph->end(); ++v)
    v->property().SetColor(_c);
  for(auto e = m_graph->edges_begin(); e != m_graph->edges_end(); ++e)
		e->property().SetColor(_c);
}

/*********************** Accessor Functions *************************************/
void
GraphModel::
AddEdge(size_t _v1, size_t _v2) {
	vector<CfgModel> intermediates;
	intermediates.push_back((*m_graph->find_vertex(_v1)).property());
	intermediates.push_back((*m_graph->find_vertex(_v2)).property());

	m_graph->add_edge(_v1, _v2, EdgeModel("",1, intermediates));
}


void
GraphModel::
AddVertex(Point3d _p) {
	auto vd = m_graph->add_vertex(CfgModel(_p));
	m_graph->find_vertex(vd)->property().SetColor(Color4(0,1,0));
}

// This funtion will write to a file formating it in the way that info is
// extracted in our Add Skeleton function so that the graph can be reopened
void
GraphModel::
SaveSkeleton(ostream& _os) const {
  _os << m_graph->get_num_vertices() << " "
      << m_graph->get_num_edges() << endl;
  for(auto v = m_graph->begin(); v != m_graph->end(); ++v)
    _os << (*v).descriptor() << " " << (*v).property().GetPoint() << endl;
  for(auto e = m_graph->edges_begin(); e != m_graph->edges_end(); ++e)  {
    auto intermediates =  e->property().GetIntermediates();
    _os << e->source() << " " << e->target() << " "
        << intermediates.size() << " ";
    for(auto v : intermediates)
      _os << v.GetPoint();
    _os << endl;
  }
}

void
GraphModel::
Refresh()	{
	Build();
	SetIndices();
	DrawGraph();
}

/*********************** Helper Functions *************************************/


template <>
void
GraphModel::
BuildGraph<ReebGraphConstruction::ReebGraph>(const ReebGraphConstruction::ReebGraph& _g) {
	// Add graph vertices
	for(auto v = _g.begin(); v != _g.end(); ++v)
		m_graph->add_vertex((*v).descriptor(), CfgModel((*v).property().m_vertex));

	// Add graph edges
	for(auto e = _g.edges_begin(); e != _g.edges_end(); ++e) {
		vector<CfgModel> intermediates;
		for(auto& v : e->property().m_path)
			intermediates.emplace_back(CfgModel(v));
		m_graph->add_edge(e->source(), e->target(),  EdgeModel("",1, intermediates));
	}
	SetIndices();
	SetColor(Color4(0,1,0));
}


template <>
void
GraphModel::
BuildGraph<ReebGraphConstruction::FlowGraph>(const ReebGraphConstruction::FlowGraph& _g) {
	// Add graph vertices
	for(auto v = _g.begin(); v != _g.end(); ++v)
		m_graph->add_vertex((*v).descriptor(), CfgModel((*v).property()));

	// Add graph edges
	for(auto e = _g.edges_begin(); e != _g.edges_end(); ++e) {
		vector<CfgModel> intermediates;
		for(auto& v : e->property())
			intermediates.emplace_back(CfgModel(v));
		m_graph->add_edge(e->source(), e->target(), EdgeModel("",1, intermediates));
	}
	SetIndices();
	SetColor(Color4(0,1,0));
}

void
GraphModel::
SetIndices()	{
	// Set names for vertices
	for(auto v = m_graph->begin(); v != m_graph->end(); ++v)
		v->property().SetIndex(v->descriptor());

	// Set edges
	for(auto e = m_graph->edges_begin(); e != m_graph->edges_end(); ++e)	{
		auto src = m_graph->find_vertex(e->source());
		auto trgt = m_graph->find_vertex(e->target());
		e->property().Set(e->descriptor().id(), &((*src).property()), &((*trgt).property()));
	}
}

