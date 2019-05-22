/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_ARRAY_BASE_CONTAINER_HPP
#define STAPL_CONTAINERS_ARRAY_BASE_CONTAINER_HPP

#include <stapl/containers/base/bc_base.hpp>
#include <stapl/containers/iterators/local_iterator.hpp>
#include <stapl/containers/iterators/const_local_iterator.hpp>
#include <stapl/containers/type_traits/index_bounds.hpp>
#include <stapl/containers/distribution/base_container_metadata.hpp>
#include <stapl/containers/type_traits/define_value_type.hpp>
#include <stapl/runtime/stapl_assert.hpp>

namespace stapl {

template<typename Traits>
class array_base_container;


template<typename Traits>
struct container_traits<array_base_container<Traits>>
{
  typedef typename Traits::value_type                         value_type;
  typedef typename Traits::domain_type                        domain_type;
  typedef typename domain_type::index_type                    gid_type;
  typedef typename Traits::container_type                     container_type;
  typedef local_accessor<array_base_container<Traits>>        accessor_t;
  typedef proxy<value_type, accessor_t>                       reference;
  typedef const_local_accessor<array_base_container<Traits>>  const_accessor_t;
  typedef proxy<value_type, const_accessor_t>                 const_reference;
};


//////////////////////////////////////////////////////////////////////
/// @brief The base container used for @ref array.
/// @ingroup parrayDist
///
/// @see array
/// @tparam Traits A traits class that defines customizable components
/// of the base container, such as the domain type and storage type.
/// The default traits class is @ref array_base_container_traits.
////////////////////////////////////////////////////////////////////////
template<typename Traits>
class array_base_container
  : public bc_base
{
public:
  typedef typename Traits::value_type                    value_type;
  typedef typename Traits::domain_type                   domain_type;
  typedef size_t                                         cid_type;
  typedef typename domain_type::index_type               gid_type;
  typedef typename domain_type::size_type                size_type;

  // backend container
  typedef typename Traits::container_type                container_type;
  typedef typename Traits::container_constructor         container_constructor;

  typedef local_accessor<array_base_container>           accessor_t;
  typedef const_local_accessor<array_base_container>     const_accessor_t;

  typedef proxy<value_type, accessor_t>                  reference;
  typedef proxy<value_type, const_accessor_t>            const_reference;

  typedef local_iterator<array_base_container>           iterator;
  typedef const_local_iterator<array_base_container>     const_iterator;

  typedef base_container_metadata<array_base_container>  loc_dist_metadata;

private:
  /// @brief The domain of this base container.
  domain_type    m_domain;

  /// @brief The underlying raw data.
  container_type m_data;

  /// @brief This base container's id.
  cid_type       m_cid;

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain and ID
  /// @param domain The explicit domain for this base container
  /// @param cid The id for this base container
  //////////////////////////////////////////////////////////////////////
  array_base_container(domain_type const& domain,
                       cid_type const& cid = index_bounds<cid_type>::invalid())
    : m_domain(domain),
      m_data(m_domain.size(), typename Traits::stored_type()),
      m_cid(cid)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain, ID and initial
  /// value.
  /// @param domain The explicit domain for this base container
  /// @param cid The id for this base container
  /// @param default_value The default value for all elements in the base
  /// container
  //////////////////////////////////////////////////////////////////////
  array_base_container(domain_type const& domain,
                       cid_type const& cid,
                       value_type const& default_value)
    : m_domain(domain),
      m_data(m_domain.size(), typename Traits::stored_type(default_value)),
      m_cid(cid)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain and using
  /// a raw pointer to a block of memory that stores data.
  /// @param domain The explicit domain for this base container
  /// @param extmem A pointer to a raw chunk of memory that
  /// contains the base container's data
  //////////////////////////////////////////////////////////////////////
  array_base_container(domain_type domain, value_type* extmem)
    : m_domain(domain),
      m_data(container_constructor(extmem, domain.size())),
      m_cid()
  { }

  ~array_base_container(void)
  {
    bc_base_impl::cleanup_elements<value_type>::apply(m_data);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Return the base container ID
  //////////////////////////////////////////////////////////////////////
  cid_type const& cid(void) const
  {
    return m_cid;
  }

  domain_type* get_domain(void) const
  {
    return &m_domain;
  }

  domain_type const& domain(void) const
  {
    return m_domain;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Truncate a base container by removing a specific GID and modifying
  /// the domain to remove from that index to the end of the container.
  /// @param gid The GID to remove.
  //////////////////////////////////////////////////////////////////////
  void truncate_tail(gid_type const& gid)
  {
    this->m_domain = domain_type(m_domain.first(), m_domain.advance(gid, -1));
    m_data.erase(m_data.begin() +
      m_domain.distance(m_domain.first(), gid), m_data.end()
    );
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Truncate a base container by removing a specific GID and modifying
  /// the domain to remove from the beginning of the container to that index.
  /// @param gid The GID to remove.
  //////////////////////////////////////////////////////////////////////
  void truncate_head(gid_type const& gid)
  {
    m_data.erase(m_data.begin(), m_data.begin() +
      m_domain.distance(m_domain.first(), gid) + 1
    );
    this->m_domain = domain_type(m_domain.advance(gid, 1), m_domain.last());
  }

  container_type& container(void)
  {
    return m_data;
  }

  container_type const& container(void) const
  {
    return m_data;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return a raw pointer to underlying data
  /// @todo This function is for BLAS interoperability, but is currently
  ///       unused.
  //////////////////////////////////////////////////////////////////////
  value_type* raw(void)
  {
    return &(m_data[0]);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the local position of a global ID.
  /// @param gid The GID for which to find the local position.
  //////////////////////////////////////////////////////////////////////
  gid_type local_position(gid_type const& gid) const
  {
    // FIXME: why is this assert commented out?
    //stapl_assert(m_domain.contains(gid), "attempting to access an element
    //that is out of bounds of the base container");
    return gid-m_domain.first();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local reference to a specific index of the base
  /// container.
  /// @param gid The GID for which to create the reference
  /// @return A proxy of the value at gid
  ////////////////////////////////////////////////////////////////////////
  reference make_reference(gid_type const& gid)
  {
    return reference(accessor_t(this, m_data.begin()+local_position(gid)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local iterator to a specific index of the base
  /// container.
  /// @param gid The GID for which to create the iterator
  /// @return An iterator of the value at gid
  ////////////////////////////////////////////////////////////////////////
  iterator make_iterator(gid_type const& gid)
  {
    return iterator(m_data.begin()+local_position(gid),this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local reference to a specific index of the base
  /// container.
  /// @param gid The GID for which to create the reference
  /// @return A proxy of the value at gid
  ////////////////////////////////////////////////////////////////////////
  reference operator[](gid_type const& gid)
  {
    return make_reference(gid);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Get the element corresponding to a specific GID.
  /// @param gid The id associated with the element for which we want to read
  /// the value.
  /// @return A copy of the element.
  //////////////////////////////////////////////////////////////////////
  value_type get_element(gid_type const& gid)
  {
    return m_data[local_position(gid)];
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Get the element corresponding to a specific GID.
  /// @param gid The id associated with the element for which we want to read
  /// the value.
  /// @return A const reference to the element.
  //////////////////////////////////////////////////////////////////////
  value_type get_element(gid_type const& gid) const
  {
    return m_data[local_position(gid)];
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Invoke the functor provided on the element with the specified key.
  /// @param gid The specified key.
  /// @param f The functor to apply
  //////////////////////////////////////////////////////////////////////
  template <typename F>
  void apply_set(gid_type const& gid, F const& f)
  {
    f(m_data[local_position(gid)]);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Apply a function object to an element at a specific GID and
  ///   return the result.
  /// @param gid The GID associated with the element for which we want to apply
  ///   the functor and read the result.
  /// @param f The functor to apply to gid
  /// @return The result of applying f to the value at gid
  //////////////////////////////////////////////////////////////////////
  template <typename Functor>
  typename Functor::result_type
  apply_get(gid_type const& gid, Functor const& f)
  {
    return f(m_data[local_position(gid)]);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Update the value of an element at a specific GID.
  /// @param gid The GID of the element to set.
  /// @param t The new value
  //////////////////////////////////////////////////////////////////////
  void set_element(gid_type const& gid, typename Traits::stored_type const& t)
  {
    gid_type pos = local_position(gid);
    m_data[pos] = t;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of elements in this base container.
  //////////////////////////////////////////////////////////////////////
  size_type size(void) const
  {
    return this->m_domain.size();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local iterator to the beginning of the base
  /// container.
  /// @return A local iterator.
  ////////////////////////////////////////////////////////////////////////
  iterator begin(void)
  {
    return iterator(m_data.begin(),this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a const local iterator to the beginning of the base
  /// container.
  /// @return A local iterator to the begin.
  ////////////////////////////////////////////////////////////////////////
  const_iterator begin(void) const
  {
    return const_iterator(m_data.begin(),this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local iterator to one past the end of the base
  /// container.
  /// @return A local iterator to the end.
  ////////////////////////////////////////////////////////////////////////
  iterator end(void)
  {
    return iterator(m_data.end(),this);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local iterator to one past the end of the base
  /// container that is const.
  /// @return A const local iterator to the end.
  ////////////////////////////////////////////////////////////////////////
  const_iterator end(void) const
  {
    return const_iterator(m_data.end(),this);
  }
}; // class array_base_container



template<typename T, typename Domain>
struct array_base_container_traits;


//////////////////////////////////////////////////////////////////////
/// @brief A basic base container for @ref array. This class
/// differs from @ref array_base_container in that it cannot be modified
/// through traits. Its domain is @ref indexed_domain.
/// @ingroup parrayDist
///
/// @see array_base_container
/// @tparam T The value type of the base container.
////////////////////////////////////////////////////////////////////////
template<typename T>
class basic_array_base_container
  : public array_base_container<
      array_base_container_traits<T, indexed_domain<size_t>>>
{
private:
  typedef array_base_container_traits<T, indexed_domain<size_t> > traits_t;
  typedef array_base_container<traits_t>                          base_t;

public:
  STAPL_IMPORT_TYPE(typename base_t, domain_type)
  STAPL_IMPORT_TYPE(typename base_t, cid_type)
  STAPL_IMPORT_TYPE(typename base_t, value_type)

  //////////////////////////////////////////////////////////////////////
  /// @copydoc array_base_container::array_base_container(domain_type const&,cid_type const&)
  //////////////////////////////////////////////////////////////////////
  basic_array_base_container(domain_type const& domain,
                             cid_type const& cid =
                               index_bounds<cid_type>::invalid())
    : base_t(domain, cid)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc array_base_container::array_base_container(domain_type const&,cid_type const&,value_type const&)
  //////////////////////////////////////////////////////////////////////
  basic_array_base_container(domain_type const& domain,
                             cid_type const& cid,
                             value_type const& default_value)
    : base_t(domain, cid, default_value)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc array_base_container::array_base_container(domain_type,value_type*)
  //////////////////////////////////////////////////////////////////////
  basic_array_base_container(domain_type domain, value_type* extmem)
    : base_t(domain, extmem)
  { }
}; // class basic_array_base_container

} // namespace stapl

#endif // STAPL_CONTAINERS_ARRAY_BASE_CONTAINER_HPP
