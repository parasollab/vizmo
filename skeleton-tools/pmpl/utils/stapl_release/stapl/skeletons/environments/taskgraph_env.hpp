/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_ENVIRONMENTS_TASKGRAPH_HPP
#define STAPL_SKELETONS_ENVIRONMENTS_TASKGRAPH_HPP

#include <stapl/views/metadata/localize_object.hpp>
#include <stapl/views/localize_element.hpp>
#include <stapl/skeletons/utility/factory_add_task_helper.hpp>
#include <stapl/skeletons/flows/producer_info.hpp>
#include <stapl/skeletons/executors/paragraph_skeleton_manager_fwd.hpp>
#include <stapl/skeletons/transformations/wrapped_skeleton.hpp>
#include <stapl/runtime/executor/scheduler/sched.hpp>
#include <stapl/paragraph/paragraph_view.hpp>

namespace stapl {
namespace skeletons {

//////////////////////////////////////////////////////////////////////
/// @brief The main environment for spawning skeletons in STAPL and
/// evaluating them as taskgraphs is @c taskgraph_env. It
/// gets the workfunction, unwraps the inputs which are given as
/// @c producer_info (@c indexed_producer_info, @c view_producer_info,
/// etc.) and calls @c add_task or @c set_num_succs from the given
/// @c PARAGRAPH via the given taskgraph view (@c TGV)
///
/// @tparam TGV taskgraph view that can be used to create tasks and
///             modify their metadata in STAPL
///
/// @ingroup skeletonsEnvironments
///////////////////////////////////////////////////////////////////////
template <typename TGV>
class taskgraph_env
{
  TGV                  m_tgv;
  std::size_t          m_num_PEs;
  runtime::location_id m_PE_id;

public:
  taskgraph_env(TGV tgv)
    : m_tgv(tgv),
      m_num_PEs(-1),
      m_PE_id(-1)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Spawns an element in a given taskgraph. It first unwraps
  /// the input given as various types of @c producer_info and converts
  /// them to acceptable arguments to @c PARAGRAPH's input arguments for
  /// @c add_task requests.
  ///
  /// @param tid      the unique task id assigned for this element
  /// @param wf       the workfunction to be executed by the task
  /// @param no_succs the exact number of successors for this task.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode)
  /// @param in       inputs
  ///
  /// @note The enable_if in here is required only by gcc 4.8.2 and older
  /// versions.
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename WF, typename ...In>
  typename std::enable_if<
    !skeletons::is_nested_skeleton<
       typename std::decay<WF>::type
     >::value,
    void
  >::type
  spawn_element(std::size_t tid,
                WF&& wf,
                std::size_t no_succs,
                In&&... in)
  {
    add_task_helper<typename std::decay<WF>::type>()(
      m_tgv, tid, std::forward<WF>(wf), no_succs,
      make_producer(std::forward<In>(in))...);

    set_result_task_id(std::integral_constant<bool, isResult>(), tid);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief A specialization of spawn_element for the case that the
  /// given workfunction is a skeleton itself and the skeleton is not
  /// reducing to a single element. Examples of such are @c zip(zip(op)),
  /// @c map(butterfly(op)).
  ///
  /// Internally, nested skeletons are wrapped inside the adapter class
  /// called @c wrapped_skeleton, which contains the preference for
  /// execution.
  ///
  /// @param tid      the unique task id assigned for this element
  /// @param wf       a wrapped_skeleton with a @ref nested_execution
  ///                 execution tag
  /// @param no_succs the exact number of successors for this task.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode)
  /// @param in       inputs
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename S, bool B, typename ...In>
  void spawn_element(std::size_t tid,
                     wrapped_skeleton<
                       S, tags::nested_execution<false>, B> const& wf,
                     std::size_t no_succs,
                     In&&... in)
  {
    auto&& f =  make_paragraph_skeleton_manager<null_coarsener>(
                  wf.get_skeleton());
    spawn_element<isResult>(tid, f, no_succs, std::forward<In>(in)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief A specialization of spawn_element for the case that the
  /// given workfunction is a skeleton itself and the skeleton is
  /// reducing to a single element. Examples of such are @c map(reduce(op)),
  /// @c map(zip_reduce(op)).
  ///
  /// @param tid      the unique task id assigned for this element
  /// @param wf       a wrapped_skeleton with a @ref nested_execution
  ///                 execution tag
  /// @param no_succs the exact number of successors for this task.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode)
  /// @param in       inputs
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename S, bool B, typename ...In>
  void spawn_element(std::size_t tid,
                     wrapped_skeleton<
                       S, tags::nested_execution<true>, B> const& wf,
                     std::size_t no_succs,
                     In&&... in)
  {
    typedef typename S::value_type value_type;
    auto&& f =  make_paragraph_skeleton_manager<null_coarsener, value_type>(
                  wf.get_skeleton());
    spawn_element<isResult>(tid, f, no_succs, std::forward<In>(in)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief A specialization of spawn_element for the case that the
  /// given workfunction is a skeleton that is going to be executed on
  /// the whole input (a.k.a. DOALL).
  ///
  /// @param tid      the unique task id assigned for this element
  /// @param wf       a wrapped_skeleton with a @ref doall
  ///                 execution tag
  /// @param no_succs the exact number of successors for this task.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode)
  /// @param in0      index input for doall execution
  /// @param in       inputs
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename S, bool B, typename In0, typename ...In>
  void spawn_element(std::size_t tid,
                     wrapped_skeleton<
                       S, tags::doall_execution<false>, B> const& wf,
                     std::size_t no_succs,
                     In0 in0,
                     In&&... in)
  {
    auto&& pr = make_producer(std::forward<In0>(in0));
    using View = typename std::decay<decltype(*pr.first)>::type;
    auto&& transformed_sk = transform<tags::doall>(
                              wf.get_skeleton(),
                              detail::make_reference<View>()(
                                *pr.first, pr.second));
    auto&& f =  make_paragraph_skeleton_manager<null_coarsener>(
                  transformed_sk);
    spawn_element<isResult>(tid, f, no_succs, std::forward<In>(in)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief A specialization of spawn_element where the notification
  /// dependencies are also specified (@c notifications).
  ///
  /// @param tid      the unique task id assigned for this element
  /// @param notifications the list of task ids which this task depends on
  /// @param wf       the workfunction to be executed by the task
  /// @param no_succs the exact number of successors for this task.
  ///                 Remember that in some cases this value is set to
  ///                 @c stapl::defer_specs (when the @c spawner is in
  ///                 @c SET_HOLD mode)
  /// @param in       inputs
  //////////////////////////////////////////////////////////////////////
  template <bool isResult, typename WF, typename ...In>
  void spawn_element(std::size_t tid,
                     std::vector<std::size_t> const& notifications,
                     WF&& wf, std::size_t no_succs,
                     In&&... in)
  {
    add_task_helper<typename std::decay<WF>::type>()(
      m_tgv, tid, std::forward<WF>(wf), notifications, no_succs,
      make_producer(std::forward<In>(in))...);
    set_result_task_id(std::integral_constant<bool, isResult>(), tid);
  }

  template <typename... Args>
  void pre_spawn(Args&&...) const
  { }

  template <typename... Args>
  void post_spawn(Args&&...) const
  { }

  void set_num_succs(std::size_t tid, std::size_t no_succs) const
  {
    m_tgv.set_num_succs(tid, no_succs);
  }

  void init_location_info(std::size_t num_PEs,
                          runtime::location_id loc_id)
  {
    m_num_PEs = num_PEs;
    m_PE_id = loc_id;
  }

  std::size_t get_num_PEs() const
  {
    return m_num_PEs;
  }

  runtime::location_id get_PE_id() const
  {
    return m_PE_id;
  }

private:
  void set_result_task_id(std::true_type, std::size_t tid)
  {
    m_tgv.set_result(tid);
  }

  void set_result_task_id(std::false_type, std::size_t tid)
  { }

  template <typename V, typename I>
  typename stapl::result_of::consume<
             typename flows::indexed_producer_info<
               V, I, skeletons::no_filter
             >::value_type
           >::type
  make_producer(flows::indexed_producer_info<
                  V, I, skeletons::no_filter> const& producer) const
  {
    return consume<V>(m_tgv, producer.get_index());
  }

  template <typename V, typename I, typename F>
  typename stapl::result_of::consume<
             typename flows::indexed_producer_info<V, I, F>::value_type,
             typename flows::indexed_producer_info<V, I, F>::filter_type>::type
  make_producer(flows::indexed_producer_info<V, I, F> const& producer) const
  {
    return consume<V>(m_tgv,
                      producer.get_index(),
                      producer.get_filter());
  }

  template <typename V, typename I, typename F>
  typename stapl::result_of::aggregated_consume<
             typename flows::indexed_producer_info<V, I, F>::value_type,
             typename flows::indexed_producer_info<V, I, F>::filter_type>::type
  make_producer(flows::indexed_producers_info<V, I, F> const& producer) const
  {
    return consume<V>(m_tgv,
                      producer.get_indices(),
                      producer.get_filter());
  }

  template <typename T, typename A>
  localize_element<
    typename flows::view_producer_info<proxy<T, A> >::value_type::value_type,
    std::size_t,
    stub_dontcare<
      typename flows::view_producer_info<proxy<T, A> >::value_type::value_type,
      std::size_t> >
  make_producer(flows::view_producer_info<proxy<T, A> > const& producer) const
  {
    return localize_ref(
             producer.get_element().operator[](producer.get_index()));
  }

  template <typename View>
  std::pair<View*, typename flows::view_producer_info<View>::index_type>
  make_producer(flows::view_producer_info<View> const& producer) const
  {
    typedef std::pair<
      View*,
      typename flows::view_producer_info<View>::index_type> producer_t;
    return producer_t(&producer.get_element(), producer.get_index());
  }

  template <typename Element>
  localize_element<Element,std::size_t,stub_self_element<Element,std::size_t> >
  make_producer(flows::reflexive_producer_info<
                  Element, true> const& producer) const
  {
    return localize_self_element(producer.get_element());
  }

  template <typename Element>
  typename stapl::result_of::localize_object<Element, true>::type
  make_producer(flows::constant_producer_info<Element> const& producer) const
  {
    return localize_object<true>(producer.get_element());
  }
};

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_ENVIRONMENTS_TASKGRAPH_HPP
