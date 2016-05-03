#ifndef TET_GEN_DECOMPOSITION_H_
#define TET_GEN_DECOMPOSITION_H_

#include <memory>
using namespace std;

#include <containers/sequential/graph/graph.h>

#define TETLIBRARY
#undef PI
#include "tetgen.h"

#include <Vector.h>
using namespace mathtool;

class Environment;
class Boundary;
class BoundingBox;
class BoundingSphere;
class StaticMultiBody;

class TetGenDecomposition {
  public:

    typedef stapl::sequential::graph<
      stapl::UNDIRECTED, stapl::NONMULTIEDGES,
      Vector3d, double
        > DualGraph;

    TetGenDecomposition(Environment* _env, string _switches,
        bool _writeFreeModel, bool _writeDecompModel);
    ~TetGenDecomposition();

    size_t GetNumPoints() const {return m_decompModel->numberofpoints;}
    const double* const GetPoints() const {return m_decompModel->pointlist;}
    size_t GetNumCorners() const {return m_decompModel->numberofcorners;}
    size_t GetNumTetras() const {return m_decompModel->numberoftetrahedra;}
    const int* const GetTetras() const {return m_decompModel->tetrahedronlist;}
    DualGraph& GetDualGraph() {return m_dualGraph;}

  private:

    void Decompose();

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

    void MakeDualGraph();

    Environment* m_env;

    tetgenio* m_freeModel;
    tetgenio* m_decompModel;
    string m_switches;
    bool m_writeFreeModel;
    bool m_writeDecompModel;

    DualGraph m_dualGraph;
};

#endif
