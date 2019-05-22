/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_PARAGRAPH_PARAGRAPH_HPP
#define STAPL_PARAGRAPH_PARAGRAPH_HPP

#include <utility>
#include <algorithm>
#include <type_traits>

#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <stapl/paragraph/paragraph_fwd.h>
#include <stapl/paragraph/wf_invoke.hpp>
#include <stapl/paragraph/paragraph_impl.hpp>
#include <stapl/skeletons/explicit/task_graph_factories.h>
#include <stapl/paragraph/view_operations/view_manager.hpp>
#include <stapl/paragraph/edge_container/views/edge_view.hpp>

#include <stapl/paragraph/make_paragraph.hpp>
#include <stapl/paragraph/tasks/task.hpp>
#include <stapl/skeletons/utility/dynamic_wf.hpp>
#include <stapl/skeletons/utility/tags.hpp>

#include <stapl/runtime.hpp>
#include <stapl/runtime/executor/terminator.hpp>

#include <stapl/utility/tuple.hpp>

#include <stapl/paragraph/tg_result_view.hpp>

namespace stapl {

struct null_coarsener;


namespace detail {


//////////////////////////////////////////////////////////////////////
/// @brief Use RAII to conditionally invoke a executor drain after the body of
///   @ref paragraph::operator().
/// @ingroup paragraph
//////////////////////////////////////////////////////////////////////
struct scoped_drain_call
{
  /// @brief Boolean guards drain call in destructor.
  bool m_b_drain;

  scoped_drain_call(bool b_drain)
    : m_b_drain(b_drain)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Empty function call used in paragraph to avoid compiler warning.
  //////////////////////////////////////////////////////////////////////
  void foo()
  { }

  ~scoped_drain_call()
  {
    if (m_b_drain)
      get_executor()(execute_all);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Responsible for initializing the termination detection object that
///   the PARAGRAPH's executor employs to determine when the local computation
///   for the PARAGRAPH is completed.
/// @ingroup paragraph
///
/// Partial specialization of this class provides a mechanism to customize
/// the termination detection algorithm of a given PARAGRAPH.
///
/// @tparam Factory The factory type of the associated PARAGRAPH.  Unused in the
/// class definition, but allows class specialization based on this type.
//////////////////////////////////////////////////////////////////////
template<typename FactoryTag>
struct terminator_initializer
{
  typedef void is_default;

  /// @brief The type of the terminator object.  By default, uses
  /// @ref terminator, defined in the runtime.
  typedef terminator<int, std::plus<int>> terminator_t;

  //////////////////////////////////////////////////////////////////////
  /// @brief Signature function operator that creates a termination
  ///   detection object in the paragraph constructor.
  ///
  /// @param tg The associated PARAGRAPH instance.
  /// @param svs The post data coarsening input views of the PARAGRAPH.
  /// @param t Uninitialized terminator stored in paragraph.  Initialized
  /// in this call as desired.
  //////////////////////////////////////////////////////////////////////
  template<typename SVS>
  void
  operator()(task_graph& tg, SVS& svs, boost::optional<terminator_t>& t) const
  {
    t = boost::in_place<terminator_t>(
          std::plus<int>(), [&tg] { return tg.termination_value(); }, 0);
  }
}; // struct terminator_initializer

} // namespace detail


namespace paragraph_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor used by @ref paragraph to reset port edges from producing
/// paragraph prior to re-invocation of the paragraph in persistent mode.
//////////////////////////////////////////////////////////////////////
struct in_edge_reset
{
  using result_type = void;

  template<typename T>
  void operator()(T const&) const
  { }

  template<typename T>
  void operator()(nested_pg_view_subview<T> const& v) const
  {
    v.edges().reset_entry_values();
  }
};


template<typename Coarsener, bool... b>
struct select_factory_coarsener;


template<typename V>
struct requires_coarsening
  : is_view<V>
{ };


template<typename View, typename Part>
struct requires_coarsening<partitioned_mix_view<View, Part>>
  : std::false_type
{ };


//////////////////////////////////////////////////////////////////////
/// @brief The PARAGRAPH is the dependence graph
///   representation of programs in STAPL.  It's generally not advised for users
///   to instantiate it directly, but rather use @ref stapl::make_paragraph.
/// @ingroup paragraph
///
/// @tparam Scheduler The scheduler this PARAGRAPH passes to the executor that
///   is responsible for ordering runnable tasks for execution.  It may also
///   request migration of runnable tasks, to support policies such as dynamic,
///   cross location task load balancing.
///
/// @tparam Factory The task factory for this PARAGRAPH which is responsible for
///   populating the PARAGRAPH with tasks and edges.  One instance of the
///   factory will be inserted into the executor on each location of the
///   PARAGRAPH during the first invocation (subsequent calls on a persistent
///   PARAGRAPH avoid this operation).
///
/// @tparam ... A list of views that the PARAGRAPH should provide to the factory
///   when it is invoked.  The factory may provide a data coarsening method to
///   be applied prior to factory invocation.
///
/// Most of the implementation is located in untyped (or less typed) base
/// classes to avoid code bloat. This derived class exists mainly to hold
/// data members needing the full PARAGRAPH type (i.e., views, factory, and
/// scheduler) and to implement several virtual functions providing an
/// interface to these members. The primary public operator is the function
/// operator (to those creating a PARAGRAPH, it appears as function object).
/// It also has interfaces for result specification by the factory.
///
/// @todo There are two copies of the post-coarsened views kept, one in the
///   factory task and one as a member of this class.  The former is probably
///   better, provide a method (@ref task_graph_impl has pointer to the
///   factory task) to access it.
///
/// @todo Further template instantiation reduction can probably be had by making
///  the PARAGRAPH a simple class instead of a class template.  This requires:
///    (1) make this struct result
///    (2) view template operator()
///    (3) defer coarsening
///    (4) make edge_view creation disappear into task_graph
//////////////////////////////////////////////////////////////////////
template<typename Scheduler, typename Factory, typename... Views>
class paragraph final
  : public task_graph_impl<Scheduler>,
    public detail::result_manager<
      typename wf_invoke<
        Factory,
        typename view_manager<
          typename select_factory_coarsener<
            typename coarsener_type<Factory>::type,
            requires_coarsening<Views>::value...
          >::type,
          Views...
        >::result_type
      >::result_type,
      has_span_type<Factory>::value
    >
{
private:
  using coarsen_sel_t    =
    select_factory_coarsener<
      typename coarsener_type<Factory>::type,
      requires_coarsening<Views>::value...
    >;

  using view_manager_t   = view_manager<typename coarsen_sel_t::type, Views...>;

  // Post Coarsening ViewSet type
  using viewset_t        = typename view_manager_t::result_type;

  using result_manager_t =
    detail::result_manager<
      typename wf_invoke<Factory, viewset_t>::result_type,
      has_span_type<Factory>::value
    >;

  using terminator_init_t =
    typename detail::terminator_initializer<typename tag_type<Factory>::type>;

  using terminator_t      = typename terminator_init_t::terminator_t;

  /// @brief Post-coarsening input views for this PARAGRAPH.
  viewset_t                                                 m_views;

  /// @brief The termination detection object used by this PARAGRAPH.
  /// Stored as a pointer so that it can be destroyed and
  /// reinitialized for persistent PARAGRAPH invocations.
  boost::optional<terminator_t>                             m_terminator;

  detail::result_container_base& get_result_container(void) final
  {
    return *result_manager_t::get_result_container();
  }

  bool has_outstanding_consumers(void) final
  {
    return result_manager_t::has_outstanding_consumers();
  }


  void reset_in_out_containers(void) final
  {
    // Call reset on the edge_container underneath inter-paragraph input views.
    vs_map(in_edge_reset(), m_views);

    // Reset the output container
    result_manager_t::reset();
  }

public:
  using result_type = typename result_manager_t::result_type;

  //////////////////////////////////////////////////////////////////////
  /// @todo @ref p_object::advance_epoch() call may not be needed anymore. The
  /// one present in operator() prior to invoking the factory is probably
  /// sufficient.
  /// However, maybe it makes sense to have a fully constructed object before
  /// proceeding.  Evaluate and remove the call or this todo.
  //////////////////////////////////////////////////////////////////////
  paragraph(paragraph const& other)
    : task_graph_impl<Scheduler>(other),
      m_views(other.m_views),
      m_terminator(other.m_terminator)
  {
    this->advance_epoch();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor called to initialize a new PARAGRAPH collectively.
  ///
  /// @param factory The factory for this PARAGRAPH.
  /// @param scheduler The scheduler to use with this PARAGRAPH.
  /// @param enable_migration Boolean denoting whether migration support should
  ///   be enabled in this PARAGRAPH instantiation.
  /// @param parent_handle_ref Handle to the parent paragraph of this paragraph.
  /// @param parent location The location in the parent paragraph responsible
  ///  for metadata about this paragraph.
  /// @param task_id The task identifier in the parent paragraph for this
  ///   paragraph.
  /// @param b_has_succs Boolean denoting whether paragraph
  ///   has intra-paragraph successors within the parent paragraph.
  /// @param ... Variadic list of views to pass to @p factory invocation.
  ///
  /// @todo In the presence of coarsening, m_views is different than the input
  /// views and hence the tid_mapper can't be called until after the elements
  /// have been initialized (which presents a problem since the mapper is passed
  /// to the constructor of task graph base class.  Resolve and reenable support
  /// for custom directory key mapping.
  ///
  /// @todo @ref p_object::advance_epoch() call may not be needed anymore. The
  /// one present in operator() prior to invoking the factory is probably
  /// sufficient.
  ///
  /// However, maybe it makes sense to have a fully constructed object before
  /// proceeding.  Evaluate and remove the call or this todo.
  //////////////////////////////////////////////////////////////////////
  paragraph(Factory const& factory,
            Views const&... views,
            Scheduler scheduler = Scheduler(),
            const bool enable_migration = false,
            rmi_handle::reference parent_handle_ref = rmi_handle::reference(),
            location_type parent_location = invalid_location_id,
            size_t task_id                = 0,
            std::size_t num_succs         = 0)
    : task_graph_impl<Scheduler>(
          // static_cast<key_mapper_t>(
          //   compute_tid_mapper_type<Factory>()(views...)
          //   ),
          new factory_task<
            Factory, viewset_t, typename Scheduler::entry_type,
            paragraph_impl::paragraph_view<Scheduler>
          >(std::size_t(10000) * get_location_id(), factory, m_views),
          std::move(scheduler),
          compute_tid_mapper_type<Factory>::perfect_mapping,
          enable_migration,
          num_succs, std::move(parent_handle_ref), parent_location, task_id
      ),
      m_views(view_manager_t::apply(coarsen_sel_t::apply(factory), views...))
  {
    terminator_init_t()(*this, m_views, m_terminator);

    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor for one-sided initialization of a new PARAGRAPH.
  ///
  /// @param ignored Tag dispatch to select this constructor in one-sided
  ///  spawning process.
  /// @param factory_param Parameter used to initialize the factory for
  ///  this paragraph.  Allows initialization of paragraph skeleton manager
  ///  from skeleton parameter, as the former is not serializable.
  /// @param scheduler The scheduler to use with this PARAGRAPH.
  /// @param enable_migration Boolean denoting whether migration support should
  ///   be enabled in this PARAGRAPH instantiation.
  /// @param parent_handle_ref Handle to the parent paragraph of this paragraph.
  /// @param parent location The location in the parent paragraph responsible
  ///  for metadata about this paragraph.
  /// @param task_id The task identifier in the parent paragraph for this
  ///   paragraph.
  /// @param b_has_succs Boolean denoting whether paragraph
  ///   has intra-paragraph successors within the parent paragraph.
  /// @param packaged_views Set of packaged views which are unpackaged,
  ///   coarsened, and then passed to @p factory invocation.
  /// @todo Replace factory_param with direct factory param when skeletons
  ///   properly serialize the paragraph skeleton manager.
  //////////////////////////////////////////////////////////////////////
  template<typename FactoryParam, typename ...PackagedViews>
  paragraph(nested_tag const& ignored,
            FactoryParam const& factory_param,
            Scheduler const& scheduler,
            const bool enable_migration,
            rmi_handle::reference parent_handle_ref,
            location_type parent_location,
            size_t task_id,
            std::size_t num_succs,
            PackagedViews /*const*/&... packaged_views)
    : task_graph_impl<Scheduler>(
          new factory_task<
            Factory, viewset_t, typename Scheduler::entry_type,
            paragraph_impl::paragraph_view<Scheduler>
          >(std::size_t(10000) * get_location_id(), factory_param, m_views),
          std::move(scheduler),
          compute_tid_mapper_type<Factory>::perfect_mapping,
          enable_migration,
          num_succs, std::move(parent_handle_ref), parent_location, task_id
      ),
      m_views(
        view_manager_t::apply(
          coarsen_sel_t::apply(
            static_cast<factory_task<
              Factory, viewset_t, typename Scheduler::entry_type,
              paragraph_impl::paragraph_view<Scheduler>>&>
            (this->factory_task()).factory()),
          pg_edge_unpackager(*this, has_enable_persistence<Scheduler>::value)(
            transporter_unpackager()(packaged_views))...) )
  {
    terminator_init_t()(*this, m_views, m_terminator);

    this->advance_epoch();
  }


  void pulse_terminator(void) final
  {
    gang g(*this);
    (*m_terminator)();
  }


  void set_result(std::size_t index, task_graph::task_id_t task_id) final
  {
    this->set_result_impl(index, task_id, *this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Method that starts population and execution of a PARAGRAPH.
  ///
  /// @param unused Unused parameter that provides unique signature to function
  ///   that enables non-blocking behavior by default.
  ///
  /// @param blocking True if blocking behavior of PARAGRAPH (i.e., heap
  ///   allocate structures, etc) should be enabled.
  ///
  /// @param force_evaluation When in non-blocking mode, still enforce that the
  ///   the function operator doesn't return until termination detection
  ///   succeeds and the value is available.
  ///
  /// @param force_evaluation When in non-blocking mode, still enforce that the
  ///   the function operator doesn't return until termination detection
  ///   succeeds and the value is available.
  ///
  /// @param one_sided When true, this paragraph was created via a one-sided,
  ///   nested parallelism invocation.  Enforce non-blocking semantics, and make
  ///   sure the terminator was initialized in the paragraph constructor.
  ///
  /// @return Type is defined by @ref result_manager, but in general is a
  ///   proxy over the value returned by the PARAGRAPH on this location
  ///   (defined by factory via call to @ref paragraph_view::set_result).
  ///
  /// @todo Reevaluate whether we need two signatures with the current
  ///   parameters or if we can simplify the calling convention.
  ///
  /// @todo Verify that @ref p_object::advance_epoch() is still needed. Maybe
  ///   required for PARAGRAPH call stack push, but for non blocking, the call
  ///   stack isn't guaranteed to be the same anyways.  Related to check for
  ///   same in paragraph constructor.
  ///
  /// @todo The parameters blocking and force_evaluation are close to
  ///   being redundant now, now that the internal implementation more or less
  ///   assumes non-blocking by default now.  Verify there's no vestiges of the
  ///   blocking implementation and remove the second parameter.
  //////////////////////////////////////////////////////////////////////
  result_type operator()(int  unused,
                         bool blocking         = false,
                         bool force_evaluation = false,
                         bool one_sided        = false)
  {
    //
    // Switch to gang of this paragraph so that traffic is properly accounted
    // for, and invoke base class operator for template independent
    // initialization code.
    //
    gang g(*this);

    task_graph::operator()(unused, blocking, force_evaluation, one_sided);

    //
    // Setup result storage and define interaction with termination detection.
    //
    typename result_manager_t::tmp_result_type result =
      result_manager_t::apply(*this, one_sided);

    typename result_manager_t::os_td_wait_t* os_td_wait_ptr = nullptr;

    if (one_sided)
      os_td_wait_ptr = this->set_os_result_notifier(*this, result);

    this->advance_epoch();


    //
    // Instantiate executor and setup interaction with terminator.
    //
    auto executor_ptr =
      new executor<task_graph_impl<Scheduler>>
        {*this, *m_terminator, this->passed_scheduler()};

    this->set_executor(executor_ptr);

    m_terminator->set_notifier(
      [this, executor_ptr, os_td_wait_ptr](void)
      {
        if (os_td_wait_ptr != nullptr)
          os_td_wait_ptr->notify_td();

        executor_ptr->notify_finished();
        this->notify_termination();
      });


    //
    // Provide executor with runnable tasks.
    //
    const auto task_range = this->persister().get_executor_start_tasks();

    for (auto** it = task_range.first; it != task_range.second; ++it)
    {
      if ((*it)->is_nested_pg_task())
      {
        // Switch into a single location which will be fenced prior to the
        // pg TD success, so the coherence traffic of the
        // async_rmi(all_locations) which fires the task is properly received.
        gang(this->no_succs_p_object());

        (*it)->operator()(this);
      }
      else
        executor_ptr->add_task(*it);
    }


    //
    // Either force blocking execution or hook this executor into the current
    // gang_executor before returning view or reference to result.
    //
    const bool b_is_nested = this->get_rmi_handle().get_gang_id() != 0;

    // PARAGRAPH may possibly be destroyed in this call.
    // Do Not access data members after this call.
    if (this->get_location_md().get_gang_md().get_id() == 0 || one_sided) {
      get_executor().add_executor(
        executor_ptr, this->passed_scheduler().get_sched_info(), false);
    }
    else {
      executor_ptr->operator()(execute_all);
      delete executor_ptr;
    }

    detail::scoped_drain_call x((blocking || force_evaluation) && !b_is_nested);

    // just to avoid warning on unused x...
    x.foo();

    return result_manager_t::finalize_result(result);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Function operator signature that engages blocking semantic of the
  /// PARAGRAPH.  The function call does not return until termination detection
  /// has succeeded and the result value for this location is available.
  //////////////////////////////////////////////////////////////////////
  result_type operator()(void)
  {
    return this->operator()((int) 0, true);
  }
}; // class paragraph

} } // namespace paragraph_impl::stapl


#undef STAPL_PARAGRAPH_DECLARATION_HEADER
#undef STAPL_PARAGRAPH_VIEW_TYPE_LIST

#include <stapl/views/metadata/coarseners/default.hpp>
#include <stapl/views/metadata/coarseners/multiview.hpp>

namespace stapl {

namespace paragraph_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Function object to return the view coarsener defined by the factory,
///   if one exists.  Otherwise, return the @ref default_coarsener or the
///   @ref null_coarsener based on the type of the first view input of the
///   PARAGRAPH.
/// @ingroup paragraph
///
/// @tparam Factory The factory type to check for a custom coarsener.
/// @tparam V0 The fist view type passed to the PARAGRAPH.
/// @tparam b Integral boolean used to partially specialize the class based on
///   existence of @p coarsener_type reflected type in @p Factory.
///
/// @todo It is more general to template this functor with all
/// view types.
//////////////////////////////////////////////////////////////////////
template<typename Coarsener, bool... b>
struct select_factory_coarsener
{
  using type = Coarsener;

  template <typename Factory>
  static type apply(Factory const& factory)
  {
    return factory.get_coarsener();
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for case when Factory does not define a coarsener.
/// @ingroup paragraph
///
/// @todo The logic of when to apply the default coarsener versus
/// the null coarsener belongs in the view coarsener codebase.
//////////////////////////////////////////////////////////////////////
template <bool... b>
struct select_factory_coarsener<stapl::use_default, true, b...>
{
  using type = default_coarsener;

  template <typename Factory>
  static type apply(Factory const& factory)
  {
    return type();
  }
};


template <bool... b>
struct select_factory_coarsener<stapl::use_default, b...>
{
  using type = null_coarsener;

  template <typename Factory>
  static type apply(Factory const& factory)
  {
    return type();
  }
};

} // namespace paragraph_impl

} // namespace stapl

#endif // STAPL_PARAGRAPH_PARAGRAPH_HPP
