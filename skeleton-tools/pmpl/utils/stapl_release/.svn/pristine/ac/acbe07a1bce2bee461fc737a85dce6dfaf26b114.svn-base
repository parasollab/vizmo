/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_UTILITY_FILTERS_HPP
#define STAPL_SKELETONS_UTILITY_FILTERS_HPP

#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/integer_sequence.hpp>

namespace stapl {
namespace skeletons {
namespace filters {


template <typename Filter, typename T>
auto apply_filter(Filter const& filter, T&& t)
STAPL_AUTO_RETURN((
  filter(std::forward<T>(t))
))

template <typename T>
T apply_filter(skeletons::no_filter const& filter, T&& t)
{
  return t;
}

template<typename Dir>
void set_direction(skeletons::no_filter, Dir&&)
{
}

template<typename Filter, typename Dir>
void set_direction(Filter& f, Dir&& d)
{
  f.set_direction(d);
}

template<typename Filter, typename V>
struct result_of : boost::result_of<Filter(V)>
{ };


template<typename V>
struct result_of<skeletons::no_filter,V>
{
  using type = V;
};


//////////////////////////////////////////////////////////////////////
/// @brief Filters are used in the skeleton library in order to reduce
/// the size of the coordinate vectors being passed around.
///
/// This mask keeps only the first @c i dimensions of the given
/// coordinate. For example, if <i, j, k, ...> is passed to a
/// filter<2>, <i, j> will be returned.
///
/// @tparam i number of dimensions to keep
///
/// @ingroup skeletonsUtilities
//////////////////////////////////////////////////////////////////////
template <int i, typename Indices = make_index_sequence<i>>
struct filter;

template <int i, std::size_t... Indices>
struct filter<i, index_sequence<Indices...>>
{
  template <typename T>
  auto operator()(T&& t)
  STAPL_AUTO_RETURN((
    stapl::make_tuple(stapl::get<Indices>(t)...)
  ))
};

} // namespace filters
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_UTILITY_FILTERS_HPP
