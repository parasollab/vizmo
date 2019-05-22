/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_MATRIX_BASE_CONTAINER_HPP
#define STAPL_CONTAINERS_MATRIX_BASE_CONTAINER_HPP

#include <stapl/views/proxy.h>
#include <stapl/containers/iterators/local_iterator.hpp>
#include <stapl/containers/iterators/local_accessor.hpp>

namespace stapl {

template<class T, class Domain, class CID, class Traits>
class matrix_base_container;


template<class T, class Domain, class CID, class Traits>
struct local_accessor_traits<matrix_base_container<T, Domain, CID, Traits>>
{
  typedef decltype(
    std::declval<typename Traits::container_type>().begin()->begin()
  ) iterator;
};


template<class T, class Domain, class CID, class Traits>
struct container_traits<matrix_base_container<T, Domain, CID, Traits> >
{
  typedef T                                              value_type;
  typedef Domain                                         domain_type;
  typedef typename domain_type::gid_type                 gid_type;
  typedef typename Traits::container_type                container_type;
  typedef local_accessor<
    matrix_base_container<T, Domain, CID, Traits>
  >                                                      accessor_t;
  typedef proxy<value_type, accessor_t>                  reference;
};


//////////////////////////////////////////////////////////////////////
/// @brief The base container used for @ref matrix.
/// @ingroup pmatrixDist
///
/// @see matrix
/// @tparam T The type of the elements stored in the base container
/// @tparam Domain Domain of this base container.
/// @tparam CID Base container id type
/// @tparam Traits A traits class that defines customizable components
/// of the base container, such as the domain type and storage type.
/// The default traits class is @ref matrix_base_container_traits.
/// @todo Implement const versions of begin() and end()
////////////////////////////////////////////////////////////////////////
template<class T, class Domain, class CID, class Traits>
class matrix_base_container
  : public bc_base
{
public:
  typedef T                                          value_type;
  typedef Domain                                     domain_type;
  typedef CID                                        cid_type;
  typedef typename domain_type::gid_type             gid_type;
  typedef typename domain_type::size_type            size_type;

  typedef typename Traits::dimension_type            dimension_type;
  typedef typename Traits::traversal_type            traversal_type;

  typedef gid_type                                   dimensions_type;

  // backend container
  typedef typename Traits::container_type            container_type;
  typedef typename Traits::container_constructor     container_constructor;

  /// Used to allow BLAS & PBLAS to be used in pAlgorithms
  typedef typename Traits::contiguous_storage        contiguous_storage;

  typedef nd_linearize<gid_type, traversal_type>     linearization_type;

  typedef local_accessor<matrix_base_container>      accessor_type;
  typedef proxy<value_type,accessor_type>            reference;
  typedef const reference                            const_reference;
  typedef local_iterator<matrix_base_container>      iterator;


  /// The domain of this base container
  domain_type            m_domain;
  /// This base container's id
  cid_type               m_cid;
 /// The first index of the base container
  gid_type               m_start_index;
  /// The underlying raw data
  container_type         m_data;

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain and ID, and
  ///  initialize m_data, as this base container has internally
  ///  managed storage.
  /// @param domain The explicit domain for this base container
  /// @param cid The id for this base container
  //////////////////////////////////////////////////////////////////////
  template<typename D>
  matrix_base_container(D const& domain, cid_type const& cid,
                        typename std::enable_if<
                          std::is_same<D, domain_type>::value
                          && !Traits::external_storage::value
                        >::type* = 0)
    : m_domain(domain), m_cid(cid),
      m_data(get<0>(domain.dimensions()), get<1>(domain.dimensions()))
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain and ID,
  ///  don't initialize m_data, as subsequent call to @ref set_storage
  ///  will initialize it.
  /// @param domain The explicit domain for this base container
  /// @param cid The id for this base container
  //////////////////////////////////////////////////////////////////////
  template<typename D>
  matrix_base_container(D const& domain, cid_type const& cid,
                        typename std::enable_if<
                          std::is_same<D, domain_type>::value
                          && Traits::external_storage::value
                        >::type* = 0)
    : m_domain(domain), m_cid(cid)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Create a base container with a specific domain and using
  /// a raw pointer to a block of memory that stores data.
  /// @param domain The explicit domain for this base container
  /// @param extmem A pointer to a raw chunk of memory that
  /// contains the base container's data
  //////////////////////////////////////////////////////////////////////
  matrix_base_container(domain_type const& domain, value_type* extmem)
    : m_domain(domain)
  {
    container_constructor tmp =
     container_constructor(extmem,
       get<0>(domain.dimensions()), get<1>(domain.dimensions())
     );
    m_data = tmp;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Set the storage
  //////////////////////////////////////////////////////////////////////
  template<typename ...Args>
  void set_storage(Args&&... args)
  {
    m_data = container_constructor(std::forward<Args>(args)...);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the base container ID
  //////////////////////////////////////////////////////////////////////
  cid_type cid() const
  {
    return m_cid;
  }

  domain_type* get_domain() const
  {
    return &m_domain;
  }

  domain_type domain() const
  {
    return m_domain;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns whether or not a GID is contained within this base
  /// container
  //////////////////////////////////////////////////////////////////////
  bool contains(gid_type const& gid) const
  {
    return this->domain().contains(gid);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return a raw pointer to underlying data.
  /// @todo This function is for BLAS interoperability, but is currently
  ///       unused.
  //////////////////////////////////////////////////////////////////////
  value_type* raw()
  {
    return m_data.data();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return a tuple representing the size of the base container
  /// in both dimensions.
  //////////////////////////////////////////////////////////////////////
  size_type dimensions() const
  {
    return this->m_domain.dimensions();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the local position of a global ID.
  /// @param gid The GID for which to find the local position.
  /// @return The local ID
  //////////////////////////////////////////////////////////////////////
  gid_type local_position(gid_type const& gid) const
  {
    gid_type first = m_domain.first();
    return gid_type(get<0>(gid)-get<0>(first), get<1>(gid)-get<1>(first));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local reference to a specific index of the base
  /// container.
  /// @param gid The GID for which to create the reference
  /// @return A proxy of the value at gid
  ////////////////////////////////////////////////////////////////////////
  reference make_reference(gid_type const& gid)
  {
    stapl_assert(!Traits::external_storage::value,
      "matrix_base_container::make_reference, can't call on external");

    const gid_type local = local_position(gid);

    return reference(accessor_type(
      this, (*(m_data.begin() + get<0>(local))).begin() + get<1>(local)
    ));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local reference to a specific value of the base
  /// container at (i, j).
  /// @param i Row index
  /// @param j Column index
  /// @return A proxy of the value at (i, j)
  ////////////////////////////////////////////////////////////////////////
  reference operator()(size_t i, size_t j)
  {
    return make_reference(gid_type(i,j));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Construct a local reference to a specific index of the base
  /// container.
  /// @param g The GID for which to create the reference
  /// @return A proxy of the value at g
  ////////////////////////////////////////////////////////////////////////
  reference operator[](gid_type const& g)
  {
    return make_reference(g);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Get the element corresponding to a specific GID.
  /// @param gid The id associated with the element for which we want to read
  /// the value.
  /// @return A copy of the element.
  //////////////////////////////////////////////////////////////////////
  value_type get_element(gid_type const& gid)
  {
    gid_type pos = local_position(gid);
    return m_data( int(get<0>(pos)), int(get<1>(pos)) );
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc get_element(gid_type const&)
  //////////////////////////////////////////////////////////////////////
  value_type get_element(gid_type const& gid) const
  {
    gid_type pos = local_position(gid);
    return m_data( int(get<0>(pos)), int(get<1>(pos)) );
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Update the value of an element at a specific row and column.
  /// @param i Row index
  /// @param j column index
  /// @param t The new value
  //////////////////////////////////////////////////////////////////////
  void set_element(size_t i, size_t j, value_type const& t)
  {
    set_element(gid_type(i,j), t);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Update the value of an element at a specific GID.
  /// @param gid The GID of the element to set.
  /// @param t The new value
  //////////////////////////////////////////////////////////////////////
  void set_element(gid_type const& gid, value_type const& t)
  {
    gid_type pos = local_position(gid);
    m_data( int(get<0>(pos)), int(get<1>(pos)) ) = t;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of elements in this base container.
  ///
  /// Note that this is the linear size, and not the size in each dimension.
  /// @see dimensions.
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  {
    return this->m_domain.size();
  }

  auto begin(void) const
    ->decltype(m_data.begin())
  {
    return m_data.begin();
  }

  auto end(void) const
    ->decltype(m_data.end())
  {
    return m_data.end();
  }
}; // class matrix_base_container

} // namespace stapl

#endif // STAPL_CONTAINERS_MATRIX_BASE_CONTAINER_HPP
