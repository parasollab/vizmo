/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_OPERATORS_ELEM_HPP
#define STAPL_SKELETONS_OPERATORS_ELEM_HPP

#include <type_traits>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/flows/elem_flow.hpp>
#include <stapl/skeletons/spans.hpp>
#include "elem_impl.hpp"

namespace stapl {
namespace skeletons {
namespace result_of {

template <typename Span,
          typename Flows,
          typename PD>
using elem = skeletons_impl::elem<
               typename std::decay<PD>::type,
               stapl::default_type<Span, spans::balanced<>>,
               stapl::default_type<Flows, flows::elem_f::doall>>;

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief An elementary is an operator that converts parametric
/// dependencies to skeletons. It wraps a parametric dependency with
/// @c Flows and @c Span information.
///
/// @tparam Span  the iteration space for elements in this elementary.
///               The default span is @c balanced
/// @tparam Flows the flow to be used for the elementary skeleton.
///               The default flow is a @c forked flow
/// @param  pd    the parametric dependency to be wrapped
/// @return an elementary skeleton
///
/// @see flows::elem_f::forked
/// @see spans::balanced
///
/// @ingroup skeletonsOperators
//////////////////////////////////////////////////////////////////////
template <typename Span  = stapl::use_default,
          typename Flows = stapl::use_default,
          typename PD>
result_of::elem<Span, Flows, PD>
elem(PD&& pd)
{
  return result_of::elem<Span, Flows, PD>(std::forward<PD>(pd));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_OPERATORS_ELEM_HPP
