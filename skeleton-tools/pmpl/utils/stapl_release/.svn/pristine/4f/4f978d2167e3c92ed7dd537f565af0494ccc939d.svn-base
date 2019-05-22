/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_REPEATED_VIEW_HPP
#define STAPL_VIEWS_REPEATED_VIEW_HPP

#include <stapl/domains/infinite.hpp>
#include <stapl/views/array_ro_view.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>
#include <stapl/views/metadata/projection/generic.hpp>
#include <stapl/views/metadata/coarsening_traits.hpp>
#include <stapl/utility/use_default.hpp>
#include <stapl/views/type_traits/upcast.hpp>
#include <stapl/views/operations/multi_dimensional_subscript.hpp>

#include <iostream>

namespace stapl {

namespace metadata {

template<typename C, typename MD>
class infinite_container;

} // namespace metadata


namespace view_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor to return a locality metadata that knows how to
///        return metadata for an infinite set of elements.
/// @tparam C container that represents an infinite collection of elements
//////////////////////////////////////////////////////////////////////
template<typename C, int N = 1>
struct infinite_locality_metadata
{
  typedef typename std::conditional<
    N == 1, infinite, infinite_nd<N>>::type             domain_type;

  typedef metadata_entry<domain_type,C*>                dom_info_type;
  typedef metadata::infinite_container<C,dom_info_type> return_type;
  typedef typename domain_type::index_type              index_type;

  return_type* operator()(C* cont) const
  {
    return new return_type(cont);
  }
};


template <typename T, int N = 1>
struct repeat_container;


template <typename Domain, typename T, int N>
struct repeat_container_distribution
  : p_object
{
  typedef Domain                                    domain_type;
  typedef repeat_container<T, N>*                   component_type;
  typedef metadata_entry<domain_type, component_type>  dom_info_type;

  future<dom_info_type> metadata_at(size_t)
  {
    return make_ready_future(dom_info_type(
             typename dom_info_type::cid_type(), domain_type(), 0,
             LQ_DONTCARE, invalid_affinity_tag, this->get_rmi_handle(), 0));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Defines the container to represent an infinite collection
///        of elements
/// @tparam T element type to repeat
//////////////////////////////////////////////////////////////////////
template<typename T, int N>
struct repeat_container
{
  typedef T                                                 value_type;
  typedef T&                                                reference;
  typedef T const&                                          const_reference;
  typedef typename std::conditional<
    N == 1, infinite, infinite_nd<N>>::type                 domain_type;
  typedef typename dimension_traits<domain_type>::type      dimensions_type;
  typedef typename dimension_traits<domain_type>::type      dimension_type;
  typedef typename domain_type::index_type                  index_type;
  typedef typename domain_type::index_type                  gid_type;
  typedef typename domain_type::index_type                  cid_type;
  typedef typename domain_type::size_type                   size_type;
  typedef infinite_locality_metadata<repeat_container, N>   loc_dist_metadata;
  typedef repeat_container_distribution<domain_type, T, N>  distribution_type;

  value_type         m_data;
  distribution_type  m_dist;

  repeat_container(void)
    : m_data()
  { }

  repeat_container(value_type const& val)
    : m_data(val)
  { }

  distribution_type& distribution(void)
  { return m_dist; }

  reference operator[](index_type)
  {
    return m_data;
  }

  value_type get_element(index_type) const
  {
    return m_data;
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return 0;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies a function f to the element specified by the GID,
  /// and returns the result.
  /// @param gid The GID of the element.
  /// @param f The Functor to apply on the element.
  /// @return The result of applying the functor to the element.
  //////////////////////////////////////////////////////////////////////
  template<typename F>
  typename F::result_type apply_get(gid_type const& gid, F const& f)
  {
    return f(m_data);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of elements associated with this
  ///        container, in this case is infinite (maximum value of \c
  ///        size_t)
  //////////////////////////////////////////////////////////////////////
  size_type size(void) const
  {
    return domain_type().size();
  }

  domain_type domain(void) const
  {
    return domain_type();
  }

  typedef std::true_type task_placement_dontcare;

  constexpr locality_info locality(size_t) const
  {
    return LQ_DONTCARE;
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    abort("repeat_container packing attempted");
  }
}; // struct repeat_container

} // namespace view_impl


template <typename C>
class repeat_view;


template<typename C>
struct view_traits<repeat_view<C>>
  : default_view_traits<C,
      typename container_traits<C>::domain_type,
      f_ident<typename container_traits<C>::domain_type::index_type>,
      repeat_view<C>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container transform for variadic
///  based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename OldC, typename NewC>
struct cast_container_view<repeat_view<OldC>, NewC>
  : new_cast_container_view<repeat_view<OldC>, NewC>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container domain and mapping function
///   transform for variadic based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename C, typename Dom, typename MF>
struct upcast_view<repeat_view<C>, Dom, MF>
  : new_upcast_view<repeat_view<C>, Dom, MF>
{ };


////////////////////////////////////////////////////////////////////////
/// @brief A view that can provide infinite copies of the same element.
/// @tparam T The type of element to be repeated.
////////////////////////////////////////////////////////////////////////
template<typename C>
class repeat_view
  : public core_view<
      C,
      typename view_traits<repeat_view<C>>::domain_type,
      typename view_traits<repeat_view<C>>::map_function>,
    public view_operations::read<repeat_view<C>>,
    public std::conditional<
      dimension_traits<C>::type::value == 1,
      view_operations::subscript<repeat_view<C>>,
      view_operations::multi_dimensional_subscript<repeat_view<C>>
    >::type,
    public view_operations::sequence<
      repeat_view<C>, detail::view_iterator<repeat_view<C>>>
{
public:
  STAPL_VIEW_REFLECT_TRAITS(repeat_view)

private:
  typedef view_operations::sequence<
    repeat_view, detail::view_iterator<repeat_view>>     sequence_op_type;

  typedef core_view<C, domain_type, map_function>        base_type;

public:
  typedef typename dimension_traits<C>::type             dimensions_type;
  typedef dimensions_type                                dimension_type;

  typedef typename sequence_op_type::iterator            iterator;
  typedef typename sequence_op_type::const_iterator      const_iterator;

  repeat_view(repeat_view const& other) = default;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor used to pass ownership of the container to the view.
  ///
  /// @param vcont pointer to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  repeat_view(view_container_type* vcont,
              domain_type const& dom,
              map_func_type mfunc = map_func_type())
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont reference to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  repeat_view(view_container_type const& vcont,
              domain_type const& dom,
              map_func_type mfunc = map_func_type())
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor that does not takes ownership over the passed
  ///        container.
  ///
  /// @param vcont reference to the container used to forward the operations.
  /// @param dom domain to be used by the view.
  /// @param mfunc mapping function to transform view indices to container
  ///        gids.
  //////////////////////////////////////////////////////////////////////
  repeat_view(view_container_type const& vcont,
              domain_type const& dom,
              map_func_type mfunc,
              repeat_view const&)
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that represents an infinite array of the
  ///        same element (@p data).
  ///
  /// @param data Element to repeat.
  //////////////////////////////////////////////////////////////////////
  repeat_view(value_type const& data)
    : base_type(new view_container_type(data), domain_type())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief use to examine this class
  /// @param msg your message (to provide context)
  //////////////////////////////////////////////////////////////////////
  void debug(char *msg=0)
  {
    std::cerr << "REPEAT_VIEW " << this << " : ";
    if (msg) {
      std::cerr << msg;
    }
    std::cerr << std::endl;
    base_type::debug();
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    stapl_assert(false, "repeat_view is not meant to be shipped");
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct a @ref repeat_view
/// @tparam T element type to repeat
/// @return a repeat view
//////////////////////////////////////////////////////////////////////
template<typename T>
repeat_view<view_impl::repeat_container<T>>
make_repeat_view(T const& data)
{
  return repeat_view<view_impl::repeat_container<T>>(data);
}


//////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct a @ref repeat_view
/// @tparam N dimensionality to create repeat view with.
/// @tparam T element type to repeat
/// @return a repeat view with @p N dimensions
//////////////////////////////////////////////////////////////////////
template<int N, typename T>
repeat_view<view_impl::repeat_container<T, N>>
make_repeat_view_nd(T const& data)
{
  return repeat_view<view_impl::repeat_container<T, N>>(data);
}

} // namespace stapl

#endif // STAPL_VIEWS_REPEATED_VIEW_HPP
