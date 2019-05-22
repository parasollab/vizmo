/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_NESTED_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_NESTED_HPP

#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/transformations/optimizers/default.hpp>
#include <stapl/skeletons/transformations/optimizer.hpp>

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief A @c nested optimizer will spawn the skeleton in parallel
/// allowing nested parallelism to be defined and exploited in a high
/// level.
///
/// @see algorithm_executor
///
/// @ingroup skeletonsTransformationsNest
//////////////////////////////////////////////////////////////////////
template <typename Tag, bool B>
struct optimizer<Tag, tags::nested_execution<B>>
  : optimizer<Tag, tags::default_execution<B>>
{ };

} // namespace optimizers
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_NESTED_HPP
