/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

// Coarsen partition based on locality information

#ifndef STAPL_VIEWS_METADATA_COARSEN_VIEW_HPP
#define STAPL_VIEWS_METADATA_COARSEN_VIEW_HPP

#include <type_traits>

#include <stapl/runtime.hpp>

#include <stapl/views/type_traits/has_locality_metadata.hpp>
#include <stapl/views/type_traits/is_segmented_view.hpp>
#include <stapl/views/type_traits/is_trivially_coarsenable.hpp>

#include <stapl/views/metadata/container_fwd.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>

#include <stapl/domains/partitioned_domain.hpp>
#include <stapl/views/metadata/coarsening_traits.hpp>

#include <stapl/views/metadata/coarsen_view_fwd.hpp>

namespace stapl {

namespace metadata {

//////////////////////////////////////////////////////////////////////
/// @brief Retrieves the metadata for a view by calling the extraction
///        operator for the container and potentially restricting the
///        domain of the metadata.
///
///        The decision to restrict (project) is decided at runtime based on
///        whether or not the view's domain and the container's domain are
///        identical.
///
///        Returns the metadata in the polymorphic @ref metadata::view_wrapper.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct extract_and_restrict_view
{
private:
  using extractor_t = extract_metadata<typename View::view_container_type>;
  using extracted_t = typename extractor_t::return_type;

  using projection_t =
    typename coarsening_traits<
      typename std::decay<View>::type
    >::template construct_projection<extracted_t>::type;

  using projected_t  = typename projection_t::return_type;

  using adapt_md_t   = projected_container<View, extracted_t>;
  using md_view_type = view<adapt_md_t, typename adapt_md_t::domain_type>;

public:
  using return_type =
    metadata::view_wrapper<
      typename adapt_md_t::value_type,
      typename adapt_md_t::domain_type,
      metadata_traits<extracted_t>::is_isomorphic::value
    >;

  static
  return_type* apply(View* vw)
  {
    extracted_t* cont_md = extractor_t()(vw->get_container());

    if (vw->domain().is_same_container_domain())
    {
      // Identity mapping function, covering entire domain, create a wrapper
      // around container metadata and return it.
      adapt_md_t* adapt_md = new adapt_md_t(vw, cont_md);

      return new return_type(new md_view_type(adapt_md));
    }
    else
    {
      // Identity mapping function, partial container domain coverage
      return new return_type(
        new view<projected_t>(
          projection_t()(vw, cont_md)
        )
      );
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Retrieves the metadata for a view by calling the extraction
///        operator for the container immediately invoking the projection
///        operator.
///
///        Returns the metadata in the polymorphic @ref metadata::view_wrapper.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct extract_and_project_view
{
private:
  using extractor_t  = extract_metadata<typename View::view_container_type>;
  using extracted_t  = typename extractor_t::return_type;

  using projection_t =
    typename coarsening_traits<typename std::decay<View>::type>
      ::template construct_projection<extracted_t>::type;
  using projected_t  = typename projection_t::return_type;

  using value_t      = typename projected_t::value_type;

public:
  using return_type =
    metadata::view_wrapper<
      value_t,
      typename projected_t::domain_type,
      metadata_traits<projected_t>::is_isomorphic::value>;

  static
  return_type* apply(View* vw)
  {
    using metadata_view_t =
     view<projected_t, typename projected_t::domain_type>;

    // extract the metadata from the container
    extracted_t* cont_md = extractor_t()(vw->get_container());

    // project the metadata onto the view's domain
    metadata_view_t* proj = new metadata_view_t(projection_t()(vw, cont_md));

    // return the metadata with the type of the metadata container erased
    return new return_type(proj);
  }
}; // struct extract_and_project_view


//////////////////////////////////////////////////////////////////////
/// @brief Retrieves the metadata for a view by calling the extraction
///        operator. Used when it is statically known that projection is not
///        necessary for the given view.
///
///        Returns the metadata in the non-polymorphic @ref metadata::view.
//////////////////////////////////////////////////////////////////////
template<typename View>
struct extract_view
{
private:
  using ct_partition_t = extract_metadata<typename View::view_container_type>;

  using ct_metadata_return_t = typename ct_partition_t::return_type;
  using adapt_md_t           = projected_container<View, ct_metadata_return_t>;
  using md_view_type         =
    view<adapt_md_t, typename adapt_md_t::domain_type>;

public:
  using return_type =
    metadata::view_wrapper<
      typename adapt_md_t::value_type,
      typename ct_metadata_return_t::domain_type,
      metadata_traits<ct_metadata_return_t>::is_isomorphic::value>;

  static
  return_type* apply(View* vw)
  {
    stapl_assert(vw->domain().is_same_container_domain(),
      "attempting to elide projection for a view whose domain does not match "
      "its container's domain");

    // extract metadata from the view
    ct_metadata_return_t* cont_md = ct_partition_t()(vw->get_container());

    adapt_md_t* adapt_md = new adapt_md_t(vw, cont_md);

    return new return_type(new md_view_type(adapt_md));
  }
}; // struct extract_view

} // namespace metadata

} // namespace stapl

#endif
