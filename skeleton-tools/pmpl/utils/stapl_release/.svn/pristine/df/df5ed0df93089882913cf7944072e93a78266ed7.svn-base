/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_ARRAY_BASE_CONTAINER_TRAITS_HPP
#define STAPL_ARRAY_BASE_CONTAINER_TRAITS_HPP

#include <vector>

#include <stapl/containers/type_traits/define_value_type.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Default traits for the array base container. Specifies customizable
///   type parameters that could be changed on a per-container basis.
/// @ingroup parrayTraits
///
/// @tparam T Type of the stored elements in the base container.
/// @tparam Domain Domain for the base container.
/// @see array_base_container
////////////////////////////////////////////////////////////////////////
template <typename T, typename Domain>
struct array_base_container_traits
{
  typedef typename define_value_type<T>::type   stored_type;
  typedef std::vector<stored_type>              container_type;
  typedef container_type                        container_constructor;
  typedef T                                     value_type;
  typedef Domain                                domain_type;
};

} // namespace stapl

#endif
