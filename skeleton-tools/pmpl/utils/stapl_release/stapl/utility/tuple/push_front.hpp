/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_TUPLE_PUSH_FRONT_HPP
#define STAPL_UTILITY_TUPLE_PUSH_FRONT_HPP

#include <tuple>
#include <stapl/utility/integer_sequence.hpp>
#include <stapl/utility/tuple/tuple_element.hpp>

namespace stapl {
namespace tuple_ops {

namespace result_of {

template<typename Tuple,
         typename T,
         typename IdxList = make_index_sequence<tuple_size<Tuple>::value>>
struct push_front;

template <typename T>
struct push_front<std::tuple<>, T, index_sequence<>>
{
  using type = std::tuple<T>;

  static type call(std::tuple<> const&, T const& val)
  {
    return type(val);
  }
};

template <typename ...Elements, typename T, std::size_t... Indices>
struct push_front<const std::tuple<Elements...>, T, index_sequence<Indices...>>
{
  using type = std::tuple<
                 T,
                 typename std::tuple_element<
                   Indices, std::tuple<Elements...>>::type...>;

  static type call(std::tuple<Elements...> const& elements, T const& val)
  {
    return type(val, std::get<Indices>(elements)...);
  }
};



template <typename ...Elements, typename T, std::size_t... Indices>
struct push_front<std::tuple<Elements...>, T, index_sequence<Indices...>>
{
  using type = std::tuple<
                 T,
                 typename std::tuple_element<
                   Indices, std::tuple<Elements...>>::type...>;

  static type call(std::tuple<Elements...> const& elements, T const& val)
  {
    return type(val, std::get<Indices>(elements)...);
  }
};

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief Returns a new tuple with @c val added at the beginning.
///
/// @param t   a tuple
/// @param val the new element to be added at the beginning
//////////////////////////////////////////////////////////////////////
template<typename ...Elements, typename T>
auto push_front(std::tuple<Elements...> const& t, T const& val)
STAPL_AUTO_RETURN((
  result_of::push_front<std::tuple<Elements...>, T>::call(t, val)
))

} // namespace tuple_ops
} // namespace stapl

#endif // STAPL_UTILITY_TUPLE_PUSH_FRONT_HPP
