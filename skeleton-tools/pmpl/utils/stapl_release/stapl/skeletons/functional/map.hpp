/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_MAP_HPP
#define STAPL_SKELETONS_FUNCTIONAL_MAP_HPP

#include <type_traits>
#include <utility>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/skeletons/transformations/transform.hpp>
#include "zip.hpp"

namespace stapl {
namespace skeletons {
namespace result_of {

template <typename Span,
          typename Flows,
          typename Op,
          typename Filter>
using map = skeletons_impl::zip<
              typename std::decay<Op>::type,
              1,
              Span, Flows,
              typename std::decay<Filter>::type>;

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief A filtered map is similar to @c map skeleton but it applies
/// a filter function on the producer side before sending data along
/// the edges to each parametric dependency.
///
/// @tparam Span   the iteration space for the elements in this
///                skeleton
/// @tparam Flows  the flow to be used for this skeleton
/// @param  op     the workfunction to be used in each map parametric
///                skeleton.
/// @param  filter the filter function to be used on the producer side
///                before sending data to a parametric dependency
/// @return a map skeleton with a filter on the incoming edges
///
/// @see map
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <typename Span   = stapl::use_default,
          typename Flows  = stapl::use_default,
          typename Op,
          typename Filter = skeletons::no_filter,
          typename =
            typename std::enable_if<
              !is_skeleton<typename std::decay<Op>::type>::value>::type>
result_of::map<Span, Flows, Op, Filter>
map(Op&& op, Filter&& filter = Filter())
{
  return result_of::map<Span, Flows, Op, Filter>(
           std::forward<Op>(op),
           std::forward<Filter>(filter));
}

//////////////////////////////////////////////////////////////////////
/// @brief Creates a nested skeleton composition by transforming the
/// inner skeleton to a suitable skeleton for nested execution.
///
/// @param  skeleton the skeleton to be used in the nested section
/// @param  filter   the filter function to be used on the producer side
///                  before sending data to a parametric dependency
///
/// @see map
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <typename S,
          typename Filter = skeletons::no_filter,
          typename =
            typename std::enable_if<
              is_skeleton<typename std::decay<S>::type>::value>::type>
result_of::map<
  stapl::use_default,
  stapl::use_default,
  decltype(skeletons::transform<tags::nest>(std::declval<S>())),
  Filter
>
map(S&& skeleton, Filter&& filter = Filter())
{
  return skeletons::map(
           transform<tags::nest>(std::forward<S>(skeleton)),
           std::forward<Filter>(filter));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_MAP_HPP
