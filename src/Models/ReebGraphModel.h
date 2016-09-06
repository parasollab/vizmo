#ifndef REEB_GRAPH_MODEL_H_
#define REEB_GRAPH_MODEL_H_

#include "Model.h"

class ReebGraphConstruction;

////////////////////////////////////////////////////////////////////////////////
/// @brief Display model of embedded Reeb Graph
////////////////////////////////////////////////////////////////////////////////
class ReebGraphModel : public Model {

  public:

    ///@name Construction
    ///@{

    ////////////////////////////////////////////////////////////////////////////
    /// @param _rg The construction object for the Reeb graph to model.
    /// @TODO Make _rg a const when STAPL fixes sequential graph.
    ReebGraphModel(ReebGraphConstruction* _rg);

    ~ReebGraphModel();

    ///@}
    ///name Model Functions
    ///@{

    virtual void Build() override;
    virtual void Select(GLuint* _index, vector<Model*>& _sel) override;
    virtual void DrawRender() override;
    virtual void DrawSelect() override;
    virtual void DrawSelected() override;
    virtual void Print(ostream& _os) const override;

    ///@}

  private:

    ///@}
    ///@name Internal State
    ///@{
    ///@TODO Make m_construction const when STAPL fixes sequential graph.

    ReebGraphConstruction* const m_construction; ///< Reeb graph builder.
    GLuint m_callList{0};    ///< Compiled GL call list for Reeb graph.
    size_t m_numVertices{0}; ///< The number of vertices in the Reeb graph.
    size_t m_numEdges{0};    ///< The number of edges in the Reeb graph.

    ///@}
};

#endif
