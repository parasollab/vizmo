/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_SLICES_VIEW_HPP
#define STAPL_VIEWS_SLICES_VIEW_HPP

#include <stapl/views/segmented_view.hpp>
#include <stapl/views/sliced_view.hpp>
#include <stapl/utility/integer_sequence.hpp>
#include <stapl/containers/partitions/sliced.hpp>
#include <stapl/views/metadata/projection/slices.hpp>

#include <stapl/views/metadata/coarsening_traits.hpp>

namespace stapl {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Defines intermediate types used to define instantiation
///  of a segmented view which underlies the slices view concept.
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Container, typename... OptParams>
struct slices_view_types
{
  typedef typename view_traits<
    SLICED_view<Slices, Container, OptParams...>
   >::map_function                                       map_function;

  typedef map_fun_gen1<map_function>                     mfg_type;

  typedef typename view_traits<
    SLICED_view<Slices, Container, OptParams...>
   >::domain_type                                        domain_type;

  typedef sliced_partition<
    Slices, typename Container::domain_type
  >                                                      partition_type;

  typedef typename compose_func<
    typename Container::map_function, map_function
  >::type                                                composed_mf_t;

  typedef SLICED_view<
    Slices, Container, domain_type, composed_mf_t
  >                                                      subview_type;

  using subview_creator_type = view_impl::default_subview_creator<subview_type>;

  typedef segmented_view<
    Container, partition_type, mfg_type, subview_creator_type
  >                                                      type;
};

} // namespace detail


//////////////////////////////////////////////////////////////////////
/// @brief Wrapper class around segmented_view template instantiation
///  used to implement slices view concept.  Exists for clarity and to
///  reduce type's symbol size.
/// @todo Propagate use of index_sequence into @ref SLICED_view
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename Container, typename... OptParams>
class slices_segmented_view;


template<std::size_t ...Indices, typename Container, typename... OptParams>
class slices_segmented_view<index_sequence<Indices...>, Container, OptParams...>
  : public detail::slices_view_types<
      tuple<std::integral_constant<int, Indices>...>,
      Container,
      OptParams...>::type
{
private:
  typedef typename detail::slices_view_types<
    tuple<std::integral_constant<int, Indices>...>,
    Container,
    OptParams...>::type                                base_type;

public:
  template<typename... Args>
  slices_segmented_view(Args&&... args)
    : base_type(std::forward<Args>(args)...)
  { }
};  // class slices_segmented_view


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for changing the container used for a
///        slices segmented view
//////////////////////////////////////////////////////////////////////
template<typename Slices,
         typename Container,
         typename... OptParams,
         typename NewC >
struct cast_container_view<
  slices_segmented_view<Slices, Container, OptParams...>, NewC
>
{
  using type = slices_segmented_view<
    Slices, typename cast_container_view<Container,NewC>::type, OptParams...
  >;
};

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Helper class to manage the lifetime of the component pointer
///        of a localized @ref slices_segmented_view. Needed to grab
///        the pointer in the case of composed slices views.
///
/// @tparam SlicesView Segmented slices view type
/// @tparam MixView    Type of the mix view associated with the slices view
/// @tparam ComposedSlicesView Boolean indicating whether the slices view
///         is a composed slices view
///
/// @todo This only works for two levels of slices view, where they are
///       both coarsened such that everything is local.
//////////////////////////////////////////////////////////////////////
template<typename SlicesView, typename MixView,
  bool ComposedSlicesView = is_segmented_view<
    typename SlicesView::view_container_type::view_container_type>::value
  >
class component_ptr_manager
{
  using component_t = typename MixView::component_type;
  using base_t = typename cast_container_view<
      SlicesView, component_t
    >::type;
  using view_container_t = typename base_t::view_container_type;

public:
  using type = typename view_container_t::view_view_container_type;

private:
  std::shared_ptr<type> m_component;

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Grab the pointer for the component. Since this is a composed
  ///        slices view, we need to create a view container that stores the
  ///        inner slices view and set its component pointer to be
  ///        the bottom-most container.
  ///
  ///        Note that this only works for two levels of composition.
  ///
  /// @param v The outermost slices view
  //////////////////////////////////////////////////////////////////////
  template<typename View>
  static type* get(View const& v)
  {
    return new type(v.get_container()->get_container(), v.get_component());
  }

  component_ptr_manager(type* component)
   : m_component(component)
  { }

};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization whete the slices view is non-composed.
//////////////////////////////////////////////////////////////////////
template<typename SlicesView, typename MixView>
class component_ptr_manager<SlicesView, MixView, false>
{
public:
  using type = typename MixView::component_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Grab the pointer for the component.
  /// @param v The slices view
  //////////////////////////////////////////////////////////////////////
  template<typename View>
  static type* get(View const& v)
  {
    return v.get_component();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Lifetime of the component is managed by the container manager
  ///        in the case where the component comes from a pContainer.
  //////////////////////////////////////////////////////////////////////
  component_ptr_manager(type* component) { }
};

} // namespace detail

template<typename View>
struct nfast_view;


//////////////////////////////////////////////////////////////////////
/// @brief Fast view class for transforming a segmented_slices_view over
///        a container to one over its base container
//////////////////////////////////////////////////////////////////////
template<typename Slices, typename C, typename... OptParams, typename Info,
         typename CID>
struct nfast_view<
  mix_view<slices_segmented_view<Slices,C,OptParams...>,Info,CID>
>
  : public cast_container_view<
      slices_segmented_view<Slices,C,OptParams...>,
      typename mix_view<slices_segmented_view<
        Slices,C,OptParams...>, Info, CID
      >::component_type
    >::type
{
private:
  using slices_view_t = slices_segmented_view<Slices, C, OptParams...>;
  using mix_view_t = mix_view<slices_view_t, Info, CID>;
  using component_t = typename mix_view_t::component_type;

  using base_t = typename cast_container_view<
      slices_view_t, component_t
    >::type;

  /// This is the type of the view_impl::view_container that is storing the
  /// view that is being segmented and its container
  using view_container_t = typename base_t::view_container_type;

  using component_ptr_manager_t = detail::component_ptr_manager<
    slices_view_t, mix_view_t
  >;

  /// Storage for the component of the fast view, which is either the component
  /// of the container, or a view_container with the bottom most component
  /// swapped out to be localized
  component_ptr_manager_t m_component_ptr;

public:
  nfast_view(mix_view_t const& v)
    :
      base_t(
        new view_container_t(
          v.get_container(), component_ptr_manager_t::get(v)
        ),
        v.domain(), v.mapfunc()
      ),
      m_component_ptr(this->get_container()->get_container())
  { }
};


template<typename Slices, typename Container, typename... OptParams>
struct coarsening_traits<slices_segmented_view<Slices, Container, OptParams...>>
{
  typedef slices_segmented_view<Slices, Container, OptParams...> view_type;

  template<typename P>
  struct construct_projection
  {
    typedef metadata::slices_projection<const view_type, P> type;
  };
};


//////////////////////////////////////////////////////////////////////
/// @brief Create a slices view over a d-dimensional view, which
///        is a view of dimension |Slices| that produces subviews
///        of dimension d-|Slices|. This essentially produces a
///        @ref slice_view for each possible value of the dimension
///        that is being sliced along.
///
///        For example, a slices view slicing off dimension 1 of a 3D
///        view will create a 1D view whose domain is all of the values
///        of the 1st dimension and whose elements are 2D views.
///
/// @tparam Slice A variadic list of dimensions to slice along
/// @param ct The view to create the slices view on top of
//////////////////////////////////////////////////////////////////////
template<std::size_t... Slice, typename Container>
slices_segmented_view<index_sequence<Slice...>, Container>
make_slices_view(Container const& ct)
{
  typedef slices_segmented_view<index_sequence<Slice...>, Container> view_type;

  return view_type(ct, typename view_type::partition_type(ct.domain()));
}

} // namespace stapl

#endif
