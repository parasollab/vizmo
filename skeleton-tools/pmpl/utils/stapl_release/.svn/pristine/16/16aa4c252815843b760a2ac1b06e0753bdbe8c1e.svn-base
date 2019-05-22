/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_PARAGRAPH_TASK_CREATION_HPP
#define STAPL_PARAGRAPH_TASK_CREATION_HPP

#include <stapl/utility/pack_ops.hpp>

namespace stapl {

namespace detail {

BOOST_MPL_HAS_XXX_TRAIT_DEF(disable_localization)

//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction reflecting @p std::true_type if a view
/// has chosen to disable localization and @p std::false_type
/// otherwise.
///
/// Primary template matches case when @p disable_localization is not
/// defined by the view.  Localization is attempted.
//////////////////////////////////////////////////////////////////////
template<typename View, bool = has_disable_localization<View>::value>
struct check_localizable
  : std::true_type
{ };


//////////////////////////////////////////////////////////////////////
/// Specialization matches case when @p disable_localization is defined
/// by the view.  Inspect the reflected integral constant to determine
/// whether localization should be attempted.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct check_localizable<View, true>
  : std::integral_constant<bool, !View::disable_localization::value>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Recursive type metafunction which returns false if any of the
/// views in the parameter pack have disabled localization.
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
struct is_localizable;


template<>
struct is_localizable<>
  : std::true_type
{ };


template<typename View, typename ...Views>
struct is_localizable<View, Views...>
  : std::integral_constant<
      bool, check_localizable<View>::value && is_localizable<Views...>::value>
{ };


enum view_wrapper_type {Standard, Fast};

//////////////////////////////////////////////////////////////////////
/// @brief View Storage used for tasks when localization is not performed.
/// @ingroup pgViewOps
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
class std_storage
  : private tuple<Views...>
{
public:
  typedef tuple<Views...> base_view_set_type;
  typedef tuple<Views...> view_set_type;

  template<typename ...ViewParams>
  std_storage(tg_callback const&, ViewParams&&... views)
    : view_set_type(std::forward<ViewParams>(views)...)
  { }

  base_view_set_type& get_orig_vs(void)
  { return static_cast<view_set_type&>(*this); }

  view_set_type& views(void)
  { return static_cast<view_set_type&>(*this); }
};


//////////////////////////////////////////////////////////////////////
/// @brief View Storage used for tasks when localization is performed
///    and migration is enabled. Store both original and localized views.
/// @ingroup pgViewOps
/// @todo Specialize for cases where localization is a noop
/// (e.g., edge flowed values) to avoid double storage.
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
class fast_mg_storage
{
public:
  typedef tuple<Views...>                                    base_view_set_type;
  typedef tuple<typename get_fast_view_type<Views>::type...> view_set_type;

private:
  /// @brief Copy of pre localization viewset.  Maintained to revert
  /// localization prior to task migration.
  base_view_set_type  m_orig_vs;

  /// @brief the transformed (i.e., possibly localized) view_set for task.
  view_set_type       m_views;

  tg_callback         m_cb;
public:
  template<typename ...ViewParams>
  fast_mg_storage(tg_callback cb, ViewParams&&... views)
    : m_orig_vs(views...),
      m_views(std::forward<ViewParams>(views)...),
      m_cb(std::move(cb))
  { }

  ~fast_mg_storage()
  {
    // Task<>:::operator() calls this on m_views, just called it
    // on m_orig_vs which Task<> doesn't see.
    vs_map(lazy_ref_release_func(m_cb), m_orig_vs);
  }

  base_view_set_type& get_orig_vs(void)
  { return m_orig_vs; }

  view_set_type& views(void)
  { return m_views; }
}; // class view_wrapper


//////////////////////////////////////////////////////////////////////
/// @brief View storage used for tasks when localization is
///   performed and migration is disabled. Only store the localized views.
/// @ingroup pgViewOps
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
class fast_no_mg_storage
  : private tuple<typename get_fast_view_type<Views>::type...>
{
public:
  typedef tuple<Views...>                                    base_view_set_type;
  typedef tuple<typename get_fast_view_type<Views>::type...> view_set_type;

  template<typename ...ViewParams>
  fast_no_mg_storage(tg_callback const&, ViewParams&&... views)
    : view_set_type(std::forward<ViewParams>(views)...)
  { }

  view_set_type& views(void)
  { return static_cast<view_set_type&>(*this); }
};


//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction to compute the storage class template used
///  by a task in the paragraph.
/// @ingroup pgViewOps
/// @tparam T enumerated type denoting whether localization should be performed.
/// @tparam Migratable Boolean denoting whether task migration is enabled.
/// @tparam Views The views for a task.
//////////////////////////////////////////////////////////////////////
template<view_wrapper_type T, bool Migratable, typename ...Views>
struct compute_task_view_storage;


//////////////////////////////////////////////////////////////////////
/// @brief Specialization used for tasks when no localization is
///   performed. Only store one copy of original views.
//////////////////////////////////////////////////////////////////////
template<bool Migratable, typename ...Views>
struct compute_task_view_storage<Standard, Migratable, Views...>
{ typedef std_storage<Views...> type; };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization used for tasks when localization is
///   performed and migration support is enabled.
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
struct compute_task_view_storage<Fast, true, Views...>
{ typedef fast_mg_storage<Views...> type; };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization used for tasks when localization is
///   performed and migration support is disabled.
//////////////////////////////////////////////////////////////////////
template<typename ...Views>
struct compute_task_view_storage<Fast, false, Views...>
{ typedef fast_no_mg_storage<Views...> type; };


//////////////////////////////////////////////////////////////////////
/// @brief Initialize view set for task with no localization applied.
///   Construct @ref Task object on heap and return a pointer to it.
/// @ingroup pgTasks
/// @copydetails create_task(detail::edge_entry_base*, SchedulerInfoParam&&, WFParam&&, ViewParams&&)
//////////////////////////////////////////////////////////////////////
template<typename SchedulerEntry,
         typename Migratable,
         typename Persistent,
         typename SchedulerInfoParam,
         typename WFParam,
         typename ...ViewParams>
paragraph_impl::task_base_intermediate<SchedulerEntry>*
standard_task_creator(tg_callback const& cb,
                      detail::edge_entry_base* edge_entry_ptr,
                      SchedulerInfoParam&& scheduler_info,
                      WFParam&& wf, ViewParams&&... views)
{
  return new paragraph_impl::Task<
    SchedulerEntry, Migratable, Persistent,
    typename std::decay<WFParam>::type,             // Workfunction
    typename compute_task_view_storage<             // ViewSet Type
      Standard, Migratable::value,
      typename std::decay<ViewParams>::type...>::type
  >(edge_entry_ptr,
    std::forward<SchedulerInfoParam>(scheduler_info),
    std::forward<WFParam>(wf),
    cb, std::forward<ViewParams>(views)...);
}


//////////////////////////////////////////////////////////////////////
/// @brief Initialize view set for task with localization transformation
///   applied. Wraps the workfunction passed to the task with mechanism to
///   promote a return value from workfunction which is dependent on an
///   input localized view.  Construct @p Task object on heap and return
///   a pointer to it.
/// @ingroup pgTasks
/// @sa paragraph_impl::compute_promotion_wf
/// @copydetails create_task(detail::edge_entry_base*, SchedulerInfoParam&&, WFParam&&, ViewParams&&)
//////////////////////////////////////////////////////////////////////
template<typename SchedulerEntry,
         typename Migratable,
         typename Persistent,
         typename SchedulerInfoParam,
         typename WFParam,
         typename ...ViewParams>
paragraph_impl::task_base_intermediate<SchedulerEntry>*
fast_task_creator(tg_callback const& cb,
                  detail::edge_entry_base* edge_entry_ptr,
                  SchedulerInfoParam&& scheduler_info,
                  WFParam&& wf, ViewParams&&... views)
{
  typedef typename compute_task_view_storage<
    Fast, Migratable::value,
    typename std::decay<ViewParams>::type...>::type      view_storage_type;

  // WF selector
  typedef paragraph_impl::compute_promotion_wf<
    typename std::decay<WFParam>::type,
    typename view_storage_type::base_view_set_type,
    typename view_storage_type::view_set_type>           wf_selector_type;

  return new paragraph_impl::Task<
    SchedulerEntry,                                // Scheduler Info
    Migratable, Persistent,                        // Migration/Persistent Bool
    typename wf_selector_type::result_type,        // Workfunction
    view_storage_type                              // ViewSet
  >(edge_entry_ptr,
    std::forward<SchedulerInfoParam>(scheduler_info),
    wf_selector_type::apply(cb, wf, views...),
    cb, std::forward<ViewParams>(views)...);
}


//////////////////////////////////////////////////////////////////////
/// @brief Called after a tasks' views have been created to make the
///   task object.  Statically guards localization with @ref is_localizable<
///   metafunction.
/// @ingroup pgTasks
/// @tparam Localize Statically denotes whether localization should be
///   performed.  Used to dispatch to proper specialization.
//////////////////////////////////////////////////////////////////////
template<bool Localize>
struct create_task_impl
{
  //////////////////////////////////////////////////////////////////////
  /// @brief Specialization for viewless task.  No need to
  ///   apply localization.
  //////////////////////////////////////////////////////////////////////
  template<typename SchedulerEntry,
           typename Migratable,
           typename Persistent,
           typename SchedulerInfoParam,
           typename WFParam>
  static paragraph_impl::task_base_intermediate<SchedulerEntry>*
  apply(tg_callback const& cb, detail::edge_entry_base* edge_entry_ptr,
        SchedulerInfoParam&& scheduler_info, WFParam&& wf)
  {
    return standard_task_creator<SchedulerEntry, Migratable, Persistent>(
      cb, edge_entry_ptr,
      std::forward<SchedulerInfoParam>(scheduler_info),
      std::forward<WFParam>(wf));
  }


  template<typename SchedulerEntry,
           typename Migratable,
           typename Persistent,
           typename SchedulerInfoParam,
           typename WFParam,
           typename ...ViewParams>
  static paragraph_impl::task_base_intermediate<SchedulerEntry>*
  apply(tg_callback const& cb,
        detail::edge_entry_base* edge_entry_ptr,
        SchedulerInfoParam&& scheduler_info,
        WFParam&& wf, ViewParams&&... views)
   {
    // Based on localizer test, create task with fast or slow views. Initialize
    // it with knowledge of succs so it passed it to tg::processed later.
    return pack_ops::functional::and_(
      localizer<typename std::decay<ViewParams>::type>::apply(views)...)
      ? fast_task_creator<SchedulerEntry, Migratable, Persistent>(
          cb, edge_entry_ptr,
          std::forward<SchedulerInfoParam>(scheduler_info),
          std::forward<WFParam>(wf),
          std::forward<ViewParams>(views)...)
      : standard_task_creator<SchedulerEntry, Migratable, Persistent>(
          cb, edge_entry_ptr,
          std::forward<SchedulerInfoParam>(scheduler_info),
          std::forward<WFParam>(wf),
          std::forward<ViewParams>(views)...);
  }
};


template<>
struct create_task_impl<false>
{
  template<typename SchedulerEntry,
           typename Migratable,
           typename Persistent,
           typename SchedulerInfoParam,
           typename WFParam,
           typename ...ViewParams>
  static paragraph_impl::task_base_intermediate<SchedulerEntry>*
  apply(tg_callback const& cb,
        detail::edge_entry_base* edge_entry_ptr,
        SchedulerInfoParam&& scheduler_info,
        WFParam&& wf, ViewParams&&... views)
  {
    return standard_task_creator<SchedulerEntry, Migratable, Persistent>(
      cb, edge_entry_ptr,
      std::forward<SchedulerInfoParam>(scheduler_info),
      std::forward<WFParam>(wf),
      std::forward<ViewParams>(views)...);
  }
};



//////////////////////////////////////////////////////////////////////
/// @brief Checks static localizability of task based on views and
///  dispatches to appropriate task creation mechanism.
/// @ingroup pgTasks
/// @tparam SchedulerEntry Intrusive entry for scheduler which
///  stores scheduling metadata and container related fields.
/// @tparam Migratable Boolean type parameter denoting whether task
///   migration enabled for the associated PARAGRAPH.
/// @tparam Persistent Boolean type parameter denoting whether
///   persistency enabled for the associated PARAGRAPH.
///
/// @param edge_entry_ptr Edge entry that this task writes its results to.
/// @param sched_info Information used to determine relative priority of task.
/// @param wf The workfunction for the task.
/// @param views The pre-localization views for this task.
//////////////////////////////////////////////////////////////////////
template<typename SchedulerEntry,
         typename Migratable,
         typename Persistent,
         typename SchedulerInfoParam,
         typename WFParam,
         typename ...ViewParams>
paragraph_impl::task_base_intermediate<SchedulerEntry>*
create_task(tg_callback const& cb,
            detail::edge_entry_base* edge_entry_ptr,
            SchedulerInfoParam&& scheduler_info,
            WFParam&& wf, ViewParams&&... views)
{
  return create_task_impl<
    is_localizable<typename std::decay<ViewParams>::type...>::value
  >::template apply<SchedulerEntry, Migratable, Persistent>
    (cb, edge_entry_ptr,
     std::forward<SchedulerInfoParam>(scheduler_info),
     std::forward<WFParam>(wf),
     std::forward<ViewParams>(views)...);
}

} // namespace detail

} // namespace stapl

#endif // STAPL_PARAGRAPH_TASK_CREATION_HPP
