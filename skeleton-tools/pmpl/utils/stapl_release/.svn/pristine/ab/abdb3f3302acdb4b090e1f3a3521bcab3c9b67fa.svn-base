/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_UTIILTY_TUPLE_DISCARD_HPP
#define STAPL_UTIILTY_TUPLE_DISCARD_HPP

#include <type_traits>
#include <tuple>

#include <stapl/utility/tuple/tuple_contains.hpp>
#include <stapl/utility/tuple/homogeneous_tuple.hpp>

namespace stapl {
namespace tuple_ops {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of both @see tuple_ops::discard and
///        @see tuple_ops::filter.
///
/// @tparam I The index of the input tuple that we are currently considering
/// @tparam D The total size of the input tuple
/// @tparam Tuple The type of the input tuple
/// @tparam FilteredTuple The type of the output tuple
/// @tparam Slices The tuple of compile-time integrals that specifies which
///                indices to keep or discard
/// @tparam Filter Flag indicating whether to filter or discard the tuple
//////////////////////////////////////////////////////////////////////
template<int I, int D, typename Tuple, typename FilteredTuple,
         typename Slices, bool Filter = false>
struct discard_impl
{
  typedef FilteredTuple type;

private:
  template<typename... T>
  static type apply_impl(std::integral_constant<bool, true>,
                         Tuple const& tup, T... t)
  {
    return discard_impl<I+1, D, Tuple, FilteredTuple, Slices, Filter>::apply(
      tup, t..., std::get<I>(tup)
    );
  }

  template<typename... T>
  static type apply_impl(std::integral_constant<bool, false>,
                         Tuple const& tup, T... t)
  {
    return discard_impl<I+1, D, Tuple, FilteredTuple, Slices, Filter>::apply(
      tup, t...
    );
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Filter the tuple and pass along the accumulated result
  /// @param tup The input tuple
  /// @param t A parameter pack of the constituent parts of the output tuple
  //////////////////////////////////////////////////////////////////////
  template<typename... T>
  static type apply(Tuple const& tup, T... t)
  {
    using contains_t = typename tuple_contains<
                         std::integral_constant<int, I>, Slices>::type;

    // if we're filtering the tuple, we should push this value, only if its
    // index appears in the Slices list. otherwise, we want to push the value
    // only if it's not in the list
    constexpr bool should_push =
      Filter ? contains_t::value : !contains_t::value;

    return apply_impl(std::integral_constant<bool, should_push>(), tup, t...);
  }
};


template<int I, typename Tuple, typename FilteredTuple,
         typename Slices, bool Filter>
struct discard_impl<I, I, Tuple,
                          FilteredTuple, Slices, Filter>
{
  typedef FilteredTuple type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Create the final output tuple
  /// @param tup The input tuple
  /// @param t A parameter pack of the constituent parts of the output tuple
  //////////////////////////////////////////////////////////////////////
  template<typename... T>
  static type apply(Tuple const&, T... t)
  {
    return type(t...);
  }
};

} // namespace detail

namespace result_of {

//////////////////////////////////////////////////////////////////////
/// @brief Metafunction that reflects the type of the
///         @see stapl::discard function
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Tuple>
struct discard
{
  using number_of_slices_t   = typename stapl::tuple_size<Slices>::type;
  using original_dimension_t = typename stapl::tuple_size<Tuple>::type;
  using result_size_t        = std::integral_constant<
                                 int,
                                 original_dimension_t::
                                   value-number_of_slices_t::value>;

  using scalar_t             = typename tuple_element<0, Tuple>::type;

  // the result type should be a tuple of size |Tuple|-|Slices|.
  // but if this value is 1, we should just use a scalar
  using type = typename std::conditional<
    result_size_t::value == 1,          // if
    scalar_t,                           // then
    typename homogeneous_tuple_type<    // else
     result_size_t::value, scalar_t
    >::type
  >::type;
};


} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief Discard elements of a d-dimensional tuple by selecting only
///        elements at the indices not specified in a tuple of compile-time
///        integer constants. This function has the inverse behavior
///        of @see filter.
///
///        Note that this function currently only supports tuples
///        of homogeneous types.
///
/// @tparam Slices A tuple of std::integral_constants which specifies
///                which indices in the tuple to discard.
/// @param  t The tuple to filter
/// @return A tuple of size |t|-|Slices| where the rest of the elements are
///         filtered out
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Tuple>
typename result_of::discard<Slices, Tuple>::type
discard(Tuple&& t)
{
  using result_t = typename result_of::discard<Slices, Tuple>::type;

  using filter_t = detail::discard_impl<
                     0,
                     tuple_size<Tuple>::type::value,
                     Tuple, result_t, Slices>;

  return filter_t::apply(std::forward<Tuple>(t));
}

} // namespace tuple_ops
} // namespace stapl

#endif // STAPL_UTIILTY_TUPLE_DISCARD_HPP
