/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_PARAGRAPH_VIEW_HPP
#define STAPL_PARAGRAPH_VIEW_HPP

#include <type_traits>

#include <stapl/utility/down_cast.hpp>
#include <stapl/runtime/type_traits/is_p_object.hpp>
#include <stapl/paragraph/paragraph_impl.hpp>
#include <stapl/paragraph/edge_container/views/edge_view.hpp>
#include <stapl/paragraph/edge_container/views/aggregated_edge_view.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Wraps a view object with a interface that makes it behave as a
///   pointer to a view of the same type would.
/// @ingroup pgView
///
/// This is used for PARAGRAPH edge views passed to @p add_task calls via
/// @p consume calls, where there is not a view copy held/owned by the PARAGRAPH
/// (i.e., those passed by user at PARAGRAPH creation).
///
/// When @ref paragraph_view::add_task pair usage is finally retired, there will
/// be no more need for this wrapper class.
///
/// @todo Default constructor should be removed.  For now, needed for
/// delazify transform for nested, out of gang migration.
///
/// @todo Aggregated subview use case may have unnecessary copy.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct ptr_wrapper
{
  View         m_vw;

  ptr_wrapper(void) = default;

  ptr_wrapper(View vw)
    : m_vw(std::move(vw))
  { }

  void define_type(typer& t)
  {
    t.member(m_vw);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Mimics the application operator* on a pointer / iterator.
  //////////////////////////////////////////////////////////////////////
  View& operator*()
  {
    return m_vw;
  }

  View const& operator*() const
  {
    return m_vw;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Mimics the application operator-> on a pointer / iterator.
  //////////////////////////////////////////////////////////////////////
  View const* operator->() const
  {
    return &m_vw;
  }
};


namespace paragraph_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Trivial view class for PARAGRAPH holding a @ref task_graph pointer.
///   Used for consume() statements and as a base class of @p Scheduler typed
///   class template @ref paragraph_view.
/// @ingroup pgView
//////////////////////////////////////////////////////////////////////
class paragraph_view_base
{
private:
  /// @brief Pointer to type erased base class of underlying PARAGRAPH.
  task_graph* m_paragraph_ptr;

public:
  task_graph& container(void) const
  {
    return *m_paragraph_ptr;
  }

  paragraph_view_base(task_graph& paragraph)
    : m_paragraph_ptr(&paragraph)
  { }

  void define_type(typer& t)
  {
    t.member(m_paragraph_ptr);
  }

protected:
  //////////////////////////////////////////////////////////////////////
  /// @note this method is implemented as a workaround of Intel 14's
  /// default destructor when the code is compiled with -O0.  The default
  /// destructor provided incorrectly zeroes out memory, resulting in
  /// seg faults in task execution of dynamic work functions.
  //////////////////////////////////////////////////////////////////////
  ~paragraph_view_base()
  { }
}; // class paragraph_view_base


//////////////////////////////////////////////////////////////////////
/// @brief View specification transformation functor used by the
///  @ref paragraph_view to include a rmi_handle::reference as well when
///  accepting and forwarding view specifications via @p add_task().
//////////////////////////////////////////////////////////////////////
template<typename V, typename = void>
struct transform_lazy_view
{
  using type = V;

  template<typename VParam>
  static VParam&& apply(VParam&& v)
  {
    return std::forward<VParam>(v);
  }
};


template<typename T1, typename T2>
struct transform_lazy_view<std::pair<T1*, T2>,
           typename std::enable_if<is_p_object<T1>::value>::type>
{
  using type = std::pair<p_object_pointer_wrapper<T1>, T2>;

  static type apply(std::pair<T1*, T2> const& v)
  {
    return type(v.first, v.second);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Provides an interface for a PARAGRAPH's factory as well as dynamic
///   tasks to add new tasks to the PARAGRAPH during execution.
/// @ingroup pgView
///
/// @tparam Scheduler The type of the scheduler that the PARAGRAPH was
/// constructed with.
///
/// Although this class only directly uses the scheduling metadata type
/// (i.e., Scheduler::sched_info_type) in its method signatures, it is
/// templated with the full Scheduler type so that the PARAGRAPH intermediate
/// base with knowledge of this type can be accessed.  This allows the
/// PARAGRAPH's internal @p add_task call to invoke the task placement policy
/// located in the scheduler object.
///
/// @todo Make scheduler template parameter optional, defaulting to
/// fifo_scheduler<default_task_placement>, to reduce symbol size.
//////////////////////////////////////////////////////////////////////
template<typename Scheduler>
class paragraph_view
  : public paragraph_view_base
{
private:
  typedef std::vector<std::size_t>            pred_list_t;
  typedef typename Scheduler::sched_info_type sched_info_type;
  typedef task_graph_impl<Scheduler>          derived_tg_t;

  //////////////////////////////////////////////////////////////////////
  /// @brief Extract a reference to the currently running PARAGRAPH from
  /// a dynamic call stack of PARAGRAPH invocations maintained in the runtime.
  ///
  /// @return A reference to an PARAGRAPH object whose type has factory and
  /// view information erased, but still maintains knowledge of the scheduler
  /// type.
  //////////////////////////////////////////////////////////////////////
public:
  derived_tg_t& graph() const
  {
    return down_cast<derived_tg_t>(this->container());
  }

public:
  paragraph_view(task_graph& paragraph)
    : paragraph_view_base(paragraph)
  { }

  void define_type(typer& t)
  {
    t.base<paragraph_view_base>(*this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param task_id   User specified identifier that other tasks and
  ///                  @p paragraph_view methods use to refer to this task.
  /// @param wf        The workfunction for this task.
  /// @param num_succs The number of consumers for this task in the PARAGRAPH.
  /// @param v         Variadic list of view inputs to workfunction.
  ///
  /// @todo The number of method signatures for @p add_task is probably bigger
  /// than it needs to be.  See if we can simplify and synthesize.
  ///
  /// @note For this and all add_task signatures, the view parameter is actually
  /// a std::pair of View* and index which will be evaluated as conceptually as
  /// *view[index] prior to being passed to the workfunction.  This allows the
  /// actual evaluation of to be delayed until the task is actual placed,
  /// hopefully with good locality, so that view creation avoids communication.
  ///
  /// @todo Everyone agrees the std::pair interface should be made simpler,
  /// but we need find a balance between clean syntax and good performance.
  /// View, container, and PARAGRAPH component owners need to work together on
  /// this one.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  void add_task(std::size_t task_id, WF const& wf, std::size_t num_succs,
                Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(
      task_id, wf, none, no_preds_t(), num_succs, LQ_LOOKUP,
      transform_lazy_view<typename std::decay<Args>::type>::
        apply(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param sched_info Scheduler metadata (e.g., priority) for task.
  /// @param task_id    User specified identifier that other tasks and
  ///                   @p paragraph_view methods use to refer to this task.
  /// @param wf         The workfunction for this task.
  /// @param num_succs  The number of consumers for this task in the PARAGRAPH.
  /// @param v          Variadic list of view inputs to workfunction.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  void add_task(sched_info_type const& si, std::size_t task_id, WF const& wf,
                std::size_t num_succs, Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(task_id, wf, si, no_preds_t(), num_succs, LQ_LOOKUP,
                           transform_lazy_view<typename std::decay<Args>
                             ::type>:: apply(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param task_id    User specified identifier that other tasks and
  ///                   @p paragraph_view methods use to refer to this task.
  /// @param wf         The workfunction for this task.
  /// @param preds      vector of predecessor task ids where there is a
  ///                   dependence to enforce but whose return value is not
  ///                   consumed on PARAGRAPH edge.
  /// @param num_succs  The number of consumers for this task in the PARAGRAPH.
  /// @param v          Variadic list of view inputs to workfunction.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  void add_task(std::size_t task_id, WF const& wf, pred_list_t preds,
                std::size_t num_succs, Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(
      task_id, wf, none, preds, num_succs, LQ_LOOKUP,
      transform_lazy_view<typename std::decay<Args>::type>::
        apply(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param sched_info Scheduler metadata (e.g., priority) for task.
  /// @param task_id    User specified identifier that other tasks and
  ///                   @p paragraph_view methods use to refer to this task.
  /// @param wf         The workfunction for this task.
  /// @param preds      vector of predecessor task ids where there is a
  ///                   dependence to enforce but whose return value is not
  ///                   consumed on PARAGRAPH edge.
  /// @param num_succs  The number of consumers for this task in the PARAGRAPH.
  /// @param v          Variadic list of view inputs to workfunction.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  void add_task(sched_info_type const& si, std::size_t task_id,
                WF const& wf, pred_list_t preds, std::size_t num_succs,
                Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(
      task_id, wf, si, preds, num_succs, LQ_LOOKUP,
      transform_lazy_view<typename std::decay<Args>::type>::
        apply(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param wf         The workfunction for this task.
  ///                   dependence to enforce but whose return value is not
  ///                   consumed on PARAGRAPH edge.
  /// @param v          Variadic list of view inputs to workfunction.
  ///
  /// @return PARAGRAPH generated task id guaranteed to be globally unique.
  ///
  /// @remark The created task implicitly has no consumers.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  typename std::enable_if<
    !std::is_integral<WF>::value && !std::is_same<WF, sched_info_type>::value,
    void
  >::type
  add_task(WF const& wf, Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(0, wf, none, no_preds_t(), 0, LQ_LOOKUP,
                           transform_lazy_view<typename std::decay<Args>
                             ::type>::apply(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Insert a new task in the currently running PARAGRAPH.
  ///
  /// @param sched_info Scheduler metadata (e.g., priority) for task.
  /// @param wf         The workfunction for this task.
  ///                   dependence to enforce but whose return value is not
  ///                   consumed on PARAGRAPH edge.
  /// @param v          Variadic list of view inputs to workfunction.
  ///
  /// @return PARAGRAPH generated task id guaranteed to be globally unique.
  ///
  /// @remark The created task implicitly has no consumers.
  //////////////////////////////////////////////////////////////////////
  template<typename WF, typename... Args>
  typename std::enable_if<!std::is_integral<WF>::value, void>::type
  add_task(sched_info_type const& si, WF const& wf, Args const&... args) const
  {
    this->graph().count_task();

    this->graph().add_task(0, wf, si, no_preds_t(), 0, LQ_LOOKUP,
                           transform_lazy_view<typename std::decay<Args>
                             ::type>::apply(args)...);
  }

  p_object const& no_succs_p_object()
  {
    return this->graph().no_succs_p_object();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Allows the consumer count for a producer task to be specified
  /// independent of its @p add_task invocation.
  ///
  /// @param task_id   Task identifier of task to have consumer count set.
  /// @param num_succs The consumer count for the task.
  ///
  /// @remark The corresponding @p add_task must be passed the defer_spec
  /// constant, denoting that the successor count will be separately specified
  /// by a call to this method.
  //////////////////////////////////////////////////////////////////////
  void set_num_succs(std::size_t task_id, std::size_t num_succs) const
  {
    this->graph().set_num_succs(task_id, num_succs);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Sets the PARAGRAPH task that will be the return value for the
  /// PARAGRAPH from this location.  Can be called by a factory or any task in
  /// the PARAGRAPH with a @p paragraph_view.
  ///
  /// @param task_id Task identifier of task to be used as a result
  //////////////////////////////////////////////////////////////////////
  void set_result(std::size_t task_id) const
  {
    this->graph().set_result(task_id);
  }

  void set_result(std::size_t index, std::size_t task_id) const
  {
    this->graph().set_result(index, task_id);
  }

  constexpr bool is_local(void) const
  { return true; }
}; // class paragraph_view

} // namespace paragraph_impl


namespace result_of {

//////////////////////////////////////////////////////////////////////
/// @brief Defines return type @p consume signature in single predecessor case.
/// @ingroup pgView
//////////////////////////////////////////////////////////////////////
template<typename T, typename... OptionalFilterParam>
struct consume
{
  typedef std::pair<
    ptr_wrapper<
      edge_view<T, typename std::decay<OptionalFilterParam>::type...>>,
    std::size_t
  > type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Defines return type @p consume signature for aggregated, multiple
///   edge consumption and user provided filter function.
/// @ingroup pgView
//////////////////////////////////////////////////////////////////////
template<typename T, typename... OptionalFilterParam>
struct aggregated_consume
{
  typedef aggregated_edge_view<
    T, typename std::decay<OptionalFilterParam>::type...> edge_view_t;
  typedef typename edge_view_t::index_type                index_type;
  typedef std::pair<ptr_wrapper<edge_view_t>, index_type> type;
};

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief Creates a parameter passed as a view argument to @p add_task
///   invocations, denoting that the task should consume the value produced
///   by another task.
/// @ingroup pgView
///
/// @tparam The return type of the consumed task.  Not deduced, must be
///         explicitly specified.
///
/// @param pgv    A view of the paragraph the consumption edge is created in.
////
/// @param tid The task identifier of the predecessor task.
///
/// @param filter Optional function object to apply to the predecessor task's
///               result prior to passing it the consumer task's workfunction.
///
/// @return std::pair that implements View/Index concept expected by @p add_task
///
/// Example Usage:
///
/// pgv.add_task(tid, wf, consume<T>(tid)); // wf is unary in this case
///
/// Logically, this should be part of the @ref paragraph_view class.
/// However, since it would then be a member template of class template, the
/// explicit use of the template keyword when calling it would be required
/// (i.e., pgv. template consume<T>(tid)), which is just unacceptable.  Hence,
/// it is a freestanding function.
///
/// The real work of consume (setting up the actual data flow in the PARAGRAPH)
/// is implemented by the @ref edge_container::setup_flow which is called on the
/// @p add_task code path, after some type metafunctions to detect consumption
/// has been requested.
///
/// The motivation behind this filtering (instead of just having the caller
/// wrap their workfunction with another to apply this transformation is that
/// the PARAGRAPH attempts to push the filter to the producing location,
/// applying before value transmission, so that no unneeded bits are
/// communicated.
//////////////////////////////////////////////////////////////////////
template<typename T, typename... OptionalFilterParam>
std::pair<
  ptr_wrapper<edge_view<T, typename std::decay<OptionalFilterParam>::type...>>,
  std::size_t>
consume(paragraph_impl::paragraph_view_base const& pgv,
        std::size_t tid,
        OptionalFilterParam&&... filter)
{
  using res_vw_t =
    edge_view< T, typename std::decay<OptionalFilterParam>::type...>;

  return std::pair<ptr_wrapper<res_vw_t>, std::size_t>(
    ptr_wrapper<res_vw_t>(
      res_vw_t(std::forward<OptionalFilterParam>(filter)...)),
    tid);
}


//////////////////////////////////////////////////////////////////////
/// @brief Sets up consumption as the basic @p consume call, except that
///   multiple predecessors task_ids can be specified.  Their results will be
///   presented to consumer task as a view implementing the array_view concept.
/// @ingroup pgView
///
/// @tparam The return type of the consumed tasks (must be the same).
/// Not deduced, must be explicitly specified.
///
/// @param pgv    A view of the paragraph the consumption edge is created in.
///
/// @param tids   List in a std::vector of task identifiers of predecessors to
///               consume.
///
/// @param filter Optional function object to apply to the predecessor task's
///               result prior to passing it the consumer task's workfunction.
///
/// @return std::pair that implements View/Index concept expected by
/// @p add_task.  The consumer tasks receives an array_view whose elements are
/// the
///
/// Example usage:
///
/// class wf
/// {
///   template<typename View>
///   void operator()(View values) const
///   {
///     x = values[0]; // x assigned result of task 5.
///   }
/// };
///
/// in factory::operator():
///
/// vector<size_t> tids;
/// tids.push_back(5);
/// tids.push_back(3);
/// pgv.add_task(task_id, wf, consume<T>(tids)); // new task has 2 preds.
///
/// @todo Generalize std::vector input parameter to be templated, allowing
/// anything implementing the array_view concept.
//////////////////////////////////////////////////////////////////////
template<typename T, typename TaskIdsRef, typename... OptionalFilterParam>
inline
typename std::enable_if<
  !std::is_integral<typename std::decay<TaskIdsRef>::type>::value,
  typename result_of::aggregated_consume<T, OptionalFilterParam...>::type
>::type
consume(paragraph_impl::paragraph_view_base const& pgv,
        TaskIdsRef&& tids,
        OptionalFilterParam&&... filter)
{
  using result_mf_t =
    typename result_of::aggregated_consume<T, OptionalFilterParam...>;

  using edge_view_t = typename result_mf_t::edge_view_t;
  using ret_t       = typename result_mf_t::type;

  return ret_t(ptr_wrapper<edge_view_t>(
                 edge_view_t(&pgv.container(),
                             std::forward<OptionalFilterParam>(filter)...)),
               std::forward<TaskIdsRef>(tids));
}


//////////////////////////////////////////////////////////////////////
/// @brief View class cused as input to a nested paragraph created
/// via @p add_task to setup dataflow from a producer, sibling paragraph.
//////////////////////////////////////////////////////////////////////
template<typename T>
class nested_pg_view_subview
{
private:
  /// @brief The task id of the producer paragraph in the parent paragraph.
  std::size_t                              m_pred_tid;

  /// @brief Shared pointer to an edge container backing this view, created
  /// lazy in @ref initialize flow during construction of nested paragraph
  /// after transport to enclosing gang.
  mutable std::shared_ptr<edge_container>  m_ct_sptr;

  using base_view = edge_view<T>;

public:
  using reference  = typename base_view::reference;
  using index_type = typename base_view::index_type;

  nested_pg_view_subview(std::size_t pred_tid)
    : m_pred_tid(pred_tid)
  { }

  bool validate(void) const
  { return true; }

  void define_type(typer& t)
  {
    t.member(m_pred_tid);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Called in constructor of nested @ref paragraph.  Create
  /// @ref edge_container to hold in flow from ports.  If the first location
  /// in nested section, initiate creation of edge from producer via the parent
  /// paragraph's directory of nested_pg handles.
  //////////////////////////////////////////////////////////////////////
  void initialize_flow(task_graph& tg, bool b_persistent) const
  {
    auto tmp = std::make_shared<edge_container>(false, &tg, b_persistent);

    m_ct_sptr.swap(tmp);

    if (tg.get_location_id() != 0)
      return;

    auto* parent_pg = resolve_handle<task_graph>(tg.parent_handle());

    stapl_assert(parent_pg != nullptr, "Failed pg handle resolution");

    gang g(*parent_pg);

    parent_pg->nested_pgs().setup_flow<T>(
      m_pred_tid, m_ct_sptr->get_rmi_handle());
  }

  edge_container& edges(void) const
  { return *m_ct_sptr; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Called in consuming paragraph's factory (or another dynamic task)
  /// to initialize out-degree / fan out of each input port for this view.
  //////////////////////////////////////////////////////////////////////
  void set_num_succs(index_type producer_tid, size_t num_succs) const
  {
    m_ct_sptr->set_num_succs(producer_tid, num_succs);
  }
}; // class nested_pg_view_subview


//////////////////////////////////////////////////////////////////////
/// @brief Lighweight view class created by @p consume and passed to
/// @ref add_task to facilitate setup of inter paragraph, port-based
/// dataflow.
//////////////////////////////////////////////////////////////////////
template<typename T>
class nested_pg_view
{
private:
  rmi_handle::reference m_parent_pg_handle;

public:
  using reference               = nested_pg_view_subview<T>;
  using index_type              = std::size_t;
  using task_placement_dontcare = std::true_type;

  nested_pg_view(rmi_handle::reference parent_pg_handle)
    : m_parent_pg_handle(parent_pg_handle)
  { }

  rmi_handle::reference nested_locality(size_t) const
  {
    return m_parent_pg_handle;
  }

  reference transporter_reference(index_type idx) const
  {
    return reference(idx);
  }

  reference operator[](index_type idx) const
  {
    stapl_assert(0, "nested_pg_view::operator[] unexpectedly called");
    return reference(idx);
  }
}; // class nested_pg_view


//////////////////////////////////////////////////////////////////////
/// @brief Call to allow consumption from a nested paragraph by a sibling
/// paragraph.  Called in the parent paragraph's factory.
//////////////////////////////////////////////////////////////////////
template<typename T>
std::pair<ptr_wrapper<nested_pg_view<T>>, std::size_t>
consume_pg(paragraph_impl::paragraph_view_base const& pgv, std::size_t tid)
{
  return std::pair<ptr_wrapper<nested_pg_view<T>>, std::size_t>(
    nested_pg_view<T>(pgv.container().get_rmi_handle()), tid);
}


//////////////////////////////////////////////////////////////////////
/// @brief Signature for task consumption specialized for consumption from
/// input port (i.e., flow from another paragraph).
//////////////////////////////////////////////////////////////////////
template<typename T>
std::pair<ptr_wrapper<pg_edge_view<T>>, std::size_t>
consume(nested_pg_view_subview<T> const& view, std::size_t tid)
{
  using view_t   = pg_edge_view<T>;
  using return_t = std::pair<ptr_wrapper<view_t>, std::size_t>;

  return return_t(ptr_wrapper<view_t>(view_t(view.edges())), tid);
}


//////////////////////////////////////////////////////////////////////
/// @brief Implementation functor of @ref pg_edge_unpackager using
/// partial specialization to call initialize_flow() on objects
/// from instances of the class template nested_pg_view_subview.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct pg_edge_initialize
{
  static void apply(View const&, task_graph&, bool)
  { }
};


template<typename T>
struct pg_edge_initialize<nested_pg_view_subview<T>>
{
  static void apply(nested_pg_view_subview<T> const& v,
                    task_graph& tg,
                    bool b_persistent)
  {
    v.initialize_flow(tg, b_persistent);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Called in view initialization of @ref paragraph constructor
/// to initialize inter-paragraph dataflow with producer paragraphs.
//////////////////////////////////////////////////////////////////////
struct pg_edge_unpackager
{
  task_graph&  m_tg;
  bool         m_b_persistent;

  pg_edge_unpackager(task_graph& tg, bool b_persistent)
    : m_tg(tg), m_b_persistent(b_persistent)
  { }

  template<typename V>
  V&& operator()(V&& v) const
  {
    pg_edge_initialize<typename std::decay<V>::type>::apply(
      v, m_tg, m_b_persistent);

    return std::forward<V>(v);
  }
};

} // namespace stapl

#endif // ifndef STAPL_PARAGRAPH_VIEW_HPP
