/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_COARSEN_UTILITY_HPP
#define STAPL_VIEWS_METADATA_COARSEN_UTILITY_HPP

#include <stapl/views/metadata/partitioned_mix_view.hpp>
#include <stapl/views/metadata/coarsen_container.hpp>
#include <stapl/views/metadata/coarsen_partition_locality.hpp>
#include <stapl/views/metadata/infinite_helpers.hpp>
#include <stapl/views/metadata/metadata_traits.hpp>

#include <stapl/views/metadata/container/growable.hpp>
#include <stapl/views/metadata/container/flat.hpp>
#include <stapl/views/metadata/container/metadata_view.hpp>

#include <stapl/utility/tuple.hpp>
#include <stapl/utility/vs_map.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Defines the coarsen metadata view type for the given
///        container type.
///
/// The metadata container type used is @ref metadata::growable_container.
/// @tparam MDCont Metadata container type.
//////////////////////////////////////////////////////////////////////
template<typename MDCont>
struct convert_to_md_vec_array
{
  typedef typename MDCont::value_type                   value_type;
  typedef metadata::growable_container<value_type>      part_type;
  typedef metadata::view<part_type>                     type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Defines the coarsen metadata view type for the given
///        container type.
///
/// The metadata container type used is @ref metadata::flat_container.
/// @tparam MDCont Metadata container type.
//////////////////////////////////////////////////////////////////////
template<typename MDCont>
struct convert_to_md_vec
{
  typedef typename MDCont::value_type                     value_type;
  typedef metadata::flat_container<value_type>            part_type;
  typedef metadata::view<part_type>                       type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Defines the coarsen metadata wrapper type for the given
///        container type.
///
/// @tparam MDCont Metadata container type.
/// @see metadata::view_wrapper.
//////////////////////////////////////////////////////////////////////
template<typename MDCont>
using convert_to_wrap_partition =
  metadata::view_wrapper<
    typename MDCont::value_type,
    typename MDCont::domain_type,
    metadata_traits<MDCont>::is_isomorphic::value
  >;


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to extract the locality metadata for each
///        view in the given @p ViewSet
//////////////////////////////////////////////////////////////////////
template<typename ViewSet,
         typename = make_index_sequence<tuple_size<ViewSet>::value>>
struct metadata_from_container;


template<typename ViewSet, std::size_t ...Indices>
struct metadata_from_container<ViewSet, index_sequence<Indices...>>
{
  template<int Index>
  using metadata_ct =
    typename metadata::extract_metadata<
      typename tuple_element<Index, ViewSet>::type>::return_type;

  using type =
   tuple<
     metadata::view_wrapper<
       typename metadata_ct<Indices>::value_type,
       typename metadata_ct<Indices>::domain_type,
       metadata_traits<metadata_ct<Indices>>::is_isomorphic::value
     >...
   >;

  static type apply(ViewSet const& vws)
  {
    return type(
      metadata::extract_metadata<
        const typename tuple_element<Indices, ViewSet>::type
      >()(&get<Indices>(vws))...
    );
  }
}; // struct metadata_from_container


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to construct the coarsened view
///        (partitioned_mix_view) for each view in the given set of
///        views, using the native coarsen metadata partition.
//////////////////////////////////////////////////////////////////////
template<typename ViewSet,
          typename = make_index_sequence<tuple_size<ViewSet>::value>>
struct coarsen_views_native;


template<typename ViewSet, std::size_t ...Indices>
struct coarsen_views_native<ViewSet, index_sequence<Indices...>>
{
  typedef metadata_from_container<ViewSet>       metadata_extractor_type;
  typedef typename metadata_extractor_type::type metadata_type;


  template<int Index>
  using return_vw =
    partitioned_mix_view<
      typename tuple_element<Index, ViewSet>::type,
      typename tuple_element<Index, metadata_type>::type
    >;

  using type = tuple<return_vw<Indices>...>;

  static type apply(ViewSet const& views, metadata_type const& parts)
  {
    return type(
      return_vw<Indices>(
        new typename return_vw<Indices>::view_container_type(
          get<Indices>(views), get<Indices>(parts)))...);
  }
};

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_COARSEN_UTILITY_HPP
