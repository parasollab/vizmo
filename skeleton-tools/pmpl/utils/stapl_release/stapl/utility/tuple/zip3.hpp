/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_TUPLE_ZIP3_HPP
#define STAPL_UTILITY_TUPLE_ZIP3_HPP

#include <stapl/utility/integer_sequence.hpp>
#include <tuple>

namespace stapl {
namespace tuple_ops {

namespace detail {

template<
  typename Tuple1,
  typename Tuple2,
  typename Tuple3,
  typename IdxList = make_index_sequence<tuple_size<Tuple1>::value>>
struct zip3_impl;


template<typename ...Elements1,
         typename ...Elements2,
         typename ...Elements3,
         std::size_t... Indices>
struct zip3_impl<
  std::tuple<Elements1...>,
  std::tuple<Elements2...>,
  std::tuple<Elements3...>,
  index_sequence<Indices...>
>
{
  auto operator()(std::tuple<Elements1...> const& t1,
                  std::tuple<Elements2...> const& t2,
                  std::tuple<Elements3...> const& t3) const
  STAPL_AUTO_RETURN(
    std::make_tuple(
      std::make_tuple(std::get<Indices>(t1), std::get<Indices>(t2),
                      std::get<Indices>(t3))...
    )
  )
};

} // namespace detail


template<typename Tuple1, typename Tuple2, typename Tuple3>
auto zip(Tuple1 const& t1, Tuple2 const& t2, Tuple3 const& t3)
STAPL_AUTO_RETURN((
  detail::zip3_impl<Tuple1, Tuple2, Tuple3>()(t1, t2, t3)
))

} // namespace tuple_ops
} // namespace stapl

#endif // STAPL_UTILITY_TUPLE_ZIP3_HPP
