/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTILITY_TUPLE_TRANSFORM_HPP
#define STAPL_UTILITY_TUPLE_TRANSFORM_HPP

#include <tuple>
#include <stapl/utility/integer_sequence.hpp>
#include <stapl/utility/tuple/tuple_size.hpp>
#include <stapl/utility/tuple/tuple_element.hpp>
#include <boost/utility/result_of.hpp>

namespace stapl {

namespace result_of {

template <
  typename Tuple,
  template <typename> class Functor,
  typename IdxList = make_index_sequence<tuple_size<Tuple>::value>>
struct transform1;


template <typename ...Elements, template <typename> class Functor,
          std::size_t... Indices>
struct transform1<std::tuple<Elements...>, Functor, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename Functor<
                   typename tuple_element<
                     Indices, std::tuple<Elements...>>::type
                 >::type...>;
};


template <template <typename> class Functor>
struct transform1<std::tuple<>, Functor, index_sequence<>>
{
  using type = std::tuple<>;
};


template <
  typename Tuple1,
  typename Tuple2,
  template <typename, typename> class Functor,
  typename IdxList = make_index_sequence<tuple_size<Tuple1>::value>>
struct transform2;


template <typename ...Elements1, typename ...Elements2,
         template <typename, typename> class Functor, std::size_t... Indices>
struct transform2<std::tuple<Elements1...>, std::tuple<Elements2...>,
                  Functor, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename Functor<
                   typename tuple_element<
                     Indices, std::tuple<Elements1...>>::type,
                   typename tuple_element<
                     Indices, std::tuple<Elements2...>>::type
                 >::type...>;
};


template <template <typename, typename> class Functor>
struct transform2<std::tuple<>, std::tuple<>, Functor, index_sequence<>>
{
  using type = std::tuple<>;
};


template <
  typename Tuple1,
  typename Tuple2,
  typename Functor = void,
  typename IdxList = make_index_sequence<tuple_size<Tuple1>::value>>
struct transform;


template <typename ...Elements1, typename ...Elements2,
         typename Functor, std::size_t... Indices>
struct transform<std::tuple<Elements1...>, std::tuple<Elements2...>,
                 Functor, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename boost::result_of<Functor(
                   typename tuple_element<
                     Indices, std::tuple<Elements1...>>::type,
                   typename tuple_element<
                     Indices, std::tuple<Elements2...>>::type
                 )>::type...>;
};

template <typename ...Elements1,
         typename Functor, std::size_t... Indices>
struct transform<std::tuple<Elements1...>, Functor,
                 void, index_sequence<Indices...>>
{
  using type = std::tuple<
                 typename boost::result_of<Functor(
                   typename tuple_element<
                     Indices, std::tuple<Elements1...>>::type
                 )>::type...>;
};

} // namespace result_of

namespace tuple_ops {

namespace detail {

template <
  typename Functor,
  typename Tuple1,
  typename IdxList = make_index_sequence<tuple_size<Tuple1>::value>>
struct transform1_impl;


template <typename Functor,
          typename ...Elements1,
          std::size_t... Indices>
struct transform1_impl<Functor,
                       std::tuple<Elements1...>,
                       index_sequence<Indices...>>
{
  auto operator()(std::tuple<Elements1...> const& t1,
                  Functor const& f) const
  STAPL_AUTO_RETURN(
    std::make_tuple(f(std::get<Indices>(t1))...)
  )
};


template <
  typename Functor,
  typename Tuple1,
  typename Tuple2,
  typename IdxList = make_index_sequence<tuple_size<Tuple1>::value>>
struct transform2_impl;


template <typename Functor,
         typename ...Elements1,
         typename ...Elements2,
         std::size_t... Indices>
struct transform2_impl<Functor,
                       std::tuple<Elements1...>,
                       std::tuple<Elements2...>,
                       index_sequence<Indices...>
>
{
  auto operator()(std::tuple<Elements1...> const& t1,
                  std::tuple<Elements2...> const& t2,
                  Functor const& f) const
  STAPL_AUTO_RETURN(
    std::make_tuple(f(std::get<Indices>(t1), std::get<Indices>(t2))...)
  )
};

} // namespace detail


//////////////////////////////////////////////////////////////////////
/// @brief Creates a new tuple by applying @c f on each element of the
/// given tuple.
///
/// @param t1 a tuple
/// @param f  a unary operator
//////////////////////////////////////////////////////////////////////
template <typename Tuple1, typename Functor>
inline
auto
transform(Tuple1 const& t1, Functor const& f)
STAPL_AUTO_RETURN((
  detail::transform1_impl<Functor, Tuple1>()(t1, f)
))


//////////////////////////////////////////////////////////////////////
/// @brief Creates a new tuple by applying @c f on each element of the
/// given tuples.
///
/// @param t1 a tuple
/// @param t2 a tuple
/// @param f  a binary operator
//////////////////////////////////////////////////////////////////////
template <typename Tuple1, typename Tuple2, typename Functor>
inline
auto
transform(Tuple1 const& t1, Tuple2 const& t2, Functor const& f)
STAPL_AUTO_RETURN((
  detail::transform2_impl<Functor, Tuple1, Tuple2>()(t1, t2, f)
))

} // namespace tuple_ops


} // namespace stapl

#endif // STAPL_UTILITY_TUPLE_TRANSFORM_HPP
