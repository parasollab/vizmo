/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_TUPLE_TUPLE_CONTAINTS_HPP
#define STAPL_UTILITY_TUPLE_TUPLE_CONTAINTS_HPP

#include <type_traits>
#include <tuple>

namespace stapl {
namespace tuple_ops {

//////////////////////////////////////////////////////////////////////
/// @brief Metafunction that computes whether an integral type
///        appears in a tuple of integral types
///
/// @tparam T The type to search for
/// @tparam Tuple The tuple to search in
//////////////////////////////////////////////////////////////////////
template <typename T, typename Tuple>
struct tuple_contains;

template <typename T, typename Head, typename... Tail>
struct tuple_contains<T, std::tuple<Head, Tail...>>
 : public std::integral_constant<bool, T::value == Head::value ||
            tuple_contains<T, std::tuple<Tail...>>::value>
{ };

template<typename T>
struct tuple_contains<T, std::tuple<>>
 : public std::integral_constant<bool, false>
{ };

} // namespace tuple_ops
} // namespace stapl

#endif // STAPL_UTILITY_TUPLE_TUPLE_CONTAINTS_HPP
