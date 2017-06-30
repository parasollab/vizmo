#ifndef GRAPH_MODEL_H_
#define GRAPH_MODEL_H_

#include "Model.h"
#include "Models/Vizmo.h"

#include "Utilities/ReebGraphConstruction.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief Display model of embedded graph.
////////////////////////////////////////////////////////////////////////////////
class GraphModel : public Model {

  public:
		///@name Local Types
		///@{
			typedef stapl::sequential::directed_preds_graph<stapl::MULTIEDGES, CfgModel, EdgeModel> SkeletonGraphType;

		///@}
    ///@name Construction
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// @param _g The graph to model.
		template <typename GraphType>
    GraphModel(const GraphType& _g) : Model("Graph") {
			BuildGraph(_g);
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
		virtual void GetChildren(list<Model*>& _models) override;

    //revision
    virtual void SaveSkeleton(ostream& _os) const;
    virtual SkeletonGraphType GetGraph() { return m_graph; }
    //virtual void HighlightVertices() const;
    ///@}

    void AddVertex(Point3d _p );
  private:
		template <typename GraphType>
    void BuildGraph(const GraphType& _g);
		void DrawGraph(bool _selected = false);

    ///@}
    ///@name Internal State
    ///@{

    SkeletonGraphType m_graph;      ///< Graph to model.
    GLuint m_callList{0};    ///< Compiled GL call list for  graph.

    ///@}
};

/*----------------------------------------------------------------------------*/


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
