#ifndef REEB_GRAPH_CONSTRUCTION_H_
#define REEB_GRAPH_CONSTRUCTION_H_

#include <vector>
#include <unordered_set>
using namespace std;

#include <boost/functional/hash.hpp>

//#include <containers/sequential/graph/graph.h>
#include <containers/sequential/graph/directed_preds_graph.h>

#include <GL/gl.h>

#include <Vector.h>
using namespace mathtool;

#include "Utilities/Color.h"
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
      size_t m_order;

      size_t m_tetra;
    };

    struct ReebNodeComp {
      ReebNodeComp(vector<Vector3d>* _v) : m_vertices(_v) {}

      bool operator()(const ReebNode& _a, const ReebNode& _b) {
        auto vertComp = [](const Vector3d _a, const Vector3d _b) {
          return _a[0] - _b[0] > 0.000001 ||
            (fabs(_a[0] - _b[0]) < 0.000001 && (_a[1] - _b[1] > 0.000001 ||
            (fabs(_a[1] - _b[1]) < 0.000001 && _a[2] - _b[2] > 0.000001)));
        };
        //clockReebNodeComp.StartClock();
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
      ReebArc(size_t _s = -1, size_t _t = -1, MeshEdge* _m = nullptr) :
        m_source(_s), m_target(_t) {
          if(_m) m_edges.insert(_m);
        }

      size_t m_source;
      size_t m_target;
      unordered_set<MeshEdge*> m_edges; ///< Edge indices
      unordered_set<size_t> m_tetra;

      //map<size_t, unordered_set<size_t>, greater<size_t>> m_buckets;
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

      bool operator()(const RGEID& _a0, const RGEID& _a1) {
        //clockReebArcComp.StartClock();
        bool ret = false;
        ReebNode& s0 = m_rg->find_vertex(_a0.source())->property();
        ReebNode& s1 = m_rg->find_vertex(_a1.source())->property();
        if(s0.m_order < s1.m_order)
          ret = true;
        else if(s0.m_order == s1.m_order) {
          ReebNode& t0 = m_rg->find_vertex(_a0.target())->property();
          ReebNode& t1 = m_rg->find_vertex(_a1.target())->property();
          if(t0.m_order < t1.m_order)
            ret = true;
          else if(t0.m_order == t1.m_order)
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

    typedef stapl::sequential::graph<
      stapl::UNDIRECTED, stapl::NONMULTIEDGES, Vector3d, double
      > TetrahedralizationGraph;

    ReebGraphConstruction(const vector<Vector3d>& _vertices,
        const vector<pair<tuple<size_t, size_t, size_t>, unordered_set<size_t>>>& _triangles,
        const int* const _tetras,
        size_t _numCorners,
        TetrahedralizationGraph& _tetraGraph);

    template<typename REAL>
    void Draw(const REAL* const _points, const int* const _tetra,
        size_t _numTetra, size_t _numCorners);

  private:

    void Construct();
    void Embed(const vector<Vector3d>& _vertices,
        const int* const _tetras, size_t _numCorners,
        TetrahedralizationGraph& _tetraGraph);

    void CreateNode(size_t _i, const Vector3d& _v, double _w);
    MeshEdge* CreateArc(size_t _s, size_t _t,
        const unordered_set<size_t>& _tetra = unordered_set<size_t>());
    void MergePaths(MeshEdge* _e0, MeshEdge* _e1, MeshEdge* _e2);
    void GlueByMergeSorting(ArcSet& _a0, MeshEdge* _e0, ArcSet& _a1, MeshEdge* _e1);
    void MergeArcs(RGEID _a0, RGEID _a1);
    ReebArc& GetReebArc(RGEID _a);
    void DeleteMeshEdge(MeshEdge* _e);

    vector<Vector3d> m_vertices;
    unordered_set<MeshEdge*, MeshEdgeHash, MeshEdgeEq> m_edges;
    vector<pair<tuple<size_t, size_t, size_t>, unordered_set<size_t>>> m_triangles;

    ReebGraph m_reebGraph;

    double m_minBucket, m_maxBucket, m_bucketRes;

    struct RGEIDComp {
      bool operator()(const RGEID& _a, const RGEID& _b) {
        return _a.source() < _b.source() ||
          (_a.source() == _b.source() && _a.target() < _b.target()) ||
           (_a.source() == _b.source() && _a.target() == _b.target() && _a.id() < _b.id());
      }
    };

    map<RGEID, Color4, RGEIDComp> m_colors;
};

template<typename REAL>
void
ReebGraphConstruction::
Draw(const REAL* const _points, const int* const _tetra,
    size_t _numTetra, size_t _numCorners) {

  static size_t numCalls = 0;
  static size_t edge = 0;

  //cout << "Num calls: " << numCalls << endl;
  numCalls = (numCalls + 1) % 60;
  if(numCalls == 0) {
    edge = (edge + 1) % m_reebGraph.get_num_edges();
  }

  glDisable(GL_LIGHTING);
  glPointSize(8);
  glLineWidth(5);

  //draw reeb graph
  glColor4f(0.0, 0.5, 0.2, 0.05);

  /*glBegin(GL_POINTS);
  for(auto v = m_reebGraph.begin(); v != m_reebGraph.end(); ++v) {
    glVertex3dv(m_vertices[v->property().m_vertex]);
  }
  glEnd();

  glColor4f(0.0, 0.2, 0.5, 0.05);*/

  glBegin(GL_POINTS);
  for(auto v = m_reebGraph.begin(); v != m_reebGraph.end(); ++v) {
    glVertex3dv(v->property().m_vertex2);
  }
  glEnd();

  size_t i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    //if(i++ == edge) {
      glBegin(GL_LINE_STRIP);
      /*glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
        glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);*/
      //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
      for(auto& v : e->property().m_path)
        glVertex3dv(v);
      //glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
      glEnd();
    //}
  }

  /*glColor4f(0.5, 0.0, 0.2, 0.05);
  glLineWidth(3);
  glBegin(GL_LINES);
  i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    if(i++ == edge) {
      glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->source())->property().m_vertex]);
      glVertex3dv(m_vertices[m_reebGraph.find_vertex(e->target())->property().m_vertex]);
    }
  }
  glEnd();*/

  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  i = 0;
  for(auto e = m_reebGraph.edges_begin(); e != m_reebGraph.edges_end(); ++e) {
    if(i++ == edge) {
      ReebArc& ra = e->property();
      glColor4fv(m_colors[e->descriptor()]);
      //cout << m_colors.size() << " Color: " << m_colors[e->descriptor()] << endl;
      glBegin(GL_TRIANGLES);
      for(const size_t& i : ra.m_tetra) {
        Vector3d vs[_numCorners];
        for(size_t j = 0; j < _numCorners; ++j)
          vs[j] = Vector3d(&_points[3*_tetra[i*_numCorners + j]]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[3]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[0]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[3]);
        glVertex3dv(vs[1]);
        glVertex3dv(vs[2]);
        glVertex3dv(vs[3]);
      }
      glEnd();
    }
  }

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  glEnable(GL_LIGHTING);
}

#endif
