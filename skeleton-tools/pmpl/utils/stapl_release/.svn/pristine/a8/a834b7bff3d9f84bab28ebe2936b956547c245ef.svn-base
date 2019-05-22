/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_GENERATORS_ERDOS_RENYI_HPP
#define STAPL_CONTAINERS_GRAPH_GENERATORS_ERDOS_RENYI_HPP

#include <stapl/containers/graph/generators/generator.hpp>

namespace stapl {

namespace generators {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Functor which adds edges to form an Erdos-Renyi random network.
//////////////////////////////////////////////////////////////////////
struct erdos_renyi_neighbors
  : protected rand_gen
{
  size_t m_size;
  double m_prob;
  bool   m_bidirectional;

  typedef void result_type;

  //////////////////////////////////////////////////////////////////////
  /// @param size Number of vertices in the output graph.
  /// @param prob The probability of adding any given edge.
  /// @param bidirectional True to add back-edges in a directed graph, false
  ///   for forward edges only.
  /// @param seed The seed for random-number generation.
  //////////////////////////////////////////////////////////////////////
  erdos_renyi_neighbors(size_t size, double prob, bool bidirectional,
                        unsigned int seed = get_location_id())
    : rand_gen(seed), m_size(size), m_prob(prob*100),
      m_bidirectional(bidirectional)
  { }

  template<typename Vertex, typename Graph>
  void operator()(Vertex v, Graph& view)
  {
    // Each vertex v attempts to add edges to all vertices u where u > v.
    // The probability of adding an edge is m_prob.
    for (size_t i = v.descriptor()+1; i < m_size; i++) {
      if (this->rand(100) <= m_prob) {
        view.add_edge_async(v.descriptor(), i);
        if (view.is_directed() && m_bidirectional)
          view.add_edge_async(i, v.descriptor());
      }
    }
  }

  void define_type(typer& t)
  {
    t.member(m_size);
    t.member(m_prob);
    t.member(m_bidirectional);
  }
};

}


//////////////////////////////////////////////////////////////////////
/// @brief Generates an Erdos-Renyi random network.
/// @param g A view over the graph to generate.
/// @param size Number of vertices in the output graph.
/// @param prob The probability of adding any given edge.
/// @param bidirectional True to add back-edges in a directed graph, false
///   for forward edges only.
/// @return The original view, now containing the generated graph.
///
/// Generates an Erdos-Renyi random network where the probability of adding
/// an edge u -> v is p, where u > v.
///
/// This function mutates the input graph.
//////////////////////////////////////////////////////////////////////
template <typename GraphView>
GraphView make_erdos_renyi(GraphView& g, size_t size, double prob,
                           bool bidirectional=true)
{
  typedef typename detail::erdos_renyi_neighbors ef_t;
  return make_generator<GraphView, ef_t>(g, size, ef_t(size, prob,
                                                       bidirectional))();
}


//////////////////////////////////////////////////////////////////////
/// @brief @copybrief make_erdos_renyi
/// @param size Number of vertices in the output graph.
/// @param prob The probability of adding any given edge.
/// @param bidirectional True to add back-edges in a directed graph, false
///   for forward edges only.
/// @return A view over the generated graph.
///
/// The returned view owns its underlying container.
/// @see make_erdos_renyi
//////////////////////////////////////////////////////////////////////
template <typename GraphView>
GraphView make_erdos_renyi(size_t size, double prob, bool bidirectional=true)
{
  typedef typename detail::erdos_renyi_neighbors ef_t;
  return make_generator<GraphView, ef_t>(size,
                                         ef_t(size, prob, bidirectional))();
}


} // namespace generators

} // namespace stapl

#endif
