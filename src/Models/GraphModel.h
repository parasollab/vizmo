#ifndef GRAPH_MODEL_H_
#define GRAPH_MODEL_H_

#include "Model.h"
#include "Models/Vizmo.h"

#include "Utilities/ReebGraphConstruction.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief Display model of embedded graph.
////////////////////////////////////////////////////////////////////////////////
template <typename GraphType>
class GraphModel : public Model {

  public:

    ///@name Construction
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// @param _g The graph to model.
    GraphModel(const GraphType& _g) : Model("Graph"), m_graph(_g) {
      SetRenderMode(INVISIBLE_MODE);
    }

    ~GraphModel() {
      glDeleteLists(m_callList, 1);
    }

    ///@}
    ///name Model Functions
    ///@{

    virtual void Build() override;
    virtual void Select(GLuint* _index, vector<Model*>& _sel) override;
    virtual void DrawRender() override;
    virtual void DrawSelect() override;
    virtual void DrawSelected() override;
    virtual void Print(ostream& _os) const override;
    //revision
    //virtual void SaveFile(ostream& _os) const;
    //virtual void SaveSkeleton(ostream& _os) const override;
    virtual GraphType GetGraph();
    //virtual void HighlightVertices() const;
    ///@}

  private:

    void BuildGraph();

    ///@}
    ///@name Internal State
    ///@{

    GraphType m_graph;      ///< Graph to model.
    GLuint m_callList{0};    ///< Compiled GL call list for  graph.

    ///@}
};

/*----------------------------------------------------------------------------*/

//revisions
template<typename GraphType>
GraphType
GraphModel<GraphType>::
GetGraph()  {
  return m_graph;
}

template <typename GraphType>
void
GraphModel<GraphType>::
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

  BuildGraph();

  glEnable(GL_LIGHTING);

  glEndList();
}


template <typename GraphType>
void
GraphModel<GraphType>::
Select(GLuint* _index, vector<Model*>& _sel) {
  if(m_selectable && _index)
    _sel.push_back(this);
}


template <typename GraphType>
void
GraphModel<GraphType>::
DrawRender() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


template <typename GraphType>
void
GraphModel<GraphType>::
DrawSelect() {
  if(m_renderMode == INVISIBLE_MODE)
    return;
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


template <typename GraphType>
void
GraphModel<GraphType>::
DrawSelected() {
  if(m_callList == 0)
    Build();
  glCallList(m_callList);
}


template <typename GraphType>
void
GraphModel<GraphType>::
Print(ostream& _os) const {
  _os << "Graph" << endl
      << "\tNum vertices: " << m_graph.get_num_vertices() << endl
      << "\tNum edges: " << m_graph.get_num_edges() << endl;
}

//revision

/*
template <typename GraphType>
void
GraphModel<GraphType>::
SaveSkeleton(ostream& _os) const {
  _os << "Graph" << endl
      << "\tNum vertices: " << m_graph.get_num_vertices() << endl
      << "\tNum edges: " << m_graph.get_num_edges() << endl;
}*/
/*----------------------------------------------------------------------------*/

#endif
