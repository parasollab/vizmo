#ifndef TET_GEN_DECOMPOSITION_H_
#define TET_GEN_DECOMPOSITION_H_

//#include <cstddef>
#include <memory>
using namespace std;

class tetgenio;

class Environment;
class StaticMultiBody;
class Boundary;
class BoundingBox;
class BoundingSphere;

class TetGenDecomposition {
  public:

    TetGenDecomposition();
    ~TetGenDecomposition();

    void Decompose(Environment* _env);

  private:

    void InitializeFreeModel();
    size_t GetNumVertices() const;
    size_t GetNumVertices(const shared_ptr<StaticMultiBody>& _obst) const;
    size_t GetNumVertices(const shared_ptr<Boundary>& _boundary) const;
    size_t GetNumFacets() const;
    size_t GetNumFacets(const shared_ptr<StaticMultiBody>& _obst) const;
    size_t GetNumFacets(const shared_ptr<Boundary>& _boundary) const;
    size_t GetNumHoles() const;

    void MakeFreeModel();
    void AddToFreeModel(const shared_ptr<StaticMultiBody>& _obst,
        size_t _pOff, size_t _fOff, size_t _hOff);
    void AddToFreeModel(const shared_ptr<Boundary>& _boundary,
        size_t _pOff, size_t _fOff);
    void AddToFreeModel(const shared_ptr<BoundingBox>& _boundary,
        size_t _pOff, size_t _fOff);
    void AddToFreeModel(const shared_ptr<BoundingSphere>& _boundary,
        size_t _pOff, size_t _fOff);

    void SaveFreeModel();
    void SaveDecompModel();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Add boundary model to tetgen
    //virtual void AddToTetGen(tetgenio* _tetModel, size_t _pOff, size_t _fOff) const = 0;

    tetgenio* m_freeModel;
    tetgenio* m_decompModel;
    char* m_switches;

    Environment* m_env;
};

#endif
