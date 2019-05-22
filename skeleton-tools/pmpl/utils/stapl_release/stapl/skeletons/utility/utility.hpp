/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_UTILITY_UTILITY_HPP
#define STAPL_SKELETONS_UTILITY_UTILITY_HPP

#include <type_traits>
#include <stapl/utility/use_default.hpp>
#include <stapl/utility/tuple/tuple_element.hpp>
#include <boost/mpl/has_xxx.hpp>

namespace stapl {

BOOST_MPL_HAS_XXX_TRAIT_DEF(result_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(metadata_type)

template <typename T, typename Default>
using default_type = typename std::conditional<
                       std::is_same<T, stapl::use_default>::value,
                       Default,
                       T
                     >::type;

template <typename C>
class repeat_view;


template <typename V, typename P, typename CC>
class partitioned_mix_view;


namespace skeletons {

struct no_filter { };

template <typename In, int i>
using flow_value_type = typename tuple_element<i, In>::type::flow_value_type;

template <typename V>
struct has_finite_domain
  : public std::true_type
{ };


template <typename C>
struct has_finite_domain<repeat_view<C>>
  : public std::false_type
{ };


template <typename C, typename P, typename CC>
struct has_finite_domain<partitioned_mix_view<repeat_view<C>, P, CC>>
  : public std::false_type
{ };


template <typename D, bool is_finite = true>
struct domain_type
  : public D
{
  using is_finite_type = std::integral_constant<bool, is_finite>;

  template<typename... Args>
  domain_type(Args&&... args)
    : D(std::forward<Args>(args)...)
  { }
};


template <typename D>
using is_finite_domain = typename D::is_finite_type;

//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction returning the index of the first element
/// (i.e., domain) of the tuple parameter @p View domains which is
/// finite.
///
/// @ingroup skeletonsUtilities
//////////////////////////////////////////////////////////////////////
template<typename VDomains>
using first_finite_domain_index = stapl::find_first_index<
                                    VDomains, is_finite_domain>;

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_UTILITY_UTILITY_HPP
