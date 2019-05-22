/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_ARRAY_VIEW_HPP
#define STAPL_VIEWS_ARRAY_VIEW_HPP

#include <stapl/views/core_view.hpp>

#include <stapl/views/mapping_functions/identity.hpp>

#include <stapl/views/operations/subscript.hpp>
#include <stapl/views/operations/read_write.hpp>
#include <stapl/views/operations/sequence.hpp>

#include <stapl/views/type_traits/upcast.hpp>

#include <stapl/containers/type_traits/container_traits.hpp>
#include <stapl/utility/use_default.hpp>

namespace stapl {

#ifdef STAPL_DOCUMENTATION_ONLY

//////////////////////////////////////////////////////////////////////
/// @brief One dimensional array view
///
/// Provides the operations that are commonly present in an array
/// (random access, iteration).
/// @tparam C Container type.
/// @tparam Dom Domain type. By default uses the same domain type
///             provided for the container.
/// @tparam Mapping function type
///    (default: identity mapping function)
/// @tparam Derived Type of the most derived class (default: itself)
/// @ingroup array_view
//////////////////////////////////////////////////////////////////////
template<typename C,
         typename Dom = typename container_traits<C>::domain_type,
         typename MapFunc = f_ident<typename Dom::index_type>,
         typename Derived = use_default>
class array_view;

#else

template<typename C, typename ...OptionalParams>
class array_view;

#endif


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when only container type parameter is specified.
//////////////////////////////////////////////////////////////////////
template<typename C>
struct view_traits<array_view<C>>
  : default_view_traits<C,
      typename container_traits<C>::domain_type,
      f_ident<typename container_traits<C>::domain_type::index_type>,
      array_view<C>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when container and domain type parameters
///    are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D>
struct view_traits<array_view<C, D>>
  : default_view_traits<C, D, f_ident<typename D::index_type>, array_view<C,D>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when container, domain, and mapping function
///  type parameters are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D, typename F>
struct view_traits<array_view<C, D, F>>
  : default_view_traits<C, D, F, array_view<C, D, F>>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for when all four type parameters are specified.
//////////////////////////////////////////////////////////////////////
template<typename C, typename D, typename F, typename Derived>
struct view_traits<array_view<C, D, F, Derived>>
  : default_view_traits<C, D, F, Derived>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container transform for variadic
///  based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename OldC, typename NewC, typename ...Params>
struct cast_container_view<array_view<OldC, Params...>, NewC>
  : new_cast_container_view<array_view<OldC, Params...>, NewC>
{ };


//////////////////////////////////////////////////////////////////////
/// @brief Specialization ensures container domain and mapping function
///   transform for variadic based optionals is used.
//////////////////////////////////////////////////////////////////////
template<typename C, typename Dom, typename MF, typename ...Params>
struct upcast_view<array_view<C, Params...>, Dom, MF>
  : new_upcast_view<array_view<C, Params...>, Dom, MF>
{ };


template <typename C, typename ...OptionalParams>
class array_view
  : public core_view<
      C,
      typename view_traits<array_view<C, OptionalParams...>>::domain_type,
      typename view_traits<array_view<C, OptionalParams...>>::map_function
    >,
    public view_operations::readwrite<array_view<C, OptionalParams...>>,
    public view_operations::subscript<array_view<C, OptionalParams...>>,
    public view_operations::sequence<
      typename view_traits<array_view<C, OptionalParams...>>::derived_type
    >
{
public:
  STAPL_VIEW_REFLECT_TRAITS(array_view)

private:
  typedef typename view_traits<array_view>::derived_type       derived_type;

  typedef core_view<C, domain_type, map_function>              base_type;

  typedef view_operations::sequence<derived_type>              sequence_op_type;

public:
  typedef typename sequence_op_type::iterator                  iterator;
  typedef typename sequence_op_type::const_iterator            const_iterator;

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructor used to pass ownership of the container to the view.
  ///
  /// @param vcont Pointer to the container used to forward the operations.
  /// @param dom Domain to be used by the view.
  /// @param mfunc Mapping function to transform view indices to container
  ///              gids.
  //////////////////////////////////////////////////////////////////////
  array_view(view_container_type* vcont,
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
  array_view(view_container_type* vcont,
             domain_type const& dom,
             map_func_type mfunc,
             array_view const&)
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
  array_view(view_container_type const& vcont,
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
  /// @param other View to copy from (ignored for array_view)
  //////////////////////////////////////////////////////////////////////
  array_view(view_container_type const& vcont,
             domain_type const& dom,
             map_func_type mfunc,
             array_view const&)
    : base_type(vcont, dom, mfunc)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that can reference all the elements of
  ///        the passed container. The view takes ownership of the container.
  ///
  /// @param vcont Pointer to the container used to forward the operations.
  //////////////////////////////////////////////////////////////////////
  array_view(view_container_type* vcont)
    : base_type(vcont, stapl::get_domain(*vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a view that can reference all the elements of
  ///        the passed container.
  ///
  /// @param vcont Reference to the container used to forward the operations.
  //////////////////////////////////////////////////////////////////////
  array_view(view_container_type& vcont)
    : base_type(vcont, stapl::get_domain(vcont))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Copy constructor when the passed view is not the most
  ///        derived view.
  /// @todo update domain to reflect change in size
  //////////////////////////////////////////////////////////////////////
  template<typename ...OtherOptionalParams>
  array_view(array_view<C, OtherOptionalParams...> const& other)
    : base_type(other.container(), other.domain(), other.mapfunc())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Update the underlying container to hold the specified
  /// number of elements.  All previous information is lost.
  //////////////////////////////////////////////////////////////////////
  void resize(size_t size)
  {
    this->incr_version();
    this->container().resize(size);
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  /// @brief use to examine this class
  //////////////////////////////////////////////////////////////////////
  void debug(char *msg=0)
  {
    std::cerr << "ARRAY_VIEW %p: " << this;
    if (msg) {
      std::cerr << msg;
    }
    std::cerr << "\n";
    (static_cast<base_type *>(this))->debug();
  }

  //////////////////////////////////////////////////////////////////////
  /// @internal
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
}; //class array_view


//////////////////////////////////////////////////////////////////////
/// @brief Helper function to construct an array_view over the passed
///        container
///
/// @param ct Underlying container used for the array_view.
/// @return An array_view over the Container.
//////////////////////////////////////////////////////////////////////
template<typename Container>
array_view<Container>
make_array_view(Container& ct)
{
  return array_view<Container>(ct);
}


namespace functional {

//////////////////////////////////////////////////////////////////////
/// @brief Functor to define a functional construction of array_view.
///
/// @return an array_view over the passed referenced container.
/// @note This functor is used to construct array_view over proxy.
//////////////////////////////////////////////////////////////////////
struct array_view
{
  template<typename Signature>
  struct result;

  template<typename Ref>
  struct result<array_view(Ref)>
  {
    typedef stapl::array_view<Ref> type;
  };

  template<typename Ref>
  stapl::array_view<Ref>
  operator()(Ref elem) const
  {
    return stapl::array_view<Ref>(new Ref(elem));
  }
};

} // namespace functional

} // namespace stapl

#endif // STAPL_VIEWS_ARRAY_VIEW_HPP
