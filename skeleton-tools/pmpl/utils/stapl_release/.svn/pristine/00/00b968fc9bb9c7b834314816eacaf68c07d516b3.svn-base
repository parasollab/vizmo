/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_PROJECTION_CONSTRUCT_DOMAIN_HPP
#define STAPL_VIEWS_METADATA_PROJECTION_CONSTRUCT_DOMAIN_HPP

#include <stapl/runtime.hpp>

namespace stapl {

// Forward declarations needed by construct_domain specialization.
template <typename Container, typename Functor>
class iterator_domain;

template <typename Distribution>
class list_distributed_domain;

template <typename Coordinates, typename Traversal>
class implicit_regular_mesh_domain;

template <typename T>
class domset1D;

template <typename Distribution>
class domainset1D;

namespace coarsen_partition_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Helper struct that allows for the construction of a domain
/// given the first and last indices and the view container to be specialized.
///
/// This is necessary because not all domains provide a uniform set of
/// constructors.
///
/// @note This can be removed should the classes for which specializations of
/// this struct be removed or updated with a set of constructors that match
/// other domains.
//////////////////////////////////////////////////////////////////////
template <typename Domain>
struct construct_domain
{
  template <typename Index, typename Distribution>
  Domain operator()(Index const& first, Index const& last,
                    Distribution const&, Domain const&) const
  {
    return Domain(first, last);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of domain construction for @ref iterator_domain.
/// Iterator domains require the container referenced by the iterators
/// to be provided.
//////////////////////////////////////////////////////////////////////
template <typename DomainContainer, typename Functor>
struct construct_domain<iterator_domain<DomainContainer, Functor> >
{
  typedef iterator_domain<DomainContainer, Functor> result_type;

  template <typename Index>
  iterator_domain<DomainContainer, Functor>
  operator()(Index const& first, Index const& last,
             DomainContainer const& container, result_type const&) const
  {
    return iterator_domain<DomainContainer, Functor>(first, last, container);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of domain construction for
/// @ref list_distributed_domain.
/// List domains require the distribution referenced by the indices
/// to be provided.
//////////////////////////////////////////////////////////////////////
template <typename ContainerDistribution>
struct construct_domain<list_distributed_domain<ContainerDistribution> >
{
  typedef list_distributed_domain<ContainerDistribution> result_type;

  template <typename Index, typename Distribution>
  list_distributed_domain<ContainerDistribution>
  operator()(Index const& first, Index const& last,
             Distribution const& dist, result_type const&)
  {
    return list_distributed_domain<ContainerDistribution>(first, last, dist);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of domain construction for
/// @ref implicit_regular_mesh_domain.  The mesh domain requires the indices
/// provided to be delinearized.
//////////////////////////////////////////////////////////////////////
template <typename Coordinates, typename Traversal>
struct construct_domain<implicit_regular_mesh_domain<Coordinates, Traversal> >
{
  typedef implicit_regular_mesh_domain<Coordinates, Traversal> result_type;

  template <typename Index, typename Distribution>
  implicit_regular_mesh_domain<Coordinates, Traversal>
  operator()(Index const& first, Index const& last,
             Distribution const& dist, result_type const&)
  {
    typename Distribution::tuple_type first_elt =
      dist.domain().reverse_linearize(first);
    typename Distribution::tuple_type last_elt =
      dist.domain().reverse_linearize(last);
    return result_type(first_elt, last_elt, dist.domain());
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of domain construction for
/// @ref domset1D.  The interval-based domain requires the original domain
/// to be provided in order to be intersected with the contiguous domain
/// formed by the indices provided.
//////////////////////////////////////////////////////////////////////
template <typename T>
struct construct_domain<domset1D<T> >
{
  typedef domset1D<T> result_type;

  template <typename Distribution>
  result_type operator()(T const& first, T const& last, Distribution const&,
                         result_type const& domain)
  {
    return result_type(first, last, domain);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Specialization of domain construction for
/// @ref domainset1D.  The interval-based domain requires the original domain
/// to be provided in order to be intersected with the contiguous domain
/// formed by the indices provided.
//////////////////////////////////////////////////////////////////////
template <typename Distribution>
struct construct_domain<domainset1D<Distribution> >
{
  typedef typename domainset1D<Distribution>::index_type index_type;

  typedef domainset1D<Distribution> result_type;

  result_type operator()(index_type const& first, index_type const& last,
                         Distribution const& dist, result_type const& domain)
  {
    return result_type(first, last, domain);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper struct used to construct a domain in the projection
///        process.
//////////////////////////////////////////////////////////////////////
template <typename Domain>
struct projection_construct_domain
{
  typedef typename Domain::index_type index_t;

  Domain
  operator()(index_t const& first, index_t const& last, Domain& vdom) const
  {
    return Domain(first, last);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper template to correctly construct domain in projection process
///   when @ref iterator_domain is involved.
//////////////////////////////////////////////////////////////////////
template <typename DomainContainer, typename Functor>
struct projection_construct_domain<iterator_domain<DomainContainer, Functor> >
{
  typedef iterator_domain<DomainContainer, Functor> domain_t;
  typedef typename domain_t::index_type             index_t;

  iterator_domain<DomainContainer, Functor>
  operator()(index_t const& first, index_t const& last, domain_t& vdom) const
  {
    return domain_t(first, last, vdom.container());
  }
};

} // namespace coarsen_partition_impl

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_PROJECTION_CONSTRUCT_DOMAIN_HPP
