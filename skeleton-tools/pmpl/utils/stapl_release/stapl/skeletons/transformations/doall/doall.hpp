/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_DOALL_DOALL_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_DOALL_DOALL_HPP

#include <stapl/skeletons/transformations/transform.hpp>
#include <stapl/skeletons/transformations/wrapped_skeleton.hpp>
#include <stapl/skeletons/transformations/doall.hpp>
#include <stapl/skeletons/functional/sink_value.hpp>

#include <stapl/skeletons/functional/map.hpp>
#include <stapl/skeletons/functional/reduce_to_locs.hpp>
#include <stapl/skeletons/functional/broadcast_to_locs.hpp>

namespace stapl {
namespace skeletons {
namespace transformations {

template <typename S, typename SkeletonTag, typename TransformTag>
struct transform;

//////////////////////////////////////////////////////////////////////
/// @brief In the default case, a transformation of a skeleton with
/// the @c tags::doall tag should not modify the skeleton. For example,
/// a nested @ref skeletons::map or @ref skeletons::zip can be executed
/// without applying any transformations.
///
/// @see make_paragraph_skeleton_manager
///
//////////////////////////////////////////////////////////////////////
template <typename S, typename SkeletonTag>
struct transform<S, SkeletonTag, tags::doall>
{
  template <typename Index>
  static S call(S const& skeleton, Index const& index)
  {
    return skeleton;
  }
};



} // namespace transformations
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_DOALL_DOALL_HPP
