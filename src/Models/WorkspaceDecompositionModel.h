#ifndef WORKSPACE_DECOMPOSITION_MODEL_H
#define WORKSPACE_DECOMPOSITION_MODEL_H

#include "Model.h"

class WorkspaceDecomposition;

////////////////////////////////////////////////////////////////////////////////
/// @brief Drawable model of a WorkspaceDecomposition.
////////////////////////////////////////////////////////////////////////////////
class WorkspaceDecompositionModel : public Model {

  public:

    ///@name Construction
    ///@{

    WorkspaceDecompositionModel(const WorkspaceDecomposition* _tetgen);
    ~WorkspaceDecompositionModel();

    ///@}
    ///@name Model Functions
    ///@{

    virtual void Build() override;
    virtual void Select(GLuint* _index, vector<Model*>& _sel) override;
    virtual void DrawRender() override;
    virtual void DrawSelect() override;
    virtual void DrawSelected() override;
    virtual void Print(ostream& _os) const override;

    ///@}

  private:

    ///@name Helpers
    ///@{

    void BuildRegions();   ///< Build GL call list for regions.
    void BuildDualGraph(); ///< Build GL call list for dual graph.

    ///@}
    ///@name Internal State
    ///@{

    const WorkspaceDecomposition* const m_decomposition;

    GLuint m_regionList{0};      ///< GL call list ID for regions.
    GLuint m_dualGraphList{0};   ///< GL call list ID for dual graph.

    ///@}
};

#endif
