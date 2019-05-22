/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_BALANCED_PARTITION_HPP
#define STAPL_CONTAINERS_BALANCED_PARTITION_HPP

#include <stapl/domains/indexed.hpp>
#include <stapl/runtime.hpp>
#include <vector>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Partition a one-dimensional domain into balanced subdomains.
///
/// If the given domain contains the GIDs [0, 1, ..., n-1] and the number
/// of partitions to generate is p, then each subdomain will have
/// size n/p and will contain contiguous chunks of GID in domain order.
/// In the case that the number of elements does not evenly divide the
/// requested number of partitions, the remaining elements are evenly
/// distributed across the subdomains, guaranteeing that no two subdomains
/// differ in size by more than one.
///
/// @tparam Dom Type of the domain to be partitioned
/// @todo Uncomment the assert in operator[] as soon as a bug in the
/// unordered_map construction is fixed.
//////////////////////////////////////////////////////////////////////
template <typename Dom>
struct balanced_partition
{
  /// The domain of the partitioner itself (i.e., the domain [0, ..., p-1])
  typedef indexed_domain<size_t>                     domain_type;
  /// Type used to describe the i'th subdomain
  typedef typename domain_type::index_type           index_type;
  /// Type of the subdomains produced by the partition
  typedef Dom                                        value_type;
  /// Type of the GIDs in the subdomains
  typedef typename value_type::index_type            gid_type;
  /// Type of the size of the subdomains
  typedef typename value_type::size_type             size_type;

protected:
  /// The global original domain
  value_type m_domain;
  /// The number of partitions
  size_t     m_npart;
public:
  //////////////////////////////////////////////////////////////////////
  /// Create a partition with a domain and a partitioning factor.
  /// @param dom The domain to partition
  /// @param p The number of partitions to generate
  //////////////////////////////////////////////////////////////////////
  balanced_partition(value_type const& dom, size_t p = get_num_locations())
    : m_domain(dom), m_npart(p)
  { }

  //////////////////////////////////////////////////////////////////////
  /// Create a partition with no domain. Leaves the partition in
  /// an inconsistent state and should be used in conjunction with @ref
  /// set_domain.
  //////////////////////////////////////////////////////////////////////
  balanced_partition()
    : m_npart(get_num_locations())
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Copy constructor using instead the given @p domain.
  //////////////////////////////////////////////////////////////////////
  balanced_partition(value_type const& domain,
                     balanced_partition const& other)
    : m_domain(domain), m_npart(other.m_npart)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Get the partition's domain [0, .., p-1].
  //////////////////////////////////////////////////////////////////////
  domain_type domain() const
  {
    return domain_type(0, m_npart - 1);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Set the global domain to partition.
  /// @param dom The domain to partition
  //////////////////////////////////////////////////////////////////////
  void set_domain(value_type const& dom)
  {
    m_domain = dom;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the subdomain at a given index. This is the inverse
  /// of @ref find.
  /// @param idx The index of the subdomain, which should be from 0 to
  /// @ref size() - 1.
  //////////////////////////////////////////////////////////////////////
  value_type operator[](index_type idx) const
  {
    stapl_assert(idx < m_npart, "partition index out of bound\n");
    if (m_domain.size() <= idx)
      return value_type();

    // remainder
    size_t rem      = m_domain.size() % m_npart;
    // block size lower
    size_t bszl     = m_domain.size() / m_npart;
    // block size greater
    size_t bszg     = bszl + (rem > 0 ? 1 : 0);

    gid_type first = (idx <= rem)
           ? m_domain.advance(m_domain.first(), idx*bszg)
           : m_domain.advance(m_domain.first(), (rem*bszg + (idx-rem)*bszl));

    gid_type last = (idx==m_npart-1)
           ? m_domain.last()
           : m_domain.advance(first, ((idx < rem) ? bszg-1 : bszl-1));

    return value_type(first, last, m_domain);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of partitions generated.
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  {
    if (m_domain.size() < m_npart)
      return m_domain.size();
    return m_npart;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the original domain that was used to create subdomains.
  //////////////////////////////////////////////////////////////////////
  value_type const& global_domain() const
  {
    return m_domain;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the index of the partition that contains the gid @c g.
  /// @param g gid to find
  /// @return The index of the subdomain that contains @c g
  //////////////////////////////////////////////////////////////////////
  index_type find(gid_type const& g) const
  {
    if (!m_domain.contains(g))
      return m_npart;

    const size_t rem  = m_domain.size() % m_npart;
    const size_t bszl = m_domain.size() / m_npart;
    const size_t bszg = bszl + (rem > 0 ? 1 : 0);

    const size_t d = m_domain.distance(m_domain.first(), g);
    const size_t i = (d <= rem*bszg) ? (d/bszg)
                               : rem + ((d - rem * bszg)/bszl);

    stapl_assert(i<m_npart, "calculated index is out of bounds");

    return i;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Determine which partition has the elements referenced
  ///        for the given domain.
  ///
  /// The returned information is a collection (possibly empty) of
  /// pairs. Each pair contains information about which partitions are
  /// included in the given domain and how they are included (True: if
  /// is fully contained, False: if is partially included). The returned
  /// collection only has elements if there is at least one partition
  /// that contains elements on the given domain.
  ///
  /// @par Example:
  ///    Partition: [0..3],[4..6],[7..9],[10..13]<br/>
  ///    Given domain: [2..9]<br/>
  ///    Returns:  {([0..0],False),([1..2],True)}<br/>
  ///
  /// @param dom Domain to compare
  /// @param mfg Mapping function generator used to get the associated
  ///            mapping function to each partition. The generated
  ///            mapping function is used to project generated
  ///            partitioned domains into the given domain.
  /// @return a vector of pairs.
  //////////////////////////////////////////////////////////////////////
  template <typename ODom, typename MFG>
  std::vector<std::pair<domain_type,bool> >
  contained_in(ODom const& dom, MFG const& mfg)
  {
    std::vector<std::pair<domain_type,bool> > doms;

    if (dom.is_contiguous()) {
      // Inverse mapping
      size_t cj = this->find(dom.first());
      size_t dj = this->find(dom.last());

      if (cj>=m_npart)
        return doms;

      typename ODom::index_type ai = mfg[cj]((*this)[cj].first());
      typename ODom::index_type bi = mfg[dj]((*this)[dj].last());

      if (cj < m_npart && ai<dom.first())
        ++cj;

      if (cj < dj) {
        if (dom.last()<bi) {
          doms.push_back(std::make_pair(domain_type(cj,dj-1),true));
          doms.push_back(std::make_pair(domain_type(dj,dj),false));
        }
        if (dom.last()==bi) {
          doms.push_back(std::make_pair(domain_type(cj,dj),true));
        }
      }

      if (cj == dj) {
        if (dom.last()<bi) {
          doms.push_back(std::make_pair(domain_type(cj,dj),false));
        }
        if (dom.last()==bi) {
          doms.push_back(std::make_pair(domain_type(cj,dj),true));
        }
      }
    }
    else {
      // std::cout << "using sparse find dom: "<< dom << std::endl;
      for (index_type j = 0; j < this->size(); ++j) {
        value_type domj = (*this)[j];
        value_type tmpdom = (domj & const_cast<ODom&>(dom));
        if (tmpdom.size() == domj.size()) {
          doms.push_back(std::make_pair(domain_type(j,j),true));
        }
        else
          if (dom.contains(domj.first())) {
            doms.push_back(std::make_pair(domain_type(j,j),false));
          }
        }
    }
    return doms;
  }

  void define_type(typer& t)
  {
    t.member(m_domain);
    t.member(m_npart);
  }
}; // struct balanced_partition

} // namespace stapl

#endif // STAPL_CONTAINERS_BALANCED_PARTITION_HPP
