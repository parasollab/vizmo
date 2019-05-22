/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_CONTAINER_ITERATOR_HPP
#define STAPL_CONTAINERS_CONTAINER_ITERATOR_HPP

#include "container_iterator_fwd.hpp"
#include "const_container_iterator_fwd.hpp"
#include <stapl/views/iterator/iterator_facade.h>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Iterator that is used to traverse a container.
///
/// @tparam Container Type of the container
/// @tparam Accessor Accessor type used for references (@ref proxy)
/// @tparam Category Iterator category of this iterator. By default, it's
/// a forward iterator.
//////////////////////////////////////////////////////////////////////
template<typename Container,
         typename Accessor,
         typename Category>
class container_iterator
  : public iterator_facade<
      container_iterator<Container, Accessor, Category>, Accessor, Category
    >
{
  friend class stapl::iterator_core_access;

  typedef Accessor                                                  accessor_t;
  typedef iterator_facade<container_iterator, accessor_t, Category> base_type;
  typedef typename base_type::difference_type                       diff_t;

  typedef typename Container::domain_type                           domain_type;

public:
  typedef typename container_traits<Container>::gid_type            index_type;

private:
  p_object_pointer_wrapper<Container> m_container;
  index_type                          m_index;
  domain_type                         m_domain;

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Create an invalid iterator.
  //////////////////////////////////////////////////////////////////////
  container_iterator(void)
    : m_index(), m_domain()
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a global iterator over a given container starting at
  /// a specified GID.
  /// @param container Pointer to the container
  /// @param index Index into the container
  //////////////////////////////////////////////////////////////////////
  container_iterator(Container* container, index_type index)
    : m_container(container),
      m_index(index), m_domain(container->domain())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a global iterator over a given container starting at
  /// a specified GID with a specific domain.
  /// @param container Pointer to the container
  /// @param domain The domain that this iterator will traverse
  /// @param index Index into the container
  //////////////////////////////////////////////////////////////////////
  container_iterator(Container* container,
                     domain_type const& domain,
                     index_type index)
    : m_container(container),
      m_index(index), m_domain(domain)
  { }

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief Create an accessor to be used in a reference for the value that
  /// the iterator is currently pointing to.
  //////////////////////////////////////////////////////////////////////
  accessor_t access(void) const
  {
    stapl_assert(m_container, "invalid container pointer");

    return accessor_t(m_container, m_index);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Increment the global iterator by one position by advancing its
  /// domain.
  //////////////////////////////////////////////////////////////////////
  void increment(void)
  {
    m_index = m_domain.advance(m_index, 1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Decrement the global iterator by one position by advancing its
  /// domain in the reverse direction.
  //////////////////////////////////////////////////////////////////////
  void decrement(void)
  {
    m_index = m_domain.advance(m_index, -1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Advance the global iterator forward or backward by a specific
  /// amount by advancing its domain.
  /// @param n The amount by which to advance the iterator
  //////////////////////////////////////////////////////////////////////
  void advance(diff_t n)
  {
    m_index = m_domain.advance(m_index, n);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Compare equality of two iterators by comparing their indices.
  /// @param rhs The other iterator to compare against
  /// @return Whether or not these two iterators point to the same value
  //////////////////////////////////////////////////////////////////////
  bool equal(container_iterator const& rhs) const
  {
    return m_index == rhs.m_index;
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc equal
  //////////////////////////////////////////////////////////////////////
  template <typename OtherContainer>
  bool equal(const container_iterator<OtherContainer, Category>& rhs) const
  {
    return m_index == rhs.m_index;
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Compute the distance between two iterators by evaluating
  /// the distance of their indices.
  /// @param rhs The other iterator to compare against
  /// @return How far apart the iterators are
  //////////////////////////////////////////////////////////////////////
  diff_t distance_to(container_iterator const& rhs) const
  {
    return m_domain.distance(m_index,rhs.m_index);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Determine whether this iterator precedes another in a proper
  /// traversal of the domain.
  /// @param rhs The other iterator to compare against
  /// @return Whether or not this iterator precedes the other
  //////////////////////////////////////////////////////////////////////
  bool less_than(container_iterator const& rhs) const
  {
    return m_domain.less_than(m_index,rhs.m_index);
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Serialization for this global iterator
  //////////////////////////////////////////////////////////////////////
  void define_type(typer& t)
  {
    t.member(m_container);
    t.member(m_index);
    t.member(m_domain);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the index (GID) of the iterator
  //////////////////////////////////////////////////////////////////////
  index_type index(void) const
  {
    return m_index;
  }

  template<typename conv_Accessor>
  operator
  const_container_iterator<Container, conv_Accessor, Category>(void) const
  {
    return const_container_iterator<Container, conv_Accessor, Category>
      (m_container, m_domain, m_index);
  }
}; // class container_iterator


template <typename C,typename A, typename Cat>
typename container_traits<C>::gid_type
gid_of(const container_iterator<C,A,Cat>& it)
{
  return it.index();
}

} // namespace stapl

#endif // STAPL_CONTAINERS_CONTAINER_ITERATOR_HPP
