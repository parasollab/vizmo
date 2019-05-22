/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_INFINITE_HELPERS_HPP
#define STAPL_VIEWS_METADATA_INFINITE_HELPERS_HPP

#include <type_traits>
#include <stapl/utility/tuple.hpp>
#include <stapl/domains/infinite.hpp>
#include <stapl/views/type_traits/has_domain.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Implementation metafunction for @ref has_finite_domain, which
///   guards inspection of  a domain_type typedef to views that define it.
///
/// @todo Remove when all views define domain_type (i.e., inherit from
///   @ref core_view)
//////////////////////////////////////////////////////////////////////
template<typename View, bool = has_domain_type<View>::value>
struct has_finite_domain_impl
  : public std::integral_constant<
             bool,
             !(std::is_base_of<infinite_impl::infinite_base,
                               typename View::domain_type>::value)
    >
{ };


template<typename View>
struct has_finite_domain_impl<View, false>
  : public std::true_type
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction returning true if View parameter has a
///   domain type other than @ref infinite.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct has_finite_domain
  : public has_finite_domain_impl<View>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction returning the index of the first element
///   (i.e., view) of the tuple parameter @p Views which has a finite
///   domain.
//////////////////////////////////////////////////////////////////////
template<typename Views>
struct first_finite_domain_index
  : public stapl::find_first_index<Views, has_finite_domain>
{ };

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_INFINITE_HELPERS_HPP
