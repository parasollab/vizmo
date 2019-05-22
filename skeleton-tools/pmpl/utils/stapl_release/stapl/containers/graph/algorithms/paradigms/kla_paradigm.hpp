/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_GRAPH_ALGORITHMS_PARADIGMS_KLA_PARADIGM_HPP
#define STAPL_CONTAINERS_GRAPH_ALGORITHMS_PARADIGMS_KLA_PARADIGM_HPP

#include <stapl/containers/graph/algorithms/paradigms/vertex_operator.hpp>
#include <stapl/containers/graph/algorithms/paradigms/neighbor_operator.hpp>
#include <stapl/algorithms/functional.hpp>
#include <stapl/views/repeated_view.hpp>
#include <stapl/skeletons/map_reduce_sched.hpp>

namespace stapl {

namespace kla_detail {

//////////////////////////////////////////////////////////////////////
/// @brief Empty function that can be used as either a null pre-execute
///        or null post-execute
/// @ingroup pgraphAlgoDetails
//////////////////////////////////////////////////////////////////////
struct empty_prepost_execute
{
  template<typename GView>
  void operator()(GView, size_t)
  { }
};

} // end namespace kla_detail


//////////////////////////////////////////////////////////////////////
/// @brief Optional parameters that controls the behavior of KLA, including
///        enabling hub avoidance and providing post/pre execute functors.
///
/// @tparam View The graph view
//////////////////////////////////////////////////////////////////////
template<typename View>
struct kla_params
{
  /// Whether or not employ hub avoidance
  bool avoid_hubs;

  /// For hub avoidance, the threshold for a vertex's degree after which
  /// it will be considered a hub
  std::size_t degree_threshold;

  /// A functor that will be executed before every KLA superstep, which
  /// will receive a view of the graph and the current level
  std::function<void(View, std::size_t)> pre_execute;

  /// A functor that will be executed after every KLA superstep, which
  /// will receive a view of the graph and the current level
  std::function<void(View, std::size_t)> post_execute;

  kla_params(void) : avoid_hubs(false),
                     degree_threshold(std::numeric_limits<std::size_t>::max()),
                     pre_execute(kla_detail::empty_prepost_execute()),
                     post_execute(kla_detail::empty_prepost_execute())
  { }
};


namespace kla_detail {

//////////////////////////////////////////////////////////////////////
/// @brief Internal implementation of the KLA paradigm @see kla_paradigm
///
/// @param wf The wrapped vertex-operator
/// @param scheduler The PARAGRAPH's scheduler
/// @param pre_execute Functor that will be executed on the
/// @ref graph_view before each KLA-SS. This will be invoked with
/// the input @ref graph_view and the last KLA-SS ID.
/// @param post_execute Optional functor that will be executed on the
/// @ref graph_view at the end of each KLA-SS. This will be invoked with
/// the input @ref graph_view and the current KLA-SS ID (the ID of the
/// KLA-SS that just finished).
/// @param g The @ref graph_view over the input graph.
/// @param k The maximum amount of asynchrony allowed in each phase.
/// 0 <= k <= inf.
/// k == 0 implies level-synchronous KLA.
/// k >= D implies fully asynchronous KLA (D is the number of iterations
/// performed by the level-synchronous variant of the algorithm).
/// @param params A struct of optional parameters that can control the behavior
/// of execution
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename InternalWF, typename UF, typename Scheduler,
         typename PreExecute, typename PostExecute, typename GView>
size_t kla_paradigm_impl(InternalWF wf, UF const&,
                         Scheduler const& scheduler, PreExecute pre_execute,
                         PostExecute post_execute,
                         GView& g, size_t k, kla_params<GView> const& params)
{
  stapl_assert(k < std::numeric_limits<std::size_t>::max(),
    "k needs to be less than the maximum allowed value for std::size_t"
  );

  size_t KLA_SS = 0;
  size_t num_iter = 0;

  while (
    pre_execute(g, KLA_SS),
    map_reduce_sched(scheduler, wf, plus<bool>(), g, make_repeat_view(g)) != 0
  )
  {
    post_execute(g, KLA_SS);

    //now change the propagation trigger
    wf.increment_iteration(k);
    KLA_SS += k;
    ++num_iter;
  }

  return num_iter;
}

} // namespace kla_detail


//////////////////////////////////////////////////////////////////////
/// @brief The Parallel k-Level-Asynchronous (KLA) Paradigm.
///
/// Implements the KLA paradigm, which iteratively executes KLA-Supersteps
/// (KLA-SS). Each KLA-SS can perform asynchronous visits up to 'k' levels deep.
/// Any communication generated within a KLA-SS is guaranteed to have finished
/// before the KLA-SS finishes.
/// The user provides a vertex-operator to express the computation to be
/// performed on each vertex, and a neighbor-operator that will be applied
/// to each neighbor that is visited.
/// The vertex-operator is passed in a vertex and a visit object. To visit a
/// neighboring vertex, the vertex-operator must call
/// visit(neighbor, neighbor-operator()).
/// The vertex-operator must return true if the vertex was active
/// (i.e. its value was updated), or false otherwise.
/// The neighbor-operator is passed in the target vertex. Neighbor-operators
/// may carry state, but must be immutable. They should return true if the
/// visit was successful (i.e. the target vertex will be activated after this
/// visit), or false otherwise.
/// Users may also provide additional functions to be executed before and after
/// each KLA-SS.
/// @tparam WF The type of the user provided vertex-operator expressing
/// computation to be performed over each vertex.
/// @tparam UF The type of the user provided neighbor-operator expressing
/// computation to be performed over neighboring vertices.
/// @param uwf Functor that implements the operation to be performed
/// on each vertex
/// @param post_execute Optional functor that will be executed on the
/// @ref graph_view at the end of each KLA-SS. This will be invoked with
/// the input @ref graph_view and the current KLA-SS ID (the ID of the
/// KLA-SS that just finished).
/// @param g The @ref graph_view over the input graph.
/// @param k The maximum amount of asynchrony allowed in each phase.
/// 0 <= k <= inf.
/// k == 0 implies level-synchronous KLA.
/// k >= D performed implies fully asynchronous KLA (D is the number of
/// iterations by the level-synchronous variant of the algorithm).
/// @return The number of iterations performed by the paradigm.
/// @ingroup pgraphAlgo
//////////////////////////////////////////////////////////////////////
template<typename WF, typename UF, typename Scheduler,
         typename GView>
size_t kla_paradigm(WF const& uwf, UF const&,
                    Scheduler const& scheduler, GView& g, size_t k = 0,
                    kla_params<GView> const& params = kla_params<GView>())
{
  const bool hub_avoidance = params.avoid_hubs;

  const std::function<void(GView, std::size_t)> pre = params.pre_execute;
  const std::function<void(GView, std::size_t)> post = params.post_execute;

  // Employ hub avoidance. When a neighbor operator is applied to a vertex
  // whose out-degree exceeds a user defined threshold, the vertex operator
  // will not be applied to this vertex, even if the neighbor operator
  // flags that it should. Instead, it will be delayed until the next KLA-SS
  if (hub_avoidance)
  {
    typedef kla_detail::vertex_operator_apply<true, GView, WF> wf_t;

    wf_t wf(uwf, 1, 1+k, params.degree_threshold);

    return kla_paradigm_impl(wf, UF(), scheduler, pre, post, g, k, params);

  }
  // Do not perform hub avoidance
  else
  {
    typedef kla_detail::vertex_operator_apply<false, GView, WF> wf_t;

    wf_t wf(uwf, 1, 1+k);

    return kla_paradigm_impl(wf, UF(), scheduler, pre, post, g, k, params);

  }
}


//////////////////////////////////////////////////////////////////////
/// @brief The Parallel k-Level-Asynchronous (KLA) Paradigm.
///
/// Overload of @ref kla_paradigm with an empty post-execute.
//////////////////////////////////////////////////////////////////////
template<typename WF, typename UF, typename GView>
size_t kla_paradigm(WF const& uwf, UF const&, GView& g, size_t k = 0,
                    kla_params<GView> const& params = kla_params<GView>())
{
  return kla_paradigm(uwf, UF(), default_scheduler(), g, k, params);
}


} // namespace stapl

#endif
