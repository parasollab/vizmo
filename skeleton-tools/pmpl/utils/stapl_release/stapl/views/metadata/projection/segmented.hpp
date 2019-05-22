/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_PROJECTION_SEGMENTED_HPP
#define STAPL_VIEWS_METADATA_PROJECTION_SEGMENTED_HPP

#include <stapl/runtime.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>
#include <stapl/domains/partitioned_domain.hpp>
#include <stapl/views/metadata/locality_dist_metadata.hpp>

#include <stapl/views/metadata/container_fwd.hpp>

#include <stapl/containers/type_traits/dimension_traits.hpp>

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
struct segmented_metadata_projection
{
  // Adjust 1D domains
  template <typename V,typename MD,typename Dim>
  struct adjust_metadata_domains_helper
  {
    typedef typename V::domain_type                      domain_type;
    typedef typename V::map_func_type                    map_func_type;

    typedef typename MD::value_type::component_type      component_type;
    typedef metadata_entry<domain_type,component_type,
              typename MD::value_type::cid_type >        dom_info_type;
    typedef metadata::growable_container<dom_info_type>  return_type;

    return_type*
    operator()(View* view, P* cpart) const
    {
      typedef typename View::view_container_type         container_type;

      typedef std::vector<std::pair<domain_type,bool> >  vec_doms_t;

      // get the partitioner from the partitioner_container
      auto par = view->container().partition();
      auto mfg = view->container().mapfunc_generator();

      return_type* res = new return_type();

      for (typename P::iterator it=cpart->begin();it!=cpart->end();++it)
      {
        auto entry = *it;
        component_type sc = entry.component();

        vec_doms_t doms = par.contained_in(entry.domain(), mfg);
        for (typename vec_doms_t::iterator dj=doms.begin();dj!=doms.end();++dj)
        {
          if ((*dj).second) {
            res->push_back_here(dom_info_type(
              typename dom_info_type::cid_type(), (*dj).first, sc,
              entry.location_qualifier(), entry.affinity(),
              entry.handle(), entry.location()
            ));
          }
          else {
            res->push_back_here(dom_info_type(
              typename dom_info_type::cid_type(), (*dj).first, NULL,
              entry.location_qualifier(), entry.affinity(),
              entry.handle(), entry.location()
            ));

          }
        }
      }

      res->update();

      delete cpart;

      return res;
    }

  };

  typedef typename View::domain_type                           domain_type;

  typedef typename dimension_traits<
    typename P::value_type::domain_type::gid_type
  >::type                                                      dimension_t;

  typedef typename adjust_metadata_domains_helper<
    View, P, dimension_t
  >::return_type                                                 return_type;

  return_type* operator()(View* view, P* part)
  {
    return adjust_metadata_domains_helper<View,P,dimension_t>()(view,part);
  }
};

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_PROJECTION_SEGMENTED_HPP
