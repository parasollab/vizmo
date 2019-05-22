/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_VIEWS_METADATA_PROJECTION_SLICES_HPP
#define STAPL_VIEWS_METADATA_PROJECTION_SLICES_HPP

#include <stapl/runtime.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>
#include <stapl/domains/partitioned_domain.hpp>
#include <stapl/views/metadata/locality_dist_metadata.hpp>
#include <stapl/utility/cross_map.hpp>
#include <stapl/utility/integer_sequence.hpp>

#include <stapl/containers/type_traits/dimension_traits.hpp>
#include <stapl/views/metadata/container/multiarray.hpp>

#include <stapl/views/type_traits/is_segmented_view.hpp>
#include <stapl/containers/type_traits/is_container.hpp>

namespace stapl {

namespace metadata {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Functor that receives a tuple and either returns the original
///        tuple if it has more than one element, or returns just the
///        one element if it's 1-dimensional.
//////////////////////////////////////////////////////////////////////
template<typename T, int = tuple_size<T>::value>
struct flatten_if_single
{
  static T get (T const& t)
  {
    return t;
  }
};

template<typename T>
struct flatten_if_single<T, 1>
{
  static typename tuple_element<0, T>::type get(T const& t)
  {
    return std::get<0>(t);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Functor to retrieve the partition size of the inner-most
///        container.
///
/// @tparam View The (possibly nested) segmented view
/// @tparam Compile-time boolean indicating whether this is a non-nested view.
//////////////////////////////////////////////////////////////////////
template<typename View, bool = !stapl::is_segmented_view<
  typename View::view_container_type::view_container_type>::value
>
class partition_dimensions
{
public:
  //////////////////////////////////////////////////////////////////////
  /// @brief When the inner view is actually a view.
  //////////////////////////////////////////////////////////////////////
  template<typename C>
  static auto apply_impl(C* c,
    typename std::enable_if<is_view<C>::value>::type* = 0)
    -> decltype(c->container().distribution().partition().domain().dimensions())
  {
    return c->container().distribution().partition().domain().dimensions();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief When the inner view is a proxy over a container.
  //////////////////////////////////////////////////////////////////////
  template<typename C>
  static auto apply_impl(C* c,
     typename std::enable_if<!is_view<C>::value>::type* = 0)
    -> decltype(c->distribution().partition().domain().dimensions())
  {
    return c->distribution().partition().domain().dimensions();
  }

public:
  static auto apply(View const* view)
    -> decltype(apply_impl(view->container().get_container()))
  {
    return apply_impl(view->container().get_container());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Recursive case when we have a segmented view over a segmented view.
//////////////////////////////////////////////////////////////////////
template<typename View>
class partition_dimensions<View, false>
{
  using recursive_call_t =
    partition_dimensions<
      typename View::view_container_type::view_container_type
    >;

public:
  static auto apply(View const* view)
    -> decltype(recursive_call_t::apply(view->container().get_view()))
  {
    return recursive_call_t::apply(view->container().get_view());
  }
};

template<typename Container, bool = stapl::is_container<Container>::value>
struct extract_slices
{
  using type =
    typename tuple_ops::from_index_sequence<
      typename Container::partition_type::distribution_view_type::
        partition_dimensions_type
    >::type;
};

template<typename View>
struct extract_slices<View, false>
{
  using type =
    typename extract_slices<typename View::view_container_type>::type;
};

} // namespace detail

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor used to project the domains in the given
///        locality metadata (@c P) to the domain of the given @c
///        View. The result is projected metadata locality
///        information.
///
/// This helper functor is invoked when the given @c View is a
/// slices view.
///
/// @todo operator should return a shared_ptr.
//////////////////////////////////////////////////////////////////////
template <typename View, typename P>
class slices_projection
{
  typedef typename detail::extract_slices<View>::type          slices_type;
  typedef typename View::domain_type                           domain_type;
  typedef typename P::value_type::component_type               component_type;
  typedef typename tuple_ops::result_of::filter<slices_type,
            typename P::value_type::cid_type>::type            cid_type;
  typedef typename dimension_traits<View>::type                dimension_t;
  typedef make_index_sequence<dimension_t::value>              index_sequence_t;

  std::size_t entry_id(std::size_t x) const
  {
    return x;
  }

  template<typename... T>
  typename tuple_ops::result_of::filter<slices_type, tuple<T...>>::type
  entry_id(tuple<T...> const& x) const
  {
    return tuple_ops::filter<slices_type>(x);
  }

public:
  typedef metadata_entry<
    domain_type, component_type, cid_type
  >                                                            md_entry_type;
  typedef typename metadata::detail::select_container_type<
    md_entry_type, dimension_t::value
  >::type                                                      return_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Project the metadata entries extracted from the underlying view
  ///        into the domain of the view.
  ///
  /// @param view The original view
  /// @param part Container of metadata entries
  //////////////////////////////////////////////////////////////////////
  return_type* operator()(View* view, P* part) const
  {
    // number of partitions of the original container
    const auto num_parts = detail::partition_dimensions<View>::apply(view);

    // need to extract it as a scalar if it's 1D
    auto const num_parts_flattened =
      detail::flatten_if_single<decltype(num_parts)>::get(num_parts);

    return_type* res = new return_type(num_parts_flattened);

    for (auto&& md : *part)
    {
      // pointer to the base container
      component_type c = md.component();

      // domain for the metadata entry
      auto const dom = md.domain();

      // reduced domain for the slices view
      const domain_type new_dom(
        tuple_ops::filter<slices_type>(dom.first()),
        tuple_ops::filter<slices_type>(dom.last())
      );

      // index of this entry in the metadata container
      const auto idx = this->entry_id(md.id());


      res->operator[](idx) = md_entry_type(
        idx, new_dom, c,
        md.location_qualifier(), md.affinity(), md.handle(), md.location()
      );
    }

    res->update();

    // Needed to handle the case where a sliced dimension has a single metadata
    // entry. part is read by multiple locations and cannot be deleted until
    // those requests have been serviced.
    rmi_fence();

    delete part;

    return res;
  }
};

} // namespace metadata

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_PROJECTION_SLICES_HPP
