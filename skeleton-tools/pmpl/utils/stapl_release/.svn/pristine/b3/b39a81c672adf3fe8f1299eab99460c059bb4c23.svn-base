/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_PAGE_RANK_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_PAGE_RANK_HPP

#include <stapl/containers/graph/algorithms/paradigms/graph_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_hubs_paradigm.hpp>

namespace stapl {

namespace page_rank_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Work-function to initialize the auxiliary and actual
/// PageRank values.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct page_rank_init_wf
{
  double m_initial_rank;
  double m_initial_aux_rank;
  typedef void result_type;

  page_rank_init_wf(double const& initial_rank, double const& initial_aux_rank)
    : m_initial_rank(initial_rank), m_initial_aux_rank(initial_aux_rank)
  { }

  template <typename T>
  void operator()(T v) const
  {
    v.property().rank(m_initial_rank);
    v.property().new_rank(m_initial_aux_rank);
    v.property().set_active(true);
  }

  void define_type(typer& t)
  {
    t.member(m_initial_rank);
    t.member(m_initial_aux_rank);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reducer functor for @ref page_rank().
///
/// Reduces two page_rank properties to update the first one.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct vp_reducer
{
  template<typename VP1, typename VP2>
  void operator()(VP1& p1, VP2& p2) const
  {
    p1.new_rank(p2.new_rank()+p1.new_rank());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function to copy the auxiliary rank to the actual
/// PageRank values and reinitialize the auxiliary rank.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct page_rank_copy_wf
{
  double m_damping;
  size_t m_max_visits;
  typedef void result_type;

  page_rank_copy_wf(double const& damping, size_t max_visits)
    : m_damping(damping), m_max_visits(max_visits)
  { }

  template <typename Vertex>
  void operator()(Vertex v) const
  {
    v.property().rank(v.property().new_rank());
    v.property().new_rank(1 - m_damping);
    v.property().set_active(true);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Algorithm to copy the auxiliary rank to the actual
  /// PageRank values and reinitialize the auxiliary rank.
  ///
  /// Executed at the end of each paradigm iteration as a post-execute.
  /// @param g The input graph_view
  /// @param iteration The iteration value.
  //////////////////////////////////////////////////////////////////////
  template <typename GraphView>
  void operator()(GraphView& g, size_t iteration) const
  {
    if (iteration < m_max_visits)
      map_func(*this, g);
  }

  void define_type(typer& t)
  {
    t.member(m_damping);
    t.member(m_max_visits);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to add the value of the incomming PageRank to the
/// target vertex's auxiliary PageRank.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct update_func
{
  double m_rank;

  typedef bool result_type;

  update_func(double const& rank = 0.0)
    : m_rank(rank)
  { }

  template <class Vertex>
  bool operator()(Vertex target) const
  {
    target.property().new_rank(m_rank + target.property().new_rank());
    return true;
  }

  void define_type(typer& t)
  { t.member(m_rank); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function to compute the PageRank of a vertex and push it
/// to the vertex's neighbors.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct page_rank_wf
{
  double m_damping;
  typedef bool result_type;

  page_rank_wf(double const& damping)
    : m_damping(damping)
  { }

  template<typename Vertex, typename GraphVisitor>
  bool operator()(Vertex v, GraphVisitor graph_visitor)
  {
    if (v.property().is_active()) {
      double u_rank_out = m_damping*v.property().rank()/double(v.size());
      graph_visitor.visit_all_edges(v, update_func(u_rank_out));
      v.property().set_active(false);
      return true;
    }
    return false;
  }

  void define_type(typer& t)
  { t.member(m_damping); }
};

}; // namespace page_rank_impl;


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized PageRank Algorithm.
///
/// Performs a PageRank on the input @ref graph_view, storing the ranks
/// of all vertices on their properties.
/// @param graph The @ref graph_view over the input graph.
/// @param iterations The number of PageRank iterations to perform.
/// @param damping The damping factor for the algorithm.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename GView>
size_t page_rank(GView& graph, size_t iterations, double damping = 0.85)
{
  using namespace page_rank_impl;
  // Initialize ranks.
  double normalized_sz = 1;
  map_func(page_rank_init_wf(normalized_sz, 1-damping), graph);

  return level_sync_paradigm(page_rank_wf(damping), update_func(),
                             page_rank_copy_wf(damping, iterations), graph);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized PageRank Algorithm using the
/// hierarchical machine paradigm (@ref h_paradigm()).
///
/// Performs a PageRank on the input @ref graph_view, storing the ranks
/// of all vertices on their properties.
/// @param graph The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param iterations The number of PageRank iterations to perform.
/// @param damping The damping factor for the algorithm.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView>
size_t page_rank_h(GView& g, HView& h, size_t iterations, double damping = 0.85)
{
  using namespace page_rank_impl;
  // Initialize ranks.
  double normalized_sz = 1;
  map_func(page_rank_init_wf(normalized_sz, 1-damping), g);

  return h_paradigm(page_rank_wf(damping), update_func(),
                    page_rank_copy_wf(damping, iterations), h, g);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized PageRank Algorithm using the
/// hierarchical machine and hubs paradigm (@ref h_hubs_paradigm()).
///
/// Performs a PageRank on the input @ref graph_view, storing the ranks
/// of all vertices on their properties.
/// @param graph The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param hubs The hierarchical hubs @ref graph_view over the input graph,
/// which creates a hierarchical view over all the high-degree vertices (hubs)
/// of the input graph. This is generated by calling @ref create_level_hubs()
/// on the input @ref graph_view.
/// @param iterations The number of PageRank iterations to perform.
/// @param damping The damping factor for the algorithm.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView, class HubsView>
size_t page_rank_h(GView& g, HView& h, HubsView& hubs,
                   size_t iterations, double damping = 0.85)
{
  using namespace page_rank_impl;
  // Initialize ranks.
  double normalized_sz = 1;
  map_func(page_rank_init_wf(normalized_sz, 1-damping), g);

  return h_hubs_paradigm(page_rank_wf(damping), update_func(), vp_reducer(),
                         page_rank_copy_wf(damping, iterations), h, hubs, g);

}


} // namespace stapl
#endif
