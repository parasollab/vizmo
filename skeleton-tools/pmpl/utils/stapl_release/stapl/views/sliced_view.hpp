/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_SLICED_VIEW_HPP
#define STAPL_VIEWS_SLICED_VIEW_HPP

#include <stapl/utility/tuple.hpp>
#include <stapl/utility/pack_ops.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/containers/type_traits/dimension_traits.hpp>
#include <stapl/views/mapping_functions/mapping_functions.hpp>
#include <stapl/views/mapping_functions/linearization.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/utility/use_default.hpp>
#include <stapl/domains/indexed.hpp>

#include <iostream>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Helper function to extract the D'th element of a tuple,
///        or return the element itself if it is not a tuple.
//////////////////////////////////////////////////////////////////////
template<int D, typename T>
struct extract
{
  typedef typename tuple_element<D, T>::type type;

  template<typename U>
  static type get(U&& x)
  {
    return std::get<D>(std::forward<U>(x));
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Specialization for non-tuple types.
//////////////////////////////////////////////////////////////////////
template<int D>
struct extract<D, size_t>
{
  typedef size_t type;

  static type get(size_t x)
  {
    return x;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to convert a set of indices to the tuple defined
///        by the template parameter @c GID.
//////////////////////////////////////////////////////////////////////
template<typename GID>
struct make_GID
{
  template<typename... Indices>
  GID operator() (Indices... indices) const
  {
    return GID(indices...);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Helper metafunction that takes a tuple of integral constants and a
///        @c size_t value @c Ins and creates an index_sequence from the values
///        of the tuple's elements, appended by @c Ins.
//////////////////////////////////////////////////////////////////////
template<typename Seq1, std::size_t Ins>
struct append_and_make_index_sequence;

template<typename... Ts, std::size_t Ins>
struct append_and_make_index_sequence<tuple<Ts...>,  Ins>
{
  // index_sequence<foo..., bar> triggers ICE in GCC 4.8
  using type =
    integer_sequence<std::size_t, static_cast<std::size_t>(Ts::value)..., Ins>;
};

//////////////////////////////////////////////////////////////////////
/// @brief Helper metafunction that takes an index sequence and adds the
///        constant @c Offset to all its elements.
//////////////////////////////////////////////////////////////////////
template<std::size_t Offset, typename Seq>
struct offset;

template<std::size_t Offset, std::size_t... Is>
struct offset<Offset, index_sequence<Is...>>
{
  using type = index_sequence<(Offset + Is)...>;
};

//////////////////////////////////////////////////////////////////////
/// @brief Helper metafunction that takes an index sequence, adds the constant
///        @c Offset1 to all its elements, appends a value @c Ins to its end
///        and concatenates it with another index sequence (with elements
///        offset by the constant @c Offset2).
//////////////////////////////////////////////////////////////////////
template<std::size_t Offset1, typename Seq1, std::size_t Ins,
          std::size_t Offset2, typename Seq2>
struct offset_insert_join;

template<std::size_t Offset1, std::size_t... Is1, std::size_t Ins,
          std::size_t Offset2, std::size_t... Is2>
struct offset_insert_join<Offset1, index_sequence<Is1...>,  Ins,
                          Offset2, index_sequence<Is2...>>
{
  // index_sequence<foo..., bar> triggers ICE in GCC 4.8
  using type =
    integer_sequence<std::size_t, (Offset1 + Is1)..., Ins, (Offset2 + Is2)...>;
};

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of @ref make_arg_pos_sequence.
///
/// The idea is to regard the fixed indices as splitting points of the final
/// sequence and recursively construct and join its parts. The algorithm starts
/// by constructing the first part of the final sequence
/// (<tt>0,...,Slices[0]-1</tt>), appends the position of the first fixed index
/// (0) and recursively repeats the same process for the remaining part
/// (<tt>Slices[0]+1,...,GID_size</tt>) with the next position (1). In the end,
/// the positions of the fixed indices within @c Slices (i.e, numbers
/// <tt>0,...,|Slices|-1</tt>) are at the right positions
/// (<tt>Slices[0],...,Slices[|Slices|-1]</tt> within the resulting sequence.
///
/// @tparam InitialOffset Initial offset of the resulting index sequence
///                       (<tt>== |Slices|</tt>)
/// @tparam P Counter for the fixed indices
/// @tparam N0, N1 Consecutive fixed indices delimiting the part processed in
///                the current recursion step (exclusive, i.e. the currently
///                processed part of the final sequence is <tt>[N0+1,N1-1]</tt>)
/// @tparam Ns  Remaining fixed indices
///
/// @sa make_arg_pos_sequence
//////////////////////////////////////////////////////////////////////
template<std::size_t InitialOffset, std::size_t P, int N0,
          std::size_t N1, std::size_t... Ns>
struct make_arg_pos_sequence_impl
{
  using type =
    typename offset_insert_join <
      InitialOffset,
      make_index_sequence<N1-N0-1>,
      P,
      N1-N0-1,
      typename make_arg_pos_sequence_impl<
        InitialOffset, P-(N1-N0-1)+1, static_cast<int>(N1), Ns...
      >::type
    >::type;
};

template<std::size_t InitialOffset, std::size_t P, int N0, std::size_t N1>
struct make_arg_pos_sequence_impl<InitialOffset, P, N0, N1>
{
  using type =
    typename offset<InitialOffset, make_index_sequence<N1-N0-1>>::type;
};

//////////////////////////////////////////////////////////////////////
/// @brief Construct an index sequence defining the order of the arguments
///        to be passed to the functor that produces the required final result
///        (reference to the element at mapped position for
///        @c sliced_mf::apply_get() or the tuple of mapped indices (@c GID) for
///        @c sliced_mf::operator()()).
///
/// Given @c Seq -- a sequence of indices that are fixed (@c Slices), appended
/// by the  final @c GID_size -- @c make_arg_pos_sequence::type will be an index
/// sequence of length @c GID_size with indices <tt>0,...,|Slices|-1</tt> at the
/// fixed positions (<tt>Slices[0],...,Slices[|Slices|-1]</tt>) and indices
/// <tt>|Slices|+1,...,GID_size-1</tt> at the remaining positions
/// (<tt>{0,...,GID_size-1} \ Slices</tt>).
///
/// Examples:
/// @verbatim
///    Slices: {}, GID_size: 3 --> Seq: index_sequence<3>
///    type: index_sequence<0,1,2>
///
///    Slices: {2}, GID_size: 3 --> Seq: index_sequence<2,3>
///    type: index_sequence<1,2,0>
///
///    Slices: {1,3}, GID_size: 5 --> Seq: index_sequence<1,3,5>
///    type: index_sequence<2,0,3,1,4>
///
///    Slices: {2,4,5}, GID_size: 8 --> Seq: index_sequence<2,4,5,8>
///    type: index_sequence<3,4,0,5,1,2,6,7>
/// @endverbatim
///
/// @tparam Seq Sequence of indices that are fixed (@c Slices), appended by the
///             final @c GID_size
///
/// @sa sliced_mf
//////////////////////////////////////////////////////////////////////
template<typename Seq>
struct make_arg_pos_sequence;

template<std::size_t... Ns>
struct make_arg_pos_sequence<index_sequence<Ns...>>
{
  using type =
    typename make_arg_pos_sequence_impl<sizeof...(Ns)-1, 0, -1, Ns...>::type;
};

//////////////////////////////////////////////////////////////////////
/// @brief Mapping function for the @see SLICED_view that maps
///        a (d-|Slices|)-dimensional index to a d-dimensional
///        one, where the other dimensions take on the value of
///        the fixed parameters passed into the constructor.
///
/// @tparam Slices Tuple of compile-time constants specifying
///                the indices that are fixed
/// @tparam Index  The index type of the view (smaller)
/// @tparam GID    The GID type of the container (larger)
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Index, typename GID>
struct sliced_mf
{
public:
  using index_type = Index;
  using gid_type = GID;

private:
  static size_t constexpr GID_size = tuple_size<GID>::value;
  static size_t constexpr Slices_size = tuple_size<Slices>::value;

  using fixed_type =
    typename homogeneous_tuple_type<Slices_size, std::size_t>::type;

  fixed_type m_fixed;

  //////////////////////////////////////////////////////////////////////
  /// @brief Evaluates the mapping function on a sequence of indices and
  ///        calls the provided functor on the resulting (mapped) indices.
  ///
  /// Conceptually, it forms an initial argument list starting with
  /// the values of the fixed indices and followed by the values of the
  /// original (non-fixed) indices, permutes it so that the fixed values
  /// appear at the positions given by the @c Slices parameter and finally
  /// calls the provided functor with the resulting argument list.
  ///
  /// Example:
  /// @verbatim
  ///   Slices: {1,3}  m_fixed: {10,4}   indices: {8,7,15}
  ///
  ///   --> pack_ops::call_with_permuted_args(functor,
  ///                                         index_sequence<2,0,3,1,4>,
  ///                                         10, 4, 8, 7, 15)
  ///       --> functor(8,10,7,4,15)
  /// @endverbatim
  ///
  /// @tparam F   Functor to be called on the mapped indices
  /// @tparam FIs Positions of the fixed indices in the @c Slices tuple
  ///             (<tt>0,...,|Slices|-1</tt>)
  /// @param indices Values of the original (non-fixed) indices
  /// @return return value of @c F
  //////////////////////////////////////////////////////////////////////
  template<typename F, std::size_t... FIs, typename... Indices>
  auto apply_get_impl(F const& functor,
                      index_sequence<FIs...> const& /*fixed_indices*/,
                      Indices... indices) const
    -> decltype(
         pack_ops::call_with_permuted_args(
           functor,
           typename make_arg_pos_sequence<
             typename append_and_make_index_sequence<
               Slices, GID_size
             >::type
           >::type(),
           std::get<FIs>(m_fixed)..., indices...
         )
       )
  {
    using ext_slices =
      typename append_and_make_index_sequence<Slices, GID_size>::type;

    return pack_ops::call_with_permuted_args(
      functor,
      typename make_arg_pos_sequence<ext_slices>::type(),
      std::get<FIs>(m_fixed)..., indices... );
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Version of @c apply_get_impl used when the source (non-fixed)
  ///        indices are given as a multidimensional @c index_type
  ///        (integral type or a tuple of these).
  //////////////////////////////////////////////////////////////////////
  template<typename F, std::size_t... FIs, std::size_t... Is>
  auto apply_get_impl(F const& functor, index_type const& nd_index,
                      index_sequence<FIs...> const& fixed_indices,
                      index_sequence<Is...> const& /*nd_index_indices*/) const
  STAPL_AUTO_RETURN (
    this->apply_get_impl(functor, fixed_indices,
      extract<Is, index_type>::get(nd_index)...)
  )

public:
  //////////////////////////////////////////////////////////////////////
  /// @param The actual values which the fixed indices are set to.
  //////////////////////////////////////////////////////////////////////
  template<typename Fixed>
  sliced_mf(Fixed const& fixed)
    : m_fixed(fixed)
  { }

  sliced_mf(void) = default;

  //////////////////////////////////////////////////////////////////////
  /// @brief  Evaluates the mapping function on a (d-|Slices|)-dimensional input
  ///         and emits a d-dimensional tuple as a result.
  ///
  /// Equivalent to calling @c sliced_mf::apply_get with unpacked @c idx and
  /// packing the resulting mapped indices to @c gid_type instead of using them
  /// to retrieve reference to the corresponding element.
  ///
  /// @param  idx Index to be mapped (integral type or a tuple of these)
  /// @return Tuple with the mapped indices
  //////////////////////////////////////////////////////////////////////
  gid_type operator()(index_type const& idx) const
  {
    int constexpr dim = std::conditional<
                          std::is_scalar<index_type>::value,
                          std::integral_constant<int,1>,
                          tuple_size<index_type>
                        >::type::value;

    return apply_get_impl(
      make_GID<gid_type>(), idx,
      make_index_sequence<Slices_size>(),
      make_index_sequence<dim>() );
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief  Applies the mapping on the input indices and calls the
  ///   @c get_reference functor with the mapped indices.
  ///
  /// @tparam RefGetter   Functor specified by an underlying container, that
  ///   returns a reference to the container's element at given position.
  /// @tparam Indices     Indices to be mapped.
  /// @return Reference to the element at the mapped position
  //////////////////////////////////////////////////////////////////////
  template<typename RefGetter, typename... Indices>
  auto
  apply_get(RefGetter const& get_ref, Indices... indices) const
  STAPL_AUTO_RETURN (
    this->apply_get_impl(
      get_ref, make_index_sequence<Slices_size>(), indices...)
  )

  void define_type(typer& t)
  {
    t.member(m_fixed);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Metafunction to compute the type of the sliced view's domain.
//////////////////////////////////////////////////////////////////////
template<int D, typename Index>
struct SLICED_view_domain
{
  typedef indexed_domain<Index, D, typename default_traversal<D>::type> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Metafunction to compute the type of the sliced view's domain.
///        Specialization for the one-dimensional case.
//////////////////////////////////////////////////////////////////////
template<typename Index>
struct SLICED_view_domain<1, Index>
{
  typedef indexed_domain<Index> type;
};


} // namespace detail

template<typename Slices, typename Container, typename... OptParams>
//template<typename Slices, typename Container, typename Domain, typename MF>
class SLICED_view;

template<typename Slices, typename Container>
struct view_traits<SLICED_view<Slices, Container>>
  : default_view_traits<Container,
    typename detail::SLICED_view_domain< // domain
      dimension_traits<Container>::type::value -
      stapl::tuple_size<Slices>::type::value,
      typename tuple_element<
        0, typename Container::gid_type
      >::type
    >::type,
    detail::sliced_mf< // mapping function
      Slices,
      typename detail::SLICED_view_domain< // domain
        dimension_traits<Container>::type::value -
        stapl::tuple_size<Slices>::type::value,
        typename tuple_element<
          0, typename Container::gid_type
        >::type
      >::type::index_type,
      typename Container::gid_type
    >,
    SLICED_view<Slices, Container> // derived
  >
{ };


template<typename Slices, typename Container, typename Domain>
struct view_traits<SLICED_view<Slices, Container, Domain>>
  : default_view_traits<Container,
    Domain,
    detail::sliced_mf< // mapping function
      Slices,
      typename Domain::index_type,
      typename Container::gid_type
    >,
    SLICED_view<Slices, Container, Domain> // derived
  >
{ };


template<typename Slices, typename Container, typename Domain, typename MF>
struct view_traits<SLICED_view<Slices, Container, Domain, MF>>
  : default_view_traits<Container, Domain, MF,
      SLICED_view<Slices, Container, Domain, MF> // derived
    >
{ };


template<typename Slices, typename Container, typename Domain, typename MF,
         typename Derived>
struct view_traits<SLICED_view<Slices, Container, Domain, MF, Derived>>
  : default_view_traits<Container, Domain, MF, Derived>
{ };


template<typename Slices, typename OldC, typename NewC, typename... OptParams>
struct cast_container_view<SLICED_view<Slices, OldC, OptParams...>, NewC>
{
  typedef SLICED_view<Slices, NewC, OptParams...> type;

  typename view_traits<type>::map_function
  mapfunc(SLICED_view<Slices, OldC, OptParams...> const& view)
  {
    return view.mapfunc();
  }
};


template<typename Slices, typename OldC, typename NewC>
struct cast_container_view<SLICED_view<Slices, OldC>, NewC>
{
  typedef SLICED_view<Slices, NewC> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Create a view over a multidimensional view that fixes a
///        a subset of dimensions to specified values.
///
///  The dimensions that are fixed are specified by a tuple of compile
///  time integral constants. The domain of the sliced view is the
///  set of possible GIDs in the dimensions that are not fixed.
///
///
/// For example, giving a 3D container C with domain [0..1]x[0..3]x[0..2],
/// a sliced view fixing dimension 1 to be 3 will produce the following
/// values:
///
/// sv(0,0) = C(0,3,0)
/// sv(0,1) = C(0,3,1)
/// sv(0,2) = C(0,3,2)
/// sv(1,0) = C(1,3,0)
/// sv(1,1) = C(1,3,1)
/// sv(1,2) = C(1,3,2)
///
/// @tparam Slices Tuple of compile time constants that specifies
///                the dimensions that are fixed.
/// @tparam Container The underlying collection of elements.
///
/// @todo Remove heap allocation in the constructor with an r-value
//////////////////////////////////////////////////////////////////////
template<typename Slices,
         typename Container, typename... OptionalParams>
class SLICED_view :
    public core_view<
      Container,
      typename view_traits<
        SLICED_view<Slices, Container, OptionalParams...>
      >::domain_type,
      typename view_traits<
        SLICED_view<Slices, Container, OptionalParams...>
      >::map_function
    >,
    public view_operations::readwrite<
      SLICED_view<Slices, Container, OptionalParams...>
    >,
    public std::conditional<
      dimension_traits<Container>::type::value == 1,
      view_operations::subscript<
        SLICED_view<Slices, Container, OptionalParams...>
      >,
      view_operations::multi_dimensional_subscript<
        SLICED_view<Slices, Container, OptionalParams...>
      >
    >::type
{
  typedef typename stapl::tuple_size<Slices>::type number_of_slices_t;

public:
  STAPL_VIEW_REFLECT_TRAITS(SLICED_view)

  typedef std::integral_constant<int,
    dimension_traits<Container>::type::value - number_of_slices_t::value
  > dimension_type;

private:
  typedef core_view<
      Container,
      typename view_traits<SLICED_view>::domain_type,
      typename view_traits<SLICED_view>::map_function
    > base_type;

public:
  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }

  explicit SLICED_view(Container const& vcont, domain_type const& dom,
                         map_function const& mf)
    : base_type(vcont, dom, mf)
  { }

  explicit SLICED_view(Container* vcont, domain_type const& dom,
                         map_function const& mf)
    : base_type(vcont, dom, mf)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief This constructor exists because the view_container in the
  ///        slices_view is storing a multiarray instead of a multiarray_view.
  ///        This can be perhaps generalized to include move semantics
  ///        that will instead take ownership.
  ///
  /// @todo Investigate a workaround that does not involve the existence
  ///       of this constructor
  //////////////////////////////////////////////////////////////////////
  template<typename ContainerContainer>
  explicit SLICED_view(ContainerContainer& vcont,
                       domain_type const& dom,
                       map_function const& mf)
    : base_type(new Container(vcont), dom, mf)
  { }

  SLICED_view(void) = default;

}; // class SLICED_view


//////////////////////////////////////////////////////////////////////
/// @brief Create a view over a multidimensional view that fixes a
///        a subset of dimensions to specified values. @see SLICED_view.
///
/// @tparam Slice Parameter pack of ints to specify dimensions that are fixed
/// @param ct Container to create the view over
/// @param fixed The values that are fixed
//////////////////////////////////////////////////////////////////////
template<int... Slice, typename Container, typename... Index>
SLICED_view<stapl::tuple<std::integral_constant<int, Slice>...>, Container>
make_SLICED_view(Container const& ct, Index&&... fixed)
{
  static_assert(sizeof...(Slice) == sizeof...(Index),
    "The number of dimensions to slice and the number of fixed parameters "
    "should be equal");

  typedef stapl::tuple<std::integral_constant<int, Slice>...> slice_t;

  return make_SLICED_view<slice_t>(
    ct, std::forward_as_tuple(std::forward<Index>(fixed)...)
  );
}


//////////////////////////////////////////////////////////////////////
/// @brief Create a view over a multidimensional view that fixes a
///        a subset of dimensions to specified values. @see SLICED_view.
///
/// @tparam Slices Tuple of compile-time integrals to specify dimensions
///                 that are fixed
/// @param ct Container to create the view over
/// @param fixed The values that are fixed
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Container, typename... Index>
SLICED_view<Slices, Container>
make_SLICED_view(Container const& ct, Index&&... fixed)
{
  static_assert(tuple_size<Slices>::value == sizeof...(Index),
    "The number of dimensions to slice and the number of fixed parameters "
    "should be equal");

  typedef typename view_traits<
    SLICED_view<Slices, Container>
  >::domain_type domain_type;

  typedef typename view_traits<
    SLICED_view<Slices, Container>
  >::map_function map_function;

  typedef typename domain_type::gid_type index_type;
  typedef decltype(ct.domain().first())  gid_type;

  // create domain from the original where the dimensions
  // specified in Slices are discarded
  auto const first = tuple_ops::discard<Slices>(ct.domain().first());
  auto const last  = tuple_ops::discard<Slices>(ct.domain().last());

  domain_type dom(first, last);

  map_function mf(std::forward<Index>(fixed)...);

  Container& tmp = const_cast<Container&>(ct);

  return SLICED_view<Slices, Container>(tmp, dom, mf);
}

} // namespace stapl

#endif // STAPL_VIEWS_MULTIARRAY_VIEW_HPP
