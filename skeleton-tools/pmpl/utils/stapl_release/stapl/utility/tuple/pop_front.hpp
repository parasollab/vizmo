/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_POP_FRONT_HPP
#define STAPL_UTILITY_POP_FRONT_HPP

#include <tuple>
#include <stapl/utility/tuple/tuple_size.hpp>
#include <stapl/utility/integer_sequence.hpp>

namespace stapl {
namespace tuple_ops {

namespace result_of {

template<typename Tuple,
         typename IdxList = make_index_sequence<tuple_size<Tuple>::value - 1>>
struct pop_front;


template<typename ...Elements, std::size_t... Indices>
struct pop_front<const std::tuple<Elements...>, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename std::tuple_element<
                   Indices+1, std::tuple<Elements...>>::type...>;

  static type call(std::tuple<Elements...> const& elements)
  {
    return type(std::get<Indices+1>(elements)...);
  }
};

template<typename ...Elements, std::size_t... Indices>
struct pop_front<std::tuple<Elements...>, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename std::tuple_element<
                   Indices+1, std::tuple<Elements...>>::type...>;

  static type call(std::tuple<Elements...> const& elements)
  {
    return type(std::get<Indices+1>(elements)...);
  }
};

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief Returns a new tuple, with the first element of the original
/// removed.
///
/// @param t a tuple
//////////////////////////////////////////////////////////////////////
template<typename Tuple>
auto
pop_front(Tuple const& t)
STAPL_AUTO_RETURN((
  result_of::pop_front<Tuple>::call(t)
))

} // namespace tuple_ops
} // namespace stapl

#endif // STAPL_UTILITY_POP_FRONT_HPP
