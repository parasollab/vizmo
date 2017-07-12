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

		GraphModel() : Model("Graph")	{}

    ////////////////////////////////////////////////////////////////////////////
    /// @param _g The graph to model.
		template <typename GraphType>
    GraphModel(const GraphType& _g) : Model("Graph") {
			if(m_graph)
				delete m_graph;
			m_graph = new SkeletonGraphType();
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
		virtual void SetColor(const Color4& _c) override;
		///@}
  	///name Accessor Functions
    ///@{
    virtual void SaveSkeleton(ostream& _os) const;	///< Save the skleton in a file
    virtual SkeletonGraphType* GetGraph() { return m_graph; }	///< Get the underlying graph
    void AddEdge(size_t _v1, size_t _v2);		///< Add an edge
    void AddVertex(Point3d _p);						///< Add a vertex
		void Refresh();						///< Refresh the skeleton

  private:
		///@}
		///name Helper Functions
		///@{

		template <typename GraphType>
    void BuildGraph(const GraphType& _g);			///< Build the graph model from a given graph
		void DrawGraph();													///< Underlying draw function
		void SetIndices();												///< Set the other parameters of the vertices and edges
	
    ///@}
    ///@name Internal State
    ///@{

    SkeletonGraphType* m_graph{nullptr};      ///< Graph to model.
    GLuint m_callList{0};    ///< Compiled GL call list for  graph.
		bool m_selected{false};

    ///@}
};

#endif
