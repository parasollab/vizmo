/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_BROADCAST_TO_LOCS_HPP
#define STAPL_SKELETONS_FUNCTIONAL_BROADCAST_TO_LOCS_HPP

#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/skeletons/spans.hpp>
#include "broadcast.hpp"

namespace stapl {
namespace skeletons {
namespace result_of {

template <typename Tag,
          typename Flows>
using broadcast_to_locs = result_of::broadcast<
                            Tag, Flows, spans::per_location, stapl::identity_op
                          >;

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief This broadcast skeleton is used when the result of the
/// broadcast should have a representative on each location.
/// Therefore, a @c span::per_location is used in this skeleton.
/// @c Flows are still allowed to be customized for this skeleton.
///
/// @tparam Flows     the customized flow to be used in the
///                   @c reverse_tree
/// @param tag        determines the type of the broadcast skeleton
/// @return a broadcast skeleton that broadcasts one element to each
///         location with custom flows
///
/// @see broadcast
/// @see spans::per_location
///
/// @ingroup skeletonsFunctionalBroadcast
//////////////////////////////////////////////////////////////////////
template <typename Tag   = stapl::use_default,
          typename Flows = stapl::use_default>
inline result_of::broadcast_to_locs<Tag, Flows>
broadcast_to_locs(void)
{
  return result_of::broadcast_to_locs<Tag, Flows>(stapl::identity_op());
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_BROADCAST_TO_LOCS_HPP
