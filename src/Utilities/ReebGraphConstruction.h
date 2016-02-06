#ifndef REEB_GRAPH_CONSTRUCTION_H_
#define REEB_GRAPH_CONSTRUCTION_H_

#include <vector>
#include <unordered_set>
using namespace std;

#include <boost/functional/hash.hpp>

//#include <containers/sequential/graph/graph.h>
#include <containers/sequential/graph/directed_preds_graph.h>

#include <Vector.h>
using namespace mathtool;

#include "Utilities/MetricUtils.h"

extern ClockClass clockReebNodeComp;
extern ClockClass clockReebArcComp;
extern ClockClass clockGetReebArc;
extern ClockClass clockMergeArcs;
extern ClockClass clockGlueByMergeSorting;

class ReebGraphConstruction {
  public:

    struct MeshEdge;

    struct ReebNode {
      ReebNode(size_t _v = -1, const Vector3d& _v2 = Vector3d(), double _w = numeric_limits<double>::max()) : m_vertex(_v), m_vertex2(_v2), m_w(_w) {}

      size_t m_vertex; ///< Vertex Index
      Vector3d m_vertex2;
      double m_w; ///< Morse function value
    };

    struct ReebNodeComp {
      ReebNodeComp(vector<Vector3d>* _v) : m_vertices(_v) {}

      bool operator()(const ReebNode& _a, const ReebNode& _b) {
        auto vertComp = [](const Vector3d _a, const Vector3d _b) {
          return _a[0] - _b[0] > 0.000001 ||
            (fabs(_a[0] - _b[0]) < 0.000001 && (_a[1] - _b[1] > 0.000001 ||
            (fabs(_a[1] - _b[1]) < 0.000001 && _a[2] - _b[2] > 0.000001)));
          /*if(_a[0] - _b[0] > 0.000001)
            return true;
          else if(_a[0] - _b[0] < 0.000001 && _a[0] - _b[0] > -0.000001) {
            if(_a[1] - _b[1] > 0.000001)
              return true;
            else if(_a[1] - _b[1] < 0.000001 && _a[1] - _b[1] > -0.000001) {
              if(_a[2] - _b[2] > 0.000001)
                return true;
            }
          }
          return false;*/
        };
        //clockReebNodeComp.StartClock();
        //const Vector3d& a = (*m_vertices)[_a.m_vertex];
        //const Vector3d& b = (*m_vertices)[_b.m_vertex];
        const Vector3d& a = _a.m_vertex2;
        const Vector3d& b = _b.m_vertex2;
        bool ret = _b.m_vertex != _a.m_vertex &&
          (_a.m_w - _b.m_w > 0.000001 ||
           (fabs(_a.m_w - _b.m_w) < 0.000001 &&
            (vertComp(a, b) || (a == b &&
            //!vertComp(a, b) &&
            //!vertComp(b, a) &&
            _a.m_vertex < _b.m_vertex))));
        //clockReebNodeComp.StopClock();
        return ret;
      }

      vector<Vector3d>* m_vertices;
    };

    struct ReebArc {
      ReebArc(size_t _s = -1, size_t _t = -1, MeshEdge* _m = nullptr) : m_source(_s), m_target(_t) {if(_m) m_edges.insert(_m);}
      size_t m_source;
      size_t m_target;
      unordered_set<MeshEdge*> m_edges; ///< Edge indices

      map<size_t, set<size_t>, greater<size_t>> m_buckets;
      vector<Vector3d> m_path;
    };

    //typedef stapl::sequential::graph<
    //  stapl::DIRECTED, stapl::MULTIEDGES, ReebNode, ReebArc> ReebGraph;
    typedef stapl::sequential::directed_preds_graph<
      stapl::MULTIEDGES, ReebNode, ReebArc> ReebGraph;
    //typedef stapl::sequential::directed_preds_graph<
    //  stapl::MULTIEDGES, ReebNode, ReebArc,
    //  stapl::sequential::adj_map_int> ReebGraph;
    typedef ReebGraph::edge_descriptor RGEID;

    struct ReebArcComp {
      ReebArcComp(vector<Vector3d>* _v, ReebGraph* _rg) : m_rnc(_v), m_rg(_rg) {}
      //ReebArcComp(const ReebArcComp& _rac) : m_rg(_rac.m_rg) {}

      bool operator()(const RGEID& _a0, const RGEID& _a1) {
        /*double fa0 = m_rg->find_vertex(_a0.source())->property().m_w;
        double fa1 = m_rg->find_vertex(_a1.source())->property().m_w;
        if(fa0 - fa1 > 0.000001)
          return true;
        else if (fabs(fa0 - fa1) < 0.000001) {
          if(_a0.source() < _a1.source())
            return true;
          else if(_a0.source() == _a1.source()) {
            if(_a0.target() < _a1.target())
              return true;
            else if(_a0.target() == _a1.target()) {
              if(_a0.id() < _a1.id())
                return true;
            }
          }
        }
        return false;*/
        //clockReebArcComp.StartClock();
        bool ret = false;
        ReebNode& s0 = m_rg->find_vertex(_a0.source())->property();
        ReebNode& s1 = m_rg->find_vertex(_a1.source())->property();
        if(_a0.source() != _a1.source() && m_rnc(s0, s1))
          ret = true;
        else if(_a0.source() == _a1.source() || (!m_rnc(s0, s1) && ! m_rnc(s1, s0))) {
          ReebNode& t0 = m_rg->find_vertex(_a0.target())->property();
          ReebNode& t1 = m_rg->find_vertex(_a1.target())->property();
          if(_a0.target() != _a1.target() && m_rnc(t0, t1))
            ret = true;
          else if(_a0.target() == _a1.target() || (!m_rnc(t0, t1) && !m_rnc(t1, t0)))
            if(_a0.id() < _a1.id())
              ret = true;
        }
        //clockReebArcComp.StopClock();
        return ret;
      }
      ReebNodeComp m_rnc;
      ReebGraph* m_rg;
    };

    struct MeshVertex {
      Vector3d m_vertex;
    };

    typedef set<RGEID, ReebArcComp> ArcSet;

    struct MeshEdge {
      MeshEdge(size_t _s, size_t _t, vector<Vector3d>* _v, ReebGraph* _rg) :
        m_source(_s), m_target(_t), m_numTris(0), m_arcs(ReebArcComp(_v, _rg)) {
          //m_arcs.insert(RGEID(_s, _t));
        }
      size_t m_source; ///< Vertex index
      size_t m_target; ///< Vertex index
      size_t m_numTris; ///< Number of triangles which it is incident on
      ArcSet m_arcs; ///< Reference to Reeb Arcs
    };

    struct MeshEdgeHash {
      size_t operator()(const MeshEdge* const _m) const {
        return h(make_pair(_m->m_source, _m->m_target));
      }
      boost::hash<pair<size_t, size_t>> h;
    };

    struct MeshEdgeEq {
      bool operator()(const MeshEdge* const _m1, const MeshEdge* const _m2) const {
        return _m1->m_source == _m2->m_source && _m1->m_target == _m2->m_target;
      }
    };

    ReebGraphConstruction(const vector<Vector3d>& _vertices,
        const vector<tuple<size_t, size_t, size_t>>& _triangles);

    void Draw();

  private:

    void Construct();

    void CreateNode(size_t _i, const Vector3d& _v, double _w);
    MeshEdge* CreateArc(size_t _s, size_t _t);
    //MeshEdge& GetMeshEdge(size_t _s, size_t _t);
    void MergePaths(MeshEdge* _e0, MeshEdge* _e1, MeshEdge* _e2);
    void GlueByMergeSorting(ArcSet& _a0, MeshEdge* _e0, ArcSet& _a1, MeshEdge* _e1);
    void MergeArcs(RGEID _a0, RGEID _a1);
    ReebArc& GetReebArc(RGEID _a);
    void DeleteMeshEdge(MeshEdge* _e);

    vector<Vector3d> m_vertices;
    unordered_set<MeshEdge*, MeshEdgeHash, MeshEdgeEq> m_edges;
    vector<tuple<size_t, size_t, size_t>> m_triangles;

    ReebGraph m_reebGraph;

    double m_minBucket, m_maxBucket, m_bucketRes;
};

#endif
