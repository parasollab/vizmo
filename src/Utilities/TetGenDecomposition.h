#ifndef TET_GEN_DECOMPOSITION_H_
#define TET_GEN_DECOMPOSITION_H_

#include <memory>
using namespace std;

#include <containers/sequential/graph/graph.h>

#include <Vector.h>
using namespace mathtool;

class tetgenio;

class Environment;
class Boundary;
class BoundingBox;
class BoundingSphere;
class ReebGraphConstruction;
class StaticMultiBody;

class TetGenDecomposition {
  public:

    TetGenDecomposition(Environment* _env, string _switches = "pqn");
    ~TetGenDecomposition();

    void Decompose();
    //vector<Vector3d> GetPath(const Vector3d& _p1, const Vector3d& _p2, double _posRes);

    typedef stapl::sequential::graph<
      stapl::DIRECTED, stapl::MULTIEDGES,
      Vector3d, vector<Vector3d>
        > FlowGraph;
    pair<FlowGraph, size_t> GetFlowGraph(const Vector3d& _p1, double _posRes);

    void DrawGraph();

    void DrawPath(/*const Vector3d& _p1, const Vector3d& _p2*/);

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

    void MakeGraph();

    size_t FindTetrahedron(const Vector3d& _p) const;

    Environment* m_env;

    tetgenio* m_freeModel;
    tetgenio* m_decompModel;
    string m_switches;

    typedef stapl::sequential::graph<
      stapl::UNDIRECTED, stapl::NONMULTIEDGES, Vector3d, double
      > TetrahedralizationGraph;
    TetrahedralizationGraph m_graph;

    vector<size_t> m_path;

    ReebGraphConstruction* m_reebGraph;
};

#endif
