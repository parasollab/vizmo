/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_DISTRIBUTOR_HPP
#define STAPL_CONTAINERS_DISTRIBUTOR_HPP

#include <boost/type_traits/remove_pointer.hpp>
#include "container_manager/local_partition_info.hpp"
#include <stapl/containers/distribution/container_manager/ordering/base_container_ordering.hpp>
#include <stapl/utility/down_cast.hpp>

namespace stapl {

namespace cm_impl {

//////////////////////////////////////////////////////////////////////
/// @brief Functor implementing assignment of a base container element
/// using the base container's set_element method.
///
/// This is the default assignment functor for the @ref distributor struct
/// that implements container redistribution.
//////////////////////////////////////////////////////////////////////
struct set_element_assign
{
  template <typename BaseContainer, typename GID, typename Value>
  void operator()(BaseContainer* bc, GID id, Value const& val)
  {
    bc->set_element(id, val);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor implementing the construction of base containers
/// using the domain of GIDs and component id provided.
///
/// This is the default base container factory for the @ref distributor struct
/// that implements container redistribution.
//////////////////////////////////////////////////////////////////////
template <typename BaseContainer>
struct construct_bc_with_dom
{
  template <typename Domain, typename CID>
  static BaseContainer* apply(Domain const& dom, CID cid)
  {
    return new BaseContainer(dom, cid);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Common data members and methods of the distributed objects that
/// implement the transfer of elements between locations during container
/// redistribution.
///
/// @tparam BaseContainers Boost ICL structure that is used to store base
/// containers.
/// @tparam PartitionInfo tuple that represents metadata of a partition
/// to be redistributed.  The elements of the tuple are the partition
/// domain, partition id, destination location, and vector of locations
/// that contribute to the partition.
/// @tparam ElemAssign Functor that implements assignment to a base container
/// element.  Required because graph base containers implement vp_set
/// instead of set_element.
//////////////////////////////////////////////////////////////////////
template <typename BaseContainer, typename PartitionInfo, typename ElemAssign>
struct distributor_base
  : public p_object
{
protected:
  /// @brief partition information that characterizes data currently
  /// stored on a location.
  std::vector<PartitionInfo> m_sender_info;

  /// @brief partition information that characterizes data that will be
  /// stored on a location after redistribution.
  std::vector<PartitionInfo> m_receiver_info;

  /// ordering of the base containers
  base_container_ordering& m_ordering;

  /// functor used to assign to base container elements
  ElemAssign m_assign;

  typedef typename tuple_element<1, PartitionInfo>::type cid_type;
  typedef PartitionInfo                                  bc_info_type;
  typedef typename tuple_element<0, PartitionInfo>::type interval_set_type;
  typedef typename interval_set_type::element_type       gid_type;
  typedef typename interval_set_type::element_type       index_type;
  typedef BaseContainer                                  base_container_type;
  typedef std::map<gid_type, typename BaseContainer::value_type>
    data_set_type;

  //////////////////////////////////////////////////////////////////////
  /// @brief Functor used to find the base container information of the
  /// cid provided.
  //////////////////////////////////////////////////////////////////////
  struct cid_eq
  {
  private:
    cid_type m_cid;
  public:
    cid_eq(cid_type cid)
      : m_cid(cid)
    {}

    template <typename Ref>
    bool operator()(Ref r) const
    {
      if (get<1>(r) != m_cid)
        return false;
      else
        return true;
    }
  };

  struct all_data_received
  {
    typedef typename std::vector<PartitionInfo>::iterator receiver_info_iter;
    receiver_info_iter m_begin;
    receiver_info_iter m_end;
    all_data_received(receiver_info_iter const& begin,
                      receiver_info_iter const& end)
      : m_begin(begin), m_end(end)
    {}
    bool operator()(void)
    {
      for (receiver_info_iter i = m_begin; i != m_end; ++i)
      {
        if (!get<0>(*i).empty())
          return false;
      }
      return true;
    }
  };

  template <typename Partition, typename Mapper>
  void remap_domain(typename Partition::value_type::index_type first,
                    typename Partition::value_type::index_type last,
                    Partition const& partition, Mapper const& mapper,
                    std::vector<bc_info_type>& partial_info)
  {
    location_type this_loc = this->get_location_id();

    typedef typename Partition::value_type::index_type      index_type;
    index_type prev  = first;

    auto first_cid = partition.find(first);
    stapl_assert(std::get<2>(first_cid) != LQ_LOOKUP,
      "distributor::remap_domain instructed to forward request.");
    location_type first_loc = mapper.map(std::get<0>(first_cid));

    // Iterate over domain until we find an element that maps to a different
    // partition.
    index_type curr = first;
    for (; curr != last; ++curr)
    {
      auto curr_cid = partition.find(curr);
      stapl_assert(std::get<2>(curr_cid) != LQ_LOOKUP,
        "distributor::remap_domain instructed to forward request.");
      if (std::get<0>(first_cid) != std::get<0>(curr_cid))
      {
        // Create the information for this subset of elements.
        stapl_assert(first <= prev, "Creating invalid domain");
        partial_info.push_back(
          bc_info_type(
            interval_set_type(
              boost::icl::interval<index_type>::closed(first, prev)),
            std::get<0>(first_cid), first_loc,
            std::vector<location_type>(1, this_loc)));
        first = curr;
        first_cid = curr_cid;
        first_loc = mapper.map(first_cid);
      }
      prev = curr;
    }
    // curr is the last element in the base container domain.
    // Determine if it needs to be in the current element or in an element of
    // its own.
    auto curr_cid = partition.find(curr);
    stapl_assert(std::get<2>(curr_cid) != LQ_LOOKUP,
      "distributor::remap_domain instructed to forward request.");
    if (std::get<0>(first_cid) == std::get<0>(curr_cid))
    {
      stapl_assert(first <= curr, "Creating invalid domain");
      partial_info.push_back(
        bc_info_type(
          interval_set_type(
            boost::icl::interval<index_type>::closed(first, curr)),
          std::get<0>(first_cid), first_loc,
          std::vector<location_type>(1, this_loc)));
    }
    else
    {
      stapl_assert(first <= prev, "Creating invalid domain");
      partial_info.push_back(
        bc_info_type(
          interval_set_type(
            boost::icl::interval<index_type>::closed(first, prev)),
          std::get<0>(first_cid), first_loc,
          std::vector<location_type>(1, this_loc)));
      first_loc = mapper.map(curr_cid);
      partial_info.push_back(
        bc_info_type(
          interval_set_type(
            boost::icl::interval<index_type>::closed(curr, curr)),
          std::get<0>(curr_cid), first_loc,
          std::vector<location_type>(1, this_loc)));
    }
  }

  template <typename Distributor, typename Partition, typename Mapper,
            typename BaseContainers, typename NewBaseContainers>
  void exchange_data(std::vector<bc_info_type>& partial_info,
                     Partition const& partition, Mapper const& mapper,
                     BaseContainers& m_bcontainers,
                     NewBaseContainers& m_new_bcontainers)
  {
    typedef typename boost::icl::interval<index_type>::type interval_type;

    // Pass the local information through a butterfly to merge the elements
    // into the set of information about what each location will receive in
    // redistribution.
    m_receiver_info = get_local_partition_info(partial_info);

    // The butterfly may not have completely merged elements.  Iterate over
    // the vector and merge elements that represent data for the same partition.
    typename std::vector<bc_info_type>::iterator curr = m_receiver_info.begin();
    typename std::vector<bc_info_type>::iterator fwd = m_receiver_info.begin();
    ++fwd;
    while (fwd != m_receiver_info.end())
    {
      if (get<1>(*curr) == get<1>(*fwd))
      {
        // Elements are for the same partition.  Merge the information.
        *curr = merge_partial_info_helper<bc_info_type>::apply(*curr, *fwd);
        get<1>(*fwd) = index_bounds<cid_type>::invalid();
        ++fwd;
      }
      else
      {
        // Elements are for different partitions.  Advance curr over elements
        // that have been merged.
        ++curr;
        for (; curr != fwd; ++curr)
        {
          stapl_assert(get<1>(*curr) == index_bounds<cid_type>::invalid(),
            "Element subset not merged as expected in distributor::operator()");
        }
        ++fwd;
      }
    }
    // Remove elements from the vector whose information was merged into other
    // elements.
    fwd = std::remove_if(m_receiver_info.begin(), m_receiver_info.end(),
                         cid_eq(index_bounds<cid_type>::invalid()));
    m_receiver_info.resize(std::distance(m_receiver_info.begin(), fwd));

    // eliminate ordering information of old base containers.
    m_ordering.clear();

    // partial_info holds info on what each location needs to send.
    // redist_info holds info on what each location will receive.
    // Iterate over partial_info, extract data, and send it.
    // Receive data, hold until base container complete, construct base cont.

    // partial_info was populated by iteration over m_bcontainers.  Maintaining
    // iterator it eliminates repeated searches for the base container
    // holding an element because the loop is iterating over the elements
    // in the order they are stored in the current base containers.
    typename BaseContainers::iterator it = m_bcontainers.begin();
    for (typename std::vector<bc_info_type>::iterator i = partial_info.begin();
                                             i!= partial_info.end();
                                           ++i)
    {
      // Construct a map of GID and data elements that will be sent to this
      // location that will store the elements' new partition.
      data_set_type new_bc_data;
      interval_type interval(*get<0>(*i).begin());
      index_type gid = boost::icl::first(interval);
      index_type gid_end = boost::icl::last(interval);
      ++gid_end;
      while (gid != gid_end)
      {
        if (it->second->domain().contains(gid))
        {
          new_bc_data.insert(std::make_pair(gid, it->second->get_element(gid)));
          ++gid;
        }
        else
        {
          // move to the next base container. Delete the current base container
          // as all information from it has been extracted.
          delete it->second;
          ++it;
        }
      }
      // send the data to its new location.
      if (get<2>(*i) != this->get_location_id())
      {
        async_rmi(get<2>(*i), this->get_rmi_handle(),
          &Distributor::receive_data, get<1>(*i), new_bc_data);
      } else {
        down_cast<Distributor*>(this)->receive_data(get<1>(*i), new_bc_data);
      }
    }
    stapl_assert(++it == m_bcontainers.end(),
      "distributor::operator() not all base containers had data sent");

    // Cleanup information used to send data.
    partial_info.clear();

    // Base container storage contains invalid pointers now. Clear it.
    m_bcontainers.clear();

    // wait until all information for new base containers has been received.
    block_until(
      all_data_received(m_receiver_info.begin(), m_receiver_info.end()));

    typedef typename NewBaseContainers::key_type new_bc_interval_type;
    typedef typename std::map<cid_type,
      std::pair<new_bc_interval_type, base_container_type*> >::iterator dbg_it;

    // Insert the new base containers and create the new ordering.
    index_type last_cid = partition.domain().last();
    typename NewBaseContainers::iterator nbc
      = m_new_bcontainers.begin();
    typename NewBaseContainers::iterator nbc_end
      = m_new_bcontainers.end();
    for (; nbc != nbc_end; ++nbc)
    {
      m_bcontainers.insert(
        std::make_pair(nbc->second.first, nbc->second.second));
      typename Mapper::cid_type next_cid = mapper.next(nbc->first);
      location_type next_loc;
      if (next_cid <= last_cid)
        next_loc = mapper.map(next_cid);
      else
        next_loc = index_bounds<location_type>::invalid();
      m_ordering.insert(nbc->second.second, nbc->first, next_loc, next_cid);
    }
    m_ordering.m_total_num_bc = partition.size();
    m_ordering.m_is_ordered   = true;
  }

public:
  distributor_base(base_container_ordering& ordering)
    : m_ordering(ordering)
  { }
};

//////////////////////////////////////////////////////////////////////
/// @brief Distributed object that implements the transfer of elements
/// between locations that is performed during container redistribution.
/// @tparam BaseContainers Boost ICL structure that is used to store base
/// containers.
/// @tparam PartitionInfo tuple that represents metadata of a partition
/// to be redistributed.  The elements of the tuple are the partition
/// domain, partition id, destination location, and vector of locations
/// that contribute to the partition.
/// @tparam ElemAssign Functor that implements assignment to a base container
/// element.  Required because graph base containers implement vp_set
/// instead of set_element.
//////////////////////////////////////////////////////////////////////
template <typename BaseContainers, typename PartitionInfo,
          typename ElemAssign = set_element_assign,
          typename BaseContainerFactory =
            construct_bc_with_dom<
              typename boost::remove_pointer<
                typename BaseContainers::value_type::second_type>::type
            >
        >
struct distributor
  : public distributor_base<typename boost::remove_pointer<
             typename BaseContainers::value_type::second_type>::type,
             PartitionInfo, ElemAssign>
{
  // PartitionInfo tuple elements are expected to be
  // interval_set_type, cid_type, location_type, std::vector<location_type>

  typedef std::vector<PartitionInfo> local_partition_information;
  typedef typename boost::remove_pointer<
    typename BaseContainers::value_type::second_type>::type base_container_type;

private:
  typedef distributor_base<base_container_type, PartitionInfo, ElemAssign>
    base_type;

  /// base containers of the container being redistributed
  BaseContainers& m_bcontainers;

  typedef BaseContainers                                 storage_type;
  typedef typename storage_type::key_type                new_bc_interval_type;
  typedef typename tuple_element<1, PartitionInfo>::type cid_type;
  typedef typename tuple_element<0, PartitionInfo>::type interval_set_type;
  typedef typename interval_set_type::element_type       gid_type;

  /// Temporary storage of new base containers
  std::map<cid_type, std::pair<new_bc_interval_type, base_container_type*> >
    m_new_bcontainers;

  // base containers are not packable, and do not provide constructors and
  // methods that allow splicing.  As a result for now we'll drop the data in
  // a map to send it to allow easy calls to bc.set_element on the destination.
  typedef std::map<gid_type, typename base_container_type::value_type>
    data_set_type;

  template <typename Domain>
  struct domain_ctor_helper
  {
    template <typename Container>
    Domain operator()(typename Domain::index_type first,
                      typename Domain::index_type last, Container*)
    { return Domain(first, last); }
  };

  template <typename Container, typename Functor>
  struct domain_ctor_helper<iterator_domain<Container, Functor> >
  {
    typedef iterator_domain<Container, Functor> domain_type;

    template <typename BC>
    domain_type operator()(typename domain_type::index_type first,
                           typename domain_type::index_type last,
                           BC* c)
    {
      return domain_type(first, last, c->container(), last-first+1);
    }
  };

public:
  distributor(BaseContainers& base_containers, base_container_ordering& order)
    : base_type(order), m_bcontainers(base_containers)
  {
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Receives a subset of elements that will be stored in a given
  /// bContainer after redistribution.
  /// @param cid Id of the bContainer that will store the elements.
  /// @param incoming_data map of (gid, value) pairs that is the set of
  /// elements to insert into the new bContainer.
  ///
  /// The method may be called multiple times for a given bContainer depending
  /// on the current distribution of elements and the desired distribution.
  //////////////////////////////////////////////////////////////////////
  void receive_data(cid_type cid, data_set_type const& incoming_data)
  {
    // find the domain of the base container in the receiver info
    typename std::vector<PartitionInfo>::iterator info =
      std::find_if(this->m_receiver_info.begin(), this->m_receiver_info.end(),
                   typename base_type::cid_eq(cid));
    stapl_assert(info != this->m_receiver_info.end(),
      "distributor::receive_data received non-local base container data.");

    typename std::map<cid_type,
               std::pair<new_bc_interval_type, base_container_type*> >::iterator
      tmp_store_iter = m_new_bcontainers.find(cid);
    if (tmp_store_iter == m_new_bcontainers.end())
    {
      // Create the base container before assigning the received values.
      m_new_bcontainers[cid] =
        std::make_pair(
          boost::icl::construct<new_bc_interval_type>(
            boost::icl::first(*get<0>(*info).begin()),
            boost::icl::last(*get<0>(*info).begin()),
            boost::icl::interval_bounds::closed()
          ),
          BaseContainerFactory::apply(
            indexed_domain<gid_type>(
              boost::icl::first(*get<0>(*info).begin()),
              boost::icl::last(*get<0>(*info).begin())),
            get<1>(*info))
        );
      tmp_store_iter = m_new_bcontainers.find(cid);
    }
    // new base container has been created.  Assign values.
    typename data_set_type::const_iterator i = incoming_data.begin();
    for (; i != incoming_data.end(); ++i)
    {
      this->m_assign((*tmp_store_iter).second.second, i->first, i->second);
      get<0>(*info) -= i->first;
    }
  }


  //////////////////////////////////////////////////////////////////////
  /// @todo the use of async_rmi and block_until calls to implement the
  /// transfer of elements to the locations that will store them is too
  /// low level.  The code should be replaced with a conditional all-to-all
  /// communication pattern that uses the information gathered from the
  /// @ref get_local_partition_info() call.
  //////////////////////////////////////////////////////////////////////
  template <typename Partition, typename Mapper>
  void operator()(Partition const& partition, Mapper const& mapper)
  {
    typedef typename Partition::value_type::index_type      index_type;
    typedef typename boost::icl::interval<index_type>::type interval_type;

    // fields are domain, partition id, destination location, source location
    typedef PartitionInfo bc_info_type;

    std::vector<bc_info_type> partial_info;

    // Iterate over the base containers, determining the new locality of
    // their elements.  The result is a vector of locality information where
    // each element contains a domain of GIDs, the partition id these elements
    // will be stored in after redistribution, the location where the new
    // partition will be stored, and a vector of the source location id.
    // The vector of source locations is built up so that for each new partition
    // we can know the set of locations that will contribute elements to it.
    typename storage_type::iterator it = m_bcontainers.begin();
    typename storage_type::iterator it_end = m_bcontainers.end();
    for ( ; it != it_end; ++it)
    {
      typename base_container_type::domain_type domain(
        domain_ctor_helper<typename base_container_type::domain_type>()
          (boost::icl::first(it->first), boost::icl::last(it->first),
           it->second));
      this->remap_domain(domain.first(), domain.last(), partition, mapper,
                         partial_info);
    }

    this->template exchange_data<distributor>(partial_info, partition, mapper,
      m_bcontainers, m_new_bcontainers);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Distributed object that implements the transfer of elements
/// between locations that is performed during container redistribution.
/// @tparam BaseContainer type of the base container used to store elements.
/// @tparam PartitionInfo tuple that represents metadata of a partition
/// to be redistributed.  The elements of the tuple are the partition
/// domain, partition id, destination location, and vector of locations
/// that contribute to the partition.
/// @tparam ElemAssign Functor that implements assignment to a base container
/// element.  Required because graph base containers implement vp_set
/// instead of set_element.
///
/// This specialization supports the @ref sparse_interval_container_registry
/// that is used by @ref map as the structure used to store base containers.
//////////////////////////////////////////////////////////////////////
template <typename RangeSet, typename BaseContainer, typename PartitionInfo,
          typename ElemAssign,
          typename BaseContainerFactory>
struct distributor<std::map<RangeSet, BaseContainer*>,
                   PartitionInfo, ElemAssign, BaseContainerFactory>
  : public distributor_base<BaseContainer, PartitionInfo, ElemAssign>
{
  // RangeSet is expected to be std::set<std::pair<GID, GID>>

  // PartitionInfo tuple elements are expected to be
  // interval_set_type, cid_type, location_type, std::vector<location_type>

  typedef std::vector<PartitionInfo> local_partition_information;
  typedef BaseContainer              base_container_type;

private:
  typedef distributor_base<BaseContainer, PartitionInfo, ElemAssign> base_type;

  /// base containers of the container being redistributed
  std::map<RangeSet, BaseContainer*>& m_bcontainers;

  typedef std::map<RangeSet, BaseContainer*>             storage_type;
  typedef typename storage_type::key_type                new_bc_interval_type;
  typedef typename tuple_element<1, PartitionInfo>::type cid_type;
  typedef typename tuple_element<0, PartitionInfo>::type interval_set_type;
  typedef typename interval_set_type::element_type       gid_type;

  /// Temporary storage of new base containers
  std::map<cid_type, std::pair<new_bc_interval_type, base_container_type*> >
    m_new_bcontainers;

  // base containers are not packable, and do not provide constructors and
  // methods that allow splicing.  As a result for now we'll drop the data in
  // a map to send it to allow easy calls to bc.set_element on the destination.
  typedef std::map<gid_type, typename base_container_type::value_type>
    data_set_type;

public:
  distributor(storage_type& base_containers, base_container_ordering& order)
    : base_type(order), m_bcontainers(base_containers)
  {
    this->advance_epoch();
  }


  //////////////////////////////////////////////////////////////////////
  /// @brief Receives a subset of elements that will be stored in a given
  /// bContainer after redistribution.
  /// @param cid Id of the bContainer that will store the elements.
  /// @param incoming_data map of (gid, value) pairs that is the set of
  /// elements to insert into the new bContainer.
  ///
  /// The method may be called multiple times for a given bContainer depending
  /// on the current distribution of elements and the desired distribution.
  //////////////////////////////////////////////////////////////////////
  void receive_data(cid_type cid, data_set_type const& incoming_data)
  {
    // find the domain of the base container in the receiver info
    typename std::vector<PartitionInfo>::iterator info =
      std::find_if(this->m_receiver_info.begin(), this->m_receiver_info.end(),
                   typename base_type::cid_eq(cid));
    stapl_assert(info != this->m_receiver_info.end(),
      "distributor::receive_data received non-local base container data.");

    typename std::map<cid_type,
               std::pair<new_bc_interval_type, base_container_type*> >::iterator
      tmp_store_iter = m_new_bcontainers.find(cid);
    if (tmp_store_iter == m_new_bcontainers.end())
    {
      // Create the base container before assigning the received values.
      new_bc_interval_type ranges;
      auto icl_container = get<0>(*info);
      for (auto interval : icl_container)
        ranges.insert(std::make_pair(boost::icl::lower(interval),
                                     boost::icl::upper(interval)));
      m_new_bcontainers[cid] =
        std::make_pair(
          ranges,
          BaseContainerFactory::apply(
            indexed_domain<gid_type>(
              boost::icl::first(*get<0>(*info).begin()),
              boost::icl::last(*get<0>(*info).begin())),
            get<1>(*info))
        );
      tmp_store_iter = m_new_bcontainers.find(cid);
    }
    else
    {
      // update ranges of container
      auto icl_container = get<0>(*info);
      for (auto interval : icl_container)
        (*tmp_store_iter).second.first.insert(
          std::make_pair(boost::icl::lower(interval),
                         boost::icl::upper(interval)));
    }

    // new base container has been created.  Assign values.
    typename data_set_type::const_iterator i = incoming_data.begin();
    for (; i != incoming_data.end(); ++i)
    {
      this->m_assign((*tmp_store_iter).second.second, i->first, i->second);
      get<0>(*info) -= i->first;
    }
  }


  //////////////////////////////////////////////////////////////////////
  /// @todo the use of async_rmi and block_until calls to implement the
  /// transfer of elements to the locations that will store them is too
  /// low level.  The code should be replaced with a conditional all-to-all
  /// communication pattern that uses the information gathered from the
  /// @ref get_local_partition_info() call.
  //////////////////////////////////////////////////////////////////////
  template <typename Partition, typename Mapper>
  void operator()(Partition const& partition, Mapper const& mapper)
  {
    typedef typename Partition::value_type::index_type      index_type;
    typedef typename boost::icl::interval<index_type>::type interval_type;

    // fields are domain, partition id, destination location, source location
    typedef PartitionInfo bc_info_type;


    std::vector<bc_info_type> partial_info;

    // Iterate over the base containers, determining the new locality of
    // their elements.  The result is a vector of locality information where
    // each element contains a domain of GIDs, the partition id these elements
    // will be stored in after redistribution, the location where the new
    // partition will be stored, and a vector of the source location id.
    // The vector of source locations is built up so that for each new partition
    // we can know the set of locations that will contribute elements to it.
    typename storage_type::iterator it = m_bcontainers.begin();
    typename storage_type::iterator it_end = m_bcontainers.end();
    for ( ; it != it_end; ++it)
    {
      for (typename RangeSet::key_type const& domain : it->first)
      {
        this->remap_domain(domain.first, domain.second, partition, mapper,
                           partial_info);
      }
    }

    this->template exchange_data<distributor>(partial_info, partition, mapper,
      m_bcontainers, m_new_bcontainers);
  }
};
}

} // namespace stapl

#endif // STAPL_CONTAINERS_DISTRIBUTOR_HPP
