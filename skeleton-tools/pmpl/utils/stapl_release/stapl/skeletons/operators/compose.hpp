/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_OPERATORS_COMPOSE_HPP
#define STAPL_SKELETONS_OPERATORS_COMPOSE_HPP

#include <type_traits>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/flows/compose_flows.hpp>
#include <stapl/skeletons/flows/inline_flows.hpp>
#include <stapl/utility/tuple/tuple.hpp>
#include "compose_impl.hpp"

namespace stapl {
namespace skeletons {

//////////////////////////////////////////////////////////////////////
/// @brief A compose operator puts a series of skeletons one after
/// another. With the help of various @c compose_flows, one can customize
/// the way these skeletons are connected. The most common flow for the
/// @c compose operator is a @c piped flow. In fact, a @c compose with
/// a piped flow is known as a pipeline in the literature.
///
/// @tparam Flows  the flow to be used for the @c compose. Some
///                skeletons need a special flow than the default one.
///                The default flow for a compose is @c piped flow. If @c
///                tags::inline_flow is passed, the flow will be computed
///                based on annotations added to the skeleton parameters.
///
/// @return a composition of the given skeletons
///
/// @see flows::compose_flows::piped
/// @see flows::inline_flows::inline_flow
///
/// @ingroup skeletonsOperators
//////////////////////////////////////////////////////////////////////
namespace result_of {

template <typename Flows, typename Skeletons>
struct compose_impl;

template <typename Flows, typename... Skeletons>
struct compose_impl<Flows, stapl::tuple<Skeletons...>>
{

  using type = typename skeletons_impl::compose<stapl::tuple<Skeletons...>,
                stapl::default_type<Flows, flows::compose_flows::piped>>;

  static type call(Skeletons const&... skeleton)
  {
    return type(stapl::tuple<Skeletons...>(skeleton...));
  }
};


template<typename... Ts>
struct compose_impl<tags::inline_flow, stapl::tuple<Ts...>>
{

  //compute the final type of the list with all defaults filled in
  using tuple_t = typename flows::inline_flows::to_flow_tuples_t<Ts...>::type;

  using skeletons_t
    = typename flows::inline_flows::get_skeletons_t<Ts...>::type;
  using flows_t
    = flows::inline_flows::inline_flow<
        typename flows::inline_flows::ph_flows_tuple_of<tuple_t>::type>;


  using type = skeletons_impl::compose<skeletons_t, flows_t>;

  static type call(Ts const&... t)
  {
    return type(flows::inline_flows::get_skeletons_from(t...));
  }
};


template<typename Flows, typename Skeletons>
using compose = typename compose_impl<Flows, Skeletons>::type;

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief A compose operator is similar to the functional composition
/// used in functional programming. However, it allows the customization
/// of composition through the given @c Flows parameter.
///
/// The @c Flows parameter defines the input/output dependencies
/// between the underlying skeletons without the knowledge of the
/// computation. The default @c Flows is called @c piped in which
/// the input/output dependencies are the same as the function
/// composition compose(S1, S2, ... Sn) input = Sn(...(S1(input))).
///
/// @tparam Flows   the flows to be used for this @c compose operator.
///                 Some skeletons require customized flows for their
///                 computation. However, most of the compositions
///                 can use the default method of composition called
///                 @c piped. If @c tags::inline_flow is passed, then
///                 the flow type will be created based on annotations
///                 added to the skeleton parameters.
/// @param skeleton skeletons to be composed.
///
/// @return a composition of the given skeletons with the given
///         customized flows.
///
/// @see flows::compose_flows::piped
/// @see flows::inline_flows::inline_flow
///
/// @ingroup skeletonsOperators
//////////////////////////////////////////////////////////////////////
template <typename Flows,
          typename ...S>
result_of::compose<
  Flows, stapl::tuple<typename std::decay<S>::type...>>
compose(S&&... skeleton)
{
  return result_of::compose_impl<
           Flows, stapl::tuple<typename std::decay<S>::type...>
         >::call(skeleton...);
}

//////////////////////////////////////////////////////////////////////
/// @brief A specialization for the functional composition style
/// compose operator.
///
/// @param skeleton skeletons to be composed.
///
/// @todo gcc 4.7 and Intel compilers fail to assign default values
/// when variadics are involved. This signature can be removed if
/// the support for gcc 4.7 and Intel compiler using gcc 4.7 are
/// discontinued.
///
/// @ingroup skeletonsOperators
//////////////////////////////////////////////////////////////////////
template <typename ...S>
result_of::compose<
  stapl::use_default, stapl::tuple<typename std::decay<S>::type...>>
compose(S&&... skeleton)
{
  return result_of::compose_impl<
           stapl::use_default, stapl::tuple<typename std::decay<S>::type...>
         >::call(skeleton...);
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_OPERATORS_COMPOSE_HPP
