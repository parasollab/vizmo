/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_CONTAINER_BY_DEMAND_HPP
#define STAPL_VIEWS_METADATA_CONTAINER_BY_DEMAND_HPP

#include <stapl/runtime.hpp>
#include <stapl/views/metadata/metadata_entry.hpp>

#include <boost/iterator/transform_iterator.hpp>

namespace stapl {

namespace metadata {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Functor to convert between different domain types.
///
/// Used in projected_container to construct the view metadata
/// from the container metadata directly.
//////////////////////////////////////////////////////////////////////
template <typename TargetDomain>
struct construct_domain
{
  TargetDomain operator()(TargetDomain const& dom)
  {
    return dom;
  }

  template <typename SourceDomain>
  TargetDomain operator()(SourceDomain const& dom)
  {
    TargetDomain ret_val;
    typename SourceDomain::index_type i      = dom.first();
    typename SourceDomain::index_type i_last = dom.last();

    for (; i != i_last; i = dom.advance(i, 1))
    {
      ret_val += i;
    }

    ret_val += dom.last();
    return ret_val;
   }
};

} // namespace detail

//////////////////////////////////////////////////////////////////////
/// @brief Functor used to compute the view's domain metadata from the
///        container's domain.
//////////////////////////////////////////////////////////////////////
template<typename View, typename MDContainer>
struct metadata_ctor_func
{
  typedef typename View::domain_type                      domain_t;
  typedef metadata_entry<
    domain_t,
    typename MDContainer::value_type::component_type,
    typename MDContainer::value_type::cid_type
  >                                                       value_type;
  typedef value_type result_type;

  template <typename T>
  value_type operator()(T const& pmd) const
  {
    // copied to remove the proxy type of T
    typename MDContainer::value_type md = pmd;

    return value_type(
      md.id(), detail::construct_domain<domain_t>()(md.domain()),
      md.component(),
      md.location_qualifier(), md.affinity(), md.handle(), md.location()
    );
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Wrapper around a metadata container to create metadata
///        information on demand, by projecting the container's domain
///        into the view's domain.
///
/// @param View Type of view from which the metadata is created.
/// @tparam MDContainer Type of the metadata container used to
///         represent the metadata information
//////////////////////////////////////////////////////////////////////
template<typename View, typename MDContainer>
struct projected_container
{
  typedef typename MDContainer::domain_type               domain_type;
  typedef typename MDContainer::dimensions_type           dimensions_type;
  typedef typename MDContainer::index_type                index_type;

  typedef typename View::domain_type                      domain_t;

  typedef metadata_entry<domain_t,
    typename MDContainer::value_type::component_type,
    typename MDContainer::value_type::cid_type>           value_type;

  typedef value_type                                      reference;

  View         m_view;
  MDContainer* m_md;
  size_t       m_version;

  typedef metadata_ctor_func<View, MDContainer> transform_t;

  void define_type(typer& t)
  {
    t.member(m_view);
    t.member(m_md);
    t.member(m_version);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Constructs a projected_container wrapper over the
  ///        @p c view using the given metadata container @p md.
  //////////////////////////////////////////////////////////////////////
  projected_container(View* view, MDContainer* md)
    : m_view(*view),
      m_md(md),
      m_version(view->version())
  {
  }

  ~projected_container()
  {
    delete m_md;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the current version number
  //////////////////////////////////////////////////////////////////////
  size_t version(void) const
  {
    return m_version;
  }

  reference operator[](index_type index)
  {
    typename MDContainer::value_type md_elem=(*m_md)[index];

    return transform_t()(md_elem);
  }

  size_t size() const
  {
   return m_md->size();
  }

  dimensions_type dimensions() const
  {
    return m_md->dimensions();
  }

  domain_type domain() const
  {
    return m_md->domain();
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc growable_container::get_local_vid(size_t)
  //////////////////////////////////////////////////////////////////////
  index_type get_local_vid(index_type index) const
  {
    return m_md->get_local_vid(index);
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc growable_container::push_back_here(MD const&)
  ///
  /// This class does not support @c push_back_here.
  /// @todo Add a stapl_assert to ensure this method is not called.
  //////////////////////////////////////////////////////////////////////
  template <typename T>
  void push_back_here(T const&)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc growable_container::local_size
  //////////////////////////////////////////////////////////////////////
  size_t local_size() const
  {
    return m_md->local_size();
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc growable_container::get_location_element(size_t)
  //////////////////////////////////////////////////////////////////////
  location_type get_location_element(index_type index) const
  {
    return m_md->get_location_element(index);
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc flat_container::update
  /// This class does not support @c push_back_here.
  /// @todo Add a stapl_assert to ensure this method is not called or
  ///       remove it.
  //////////////////////////////////////////////////////////////////////
  void update()
  { }
};

} // namespace metadata

template<typename View, typename MDContainer>
struct metadata_traits<metadata::projected_container<View, MDContainer>>
{
  typedef typename metadata_traits<MDContainer>::is_isomorphic is_isomorphic;
};

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_CONTAINER_BY_DEMAND_HPP
