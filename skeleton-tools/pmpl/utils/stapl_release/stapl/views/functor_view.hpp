/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_FUNCTOR_VIEW_HPP
#define STAPL_VIEWS_FUNCTOR_VIEW_HPP

#include <stapl/views/array_view.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/containers/generators/functor.hpp>

#include <iostream>

namespace stapl {


//////////////////////////////////////////////////////////////////////
/// @brief Defines a nested trait type that is the type of a functor view
/// parameterized with a functor type
//////////////////////////////////////////////////////////////////////
template <typename Func, int n>
struct functor_view_type
{
  typedef multiarray_view<functor_container<Func, n>> type;
};

//////////////////////////////////////////////////////////////////////
/// @brief Defines a nested trait type that is the type of a functor view
/// parameterized with a functor type
//////////////////////////////////////////////////////////////////////
template <typename Func>
struct functor_view_type<Func, 1>
{
  typedef array_view<functor_container<Func, 1>> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper function that creates an array view on top of a
///        functor container.
/// @param n Number of elements in the container.
/// @param func Functor to be called on each [] operator call.
//////////////////////////////////////////////////////////////////////
template <typename... D, typename Func>
typename functor_view_type<Func, sizeof...(D)>::type
functor_view(std::tuple<D...> const& size, Func const& func)
{
  return typename functor_view_type<Func, sizeof...(D)>::type(
                    new functor_container<Func, sizeof...(D)>(size, func));
}

//////////////////////////////////////////////////////////////////////
/// @brief Helper function that creates an array view on top of a
///        functor container.
/// @param size Number of elements in the container.
/// @param func Functor to be called on each [] operator call.
//////////////////////////////////////////////////////////////////////
template <typename Func>
typename functor_view_type<Func, 1>::type
functor_view(std::size_t size, Func const& func)
{
  return typename functor_view_type<Func, 1>::type(
                    new functor_container<Func, 1>(size, func));
}

} // stapl namespace

#endif // STAPL_VIEWS_FUNCTOR_VIEW_HPP
