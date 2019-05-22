/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_PARAGRAPH_TASK_CREATION_H
#define STAPL_PARAGRAPH_TASK_CREATION_H

#include <stapl/paragraph/view_operations/subview_type.hpp>
#include <stapl/paragraph/view_operations/fast_view.hpp>
#include <stapl/paragraph/view_operations/localization.hpp>
#include <stapl/paragraph/tasks/return_promotion.hpp>
#include <boost/mpl/has_xxx.hpp>

namespace stapl {

namespace detail {

template<typename SchedulerEntry,
         typename Migratable,
         typename Persistent,
         typename SchedulerInfoParam,
         typename WFParam,
         typename ...ViewParams>
paragraph_impl::task_base_intermediate<SchedulerEntry>*
create_task(tg_callback const&,
            detail::edge_entry_base*, SchedulerInfoParam&&,
            WFParam&&, ViewParams&&...);

} // namespace detail

} // namespace stapl

#endif // STAPL_PARAGRAPH_TASK_CREATION_H
