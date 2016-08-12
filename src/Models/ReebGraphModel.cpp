#include "ReebGraphModel.h"
#include "Models/Vizmo.h"
#include "Utilities/ReebGraphConstruction.h"

ReebGraphModel::
ReebGraphModel(ReebGraphConstruction* _reebGraph) :
  Model("Reeb Graph"), m_reebGraph(_reebGraph), m_reebGraphID(0) {
    SetRenderMode(INVISIBLE_MODE);
  }

ReebGraphModel::
~ReebGraphModel() {
  glDeleteLists(m_reebGraphID, 1);
}

void
ReebGraphModel::
Build() {
  m_reebGraphID = glGenLists(1);
  glNewList(m_reebGraphID, GL_COMPILE);

  ReebGraphConstruction::ReebGraph& reebGraph = m_reebGraph->GetReebGraph();

  glDisable(GL_LIGHTING);

  glPointSize(6);
  glLineWidth(3);

  glBegin(GL_POINTS);
  for(auto v = reebGraph.begin(); v != reebGraph.end(); ++v) {
    CfgModel* cfg = new CfgModel(v->property().m_vertex);
    if(GetVizmo().IsInsideCheck(*cfg)) {
      m_temp.AddModel(cfg, Color4(1,0, 0, 0));
      glColor3f(1., 0., 0.);
    }
    else {
      delete cfg; 
      glColor3f(0., 1., 0.);
    }
    glVertex3dv(v->property().m_vertex);
  }
  glEnd();

  glPointSize(4);

  glColor3f(1.,1.,0.);

  for(auto e = reebGraph.edges_begin(); e != reebGraph.edges_end(); ++e) {
    glBegin(GL_LINE_STRIP);
    for(auto& v : e->property().m_path) {
      CfgModel* cfg = new CfgModel(v);
      if(GetVizmo().IsInsideCheck(*cfg)) {
        m_temp.AddModel(cfg, Color4(1,0, 0, 0));
        glColor3f(1., 0., 0.);
      }
      else {
        glColor3f(0., 1., 0.);
        delete cfg;
      }
      glVertex3dv(v);
    } 
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

  if(m_reebGraphID == 0)
    Build();

  glColor4f(0.0, 0.5, 0.2, 0.05);
  glCallList(m_reebGraphID);
}

void
ReebGraphModel::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;

  if(m_reebGraphID == 0)
    Build();

  glCallList(m_reebGraphID);
}

void
ReebGraphModel::
DrawSelected() {
  if(m_reebGraphID == 0)
    Build();

  glCallList(m_reebGraphID);
}

void
ReebGraphModel::
Print(ostream& _os) const {
  _os << "Reeb Graph" << endl
    << "Num vertices: " << m_reebGraph->GetReebGraph().get_num_vertices() << endl
    << "Num edges: " << m_reebGraph->GetReebGraph().get_num_edges() << endl;
}

