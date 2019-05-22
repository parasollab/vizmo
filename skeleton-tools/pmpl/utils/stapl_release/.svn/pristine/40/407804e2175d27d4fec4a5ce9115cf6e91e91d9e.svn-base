/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_BREADTH_FIRST_SEARCH_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_BREADTH_FIRST_SEARCH_HPP

#include <stapl/containers/graph/algorithms/paradigms/graph_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h2_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_hubs_paradigm.hpp>

namespace stapl {

namespace bfs_algo_detail {

//////////////////////////////////////////////////////////////////////
/// @brief Vertex-initializer functor for @ref breadth_first_search().
///
/// Initializes vertices' BFS-parent, BFS-level.
/// If the vertex is the source, the level is one (active).
/// All other vertices' levels are set to zero (inactive).
/// All vertices' BFS-parents are set to their own descriptors.
/// @tparam VD Type of the vertex-descriptor.
/// @tparam LevelType Type of the vertex-level.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
template <class VD, class LevelType>
class bfs_init_wf
{
  typedef VD vd_type;
  vd_type    m_source;

public:
  typedef void result_type;

  bfs_init_wf(vd_type source)
    : m_source(source)
  { }

  template <class Vertex>
  void operator()(Vertex v)
  {
    if (v.descriptor() == m_source) {
      v.property().level(LevelType(1));
    } else {
      v.property().level(LevelType(0));
    }
    v.property().parent(v.descriptor());
  }

  void define_type(typer& t)
  { t.member(m_source); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reducer functor for @ref breadth_first_search().
///
/// Reduces two BFS properties to update the first one.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct vp_reducer
{
  template<typename VP1, typename VP2>
  void operator()(VP1& p1, VP2& p2) const
  {
    if (p1.level() == 0 || p1.level() > p2.level()) {
      p1.level(p2.level());
      p1.parent(p2.parent());
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Visitor functor for @ref breadth_first_search().
///
/// Updates the target vertex with BFS-parent and BFS-level
/// information, if the target vertex has not been visited before, or
/// if the target's level is greater than the incoming level.
/// @tparam VD Type of the vertex-descriptor.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
template <typename VD>
class update_func
{
public:
  typedef VD        parent_type;
  parent_type       m_parent;
  size_t            m_level;

  typedef bool result_type;

  //////////////////////////////////////////////////////////////////////
  /// @param p The BFS-parent of the target vertex.
  /// @param level The BFS-level of the target vertex.
  //////////////////////////////////////////////////////////////////////
  update_func(parent_type p = 0, size_t level = 0)
    : m_parent(p), m_level(level)
  { }

  template <class Vertex>
  result_type operator()(Vertex target) const
  {
    if (target.property().level() == 0 ||
        target.property().level() > m_level) {
      target.property().level(m_level);
      target.property().parent(m_parent);
      return true;
    }
    //else ignore.
    return false;
  }

  void define_type(typer& t)
  {
    t.member(m_parent);
    t.member(m_level);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function for @ref breadth_first_search().
/// A vertex is visited if it is active (the level on the vertex matches the
/// current level of the paradigm. Active vertices update their neighbors with
/// new BFS level and parent information.
/// Returns true if vertex was active, false otherwise.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct bfs_map_wf
{
  typedef bool result_type;

  template<class Vertex, class GraphVisitor>
  result_type operator()(Vertex v, GraphVisitor graph_visitor)
  {
    typedef typename Vertex::vertex_descriptor descriptor_t;
    if (v.property().level() == graph_visitor.level()) {  // if v is active.
      graph_visitor.visit_all_edges(v,
        update_func<descriptor_t>(v.descriptor(), v.property().level()+1));
      return true;
    }
    return false;
  }
};
} // namespace bfs_algo_detail


//////////////////////////////////////////////////////////////////////
/// @brief A Parallel Breadth-First Search (BFS)
///
/// Performs a breadth-first search on the input @ref graph_view, storing
/// the BFS-level and BFS-parent on each reachable vertex.
/// @param g The @ref graph_view over the input graph.
/// @param source The descriptor of the source vertex for this traversal.
/// @param k The maximum amount of asynchrony allowed in each phase.
/// 0 <= k <= inf.
/// k == 0 implies level-synchronous BFS.
/// k >= D implies fully asynchronous BFS (D is diameter of graph).
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView>
size_t breadth_first_search(GView& g,
                            typename GView::vertex_descriptor const& source,
                            size_t k=0)
{
  using namespace bfs_algo_detail;
  typedef typename GView::vertex_descriptor vd_type;
  typedef update_func<vd_type> update_func_t;

  // Initialize the vertices.
  map_func(bfs_init_wf<vd_type, size_t>(source), g);
  return graph_paradigm(bfs_map_wf(), update_func_t(), g, k);
}


//////////////////////////////////////////////////////////////////////
/// @brief A Parallel Breadth-First Search (BFS) using the hierarchical
/// machine paradigm (@ref h_paradigm()).
///
/// Performs a breadth-first search on the input @ref graph_view and its
/// hierarchical machine @ref graph_view, storing the BFS-level and
/// BFS-parent on each reachable vertex of the input.
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param source The descriptor of the source vertex for this traversal.
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView>
size_t breadth_first_search_h(GView& g, HView& h,
                              typename GView::vertex_descriptor const& source)
{
  using namespace bfs_algo_detail;
  typedef typename GView::vertex_descriptor vd_type;
  typedef update_func<vd_type> update_func_t;

  // Initialize the vertices.
  map_func(bfs_init_wf<vd_type, size_t>(source), g);
  return h_paradigm(bfs_map_wf(), update_func_t(), h, g);
}


//////////////////////////////////////////////////////////////////////
/// @brief A Parallel Breadth-First Search (BFS) using the 2-level
/// hierarchical machine paradigm (@ref h2_paradigm()).
///
/// Performs a breadth-first search on the input @ref graph_view and its
/// hierarchical machine @ref graph_view, storing the BFS-level and
/// BFS-parent on each reachable vertex of the input.
/// The 2-level hierarchy uses knowledge of machine-locality on a per-core
/// as well as per-node basis in an effort to improve performance.
///
/// @param g The @ref graph_view over the input graph.
/// @param h1 The first-level hierarchical machine @ref graph_view over
/// the input graph. This is generated by calling @ref create_level_machine()
/// on the input @ref graph_view.
/// @param h2 The second-level hierarchical machine @ref graph_view over
/// the input graph. This is generated by calling @ref create_level2_machine()
/// on the input @ref graph_view.
/// @param source The descriptor of the source vertex for this traversal.
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class H1View, class H2View>
size_t breadth_first_search_h2(GView& g, H1View& h1, H2View& h2,
                               typename GView::vertex_descriptor const& source)
{
  using namespace bfs_algo_detail;
  typedef typename GView::vertex_descriptor vd_type;
  typedef update_func<vd_type> update_func_t;

  // Initialize the vertices.
  map_func(bfs_init_wf<vd_type, size_t>(source), g);
  return h2_paradigm(bfs_map_wf(), update_func_t(), h2, h1, g);
}



//////////////////////////////////////////////////////////////////////
/// @brief A Parallel Breadth-First Search (BFS) using the hierarchical
/// machine and hubs paradigm (@ref h_hubs_paradigm()).
///
/// Performs a breadth-first search on the input @ref graph_view and its
/// hierarchical machine and hierarchical hubs @ref graph_view, storing
/// the BFS-level and BFS-parent on each reachable vertex of the input.
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param hubs The hierarchical hubs @ref graph_view over the input graph,
/// which creates a hierarchical view over all the high-degree vertices (hubs)
/// of the input graph. This is generated by calling @ref create_level_hubs()
/// on the input @ref graph_view.
/// @param source The descriptor of the source vertex for this traversal.
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView, class HubsView>
size_t breadth_first_search_h(GView& g, HView& h, HubsView& hubs,
                              typename GView::vertex_descriptor const& source)
{
  using namespace bfs_algo_detail;
  typedef typename GView::vertex_descriptor vd_type;
  typedef update_func<vd_type> update_func_t;

  // Initialize the vertices.
  map_func(bfs_init_wf<vd_type, size_t>(source), g);
  return h_hubs_paradigm(bfs_map_wf(), update_func_t(), vp_reducer(),
                         h, hubs, g);
}

} // namespace stapl

#endif
