/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_LIST_LIST_CONTAINER_MANAGER_HPP
#define STAPL_CONTAINERS_LIST_LIST_CONTAINER_MANAGER_HPP

#include <boost/unordered_set.hpp>
#include <boost/icl/interval_set.hpp>

#include <stapl/containers/distribution/container_manager/local_partition_info.hpp>
#include <stapl/containers/distribution/container_manager/ordering/base_container_ordering.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Defines the container manager used by the @ref list.
/// @ingroup plistDist
///
/// @tparam BContainer Base container type.
//////////////////////////////////////////////////////////////////////
template<typename BContainer>
struct list_container_manager
{
  typedef BContainer                                      base_container_type;
  typedef typename BContainer::cid_type                   cid_type;
  typedef typename BContainer::gid_type                   gid_type;
  typedef typename BContainer::value_type                 value_type;

protected:
  typedef boost::unordered_set<BContainer*>               storage_type;

public:
  // base container ordering
  typedef base_container_ordering                         ordering_type;
  typedef typename ordering_type::ptr_bcontainer_type     ptr_bcontainer_type;

  typedef typename storage_type::iterator                 iterator;
  typedef typename storage_type::const_iterator           const_iterator;

protected:
  storage_type   m_bcontainers;

public:
  ordering_type  m_ordering;

  //////////////////////////////////////////////////////////////////////
  /// @brief Initializes the list base containers based on the
  ///        information provided for the @p partition and @p mapper and
  ///        the @p factory to create base container instances.
  //////////////////////////////////////////////////////////////////////
  template<typename Partition, typename Mapper, typename Factory>
  void init(Partition const& partition, Mapper const& mapper,
            Factory const& factory)
  {
    // get local base container ids
    typedef typename Partition::value_type::index_type      index_type;
    typedef boost::icl::interval_set<index_type>            interval_set_type;
    typedef typename boost::icl::interval<index_type>::type interval_type;

    typedef tuple<
      boost::icl::interval_set<typename Partition::value_type::index_type>,
      typename Mapper::cid_type, location_type
    >  bc_info_type;

    std::vector<bc_info_type> bc_info =
      cm_impl::get_local_partition_info(
        cm_impl::get_partial_partition_info(partition, mapper));

    if (!bc_info.empty())
    {
      index_type last_cid = partition.domain().last();

      // add local base containers
      for (typename std::vector<bc_info_type>::iterator bc_it = bc_info.begin();
                     bc_it != bc_info.end(); ++bc_it) {
        typename Mapper::cid_type it = get<1>(*bc_it);
        stapl_assert(get<0>(*bc_it).iterative_size() == 1,
          "base containers cannot be created for disjoint domains");

        interval_type interval(*get<0>(*bc_it).begin());
        typename Partition::value_type
          domain(boost::icl::first(interval), boost::icl::last(interval));
        size_t n_elems = domain.size();

        if (!domain.empty())
        {
          base_container_type* const bc = factory(n_elems);
          m_bcontainers.insert(bc);

          size_t next_rank =  mapper.next(it);
          location_type next_loc;
          if (next_rank <= last_cid)
          {
            next_loc = mapper.map(next_rank);
            m_ordering.insert(bc, it, next_loc, next_rank);
          }
          else
          {
            next_loc = index_bounds<location_type>::invalid();
            next_rank = index_bounds<size_t>::invalid();
            m_ordering.insert(bc, it, next_loc, next_rank);
          }
        }
      }
    }
    m_ordering.m_total_num_bc = partition.size();
    m_ordering.m_is_ordered   = true;

    // This fence is necessary to ensure the RMIs generated by m_ordering.insert
    // calls are received. Without this the container_manager for the list can
    // enter a state where no previous base container information is available
    // and the location will mistakenly return the first index of its first
    // base container. This causes view alignment to crash as size_offset in
    // guided_offset.hpp will incorrectly report that a partition is at the
    // beginning of the container domain, causing alignment to attempt to send
    // a message to a partition id that doesn't exist.
    //
    // The fence should be replaced with an advance_epoch and a block_until that
    // ensures m_first_loc is set on a location.  All container methods need to
    // become non-blocking to avoid the potential for deadlock when they're
    // declared on overlapping sets of locations.
    stapl_assert(m_ordering.get_rmi_handle().get_gang_id() == 0,
      "rmi_fence() invoked in construction of nested list container.");
    rmi_fence();
  }

public:
  list_container_manager(void) = default;

  template<typename Partition, typename Mapper>
  list_container_manager(Partition const& partition, Mapper const& mapper)
  {
    const location_type lid = m_ordering.get_location_id();

    init(partition, mapper,
         [lid](size_t num_elems) {
           return new base_container_type(num_elems, lid);
         }
    );
  }

  template<typename Partition, typename Mapper>
  list_container_manager(Partition const& partition, Mapper const& mapper,
                         value_type const& default_value)
  {
    const location_type lid     = m_ordering.get_location_id();
    value_type const* value_ptr = &default_value;

    init(partition, mapper,
         [lid, value_ptr](size_t num_elems) {
           return new base_container_type(num_elems, *value_ptr, lid);
         }
    );
  }

  list_container_manager(list_container_manager const& other)
    : m_ordering(other.m_ordering)
  {
    m_bcontainers.clear();
    for (typename storage_type::const_iterator it = other.m_bcontainers.begin();
         it != other.m_bcontainers.end(); ++it) {
      base_container_type* const bc = new base_container_type(*(*it));
      m_bcontainers.insert(bc);
      m_ordering.replace((*it),bc);
    }
  }

  list_container_manager& operator=(list_container_manager const& other)
  {
    m_ordering = other.m_ordering;

    for (typename storage_type::iterator it = m_bcontainers.begin();
         it != m_bcontainers.end(); ++it)
      delete *it;

    m_bcontainers.clear();


    for (typename storage_type::iterator it = other.m_bcontainers.begin();
         it != other.m_bcontainers.end(); ++it) {
      base_container_type* const bc = new base_container_type(*(*it));
      m_bcontainers.insert(bc);
      m_ordering.replace((*it),bc);
    }
    return *this;
  }

  ~list_container_manager(void)
  {
    for (typename storage_type::iterator it = m_bcontainers.begin();
         it != m_bcontainers.end(); ++it)
      delete (*it);
    m_bcontainers.clear();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Clears the storage and destroys the metadata for all
  ///        local elements.
  //////////////////////////////////////////////////////////////////////
  void clear(void)
  {
    for (typename storage_type::iterator it = m_bcontainers.begin();
         it != m_bcontainers.end(); ++it)
      (*it)->clear();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the rank associated to the given base container
  ///        @p bc.
  //////////////////////////////////////////////////////////////////////
  size_t rank(base_container_type* const bc) //const
  {
    return m_ordering.get_rank(bc);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns whether or not this base container manager is
  ///        responsible for the element referenced with the given @p gid.
  //////////////////////////////////////////////////////////////////////
  bool contains(gid_type const& gid) const
  {
    if (gid.m_location == m_ordering.get_location_id()
        && m_bcontainers.find(gid.m_base_container) != m_bcontainers.end())
      return gid.m_base_container->domain().contains(gid);

    return false;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the id of the base container that stores the
  ///        element with the given @p gid.
  //////////////////////////////////////////////////////////////////////
  cid_type within(gid_type const& gid) //const
  {
    stapl_assert(this->contains(gid),
      "rank called on gid not stord in base container manager");

    return rank(*(m_bcontainers.find(gid.m_base_container)));
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the total number of elements stored in the base
  ///        containers.
  //////////////////////////////////////////////////////////////////////
  size_t size(void) const
  {
    size_t sz = 0;

    for (auto&& bc_ptr : m_bcontainers)
      sz += bc_ptr->size();

    return sz;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the first base container managed
  ///        by this container manager.
  //////////////////////////////////////////////////////////////////////
  iterator begin(void)
  {
    return m_bcontainers.begin();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns an iterator to the end of the storage of base
  ///        containers.
  //////////////////////////////////////////////////////////////////////
  iterator end(void)
  {
    return m_bcontainers.end();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the first gid of the first base container.
  ///
  /// This method is used for list distributed domain.
  //////////////////////////////////////////////////////////////////////
  gid_type first(void) const
  {
    base_container_type* const bc =
      down_cast<base_container_type*>(this->m_ordering.first());

    if (bc != nullptr)
    {
      if (!bc->domain().empty())
        return bc->domain().first();
    }
    return index_bounds<gid_type>::invalid(); //return invalid gid
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the last valid gid of the last base container.
  ///
  /// This method is used for list distributed domain.
  //////////////////////////////////////////////////////////////////////
  gid_type last(void) const
  {
    base_container_type* const bc =
      down_cast<base_container_type*>(this->m_ordering.last());

    if (bc!=nullptr)
    {
      if (!bc->domain().empty())
        return bc->domain().last();
    }
    return index_bounds<gid_type>::invalid(); //return invalid gid
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to find the first gid of the list.
  ///
  /// This method uses the base container ordering methods to traverse
  /// the base containers using the @ref find_fw visitor functor.
  /// @param promise Promise used to return the gid.
  /// @todo Uncomment the const qualifier.
  //////////////////////////////////////////////////////////////////////
  template<typename Prom>
  void find_first(Prom const& promise) const
  {
    ordering_detail::find_first<Prom, ordering_type, base_container_type>
      findfirst(promise, &(this->m_ordering));

    typedef void (ordering_type::*fn)
      (ordering_detail::find_first<Prom, ordering_type, base_container_type>&)
      const;

    location_type first_loc =
      m_ordering.m_first_loc != index_bounds<location_type>::invalid() ?
      m_ordering.m_first_loc : m_ordering.get_location_id();

    if (m_ordering.get_location_id()!=first_loc) {
      async_rmi(first_loc,m_ordering.get_rmi_handle(),
                (fn) &ordering_type::find_first, findfirst);
    }
    else {
      this->m_ordering.find_first(findfirst);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to find the last gid of the list.
  ///
  /// This method uses the base container ordering methods to traverse
  /// the base containers using the @ref find_last and @ref find_bw
  /// visitor functors.
  /// @param promise Promise used to return the gid.
  /// @todo Uncomment the const qualifier.
  //////////////////////////////////////////////////////////////////////
  template<typename Prom>
  void find_last(Prom const& promise) const
  {
    ordering_detail::find_last<Prom, ordering_type, base_container_type>
    findlast(promise, &(this->m_ordering));

    typedef void (ordering_type::*fn)
      (ordering_detail::find_last<Prom, ordering_type, base_container_type>&)
      const;

    location_type last_loc =
      m_ordering.m_last_loc != index_bounds<location_type>::invalid() ?
      m_ordering.m_last_loc : m_ordering.get_location_id();

    if (m_ordering.get_location_id()!=last_loc) {
      async_rmi(last_loc,m_ordering.get_rmi_handle(),
                (fn) &ordering_type::find_last, findlast);
    }
    else {
      this->m_ordering.find_last(findlast);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to advance the given @p gid, @p n
  ///        positions. Result gid written to @ref promise @p.
  ///
  /// This method uses the base container ordering methods to traverse
  /// the base containers and the visitor functors @ref advance_fw and
  /// @ref advance_bw.
  //////////////////////////////////////////////////////////////////////
  void defer_advance(gid_type const& gid, long long n, bool globally,
                     promise<gid_type> p) const
  {
    if (n == 0)
    {
      p.set_value(gid);
      return;
    }

    base_container_type* const bc = gid.m_base_container;

    if (globally)
    {
      if (n>0)
      {
        ordering_detail::advance_fw<gid_type, base_container_type>
          advfw(gid, n, p);
        this->m_ordering.traverse_forward(advfw, bc);
      }
      else
      {
        ordering_detail::advance_bw<gid_type, base_container_type>
          advbw(gid, n, p);
        this->m_ordering.traverse_backward(advbw, bc);
      }

      return;
    }

    stapl_assert(contains(gid), "gid not found");

    p.set_value(bc->domain().advance(gid, n));
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to compute the distance between the two
  ///   given gids (@p a and @p b). Result distance written to @ref promise @p.
  ///
  /// This method uses the base container ordering methods to traverse
  /// the base containers and the visitor functor @ref distance_fw.
  //////////////////////////////////////////////////////////////////////
  void defer_distance(gid_type const& gid_a,
                      gid_type const& gid_b,
                      promise<size_t> p) const
  {
    stapl_assert(contains(gid_a), "gid not found");

    base_container_type* const bca = gid_a.m_base_container;
    base_container_type* const bcb = gid_b.m_base_container;

    if (gid_a.m_location == gid_b.m_location && bca == bcb)
    {
      p.set_value(bca->domain().distance(gid_a, gid_b) + 1);
      return;
    }

    typedef ordering_detail::distance_fw<
      gid_type, base_container_type
    > distance_fw_t;

    this->m_ordering.traverse_forward(distance_fw_t(gid_a, gid_b, p), bca);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper method to search for the given @p gid between gids
  ///   @p gid_a to @p gid_b, Write true to @ref promise @p p if found,
  ///   otherwise false.
  ///
  /// This method uses the base container ordering methods to traverse
  /// the base containers and the visitor functor @ref search_fw.
  //////////////////////////////////////////////////////////////////////
  void defer_search(gid_type const& gid_a, gid_type const& gid_b,
                   gid_type const& gid, promise<bool> p) const
  {
    typedef ordering_detail::search_fw<
      gid_type, base_container_type
    > search_fw_t;

    this->m_ordering.traverse_forward(
      search_fw_t(gid_a, gid_b, gid, p), gid_a.m_base_container);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Inserts the given @p value before the element referenced
  /// by the given @p gid.
  //////////////////////////////////////////////////////////////////////
  void insert(gid_type const& gid, value_type const& value)
  {
    if (gid.m_base_container != nullptr)
    {
      gid.m_base_container->insert(gid,value);
      return;
    }

    // else
    if (m_bcontainers.size() == 0) {
      base_container_type* const bc =
        new base_container_type(m_ordering.get_location_id());
      bc->insert(gid,value);
      m_bcontainers.insert(bc);
      m_ordering.insert_after(nullptr, bc);
    }
    else
    {
      // Insert the element at the beginning of the first base container.
      base_container_type* const bc = this->m_ordering.first();

      stapl_assert(bc != nullptr,
                   "No base container in list_container_manager.");

      bc->insert(gid, value);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Adds the given @p value to the last base container.
  ///
  /// If @p add_front is @c true, the given @p value is inserted at
  /// the beginning of the last base container, otherwise is inserted
  /// at the end.
  //////////////////////////////////////////////////////////////////////
  void add(value_type const& value, bool add_front = false)
  {
    base_container_type* const bc =
      down_cast<base_container_type*>(this->m_ordering.last());

    if (bc == nullptr)
    {
      base_container_type* const new_bc =
        new base_container_type(m_ordering.get_location_id());

      new_bc->push_back(value);

      m_bcontainers.insert(new_bc);
      m_ordering.insert_after(nullptr, new_bc);
    }
    else
    {
      if (add_front)
        bc->push_front(value);
      else
        bc->push_back(value);
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Removes the elements referenced for the given @p gid.
  //////////////////////////////////////////////////////////////////////
  void remove(gid_type const& gid)
  {
    base_container_type* const bc = gid.m_base_container;
    bc->remove(gid);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Method used only for debugging and testing purposes.
  //////////////////////////////////////////////////////////////////////
  void print(void) const
  {
    typename storage_type::const_iterator it = this->m_bcontainers.begin();
    for (;it!=this->m_bcontainers.end();++it) {
      std::cout << m_ordering.get_location_id()
                << ") ## [bc] print: " << it->first;
      (it->second)->print();
    }
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the given functor @p f to the element referenced
  ///        for the given @p gid.
  //////////////////////////////////////////////////////////////////////
  template<typename Functor>
  void apply(gid_type const& gid, Functor const& f)
  {
    stapl_assert(contains(gid),
                 "invoking a function on an unknown base container");

    f(*gid.m_pointer);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the given functor @p f to the element referenced
  ///        for the given @p gid and returns the functor evaluation
  ///        result.
  //////////////////////////////////////////////////////////////////////
  template<typename Functor>
  typename Functor::result_type apply_get(gid_type const& gid, Functor const& f)
  {
    stapl_assert(contains(gid),
                 "invoking a function on an unknown base container");

    return f(*gid.m_pointer);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the given functor @p f to the element referenced
  ///        for the given @p gid and returns the functor evaluation
  ///        result.
  //////////////////////////////////////////////////////////////////////
  template<typename Functor>
  typename Functor::result_type
  apply_get(gid_type const& gid, Functor const& f) const
  {
    stapl_assert(contains(gid),
                 "invoking a function on an unknown base container");

    return f(*gid.m_pointer);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the given functor @p f to the element referenced
  ///        by the given @p gid if it is stored locally and returns
  ///        the functor evaluation result.
  //////////////////////////////////////////////////////////////////////
  template<typename Functor>
  boost::optional<typename Functor::result_type>
  contains_apply_get(gid_type const& gid, Functor const& f)
  {
    if (gid.m_location == m_ordering.get_location_id())
      return boost::optional<typename Functor::result_type>(f(*gid.m_pointer));

    return boost::optional<typename Functor::result_type>();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Applies the given functor @p f to the element referenced
  ///        by the given @p gid if it is stored locally and returns
  ///        the functor evaluation result.
  //////////////////////////////////////////////////////////////////////
  template<typename Functor>
  boost::optional<typename Functor::result_type>
  contains_apply_get(gid_type const& gid, Functor const& f) const
  {
    if (gid.m_location == m_ordering.get_location_id())
      return boost::optional<typename Functor::result_type>(f(*gid.m_pointer));

    return boost::optional<typename Functor::result_type>();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Invokes a base container method on the given @p gid. The
  ///        element must exist in the current base container manager.
  ///
  /// @param gid The GID of the element to invoke the method on.
  /// @param pmf A pointer to a base container's member method.
  /// @param u   Arguments to pass to the member function.
  //////////////////////////////////////////////////////////////////////
  template<typename C, typename... T, typename... U>
  void invoke(gid_type const& gid, void (C::* const& pmf)(T...), U&&... u)
  {
    stapl_assert(contains(gid), "failed to find gid in container manager");
    stapl_assert(gid.m_base_container != nullptr, "gid has no base container");

    (gid.m_base_container->*pmf)(std::forward<U>(u)...);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Invokes a base container method on the given @p gid if it is
  ///        present on this location.
  /// @param gid The GID of the element to invoke the method on.
  /// @param pmf   A pointer to a base container's member method.
  /// @return True if gid was found and functor applied, otherwise returns false.
  /// @param u     Arguments to pass to the member function.
  //////////////////////////////////////////////////////////////////////
  template<typename C, typename... T, typename... U>
  bool
  contains_invoke(gid_type const& gid, void (C::* const& pmf)(T...), U&&... u)
  {
    if (!contains(gid))
      return false;

    stapl_assert(gid.m_base_container != nullptr, "gid has no base container");

    (gid.m_base_container->*pmf)(std::forward<U>(u)...);

    return true;
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Invokes a base container method on the given @p gid and
  ///        returns the result. The element must exist in the current
  ///        base container manager.
  ///
  /// @param gid The GID of the element to invoke the method on.
  /// @param pmf A pointer to a base container's member method.
  /// @param u   Arguments to pass to the member function.
  //////////////////////////////////////////////////////////////////////
  template<typename C, typename Rtn, typename... T, typename... U>
  Rtn invoke(gid_type const& gid, Rtn (C::* const& pmf)(T...), U&&... u)
  {
    stapl_assert(contains(gid), "failed to find gid in container manager");
    stapl_assert(gid.m_base_container != nullptr, "gid has no base container");

    return (gid.m_base_container->*pmf)(std::forward<U>(u)...);
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Invokes a base container method on the given @p gid if it is
  ///        present on this location.
  /// @param gid The GID of the element to invoke the method on.
  /// @param pmf A pointer to a base container's member method.
  /// @param u   Arguments to pass to the member function.
  /// @return    boost::optional with result of invocation if element was found.
  //////////////////////////////////////////////////////////////////////
  template<typename C, typename Rtn, typename... T, typename... U>
  boost::optional<Rtn>
  contains_invoke(gid_type const& gid, Rtn (C::* const& pmf)(T...), U&&... u)
  {
    if (!contains(gid))
      return boost::optional<Rtn>();

    stapl_assert(gid.m_base_container != nullptr, "gid has no base container");

    return boost::optional<Rtn>(
      (gid.m_base_container->*pmf)(std::forward<U>(u)...));
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Invokes a const base container method on the given @p gid
  ///        and returns the result. The element must exist in the
  ///        current base container manager.
  ///
  /// @param gid The GID of the element to invoke the method on.
  /// @param pmf A pointer to a base container's member method.
  /// @param u   Arguments to pass to the member function.
  //////////////////////////////////////////////////////////////////////
  template<typename C, typename Rtn, typename... T, typename... U>
  Rtn
  const_invoke(gid_type const& gid, Rtn (C::* const& pmf)(T...), U&&... u) const
  {
    stapl_assert(contains(gid), "failed to find gid in container manager");
    stapl_assert(gid.m_base_container != nullptr, "gid has no base container");

    return (gid.m_base_container->*pmf)(std::forward<U>(u)...);
  }
}; // struct list_container_manager

} // namespace stapl

#endif // STAPL_CONTAINERS_LIST_LIST_CONTAINER_MANAGER_HPP
