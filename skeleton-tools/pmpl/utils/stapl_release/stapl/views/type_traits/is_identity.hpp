/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_TYPE_TRAITS_IS_IDENTITY_HPP
#define STAPL_VIEWS_TYPE_TRAITS_IS_IDENTITY_HPP

#include <boost/mpl/bool.hpp>
#include <stapl/views/mapping_functions/identity.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Type checker to determine if a mapping function is an
///        identity mapping function.
//////////////////////////////////////////////////////////////////////
template <typename MF>
struct is_identity
  : boost::mpl::false_
{ };


template <typename T>
struct is_identity<f_ident<T> >
  : boost::mpl::true_
{ };

} // stapl namespace

#endif /* STAPL_VIEWS_TYPE_TRAITS_IS_IDENTITY_HPP */
