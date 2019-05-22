/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_VIEWS_METADATA_PROJECTION_INVERTIBLE_HPP
#define STAPL_VIEWS_METADATA_PROJECTION_INVERTIBLE_HPP

#include <stapl/runtime.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>
#include <stapl/views/metadata/metadata_traits.hpp>
#include <stapl/views/metadata/container_fwd.hpp>

#include <stapl/domains/invert_domain.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor used to project the domains in the given
///        locality metadata (@c P) to the domain of the given @c
///        View. The result is projected metadata locality
///        information.
///
/// This helper functor is invoked when the given @c View is a
/// partitioned view.
///
/// @todo operator should return a shared_ptr.
//////////////////////////////////////////////////////////////////////
template <typename View, typename P>
class invertible_metadata_projection
{
  typedef typename View::domain_type                         domain_type;
  typedef typename metadata_traits<P>::value_type            value_type;
  typedef typename value_type::component_type             component_type;
  typedef typename value_type::cid_type                   cid_type;
  typedef typename View::map_func_type::inverse              inverse_mf_type;

public:
  typedef metadata_entry<
    domain_type, component_type, cid_type
  >                                                          md_entry_type;
  typedef metadata::growable_container<md_entry_type>        return_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Project the metadata entries extracted from the underlying view
  ///        into the domain of the view.
  ///
  /// @param view The original view
  /// @param part Container of metadata entries
  //////////////////////////////////////////////////////////////////////
  return_type* operator()(View* view, P* part) const
  {
    return_type* res = new return_type();

    inverse_mf_type inv(view->mapfunc());

    for (auto&& md : *part)
    {
      // the base container's domain
      auto dom = md.domain();
      component_type c = md.component();

      // project the base container's domain into the view's domain
      // using the provided inverse function
      domain_type projected_dom =
        invert_domain<domain_type>(dom, inv) & view->domain();

      // avoid creating empty md entries
      if (projected_dom.empty())
        continue;

      // if the projected domain is somehow bigger than the original domain
      // of the base container, we will disable fast view optimization
      component_type comp = projected_dom.size() <= dom.size() ? c : nullptr;

      res->push_back_here(md_entry_type(
        cid_type(),
        projected_dom, comp,
        md.location_qualifier(), md.affinity(), md.handle(), md.location()
      ));
    }

    res->update();

    delete part;

    return res;
  }
};

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_PROJECTION_INVERTIBLE_HPP
