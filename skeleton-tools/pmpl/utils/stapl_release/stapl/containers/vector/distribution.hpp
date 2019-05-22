/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_VECTOR_DISTRIBUTION_HPP
#define STAPL_CONTAINERS_VECTOR_DISTRIBUTION_HPP

#include <stapl/runtime.hpp>
#include <stapl/containers/iterators/container_accessor.hpp>
#include <stapl/containers/iterators/container_iterator.hpp>

#include <stapl/domains/indexed.hpp>

#include <stapl/containers/distribution/static_metadata.hpp>
#include <stapl/containers/distribution/distribution.hpp>
#include <stapl/containers/distribution/container_manager/ordering/base_container_ranking.hpp>
#include <stapl/containers/distribution/operations/base.hpp>

#include <stapl/views/proxy.h>

#include <stapl/algorithms/numeric.hpp>
#include <stapl/containers/array/static_array.hpp>
#include <stapl/views/vector_view.hpp>
#include <stapl/views/array_view.hpp>

namespace stapl {

template<typename Container>
class vector_distribution;


//////////////////////////////////////////////////////////////////////
/// @brief Specialization for @ref distribution_traits over
///        @ref vector_distribution.
//////////////////////////////////////////////////////////////////////
template<typename C>
struct distribution_traits<vector_distribution<C> >
{
  typedef C                                              container_type;

  typedef typename container_traits<C>::directory_type   directory_type;

  typedef typename container_traits<
    C>::container_manager_type                           container_manager_type;

  typedef typename container_manager_type::
               base_container_type                       base_container_type;

  typedef typename container_traits<C>::gid_type         gid_type;
  typedef gid_type                                       index_type;
  typedef typename container_traits<C>::value_type       value_type;
  typedef container_accessor<C>                          accessor_type;
  typedef proxy<value_type, accessor_type>               reference;
};


//////////////////////////////////////////////////////////////////////
/// @brief Defines the vector distribution.
///
/// Provides the functionality required to manage the data
/// distributions of the @ref vector.
/// @tparam Container Type of the container that is managing this
///                   distribution.
//////////////////////////////////////////////////////////////////////
template<typename Container>
class vector_distribution
  : public distribution<Container>,
    public operations::base<vector_distribution<Container> >,
    public operations::iterable<vector_distribution<Container> >,
    public operations::random_access<vector_distribution<Container> >
{
private:
  typedef distribution<Container>                                base_type;
  typedef operations::iterable<vector_distribution<Container> >  it_base_t;

public:
  STAPL_IMPORT_TYPE(typename base_type, directory_type)
  STAPL_IMPORT_TYPE(typename base_type, container_manager_type)
  STAPL_IMPORT_TYPE(typename base_type, partition_type)
  STAPL_IMPORT_TYPE(typename base_type, mapper_type)

  STAPL_IMPORT_TYPE(typename container_manager_type, base_container_type)
  STAPL_IMPORT_TYPE(typename container_manager_type, gid_type)
  STAPL_IMPORT_TYPE(typename container_manager_type, value_type)

  STAPL_IMPORT_TYPE(typename base_container_type, cid_type)

  typedef gid_type                                       index_type;
  typedef Container                                      container_type;

  /// Distribution metadata type used for coarsening
  /// @todo Change to associative_metadata or move it into set or map
  ///       distribution.
  typedef metadata::static_container_extractor<
    vector_distribution
  >                                                      loc_dist_metadata;

  typedef indexed_domain<gid_type>                       domain_type;

  STAPL_IMPORT_TYPE(typename it_base_t, iterator);

protected:
  enum action_type {INSERT, DELETE};

  typedef std::queue<std::pair<gid_type,value_type> >    insert_queue_type;
  typedef std::queue<gid_type>                           erase_queue_type;

  insert_queue_type        m_insert_queue;
  erase_queue_type         m_erase_queue;
  domain_type              m_domain;

private:
  void register_local_keys()
  {
    typedef typename container_manager_type::iterator bc_iterator;
    typedef typename container_manager_type::base_container_type
      ::domain_type::gid_type gid_type;
    bc_iterator bc_it = this->m_container_manager.begin();
    bc_iterator bc_end = this->m_container_manager.end();
    for (; bc_it != bc_end; ++bc_it)
    {
      std::pair<gid_type, gid_type> range =
        std::make_pair((*bc_it).domain().first(), (*bc_it).domain().last());
      this->directory().register_keys(range);
    }
  }
public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Copy construction of this distribution.
  ///
  /// @param other Another distribution to copy from.
  //////////////////////////////////////////////////////////////////////
  vector_distribution(vector_distribution const& other)
    : base_type(other),
      m_domain(other.m_domain)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a distribution with default constructed elements.
  ///
  /// @param partition Partition used by the container.
  /// @param mapper Mapper used by the container.
  //////////////////////////////////////////////////////////////////////
  vector_distribution(partition_type const& partition,
                      mapper_type const& mapper)
    : base_type(partition, mapper),
      m_domain(partition.global_domain())
  {
    register_local_keys();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a distribution with an initial value for elements.
  ///
  /// @param partition Partition used by the container.
  /// @param mapper Mapper used by the container.
  /// @param default_value The value that the elements in this
  ///                      distribution will be initialized with.
  //////////////////////////////////////////////////////////////////////
  vector_distribution(partition_type const& partition,
                      mapper_type const& mapper,
                      value_type const& default_value)
    : base_type(partition, mapper, default_value),
      m_domain(partition.global_domain())
  {
    register_local_keys();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Creates a distribution with with the given @p directory
  ///        and base container manager @p bcmangr.
  ///
  /// @param directory Directory used by the container.
  /// @param bcmangr Base container manager used by the container.
  //////////////////////////////////////////////////////////////////////
  vector_distribution(directory_type const& directory,
                      container_manager_type const& bcmangr)
    : base_type(directory, bcmangr)
  {
    register_local_keys();
  }

  void set_domain(domain_type const& domain)
  {
    m_domain = domain;
  }

protected:
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to update the domain by increasing or
  ///        reducing the number of elements in the domain based on
  ///        the given @p action.
  //////////////////////////////////////////////////////////////////////
  void update_metadata(action_type action, location_type origin)
  {
    if (origin != this->get_location_id())
    {
      // Inserting a new element
      if (action==INSERT) {
        if (m_domain.empty())
          this->m_domain = domain_type(0,0);
        else
          this->m_domain = domain_type(m_domain.first(),m_domain.last()+1);
      }
      else {
        this->m_domain = domain_type(m_domain.first(),m_domain.last()-1);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to insert the given @p value at the given
  ///        @p gid position, updating the domain.
  //////////////////////////////////////////////////////////////////////
  void insert_bcast(gid_type gid, value_type const& value, cid_type dest,
                    promise<void> p)
  {
    // Update container_manager domains to base containers and insert the value
    // into the predetermined base container
    this->m_container_manager.insert(gid, value, dest);

    // Update registry
    this->directory().insert(gid, m_domain.last(), std::move(p));

    // Update global directory
    if (m_domain.empty())
      this->m_domain = domain_type(0, 0);
    else
      this->m_domain = domain_type(m_domain.first(), m_domain.last() + 1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to determine into which base container the
  //         given @p gid and @p value pair should be inserted
  //////////////////////////////////////////////////////////////////////
  void try_insert(gid_type gid, value_type const& value, promise<void> p)
  {
    cid_type insert_cid;

    // We arrived at this location by following (gid - 1), so check for the
    // location of gid to see where we should insert
    if (gid == 0)
      insert_cid = 0;
    else if (this->m_container_manager.contains(gid))
      insert_cid = this->m_container_manager.within(gid);
    else // Element should go into the next numbered base container
      insert_cid = this->m_container_manager.within(gid - 1) + 1;

    async_rmi(all_locations, this->get_rmi_handle(),
              &vector_distribution::insert_bcast,
              gid, value, insert_cid, std::move(p));
  }


public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Inserts the given @p value at the given @p gid position.
  //////////////////////////////////////////////////////////////////////
  void insert(gid_type gid, value_type const& value,
              rmi_handle::reference const& view_handle)
  {
    m_insert_queue.push(std::make_pair(gid, value));

    if (m_insert_queue.size() > 1)
      return;

    while (!m_insert_queue.empty())
    {
      const std::pair<gid_type, value_type> val = m_insert_queue.front();

      typedef promise<void> promise_type;

      promise_type p;
      auto f = p.get_future();

      this->directory().invoke_where(
          std::bind(
            [](p_object& d, gid_type gid, value_type const& value,
               promise_type& p)
            {
              down_cast<vector_distribution>(d).try_insert(
                gid, value, std::move(p));
            },
            std::placeholders::_1, val.first, val.second, std::move(p)),
          gid == 0 ? gid : gid - 1);

      f.get();

      m_insert_queue.pop();
    }

    if (view_handle.valid())
      async_rmi(all_locations, view_handle,
          &vector_view<container_type>::update_domain);
  }

protected:
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to remove the element at the given @p gid
  ///        position, updating the domain.
  //////////////////////////////////////////////////////////////////////
  void erase_bcast(gid_type gid, promise<void> p)
  {
    // Update registry
    this->directory().erase(gid, m_domain.last(), std::move(p));

    // Update container_manager domains to base containers and insert the value
    this->m_container_manager.erase(gid);

    // Update global directory
    this->m_domain = domain_type(m_domain.first(), m_domain.last() - 1);
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Removes the element at the given @p gid position.
  //////////////////////////////////////////////////////////////////////
  void erase(gid_type gid, rmi_handle::reference const& view_handle)
  {
    stapl_assert(!m_domain.empty(), "list is empty");

    m_erase_queue.push(gid);

    if (m_erase_queue.size() > 1)
      return;

    while (!m_erase_queue.empty())
    {
      const gid_type tmp_gid = m_erase_queue.front();

      typedef promise<void> promise_type;

      promise_type p;
      auto f = p.get_future();

      async_rmi(all_locations, this->get_rmi_handle(),
                &vector_distribution::erase_bcast, tmp_gid, std::move(p));

      f.get();

      m_erase_queue.pop();
    }

    if (view_handle.valid())
      async_rmi(all_locations, view_handle,
          &vector_view<container_type>::update_domain);
  }

  //////////////////////////////////////////////////////////////////////
  /// @copydoc stapl::vector::clear(void)
  //////////////////////////////////////////////////////////////////////
  void clear(void)
  {
    this->m_container_manager.clear();
    this->directory().reset();
    this->m_domain = domain_type();
  }

protected:
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to insert the given @p value locally.
  //////////////////////////////////////////////////////////////////////
  void push_back_local(value_type const& value, location_type origin)
  {
    size_t new_index = this->m_container_manager.push_back(value);
    this->directory().register_key(new_index);

    async_rmi(all_locations, this->get_rmi_handle(),
              &vector_distribution::update_metadata, INSERT, origin);
    rmi_flush();
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Inserts the given @p value an the end of the container.
  //////////////////////////////////////////////////////////////////////
  void push_back(value_type const& value)
  {
    if (size() == 0)
    {
      if (0 == get_location_id())
      {
        push_back_local(value, 0);
      }
      else
      {
        async_rmi(0, this->get_rmi_handle(),
         &vector_distribution::push_back_local, value, get_location_id());
      }
    }
    else
      this->directory().invoke_where(
        std::bind(
          [](p_object& d, value_type const& val, location_type src)
            { down_cast<vector_distribution&>(d).push_back_local(val, src); },
          std::placeholders::_1, value, this->get_location_id()),
        this->m_domain.last());

    // Update local metadata immediately.  An invalid location id is used
    // because the location need only be different from the current location id.
    update_metadata(INSERT, invalid_location_id);
  }

protected:
  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to insert the given @p value locally.
  ///
  /// An invalid origin is provided by default to allow the method to be
  /// called directly instead of via pop_back to remove a local element
  /// and still update the metadata correctly.
  //////////////////////////////////////////////////////////////////////
  void pop_back_local(location_type origin)
  {
    gid_type gid = this->container_manager().pop_back();
    this->directory().unregister_key(gid);
    async_rmi(all_locations, this->get_rmi_handle(),
              &vector_distribution::update_metadata, DELETE, origin);
    rmi_flush();
  }

public:
  void pop_back()
  {
    gid_type gid = this->m_domain.last();

    if (this->container_manager().contains(gid))
    {
      pop_back_local(this->get_location_id());
    }
    else
    {
      this->directory().invoke_where(
        std::bind(
          [](p_object& d, location_type src)
            { down_cast<vector_distribution&>(d).pop_back_local(src); },
          std::placeholders::_1, this->get_location_id()),
        gid);
    }

    // Update local metadata immediately.  An invalid location id is used
    // because the location need only be different from the current location id.
    update_metadata(DELETE, invalid_location_id);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to insert the given @p value locally.
  //////////////////////////////////////////////////////////////////////
  void local_push_back(value_type const& value)
  {
    this->m_container_manager.push_back(value);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Updates the stored domain to represent a domain with
  ///        @p num_elem number of elements.
  //////////////////////////////////////////////////////////////////////
  void update_domain(size_t num_elem)
  {
    if (m_domain.empty())
      this->m_domain = domain_type(0, num_elem-1);
    else
      this->m_domain = domain_type(m_domain.first(),m_domain.last()+num_elem);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Method used to update the container's metadata
  ///       (directory's registry, domain) based on the information in
  ///       the base containers.
  /// @bug Generalize this code to handle arbitrary number of base
  ///   containers with arbitrary domains.  Right now only works for
  ///   for one base container per location, with gid strictly increasing
  ///   per location.  Related todo in
  ///   @ref vector_container_manager::update_domains.  See gforge #1240.
  //////////////////////////////////////////////////////////////////////
  void synchronize_metadata(void)
  {
    STAPL_IMPORT_TYPE(typename container_manager_type, iterator)

    iterator it           = this->m_container_manager.begin();
    const iterator end_it = this->m_container_manager.end();

    const size_t num_local_elements = it == end_it ? 0 : it->domain().size();

    size_t nelems = num_local_elements;

    size_t offset = 0;

    if (this->get_num_locations() > 1)
    {
      typedef static_array<size_t> array_t;

      array_t psum(this->get_num_locations());
      psum[this->get_location_id()] = num_local_elements;
      array_view<array_t> vpsum(psum);

      typedef plus<size_t> wf_t;

      nelems = accumulate(vpsum, (size_t) 0, wf_t());
      stapl::partial_sum(vpsum, vpsum, wf_t(), true);


      offset = vpsum.get_element(this->get_location_id());
    }

    this->m_container_manager.update_domains(offset);
    this->m_domain = domain_type(nelems);

    this->directory().reset();

    // advance the epoch to ensure no key registrations are received on a
    // location before the directory on the location is reset.
    this->advance_epoch();

    for (it = this->m_container_manager.begin(); it != end_it; ++it)
    {
      this->directory().register_keys(
        std::make_pair(it->domain().first(), it->domain().last())
      );
    }

    base_container_ranking(this->container_manager().m_ordering);
  }

  domain_type const& domain(void) const
  {
    return m_domain;
  }

  iterator find(gid_type index)
  {
    return make_iterator(index);
  }

  size_t local_size()
  {
    return domain_type(this).local_size();
  }

  size_t size(void) const
  {
    return domain().size();
  }
}; // class vector_distribution

} // namespace stapl

#endif // STAPL_CONTAINERS_VECTOR_DISTRIBUTION_HPP
