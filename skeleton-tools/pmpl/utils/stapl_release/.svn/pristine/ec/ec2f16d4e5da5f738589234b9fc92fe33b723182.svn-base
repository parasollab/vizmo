/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_TYPE_TRAITS_TRANSPORT_QUALIFIER_HPP
#define STAPL_RUNTIME_TYPE_TRAITS_TRANSPORT_QUALIFIER_HPP

#include "is_copyable.hpp"
#include "is_movable.hpp"
#include <type_traits>

namespace stapl {

namespace runtime {

////////////////////////////////////////////////////////////////////
/// @brief If the type @p T is an rvalue reference type and @c is_movable<T> is
///        @c true_type, it provides the member typedef @p type which is @p T.
///        Otherwise @p type is the type referred to by @p T.
///
/// @ingroup runtimeTypeTraits
////////////////////////////////////////////////////////////////////
template<typename T>
struct transport_qualifier;


////////////////////////////////////////////////////////////////////
/// @internal
/// @brief Specialization of @ref transport_qualifier for lvalue references.
///
/// @ingroup runtimeTypeTraits
////////////////////////////////////////////////////////////////////
template<typename T>
struct transport_qualifier<T&>
: public std::conditional<is_copyable<T>::value, copyable<T>, T>
{ };


////////////////////////////////////////////////////////////////////
/// @internal
/// @brief Specialization of @ref transport_qualifier for rvalue references.
///
/// @ingroup runtimeTypeTraits
////////////////////////////////////////////////////////////////////
template<typename T>
struct transport_qualifier<T&&>
: public std::conditional<is_movable<T>::value, movable<T>, T>
{ };

} // namespace runtime

} // namespace stapl

#endif
