#ifndef REEB_GRAPH_MODEL_H_
#define REEB_GRAPH_MODEL_H_

#include "Model.h"

class ReebGraphConstruction;

////////////////////////////////////////////////////////////////////////////////
/// @brief Display model of embedded Reeb Graph
////////////////////////////////////////////////////////////////////////////////
class ReebGraphModel : public Model {

  public:

    ////////////////////////////////////////////////////////////////////////////
    /// @param _reebGraph Reeb Graph. Must be valid pointer to already
    ///                   constructed and embedded reeb graph.
    ReebGraphModel(ReebGraphConstruction* _reebGraph);
    ~ReebGraphModel();

    // Model functions
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

  private:

    ReebGraphConstruction* m_reebGraph; ///< Embedded reeb graph

    GLuint m_reebGraphID; ///< Compiled GL call list for reeb graph
};

#endif
