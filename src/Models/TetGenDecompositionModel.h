#ifndef TET_GEN_DECOMPOSITION_MODEL_H
#define TET_GEN_DECOMPOSITION_MODEL_H

#include "Model.h"

class TetGenDecomposition;

////////////////////////////////////////////////////////////////////////////////
///@brief Drawable model for TetGenDecomposition utility
////////////////////////////////////////////////////////////////////////////////
class TetGenDecompositionModel : public Model {

  public:

    ///@param _tetgen TetGen tetrahedralization
    TetGenDecompositionModel(TetGenDecomposition* _tetgen);
    ~TetGenDecompositionModel();

    // Model functions
    void Build();
    void Select(GLuint* _index, vector<Model*>& _sel);
    void DrawRender();
    void DrawSelect();
    void DrawSelected();
    void Print(ostream& _os) const;

  private:

    ////////////////////////////////////////////////////////////////////////////
    ///@brief Build compiled GL for tetrahedrons
    void BuildTetrahedrons();
    ////////////////////////////////////////////////////////////////////////////
    ///@brief Build compiled GL for dual graph
    void BuildDualGraph();

    TetGenDecomposition* m_tetgen; ///< TetGen tetrahedralization

    GLuint m_tetrahedronID;        ///< Compiled GL for tetrahedralization
    GLuint m_dualGraphID;          ///< Compiled GL for dual graph
};

#endif
