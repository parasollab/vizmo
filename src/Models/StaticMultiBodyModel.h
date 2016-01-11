#ifndef STATIC_MULTI_BODY_MODEL_H_
#define STATIC_MULTI_BODY_MODEL_H_

#include "MultiBodyModel.h"

#define TETLIBRARY
#include "tetgen1.5.0/tetgen.h"

class StaticMultiBody;

class StaticMultiBodyModel : public MultiBodyModel {
  public:
    StaticMultiBodyModel(shared_ptr<StaticMultiBody> _s);
    StaticMultiBodyModel(string _name, shared_ptr<StaticMultiBody> _s);

    virtual void Build();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Number of vertices in model
    size_t GetNumVertices() const;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Number of facets in model
    size_t GetNumFacets() const;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add model to tetgen
    /// @param _tetModel tetgen model
    /// @param _pOff Point array offset in tetgen model
    /// @param _fOff Facet array offset in tetgen model
    /// @param _hOff Hole array offset in tetgen model
    void AddToTetGen(tetgenio* _tetModel,
        size_t _pOff, size_t _fOff, size_t _hOff) const;

  private:
    shared_ptr<StaticMultiBody> m_staticMultiBody;
};

#endif
