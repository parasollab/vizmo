/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_COARSEN_MULTIVIEWS_HPP
#define STAPL_VIEWS_COARSEN_MULTIVIEWS_HPP

#include <boost/mpl/int.hpp>
#include <boost/mpl/count_if.hpp>

#include <stapl/runtime.hpp>
#include <stapl/utility/tuple.hpp>
#include <stapl/paragraph/paragraph_fwd.h>

#include <stapl/views/metadata/container/growable.hpp>
#include <stapl/views/metadata/container/flat.hpp>
#include <stapl/views/metadata/container/projected.hpp>
#include <stapl/views/metadata/container/infinite.hpp>
#include <stapl/views/metadata/container/generator.hpp>
#include <stapl/views/metadata/container/metadata_view.hpp>

#include <stapl/views/metadata/coarseners/default.hpp>
#include <stapl/views/metadata/coarsen_utility.hpp>
#include <stapl/views/metadata/coarseners/null.hpp>
#include <stapl/views/metadata/alignment/guided.hpp>

#include <stapl/views/metadata/utility/are_aligned.hpp>
#include <stapl/views/metadata/utility/have_equal_sizes.hpp>

namespace stapl {

namespace metadata {

//////////////////////////////////////////////////////////////////////
/// @brief helper class template for @ref multiview_coarsener, dispatching
///   coarsener implementation based on the number of finite views found
///   in the set of views passed in tuple parameter @p Views.
///
/// Primary template is default implementation, used when multiple finite
/// domain views are detected, requiring alignment checks / enforcement.
//////////////////////////////////////////////////////////////////////
template<typename Views,
         bool Align,
         int FiniteViews= boost::mpl::count_if<
           Views, has_finite_domain<boost::mpl::_>
          >::value>
struct multiview_coarsener_impl
{
private:
  typedef metadata_from_container<Views>                  md_extractor_t;
  typedef typename md_extractor_t::type                   md_t;
  typedef typename first_finite_domain_index<Views>::type guide_idx_t;

public:
  static
  auto apply(Views const& views)
    ->decltype(coarsen_views_native<Views>::apply(
                 views, md_extractor_t::apply(views)))
  {
    constexpr int guide = guide_idx_t::value;

    stapl_assert(metadata::have_equal_sizes<guide>(views),
      "Attempting to coarsen views that have unequal sizes");

    // get metadata from the views
    md_t md_conts = md_extractor_t::apply(views);

    // check to see if the metadata containers have the same number of entries
    const bool b_same_num_entries = metadata::have_equal_sizes(md_conts);

    // if the metadata containers have the same number of entries and they
    // are already aligned, then just transform the fine-grained views
    // using the standard metadata
    if (b_same_num_entries && metadata::entries_are_aligned<guide>(md_conts))
      return coarsen_views_native<Views>::apply(views, md_conts);

    // views are not aligned, so invoke the alignment algorithm
    return metadata::guided_alignment<Views>::apply(
      views, md_conts, guide_idx_t()
    );
  }
};


template<typename Views, int FiniteViews>
struct multiview_coarsener_impl<Views, false, FiniteViews>
{
private:
  typedef metadata_from_container<Views>                  md_extractor_t;
  typedef typename md_extractor_t::type                   md_t;

public:
  static
  auto apply(Views const& views)
    ->decltype(coarsen_views_native<Views>::apply(
                 views, md_extractor_t::apply(views)))
  {
    using guide = typename first_finite_domain_index<Views>::type;

    stapl_assert(metadata::have_equal_sizes<guide::value>(views),
      "Attempting to coarsen views that have unequal sizes");

    // create metadata containers for each view using extractors
    // and projection algorithms
    md_t md_conts = md_extractor_t::apply(views);

    // create coarsened views from the metadata containers
    return coarsen_views_native<Views>::apply(views, md_conts);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when one finite view is found.  No alignment
///   (or corresponding check needed).  Call native coarsener.
//////////////////////////////////////////////////////////////////////
template<typename Views, bool Align>
struct multiview_coarsener_impl<Views, Align, 1>
{
private:
  typedef coarsen_views_native<Views> base_coarsener;

public:
  static auto apply(Views const& views)
  STAPL_AUTO_RETURN(
    base_coarsener::apply(
      views, base_coarsener::metadata_extractor_type::apply(views)))
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization when no finite views are found.
///   Statically assert as this case does not make sense for this
///   coarsening approach.
//////////////////////////////////////////////////////////////////////
template<typename Views, bool Align>
struct multiview_coarsener_impl<Views, Align, 0>
{
  static_assert(sizeof(Views) == 0,
                "Must provide at least one view with a finite domain.");
};

} // namespace metadata


//////////////////////////////////////////////////////////////////////
/// @brief Functor to coarsen a set of given views.
///
/// If all views are aligned a native coarsener is used.  Otherwise, a
/// aligning coarsener using one of the finite views as the "guide" is
/// employed.
//////////////////////////////////////////////////////////////////////
template<bool Align>
struct multiview_coarsener
{
  template<typename Views>
  auto operator()(Views const& views) const
  STAPL_AUTO_RETURN(
    STAPL_PROXY_CONCAT(
      metadata::multiview_coarsener_impl<Views, Align>::apply(views)
    )
  )
};

} // namespace stapl

#endif // STAPL_VIEWS_COARSEN_MULTIVIEWS_HPP
