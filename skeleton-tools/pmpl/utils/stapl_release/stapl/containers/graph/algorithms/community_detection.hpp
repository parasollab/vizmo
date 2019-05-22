/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_COMMUNITY_DETECTION_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_COMMUNITY_DETECTION_HPP

#include <stapl/containers/graph/algorithms/paradigms/graph_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_paradigm.hpp>
#include <stapl/containers/graph/algorithms/paradigms/h_hubs_paradigm.hpp>

namespace stapl {

namespace community_detection_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Function to compute the most frequent label occurring in a
/// set of labels.
///
/// @param labels The comtainer of labels.
/// @param vd Any initial label.
/// @return The most frequent label.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
template<typename C, typename VD>
VD compute_most_frequent_label(C const& labels, VD const& vd)
{
  std::map<VD, size_t> counts;
  auto min_label = vd;
  for (auto const& e : labels) {
    counts[e]++;
    if (e < vd)
      min_label = e;
  }
  size_t max_count = 1;
  VD most_frequent = min_label;
  for (auto const& x : counts)
    if (max_count < x.second) {
      max_count = x.second;
      most_frequent = x.first;
    }

  return most_frequent;
}


//////////////////////////////////////////////////////////////////////
/// @brief Reducer functor for @ref community_detection().
///
/// Reduces two community detection properties to update the first one.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct vp_reducer
{
  template<typename VP1, typename VP2>
  void operator()(VP1& p1, VP2& p2) const
  {
    std::stringstream ss;
    for (auto const& e : p2.label_vector())
      p1.label_vector_add(e);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor to add the value of the incomming label to the
/// target vertex's auxiliary labels.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct update_func
{
  typedef bool result_type;
  typedef size_t vd_type;

  vd_type m_label;

  update_func(vd_type const& label = std::numeric_limits<vd_type>::max())
    : m_label(label)
  { }

  template <class Vertex>
  bool operator()(Vertex target) const
  {
    target.property().label_vector_add(m_label);
    return true;
  }

  void define_type(typer& t)
  { t.member(m_label); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Work-function to compute the label of a vertex and push it
/// to the vertex's neighbors.
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct community_detection_wf
{
  size_t m_max_iter;

  community_detection_wf(size_t max_iter)
    : m_max_iter(max_iter)
  { }

  typedef bool result_type;

  template<typename Vertex, typename GraphVisitor>
  bool operator()(Vertex v, GraphVisitor graph_visitor)
  {
    if (graph_visitor.level() == 1) {
      v.property().label(v.descriptor());
    }
    if (graph_visitor.level() % 2 == 0) {
      auto most_frequent_label
        = compute_most_frequent_label(v.property().label_vector(),
                                      v.descriptor());

      v.property().label_vector_clear();
      bool active;
      if (most_frequent_label == v.property().label()) {
        active = false;
      } else {
        v.property().label(most_frequent_label);
        active = true;
      }
      return active && graph_visitor.level() <= m_max_iter;
    } else if (graph_visitor.level() % 2 == 1) {
      graph_visitor.visit_all_edges(v, update_func(v.property().label()));
      // return true if the label was changed, and the iterations are
      // less than the maximum allowed.
      return true;
    }
    return true;
  }

  void define_type(typer& t)
  { t.member(m_max_iter); }
};

}; // namespace community_detection_impl;


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized Community Detection Algorithm
/// based on modularity maximization.
///
/// Performs a variant of label-propagation on the input @ref graph_view,
/// to find the most frequent label in each vertex's neighborhood, which
/// is set as its community on its property.
/// @param graph The @ref graph_view over the input graph.
/// @param max_iter The maximum number of iterations allowed to execute.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename GView>
size_t community_detection(GView& graph, size_t max_iter = 20)
{
  using namespace community_detection_impl;
  return level_sync_paradigm(community_detection_wf(max_iter), update_func(),
                             graph);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized Community Detection Algorithm
/// based on modularity maximization, using the hierarchical machine
/// paradigm (@ref h_paradigm()).
///
/// Performs a variant of label-propagation on the input @ref graph_view,
/// to find the most frequent label in each vertex's neighborhood, which
/// is set as its community on its property.
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param max_iter The maximum number of iterations allowed to execute.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView>
size_t community_detection(GView& g, HView& h, size_t max_iter = 20)
{
  using namespace community_detection_impl;
  return h_paradigm(community_detection_wf(max_iter), update_func(), h, g);
}


//////////////////////////////////////////////////////////////////////
/// @brief Parallel Level-Synchronized Community Detection Algorithm
/// based on modularity maximization, using the hierarchical machine
/// and hubs paradigm (@ref h_hubs_paradigm()).
///
/// Performs a variant of label-propagation on the input @ref graph_view,
/// to find the most frequent label in each vertex's neighborhood, which
/// is set as its community on its property.
/// @param g The @ref graph_view over the input graph.
/// @param h The hierarchical machine @ref graph_view over the input graph.
/// This is generated by calling @ref create_level_machine() on the input
/// @ref graph_view.
/// @param hubs The hierarchical hubs @ref graph_view over the input graph,
/// which creates a hierarchical view over all the high-degree vertices (hubs)
/// of the input graph. This is generated by calling @ref create_level_hubs()
/// on the input @ref graph_view.
/// @param max_iter The maximum number of iterations allowed to execute.
/// @return The number of iterations performed.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<class GView, class HView, class HubsView>
size_t community_detection(GView& g, HView& h, HubsView& hubs,
                           size_t max_iter = 20)
{
  using namespace community_detection_impl;
  return h_hubs_paradigm(community_detection_wf(max_iter), update_func(),
                         vp_reducer(), h, hubs, g);
}


} // namespace stapl
#endif
