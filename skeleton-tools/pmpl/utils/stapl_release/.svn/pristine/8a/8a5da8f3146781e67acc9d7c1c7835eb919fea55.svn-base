/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_ARRAY_VIEW_OPERATIONS_HPP
#define STAPL_VIEWS_ARRAY_VIEW_OPERATIONS_HPP

#include <stapl/paragraph/paragraph.hpp>
#include <stapl/views/repeated_view.hpp>
#include <stapl/containers/array/static_array.hpp>

namespace stapl {

namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Work function to multiply two values (@p elt0, @p elt1),
///        assigning the result to the given reference @p result.
/// @note @p result should be a proxy.
/// @todo Verify if this work function can be implemented using the
///       functions in @ref functional.hpp.
//////////////////////////////////////////////////////////////////////
struct multiplication_map_wf
{
  typedef void result_type;

  template<typename Elt0, typename Elt1, typename Result>
  void
  operator()(Elt0 const& elt0, Elt1 const& elt1, Result result)
  {
    result = elt0 * elt1;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function to add two values (@p elt0, @p elt1),
///        assigning the result to the given reference @p result.
/// @note @p result should be a proxy.
/// @todo Verify if this work function can be implemented using the
///       functions in @ref functional.hpp.
//////////////////////////////////////////////////////////////////////
struct addition_map_wf
{
  typedef void result_type;

  template<typename Elt0, typename Elt1, typename Result>
  void
  operator()(Elt0 const& elt0, Elt1 const& elt1, Result result)
  {
    result = elt0 + elt1;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Work function to subtract two values (@p elt0 - @p elt1),
///        assigning the result to the given reference @p result.
/// @note @p result should be a proxy.
/// @todo Verify if this work function can be implemented using the
///       functions in @ref functional.hpp.
//////////////////////////////////////////////////////////////////////
struct subtraction_map_wf
{
  typedef void result_type;

  template<typename Elt0, typename Elt1, typename Result>
  void
  operator()(Elt0 const& elt0, Elt1 const& elt1, Result result)
  {
    result = elt0 - elt1;
  }
};

} //namespace view_impl


template <typename Container, typename ...OptionalParams>
inline array_view<static_array<typename Container::value_type> >
operator*(array_view<Container, OptionalParams...> const& x,
          array_view<Container, OptionalParams...> const& y)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::multiplication_map_wf(), x, y, z);
  return z;
}


//////////////////////////////////////////////////////////////////////
/// @brief Multiplies each element referenced through @ref array_view
///        @p x by the @p scalar provided, returning an @ref array_view
///        over the resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator*(typename Container::value_type const& scalar,
          array_view<Container, OptionalParams...> const& x)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::multiplication_map_wf(), x, make_repeat_view(scalar), z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Multiplies each element referenced through @ref array_view
///        @p x by the @p scalar provided, returning an @ref array_view
///        over the resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator*(array_view<Container, OptionalParams...> const& x,
          typename Container::value_type const& scalar)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::multiplication_map_wf(), x, make_repeat_view(scalar), z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Performs pair-wise addition of the elements referenced by
///        the two given views @p x and @p y, returning a new
///        array_view over the resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator+(array_view<Container, OptionalParams...> const& x,
          array_view<Container, OptionalParams...> const& y)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::addition_map_wf(), x, y, z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Adds a @p scalar to each element referenced through
///        the given array_view, returning a new array_view over the
///        resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator+(typename Container::value_type const& scalar,
          array_view<Container, OptionalParams...> const& x)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::addition_map_wf(), x, make_repeat_view(scalar), z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Adds a @p scalar to each element referenced through
///        the given array_view, returning a new array_view over the
///        resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator+(array_view<Container, OptionalParams...> const& x,
          typename Container::value_type const& scalar)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::addition_map_wf(), x, make_repeat_view(scalar), z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Performs pair-wise subtraction (@p x - @p y) of the elements
///        referenced by the two given views @p x and @p y, returning
///        a new array_view over the resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator-(array_view<Container, OptionalParams...> const& x,
          array_view<Container, OptionalParams...> const& y)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::subtraction_map_wf(), x, y, z);
  return z;
}

//////////////////////////////////////////////////////////////////////
/// @brief Subtracts a @p scalar from each element referenced through
///        the given array_view, returning a new array_view over the
///        resulting container.
//////////////////////////////////////////////////////////////////////
template <typename Container, typename ...OptionalParams>
array_view<static_array<typename Container::value_type> >
operator-(array_view<Container, OptionalParams...> const& x,
          typename Container::value_type const& scalar)
{
  typedef typename Container::value_type        value_type;
  typedef array_view<static_array<value_type> > view_type;

  view_type z(new static_array<value_type>(x.size()));
  map_func(view_impl::subtraction_map_wf(), x, make_repeat_view(scalar), z);
  return z;
}

} //namespace stapl

#endif // STAPL_VIEWS_ARRAY_VIEW_OPERATIONS_HPP
