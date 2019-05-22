/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_TUPLE_FIND_FIRST_INDEX_HPP
#define STAPL_UTILITY_TUPLE_FIND_FIRST_INDEX_HPP

#include <tuple>
#include <type_traits>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Helper function to find the first item in a given tuple
/// that satisfies a given predicate.
///
/// @tparam Items      the tuple of input elements
/// @tparam Predicate  the predicate to be tested on each element
/// @tparam index      the starting index to start the search
/// @tparam items_left determines the number of items left to
///                    check the predicate
/// @tparam Arg        additional arguments to pass to Predicate after
///                    each element
//////////////////////////////////////////////////////////////////////
template <typename Items, template <typename...> class Predicate,
          int index,
          int items_left,
          typename... Arg>
struct find_first_index_impl
  : std::conditional<
      Predicate<typename std::tuple_element<index, Items>::type, Arg...>::value,
      std::integral_constant<int, index>,
      find_first_index_impl<Items, Predicate, index+1, items_left - 1, Arg...>
    >::type
{ };


//////////////////////////////////////////////////////////////////////
/// @brief The base case for @c find_first_index_impl
///
/// @tparam Items     the tuple of input elements
/// @tparam Predicate the predicate to be tested on each element
/// @tparam index     the starting index to start the search
/// @tparam Args      additional arguments to pass to Predicate after
///                   each element
//////////////////////////////////////////////////////////////////////
template<typename Items, template <typename...> class Predicate,
         int index, typename... Arg>
struct find_first_index_impl<Items, Predicate, index, 0, Arg...>
  : public std::integral_constant<int, -1>
{ };

} // namespace detail

//////////////////////////////////////////////////////////////////////
/// @brief Finds the index of the first item in a given tuple that
/// satisfies a given predicate.
///
/// @tparam Items      the tuple of input elements
/// @tparam Predicate  the predicate to be tested on each element
/// @tparam Arg        additional arguments to pass to Predicate after
///                    each element
//////////////////////////////////////////////////////////////////////
template <typename Items,
          template <typename...> class Predicate,
          typename... Arg>
using find_first_index = detail::find_first_index_impl<
                           Items, Predicate,
                           0, std::tuple_size<Items>::value, Arg...>;
} // namespace stapl

#endif // STAPL_UTILITY_TUPLE_FIND_FIRST_INDEX_HPP
