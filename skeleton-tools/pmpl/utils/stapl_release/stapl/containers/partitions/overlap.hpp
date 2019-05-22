/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_CONTAINERS_PARTITIONS_OVERLAP_HPP
#define STAPL_CONTAINERS_PARTITIONS_OVERLAP_HPP

#include <boost/iterator/transform_iterator.hpp>
#include <stapl/domains/intersect.hpp>
#include <stapl/domains/indexed.hpp>

namespace stapl {

//////////////////////////////////////////////////////////////////////
/// @brief Partition overlap for one dimensional domains.
///
/// A domain is partitioned based on the specified number of elements
///   overlapped.
/// @tparam Dom Domain type.
//////////////////////////////////////////////////////////////////////
template <typename Dom>
struct overlap_partition
{
  typedef indexed_domain<size_t>               domain_type;
  typedef Dom                                  value_type;
  typedef typename value_type::index_type      gid_type;
  typedef typename domain_type::index_type     index_type;

private:
  Dom    m_domain;
  size_t m_left;
  size_t m_right;
  size_t m_core;

public:

  //////////////////////////////////////////////////////////////////////
  /// @brief Overlap partition constructor.
  ///
  /// The overlapped domains are defined specifying the number of
  /// elements that are overlapped to the left (@p l), the number of
  /// elements that are not overlapped (@c c) and the number of
  /// elements overlapped to the right (@c r). Each subdomain has
  /// size: l+c+r.
  /// @par Example:
  ///     Domain to partition: [0..8]<br/>
  ///     left overlap (l): 2<br/>
  ///     non overlap (c): 3<br/>
  ///     right overlap (r): 1<br/>
  ///     Resulting partition: {[0..5],[3..8]}<br/>
  /// @param domain Domain to partition.
  /// @param c Number of elements not overlapped.
  /// @param l Number of elements overlapped to the left.
  /// @param r Number of elements overlapped to the right.
  //////////////////////////////////////////////////////////////////////
  overlap_partition(value_type const& domain,
                    size_t c=1, size_t l=0, size_t r=0)
    : m_domain(domain), m_left(l), m_right(r), m_core(c)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Copy constructor that replaces the domain with the domain provided.
  //////////////////////////////////////////////////////////////////////
  overlap_partition(value_type const& domain,
               overlap_partition const& other)
    : m_domain(domain),
      m_left(other.m_left), m_right(other.m_right), m_core(other.m_core)
  { }

  Dom const& global_domain() const
  { return m_domain; }

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the @p idx-th domain in the partition.
  //////////////////////////////////////////////////////////////////////
  value_type operator[](size_t idx) const
  {
    gid_type lower = m_domain.advance(m_domain.first(), idx*m_core);
    gid_type upper = m_domain.advance(lower, m_left + m_core + m_right - 1);
    return value_type(lower,upper,m_domain);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the number of partitions generated.
  //////////////////////////////////////////////////////////////////////
  size_t size() const
  {
    return (m_domain.size()-m_core-m_left-m_right)/m_core+1;
  }

private:

  //////////////////////////////////////////////////////////////////////
  /// @brief Return the index of the subdomain that contains the GID @p g.
  ///
  /// If the flag @p first is true, the returned index of the first
  /// subdomain that contains the given GID @p g, otherwise returns
  /// the index of the last subdomain that contains @p g.
  //////////////////////////////////////////////////////////////////////
  index_type in_domain(gid_type const& g,bool first=true)
  {
    size_t dist = m_domain.distance(m_domain.first(),g);
    if (first) {
      size_t pos = size_t(dist / m_core);
      if (pos>=this->size())
        pos = this->size()-1;
      return pos;
    }
    else {
      size_t sz = (m_left+m_core+m_right-1);
      size_t diff = size_t(sz/m_core);
      size_t pos = size_t(dist / m_core) - diff;
      if (pos>=this->size())
        pos = this->size()-1;
      return pos;
    }
  }

public:

  //////////////////////////////////////////////////////////////////////
  /// @brief Returns the index of the partition that contains the GID
  ///        @p g.
  //////////////////////////////////////////////////////////////////////
  index_type find(gid_type const& g) const
  {
    stapl_assert(false,
                 "This partition should not be used to specify "
                 "the partition of a container");
    return index_type();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Determine which partition has the elements referenced
  ///        for the given domain.
  ///
  /// The returned information is a collection (possibly empty) of
  /// pairs. Each pair contains information about which partitions are
  /// included in the given domain and how they are included (True: if
  /// it is fully contained, False: if it is partially included). The
  /// returned collection contains elements for the partitions that
  /// contain elements on the given domain.
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
      index_type cj = this->in_domain(dom.first(),true);
      index_type dj = this->in_domain(dom.last(),false);

      if (cj>dj)
            return doms;

      if ( !dom.contains(mfg[cj]((*this)[cj].first())) ) {
        cj++;
      }

      if ( (cj<dj) && (!dom.contains(mfg[dj]((*this)[dj].last()))) ) {
        // std::cout << "case 1 : " << domain_type(cj,dj-1) << std::endl;
        doms.push_back(std::make_pair(domain_type(cj,dj-1),true));

        size_t k = this->in_domain(dom.last(),true);
        if ((dj<size_t(this->size())) && (dj<=k)) {
          // std::cout << "case 2 : " << domain_type(dj,k) << std::endl;
          doms.push_back(std::make_pair(domain_type(dj,k),false));
        }

      }
      else  {
        // std::cout << "case 3 : " << domain_type(cj,dj) << std::endl;
        domain_type ndom(cj,dj);
        if (!ndom.empty())
          doms.push_back(std::make_pair(ndom,true));

        size_t k = this->in_domain(dom.last(),true);
        if ((dj<size_t(this->size()-1)) && (dj+1<=k)) {
          // std::cout << "case 4 : " << domain_type(dj+1,k) << std::endl;
          doms.push_back(std::make_pair(domain_type(dj+1,k),false));
        }
      }
    }
    else {
      // std::cout << "using sparse find dom: "<< dom << std::endl;
      for (index_type j = 0; j < this->size(); ++j) {
        value_type domj = (*this)[j];
        value_type tmpdom = intersect(dom, domj);
        if (tmpdom.size() == domj.size()) {
          // std::cout << "  full dom: "<< domj <<std::endl;
          doms.push_back(std::make_pair(domain_type(j,j),true));
        }
        else
          if (dom.contains(domj.first())) {
            // std::cout << "  partial dom: "<< domj <<std::endl;
            doms.push_back(std::make_pair(domain_type(j,j),false));
          }
        }
    }
    return doms;
  }
}; // struct overlap_partition

} // namespace stapl

#endif // STAPL_VIEWS_PARTITIONS_OVERLAP_HPP
