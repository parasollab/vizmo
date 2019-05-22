/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_EXTRACTION_REVERSE_HPP
#define STAPL_VIEWS_METADATA_EXTRACTION_REVERSE_HPP

#include <stapl/views/metadata/container/reverse.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>

namespace stapl {

namespace metadata {

//////////////////////////////////////////////////////////////////////
/// @brief Functor used to extract the locality metadata associated
///        with a @ref reverse_view.
///
/// @tparam View A reverse_view
//////////////////////////////////////////////////////////////////////
template<typename View>
class reverse_extractor
{
  using view_part_type =
    typename coarsen_partition_impl::coarsen_partition<
      typename View::target_view_type
    >::return_type;

  using md_container_type = metadata::reverse_container<view_part_type>;

public:
  using return_type = view_wrapper<
    typename md_container_type::value_type,
    typename view_part_type::domain_type,
    metadata_traits<view_part_type>::is_isomorphic::value
  >;

  template<typename V>
  return_type* operator()(V* vw)
  {
    using original_view_type = typename V::target_view_type;

    // construct the original (non-reversed) view
    original_view_type original_view(
      vw->container(), vw->domain(), vw->mapfunc()
    );

    // get the metadata for the original view
    auto md_cont =
      coarsen_partition_impl::
        coarsen_partition<original_view_type>::apply(&original_view);

    // adapt the metadata to be in reverse order
    md_container_type* reverse_md =
      new md_container_type(md_cont, vw->m_total_size);

    // wrap the reversed metadata container in a view
    return new return_type(
      new view<md_container_type>(reverse_md)
    );
  }
};

} // namespace metadata

} // namespace stapl

#endif
