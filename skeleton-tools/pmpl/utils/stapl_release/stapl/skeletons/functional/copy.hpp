/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_COPY_HPP
#define STAPL_SKELETONS_FUNCTIONAL_COPY_HPP

#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/utility/use_default.hpp>
#include <stapl/algorithms/functional.hpp>
#include "zip.hpp"

namespace stapl {
namespace skeletons {
namespace result_of {

template <typename ValueType,
          typename Span>
using copy = result_of::zip<
               2, Span, stapl::use_default,
               stapl::assign<ValueType>, skeletons::no_filter>;

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief A copy skeleton is used when writes to views are happening.
/// The second flow to this skeleton will be writable.
///
/// @tparam ValueType the type of each element to be copied
/// @tparam Span      the iteration space for elements of the copy
///                   skeleton
/// @return a copy skeleton
///
/// @see zip
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <typename ValueType,
          typename Span = stapl::use_default>
result_of::copy<ValueType, Span>
copy(void)
{
  return skeletons::zip<2, Span>(stapl::assign<ValueType>());
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_COPY_HPP
