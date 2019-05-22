/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_CLOSENESS_CENTRALITY_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_CLOSENESS_CENTRALITY_HPP

#include <stapl/containers/graph/algorithms/breadth_first_search.hpp>

namespace stapl {

namespace closeness_centrality_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor to compute the Farness of each vertex.
///
/// Farness(v) = Sum(d(u,v)), where d(u,v) is the shortest-distance
/// from every vertex u in the graph to the given vertex v.
/// @note In our implementation, we temporarily store the farness in the
/// same field as closeness to save space.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct farness_add_wf
{
  typedef void result_type;
  template<typename Vertex>
  result_type operator()(Vertex v) const
  { v.property().closeness(v.property().closeness()+v.property().level()); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to compute the Closeness of each vertex.
///
/// Closeness(v) = 1 / Farness(v)
///              = 1 / Sum(d(u,v)), where d(u,v) is the shortest-distance
///                from every vertex u in the graph to the given vertex v.
/// @note In our implementation, we temporarily store the farness in the
/// same field as closeness to save space, so computing the closeness
/// simply involves inverting the currently-stored value (farness).
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct compute_closeness_wf
{
  typedef void result_type;
  template<typename Vertex>
  result_type operator()(Vertex v) const
  {
    if (v.property().closeness() != 0)
      v.property().closeness(1.0 / v.property().closeness());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to initialize the Closeness of each vertex.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct init_closeness_wf
{
  typedef void result_type;
  template<typename Vertex>
  result_type operator()(Vertex v) const
  { v.property().closeness(0.0); }
};

}; // namespace closeness_centrality_impl;


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Closeness Centrality Algorithm.
///
/// Calculates the closeness-centrality of each vertex in a graph.
/// Closeness(v) = 1 / Farness(v)
///              = 1 / Sum(d(u,v)), where d(u,v) is the shortest-distance
///                from every vertex u in the graph to the given vertex v.
/// @param graph The @ref graph_view over the input graph.
/// @param k The level of asynchrony in the algorithm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename GView>
void closeness_centrality(GView& graph, size_t k=0)
{
  using namespace closeness_centrality_impl;
  map_func(init_closeness_wf(), graph);
  size_t size = graph.size();
  for (size_t i=0; i<size; ++i) {
    // Compute shortest-distance from this new source.
    breadth_first_search(graph, i, k);
    // Add the previously computed values to farness.
    map_func(farness_add_wf(), graph);
  }
  // Calculate the closeness by inverting the farness.
  map_func(compute_closeness_wf(), graph);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Closeness Centrality Algorithm using the
/// hierarchical breadth-first search (@ref breadth_first_search_h()).
///
/// Calculates the closeness-centrality of each vertex in a graph.
/// Closeness(v) = 1 / Farness(v)
///              = 1 / Sum(d(u,v)), where d(u,v) is the shortest-distance
///                from every vertex u in the graph to the given vertex v.
///
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView>
void closeness_centrality(GView& g, HView& h)
{
  using namespace closeness_centrality_impl;
  map_func(init_closeness_wf(), g);
  size_t size = g.size();
  for (size_t i=0; i<size; ++i) {
    // Compute shortest-distance from this new source.
    breadth_first_search_h(g, h, i);
    // Add the previously computed values to farness.
    map_func(farness_add_wf(), g);
  }
  // Calculate the closeness by inverting the farness.
  map_func(compute_closeness_wf(), g);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Closeness Centrality Algorithm using the
/// hierarchical and hubs breadth-first search (@ref breadth_first_search_h()).
///
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param hubs The hierarchical hubs @ref graph_view over the input graph,
/// which creates a hierarchical view over all the high-degree vertices (hubs)
/// of the input graph. This is generated by calling @ref create_level_hubs()
/// on the input @ref graph_view.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView, class HubsView>
void closeness_centrality(GView& g, HView& h, HubsView& hubs)
{
  using namespace closeness_centrality_impl;
  map_func(init_closeness_wf(), g);
  size_t size = g.size();
  for (size_t i=0; i<size; ++i) {
    // Compute shortest-distance from this new source.
    breadth_first_search_h(g, h, hubs, i);
    // Add the previously computed values to farness.
    map_func(farness_add_wf(), g);
  }
  // Calculate the closeness by inverting the farness.
  map_func(compute_closeness_wf(), g);
}

} // namespace stapl
#endif
