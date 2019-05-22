/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_MULTIARRAY_VIEW_HPP
#define STAPL_VIEWS_MULTIARRAY_VIEW_HPP

#include <stapl/utility/tuple.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/containers/type_traits/dimension_traits.hpp>
#include <stapl/views/mapping_functions/mapping_functions.hpp>
#include <stapl/views/mapping_functions/linearization.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/utility/use_default.hpp>
#include <stapl/views/operations/multi_dimensional_subscript.hpp>
#include <stapl/domains/indexed.hpp>

#include <iostream>

namespace stapl {

#ifdef STAPL_DOCUMENTATION_ONLY

//////////////////////////////////////////////////////////////////////
/// @brief Provide a one dimensional, random access view over a
///   multi-dimensional view.
/// @tparam View The underlying multi-dimensional view to linearize.
/// @tparam Traversal The traversal method to use for linearization
/// @todo Remove linear_view freestanding function idiom and rename
///  this class to linear_view.
//////////////////////////////////////////////////////////////////////
template<typename View,
         typename Traversal = typename default_traversal<
             tuple_size<typename view_traits<View>::index_type>::value
           >::type>
class linear_view_impl;

#else

template<typename View, typename ...OptionalParams>
class linear_view_impl;

#endif


//////////////////////////////////////////////////////////////////////
/// @brief Type metafunction to select a default traversal if a
///   @ref linear_view has not been passed an optional traversal
///   type parameter.  Otherwise, return the passed type.
//////////////////////////////////////////////////////////////////////
template<int N, typename ...OptionalTraversal>
struct compute_traversal_type;


template<int N>
struct compute_traversal_type<N>
{ typedef typename default_traversal<N>::type type; };


template<int N, typename Traversal>
struct compute_traversal_type<N, Traversal>
{ typedef Traversal type; };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when all four type parameters are specified.
//////////////////////////////////////////////////////////////////////
template<typename View, typename ...OptionalParams>
struct view_traits<linear_view_impl<View, OptionalParams...>>
  : default_view_traits<
      typename view_traits<View>::container,
      indexed_domain<size_t>,
      typename compose_func<
        typename view_traits<View>::map_function,
        nd_reverse_linearize<
          typename view_traits<View>::index_type,
          typename compute_traversal_type<
           tuple_size<typename view_traits<View>::index_type>::value,
           OptionalParams...>::type>>::type,
      linear_view_impl<View, OptionalParams...>
    >
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container transform for variadic
///  based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename OldC, typename ...OptionalParams, typename NewC>
struct cast_container_view<linear_view_impl<OldC, OptionalParams...>, NewC>
  : new_cast_container_view<linear_view_impl<OldC, OptionalParams...>, NewC>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container domain and mapping function
///   transform for variadic based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename C, typename ...OptionalParams, typename Dom, typename MF>
struct upcast_view<linear_view_impl<C, OptionalParams...>, Dom, MF>
  : new_upcast_view<linear_view_impl<C, OptionalParams...>, Dom, MF>
{ };


template<typename View, typename ...OptionalParams>
class linear_view_impl
  : public array_view<
      typename view_traits<View>::container,
      indexed_domain<size_t>,
      typename compose_func<
        typename view_traits<View>::map_function,
        nd_reverse_linearize<
         typename view_traits<View>::index_type,
         typename compute_traversal_type<
           tuple_size<typename view_traits<View>::index_type>::value,
           OptionalParams...>::type>>::type,
      linear_view_impl<View, OptionalParams...>
  >
{
public:
  STAPL_VIEW_REFLECT_TRAITS(linear_view_impl)

private:
  typedef typename view_traits<linear_view_impl>::derived_type derived_type;

  typedef array_view<view_container_type, domain_type,
                     map_function, linear_view_impl>           base_type;

  typedef view_operations::sequence<derived_type>              sequence_op_type;

public:
  typedef typename sequence_op_type::iterator                  iterator;
  typedef typename sequence_op_type::const_iterator            const_iterator;

  linear_view_impl(void) = default;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor used to pass ownership of the container to the view.
  ///
  /// @param vcont Pointer to the container used to forward the operations.
  /// @param dom Domain to be used by the view.
  /// @param mfunc Mapping function to transform view indices to container
  ///              gids.
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type* vcont,
             domain_type const& dom,
             map_func_type mfunc = map_function())
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor used to pass ownership of the container to the view.
  ///
  /// @param vcont Pointer to the container used to forward the operations.
  /// @param dom Domain to be used by the view.
  /// @param mfunc Mapping function to transform view indices to container
  ///              gids.
  /// @param other View to copy from.
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type* vcont,
             domain_type const& dom,
             map_func_type mfunc,
             linear_view_impl const&)
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont Reference to the container used to forward the operations.
  /// @param dom Domain to be used by the view.
  /// @param mfunc Mapping function to transform view indices to container
  ///              gids.
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type const& vcont,
             domain_type const& dom,
             map_func_type mfunc = map_function())
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont Reference to the container used to forward the operations.
  /// @param dom Domain to be used by the view.
  /// @param mfunc Mapping function to transform view indices to container
  ///              gids.
  /// @param other View to copy from (ignored for linear_view_impl)
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type const& vcont,
             domain_type const& dom,
             map_func_type mfunc,
             linear_view_impl const&)
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that can reference all the elements of
  ///        the passed container. The view takes ownership of the container.
  ///
  /// @param vcont Pointer to the container used to forward the operations.
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type* vcont)
    : base_type(vcont, stapl::get_domain(*vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that can reference all the elements of
  ///        the passed container.
  ///
  /// @param vcont Reference to the container used to forward the operations.
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(view_container_type& vcont)
    : base_type(vcont, stapl::get_domain(vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Copy constructor when the passed view is not the most
  ///        derived view.
  /// @todo update domain to reflect change in size
  //////////////////////////////////////////////////////////////////////
  linear_view_impl(linear_view_impl const& other)
    : base_type(other.container(), other.domain(), other.mapfunc())
  { }
}; // class linear_view_impl


namespace result_of {

//////////////////////////////////////////////////////////////////////
/// @brief Helper struct to determine the one dimensional view type
///        over the given multidimensional @p View based on the
///        specified @p Traversal.
//////////////////////////////////////////////////////////////////////
template<typename View, typename... OptionalParams>
struct linear_view
{
  typedef linear_view_impl<View, OptionalParams...> type;
};

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief Helper function to return a one dimensional view type
///        over the given multidimensional @p view based on the
///        specified @p Traversal type.
//////////////////////////////////////////////////////////////////////
template<typename View, typename ...OptionalTraversal>
typename result_of::linear_view<View, OptionalTraversal...>::type
linear_view(View const& view, OptionalTraversal const&...)
{
  typedef typename compute_traversal_type<
    tuple_size<typename view_traits<View>::index_type>::value,
    OptionalTraversal...
  >::type                                                   traversal_type;

  typedef typename View::domain_type::size_type             size_type;
  typedef typename view_traits<View>::index_type            index_type;
  typedef typename view_traits<View>::map_function          map_func_type;

  typedef typename result_of::linear_view<
    View, OptionalTraversal...>::type                       result_t;
  typedef nd_reverse_linearize<index_type, traversal_type>  reverse_lin_t;
  typedef compose_func<map_func_type, reverse_lin_t>         comp_mapfunc_t;

  const size_type size = view.domain().dimensions();

  nd_linearize<index_type, traversal_type> mf(size);

  const size_t first = mf(view.domain().first());
  const size_t last  = mf(view.domain().last());

  return result_t(
    view.container(),
    indexed_domain<size_t>(first, last),
    comp_mapfunc_t::apply(view.mapfunc(), reverse_lin_t(size)));
}


#ifdef STAPL_DOCUMENTATION_ONLY

//////////////////////////////////////////////////////////////////////
/// @brief Defines a multi-dimensional array view.
///
/// Provides the operations that are commonly present in an array
/// (random access, iteration).
/// @tparam C Container type.
/// @tparam Dom Domain type. By default uses the same domain type
///             provided for the container.
/// @tparam Mapping function type
///    (default: identity mapping function)
/// @tparam Derived Type of the most derived class (default: itself)
/// @ingroup multi_array_view
//////////////////////////////////////////////////////////////////////
template<typename C,
         typename Dom = typename container_traits<C>::domain_type,
         typename MapFunc = f_ident<typename Dom::index_type>,
         typename Derived = use_default>
class multiarray_view;

#else

template<typename C, typename ...OptionalParams>
class multiarray_view;

#endif


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when only container type parameter is specified.
//////////////////////////////////////////////////////////////////////
template<typename C>
struct view_traits<multiarray_view<C>>
  : default_view_traits<C,
      typename container_traits<C>::domain_type,
      f_ident<typename container_traits<C>::domain_type::index_type>,
      multiarray_view<C>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when container and domain type parameters
///    are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D>
struct view_traits<multiarray_view<C, D>>
  : default_view_traits<C, D, f_ident<typename D::index_type>,
                        multiarray_view<C,D>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when container, domain, and mapping function
///  type parameters are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D, typename F>
struct view_traits<multiarray_view<C, D, F>>
  : default_view_traits<C, D, F, multiarray_view<C, D, F>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when all four type parameters are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D, typename F, typename Derived>
struct view_traits<multiarray_view<C, D, F, Derived>>
  : default_view_traits<C, D, F, Derived>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container transform for variadic
///  based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename OldC, typename ...OptionalParams, typename NewC>
struct cast_container_view<multiarray_view<OldC, OptionalParams...>, NewC>
  : new_cast_container_view<multiarray_view<OldC, OptionalParams...>, NewC>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container domain and mapping function
///   transform for variadic based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename C, typename ...OptionalParams, typename Dom, typename MF>
struct upcast_view<multiarray_view<C, OptionalParams...>, Dom, MF>
  : new_upcast_view<multiarray_view<C, OptionalParams...>, Dom, MF>
{ };


template<typename C, typename ...OptionalParams>
class multiarray_view
  : public core_view<
      C,
      typename view_traits<multiarray_view<C, OptionalParams...>>::domain_type,
      typename view_traits<multiarray_view<C, OptionalParams...>>::map_function
    >,
    public view_operations::readwrite<multiarray_view<C, OptionalParams...>>,
    public view_operations::multi_dimensional_subscript<
      typename view_traits<multiarray_view<C, OptionalParams...>>::derived_type
    >
{
public:
  STAPL_VIEW_REFLECT_TRAITS(multiarray_view)

  typedef index_type                              dimensions_type;
  typedef typename dimension_traits<C>::type      dimension_type;
private:
  typedef core_view<C, domain_type, map_function> base_type;

public:
  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::array_view::array_view(view_container_type&,domain_type const&,map_func_type)
  //////////////////////////////////////////////////////////////////////
  multiarray_view(view_container_type& vcont,
                  domain_type const& dom,
                  map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  multiarray_view(view_container_type const& vcont,
                  domain_type const& dom,
                  map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::array_view::array_view(view_container_type*)
  //////////////////////////////////////////////////////////////////////
  multiarray_view(view_container_type* vcont)
    : base_type(vcont, stapl::get_domain(*vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::array_view::array_view(view_container_type&)
  //////////////////////////////////////////////////////////////////////
  multiarray_view(view_container_type& vcont)
    : base_type(vcont, stapl::get_domain(vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::array_view::array_view(view_container_type*,domain_type const&,map_func_type)
  //////////////////////////////////////////////////////////////////////
  multiarray_view(view_container_type* vcont,
                  domain_type const& dom,
                  map_function mfunc = map_function())
    : base_type(vcont,dom,mfunc)
  { }

  /////////////////////////////////////////////////////////////////
  /// @brief Copy constructor when the passed view is not the most
  ///        derived view.
  //////////////////////////////////////////////////////////////////////
  template<typename ...OtherOptionalParams>
  multiarray_view(multiarray_view<C, OtherOptionalParams...> const& other)
    : base_type(other.container(), other.domain(), other.mapfunc())
  { }

  multiarray_view(view_container_type& vcont,
                  domain_type const& dom,
                  map_function mfunc,
                  multiarray_view const&)
    : base_type(vcont,dom,mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief The length of each of the dimensions of the multiarray.
  //////////////////////////////////////////////////////////////////////
  dimensions_type dimensions(void) const
  {
    return this->domain().dimensions();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Update the underlying container to hold the specified
  ///        number of elements.  All previous information is lost.
  //////////////////////////////////////////////////////////////////////
  void resize(typename domain_type::size_type size)
  {
    this->incr_version();
    this->container().resize(size);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief use to examine this class
  /// @param msg your message (to provide context)
  //////////////////////////////////////////////////////////////////////
  void debug(char *msg=0)
  {
    std::cerr << "MULTI_ARRAY_VIEW " << this << " : ";
    if (msg) {
      std::cerr << msg;
    }
    std::cerr << "\n";
    base_type::debug();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
}; // class multiarray_view


//////////////////////////////////////////////////////////////////////
/// @brief A linearization mapping function which accepts an initial
/// offset. This mapping function is used in the cases where a
/// @c multiarray_view is defined over a 1D view.
///
/// @tparam GID the GID for an n-dimensional view.
//////////////////////////////////////////////////////////////////////
template <typename GID>
struct nd_linearize_with_offset
  : public nd_linearize<
      GID, typename default_traversal<tuple_size<GID>::value>::type>
{
  typedef nd_linearize<
    GID, typename default_traversal<tuple_size<GID>::value>::type> base_t;

  STAPL_IMPORT_DTYPE(base_t, size_type)
  STAPL_IMPORT_DTYPE(base_t, index_type)
  STAPL_IMPORT_DTYPE(base_t, gid_type)

  gid_type m_offset;

  nd_linearize_with_offset(gid_type offset, size_type const& sizes)
    : base_t(sizes),
      m_offset(offset)
  { }

  gid_type operator()(index_type const& gid) const
  {
    return base_t::operator()(gid) + m_offset;
  }

  void define_type(typer& t)
  {
    t.member(m_offset);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Given dimensions as variadic arguments, creates an
/// n-dimensional domain.
///
/// @param dims the dimensions of the new domain.
///
/// @return an @c indexed_domain
//////////////////////////////////////////////////////////////////////
template <typename... Dims>
indexed_domain<std::size_t, sizeof...(Dims)>
make_indexed_domain(Dims... d)
{
  return indexed_domain<std::size_t, sizeof...(Dims)>(std::make_tuple(d...));
}

//////////////////////////////////////////////////////////////////////
/// @brief Given dimensions as variadic arguments, creates an
/// n-dimensional view using default linearization.
/// The created view can be used inside workfunctions to access elements
/// of a 1D view using local nD indices.
///
/// @param dims the dimensions of the new domain.
///
/// @return an @c nd_domain
//////////////////////////////////////////////////////////////////////
template <typename View, typename D, typename... Dims>
multiarray_view<
  View,
  decltype(make_indexed_domain(std::declval<D>(), std::declval<Dims>()...)),
  nd_linearize_with_offset<
    typename decltype(
      make_indexed_domain(std::declval<D>(),
        std::declval<Dims>()...))::index_type
  >
>
make_multiarray_view(View& view, D dim0, Dims... dims)
{
  auto dom = make_indexed_domain(dim0, dims...);
  typedef decltype(dom)                                        dom_t;
  typedef typename dom_t::index_type                           index_t;
  typedef nd_linearize_with_offset<typename dom_t::index_type> mapping_t;

  return multiarray_view<View, dom_t, mapping_t>(
           view, dom,
           mapping_t(view.domain().first(), make_tuple(dim0, dims...)));
}

//////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct a multiarray_view over the passed
///        container
///
/// @param ct Underlying container used for the multiarray_view.
/// @return A multiarray_view over the Container.
//////////////////////////////////////////////////////////////////////
template<typename Container>
multiarray_view<Container>
make_multiarray_view(Container& ct)
{
  return multiarray_view<Container>(ct);
}

} // namespace stapl

#endif // STAPL_VIEWS_MULTIARRAY_VIEW_HPP
