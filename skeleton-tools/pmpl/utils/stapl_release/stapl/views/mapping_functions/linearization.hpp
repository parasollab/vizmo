/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_MAPFUNCS_LINEARIZATION_HPP
#define STAPL_MAPFUNCS_LINEARIZATION_HPP

#include <stapl/algorithms/functional.hpp>
#include <stapl/views/mapping_functions/mapping_functions.hpp>
#include <stapl/utility/tuple.hpp>
#include <stapl/utility/integer_sequence.hpp>

#include <boost/mpl/find_if.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/distance.hpp>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Boost.MPL conforming metafunction to check that the passed
///   typedef constant is equal to @p N, regardless of whether it is
///   a boost::mpl::int_ or std::integral_constant.
///
/// @todo Remove when we have completely moved to std::integral_constant.
//////////////////////////////////////////////////////////////////////
template<int N>
struct type_constant_equal
{
  template<typename Arg1>
  struct apply
  {
    typedef boost::mpl::bool_<Arg1::value == N> type;
  };
};


//////////////////////////////////////////////////////////////////////
/// @brief Rearrange the contents of a tuple to be in the correct
///        traversal order.
///
/// For example, given the tuple (x, y, z) and the traversal <1, 2, 0>,
/// output the tuple (z, x, y).
/// @tparam N Index into the tuple that is being iterated on.
/// @tparam Traversal Traversal ordering.
//////////////////////////////////////////////////////////////////////
template<typename Tuple, typename Traversal,
         typename Indices = make_index_sequence<tuple_size<Tuple>::value>>
struct traversal_order;


template<typename Tuple, typename... IntTypes, std::size_t... Indices>
struct traversal_order<Tuple, tuple<IntTypes...>, index_sequence<Indices...>>
{
private:
  typedef boost::mpl::vector<IntTypes...>                  vector_t;

  typedef tuple<
    typename boost::mpl::distance<
      typename boost::mpl::begin<vector_t>::type,
      typename boost::mpl::find_if<
        vector_t, type_constant_equal<Indices>>::type
    >::type...>                                            indices_t;

public:
  static Tuple apply(Tuple const& src)
  {
    return Tuple(get<tuple_element<Indices, indices_t>::type::value>(src)...);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Rearrange the contents of a tuple to be in the reverse
///        traversal order.
/// @tparam Tuple Type of tuple to rearrange.
/// @tparam Travesal Traversal ordering.
///
/// For example, given the tuple (x, y, z) and the traversal <1, 2, 0>,
/// output the tuple (y, z, x).
//////////////////////////////////////////////////////////////////////
template<typename Tuple, typename Traversal,
         typename Indices = make_index_sequence<tuple_size<Tuple>::value>>
struct reverse_traversal_order;


template<typename Tuple, typename Traversal, std::size_t... Indices>
struct reverse_traversal_order<Tuple, Traversal, index_sequence<Indices...>>
{
  static Tuple apply(Tuple const& src)
  {
    return Tuple(get<tuple_element<Indices, Traversal>::type::value>(src)...);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Computes the product of all of the elements of a tuple.
/// @tparam N Tuple index that is currently being computed.
/// @tparam Size Tuple type.
//////////////////////////////////////////////////////////////////////
template<int N, typename Size>
struct linearize_size
{
  static size_t apply(Size const& size)
  {
    return get<N>(size) * linearize_size<N-1, Size>::apply(size);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case of recursion for @ref linearize_size.
//////////////////////////////////////////////////////////////////////
template<typename Size>
struct linearize_size<-1, Size>
{
  static size_t apply(Size const&)
  { return 1; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Translates a n-dimensional gid to a 1-dimensional linearization.
/// @tparam N Tuple index that is currently being computed.
/// @tparam GID Tuple type.
///
/// $p = \sum_{i=0}^n gid[i] * \prod_{j=0}^i-1 size[i]$
//////////////////////////////////////////////////////////////////////
template<int N, typename GID>
struct linearize
{
  static size_t apply(GID const& size, GID const& gid)
  {
    return get<N>(gid) *
      linearize_size<N-1, GID>::apply(size) +
      linearize<N-1, GID>::apply(size, gid);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case of recursion for @ref linearize.
//////////////////////////////////////////////////////////////////////
template<typename GID>
struct linearize<-1, GID>
{
  static size_t apply(GID const&, GID const&)
  { return 0; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case of recursion for @ref linearize_size for
///        1-dimension (identity).
//////////////////////////////////////////////////////////////////////
template<int N>
struct linearize_size<N, size_t>
{
  size_t operator()(size_t size) const
  { return size; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Computes the modulus of a 1-dimensional number with the
///        linear size of all of the dimensions before the current
///        dimension @p N.
//////////////////////////////////////////////////////////////////////
template<int N>
struct apply_modulo
{
  template<class Tuple>
  static size_t apply(Tuple const& sizes, size_t result)
  {
    return apply_modulo<N-1>::apply(
      tuple_ops::pop_back(sizes),
      result % tuple_ops::fold(sizes, 1, stapl::multiplies<size_t>()));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case @ref apply_modulo.
//////////////////////////////////////////////////////////////////////
template<>
struct apply_modulo<0>
{
  template<class Tuple>
  static size_t apply(Tuple const&, size_t result)
  { return result; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Divides a given number by all of the elements of a tuple.
///
/// Start with N = N-1 and sizes in traversal order
//////////////////////////////////////////////////////////////////////
template<int N>
struct apply_divide
{
  template<class Tuple>
  static size_t apply(Tuple const& sizes, size_t result)
  {
    return apply_divide<N-1>::apply(sizes, result / get<N>(sizes));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case @ref apply_divide.
//////////////////////////////////////////////////////////////////////
template<>
struct apply_divide<-1>
{
  template<class Tuple>
  static size_t apply(Tuple const&, size_t result)
  { return result; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Translates a 1-dimensional linearization back to its
///        n-dimensional form.
///
/// This functor assumes that the sizes are in the correct traversal
/// order.
//////////////////////////////////////////////////////////////////////
template<int I, typename GID>
struct reverse_linearize
{
  static GID apply(GID const& m_size, size_t linear, GID gid)
  {
    size_t result =
      apply_modulo<tuple_size<GID>::value-1-I>::apply(
        tuple_ops::pop_back(m_size), linear);

    result = apply_divide<I-1>::apply(m_size, result);

    get<I>(gid) = result;

    return reverse_linearize<I-1, GID>::apply(m_size, linear, gid);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case of recursion for @ref reverse_linearize.
/// @note Intel fails if function operator takes @p gid by reference.
//////////////////////////////////////////////////////////////////////
template<typename GID>
struct reverse_linearize<-1, GID>
{
  static GID apply(GID const&, size_t, GID gid)
  { return gid; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Small function to optimize index linearization by not
/// performing multiplication on the first element.
//////////////////////////////////////////////////////////////////////
template<int Idx>
struct conditional_multiply
{
  template<typename Tuple>
  static size_t apply(Tuple const& plane_sizes, size_t lhs)
  {
    return lhs * get<Idx>(plane_sizes);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for first element of tuple.  Do not perform
/// multiplication and assert the first element in plane_sizes is 1.
//////////////////////////////////////////////////////////////////////
template<>
struct conditional_multiply<0>
{
  template<typename Tuple>
  static size_t apply(Tuple const& plane_sizes, size_t lhs)
  {
    stapl_assert(get<0>(plane_sizes) == 1, "found last idx != 1");
    return lhs;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Reorders input Indices according to provided traversal order,
/// decrements each by corresponding value in @p first and then multiplies
/// by corresponding value in @p plane_sizes, accumulating the results into
/// a linearized index.
//////////////////////////////////////////////////////////////////////
template <int Iterate, int LastIdx, typename Traversal>
struct reorder_localize_linearize
{
  template<typename Tuple, typename Index, typename... Indices>
  static size_t apply(Tuple const& first, Tuple const& plane_sizes,
                    Index&& i, Indices&&... is)

 {
   constexpr size_t idx = std::tuple_element<Iterate, Traversal>::type::value;

   return conditional_multiply<idx>::apply(
     plane_sizes, i - std::get<idx>(first))
     + reorder_localize_linearize<Iterate + 1, LastIdx, Traversal>::apply(
         first, plane_sizes, std::forward<Indices>(is)...);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Base case of the recursion.
/// @note Traversal of the tuple occurs from 0 to tuple_size-1 for performance
/// reasons.
//////////////////////////////////////////////////////////////////////
template<int LastIdx, typename Traversal>
struct reorder_localize_linearize<LastIdx, LastIdx, Traversal>
{
  template<typename Tuple, typename Index>
  static size_t apply(Tuple const& first, Tuple const& plane_sizes, Index&& i)
  {
    constexpr size_t idx = std::tuple_element<LastIdx, Traversal>::type::value;

    return conditional_multiply<idx>::apply(
      plane_sizes, i - std::get<idx>(first));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Computes the factor by which a component of an n-dimensional GID
/// should be multiplied when linearizing the GID.
//////////////////////////////////////////////////////////////////////
template<int Iterate, int LastIdx>
struct compute_plane_sizes
{
  template <typename Tuple>
  static void apply(Tuple const& size, Tuple& plane_size)
  {
    // Other indices multiplied by size of all inner dimensions.
    std::get<Iterate>(plane_size) =
      std::get<Iterate-1>(size)*std::get<Iterate-1>(plane_size);
    compute_plane_sizes<Iterate+1, LastIdx>::apply(size, plane_size);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for first iteration of @ref compute_plane_sizes.
///
/// The first element represents the innermost dimension of the traversal,
/// and thus isn't multiplied by any of the other dimensions.
/// @note Traversal of the tuple occurs from 0 to tuple_size-1 for performance
/// reasons.
//////////////////////////////////////////////////////////////////////
template<int LastIdx>
struct compute_plane_sizes<0, LastIdx>
{
  template <typename Tuple>
  static void apply(Tuple const& size, Tuple& plane_size)
  {
    // Innermost dimension is multiplied by 1 when linearizing.
    std::get<0>(plane_size) = 1;
    compute_plane_sizes<1, LastIdx>::apply(size, plane_size);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for final iteration of @ref compute_plane_sizes.
///
/// The last element represents the outermost dimension of the traversal,
/// and thus is multiplied by the size of all inner dimensions when linearized.
/// @note Traversal of the tuple occurs from 0 to tuple_size-1 for performance
/// reasons.
//////////////////////////////////////////////////////////////////////
template<int LastIdx>
struct compute_plane_sizes<LastIdx, LastIdx>
{
  template <typename Tuple>
  static void apply(Tuple const& size, Tuple& plane_size)
  {
    // Plane size is size multiplied by size of all inner dimensions.
    std::get<LastIdx>(plane_size) =
      std::get<LastIdx-1>(size)*std::get<LastIdx-1>(plane_size);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for the 1-D case.
//////////////////////////////////////////////////////////////////////
template<>
struct compute_plane_sizes<0, 0>
{
  template <typename Tuple>
  static void apply(Tuple const& size, Tuple& plane_size)
  {
    // Innermost dimension is multiplied by 1 when linearizing.
    std::get<0>(plane_size) = 1;
  }
};
} // namespace detail


template<typename GID, typename Traversal>
struct nd_reverse_linearize;


//////////////////////////////////////////////////////////////////////
/// @brief Translates an n-dimensional @p gid to a 1-dimensional
///        linearization, based on the given @p Traversal order.
//////////////////////////////////////////////////////////////////////
template<typename GID, typename Traversal>
struct nd_linearize
{
  typedef Traversal  traversal_type;
  typedef GID        index_type;
  typedef size_t     gid_type;
  typedef index_type size_type;
  typedef nd_reverse_linearize<GID, Traversal>      inverse;
  typedef std::true_type is_bijective;

  enum { last_idx = tuple_size<size_type>::value - 1 };

  size_type         m_size;

private:
  size_type         m_plane_sizes;
  size_type         m_first;

public:
  nd_linearize(void) = default;

  explicit
  nd_linearize(size_type size)
    : m_size(detail::traversal_order<GID, Traversal>::apply(size)), m_first()
  {
    detail::compute_plane_sizes<0, last_idx>::apply(m_size, m_plane_sizes);
  }

  explicit
  nd_linearize(size_type size, size_type first)
    : m_size(detail::traversal_order<GID, Traversal>::apply(size)),
      m_first(detail::traversal_order<GID, Traversal>::apply(first))
  {
    detail::compute_plane_sizes<0, last_idx>::apply(m_size, m_plane_sizes);
  }

  explicit
  nd_linearize(nd_reverse_linearize<GID, Traversal> const& other)
    : m_size(other.m_size), m_first()
  {
    detail::compute_plane_sizes<0, last_idx>::apply(m_size, m_plane_sizes);
  }

  //////////////////////////////////////////////////////////////////////
  /// Localize and then linearize the indices that are the components of
  /// an n-dimensional GID.
  //////////////////////////////////////////////////////////////////////
  template <typename... Indices>
  gid_type operator()(Indices const&... is) const
  {
    return detail::reorder_localize_linearize<0, last_idx, Traversal>::apply(
      m_first, m_plane_sizes, is...);
  }

private:
  template<std::size_t... I>
  gid_type
  linearize_impl(index_type const& gid, index_sequence<I...> const&) const
  {
    return detail::reorder_localize_linearize<0, last_idx, Traversal>::apply(
      m_first, m_plane_sizes, std::get<I>(gid)...);
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// Localize and then linearize the n-dimensional GID provided.
  //////////////////////////////////////////////////////////////////////
  template<typename Indices =
             make_index_sequence<tuple_size<index_type>::value>>
  gid_type operator()(index_type const& gid) const
  {
    return linearize_impl(gid, Indices());
  }

  void define_type(typer& t)
  {
    t.member(m_size);
    t.member(m_plane_sizes);
    t.member(m_first);
  }
}; // struct nd_linearize


//////////////////////////////////////////////////////////////////////
/// @brief Base case for @ref nd_linearize for 1-dimensional GID.
///
/// @todo Possibly use of the f_ident mapping function for this.
//////////////////////////////////////////////////////////////////////
template<typename Traversal>
struct nd_linearize<size_t, Traversal>
{
  nd_linearize(void) = default;

  explicit
  nd_linearize(size_t)
  { }

  size_t operator()(size_t gid) const
  { return gid; }
};


//////////////////////////////////////////////////////////////////////
/// @brief Translates a 1-dimensional linearized GID to its
///        n-dimensional GID, based on the given @p Traversal order.
//////////////////////////////////////////////////////////////////////
template<typename GID, typename Traversal>
struct nd_reverse_linearize
{
  typedef GID                               gid_type;
  typedef gid_type                          result_type;
  typedef std::size_t                       index_type;
  typedef gid_type                          size_type;
  typedef nd_linearize<GID, Traversal>      inverse;
  typedef std::true_type is_bijective;

  enum { last_idx = tuple_size<size_type>::value - 1 };

  size_type m_size;

  //////////////////////////////////////////////////////////////////////
  /// @todo Is the default constructor needed? if it is size should be
  ///       correctly initialized (default constructed in domain.hpp).
  //////////////////////////////////////////////////////////////////////
  nd_reverse_linearize(void) = default;

  explicit
  nd_reverse_linearize(size_type const& size)
    : m_size(detail::traversal_order<gid_type, Traversal>::apply(size))
  { }

  explicit
  nd_reverse_linearize(nd_linearize<GID, Traversal> const& other)
    : m_size(other.m_size)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @todo merge calls to inverse_linearizer_t::operator() and
  /// inverse_index_order_t::operator() into a single call
  /// to minimize the number of traversals of the tuple.
  //////////////////////////////////////////////////////////////////////
  gid_type operator()(index_type linear) const
  {
    return detail::reverse_traversal_order<gid_type, Traversal>::apply(
      detail::reverse_linearize<last_idx, gid_type>::apply(
        m_size, linear, gid_type())
    );
  }

  void define_type(typer& t)
  {
    t.member(m_size);
  }
}; // struct nd_reverse_linearize


//////////////////////////////////////////////////////////////////////
/// @brief Base case for @ref nd_reverse_linearize for 1-dimensional GID.
///
/// @todo Possible use of the f_ident mapping function for this.
//////////////////////////////////////////////////////////////////////
template<typename Traversal>
struct nd_reverse_linearize<size_t, Traversal>
{
  nd_reverse_linearize(void) = default;

  explicit
  nd_reverse_linearize(size_t)
  { }

  size_t operator()(size_t linear) const
  { return linear; }
};

} // namespace stapl

#endif // STAPL_MAPFUNCS_LINEARIZATION_HPP
