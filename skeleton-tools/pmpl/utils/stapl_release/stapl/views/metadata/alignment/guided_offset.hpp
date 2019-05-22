/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_COARSEN_ALIGN_GUIDED_OFFSET_HPP
#define STAPL_VIEWS_METADATA_COARSEN_ALIGN_GUIDED_OFFSET_HPP

#include <stapl/runtime.hpp>
#include <numeric>
#include <functional>

#include <stapl/algorithms/functional.hpp>

#include <stapl/utility/tuple.hpp>
#include <stapl/views/metadata/coarsen_utility.hpp>
#include <stapl/views/metadata/transform.hpp>

#include <stapl/paragraph/paragraph_fwd.h>
#include <stapl/utility/vs_map.hpp>
#include <stapl/utility/do_once.hpp>

namespace stapl {

namespace metadata {

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to determine the local storage type for the
///        metadata information.
//////////////////////////////////////////////////////////////////////
template <typename Part>
struct get_copy_md_entry_type
{
  typedef std::map<size_t, typename Part::value_type> type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to compute the minimum size and offset from
///        the given pairs of size and offset.
//////////////////////////////////////////////////////////////////////
struct min_size_offset
{
  typedef std::pair<size_t,size_t> result_type;
  result_type operator()(result_type const& a, result_type const& b) const
  {
    return std::make_pair(std::min(a.first,b.first),
                          std::min(a.second,b.second));
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to determine if the given pair <size, offset>
///        @p p, has the same specified offset.
//////////////////////////////////////////////////////////////////////
struct equal_second
{
  size_t m_offset;
  equal_second(size_t offset)
    : m_offset(offset)
  { }

  bool operator()(std::pair<size_t,size_t> const& p) const
  {
    return (m_offset==p.second || p.first==index_bounds<size_t>::highest());
  }
};


struct inc_position
{
  typedef void result_type;

  void operator()(long long i, long long& j) const
  {
    j+=i;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to compute the size and offset from the
///        metadata at position @p i.
//////////////////////////////////////////////////////////////////////
struct size_offset
{
  typedef std::pair<size_t, size_t> result_type;

  template <typename View, typename Part, typename LocalMD>
  std::pair<size_t,size_t>
  operator()(View const& view, Part const& part, LocalMD& local_md,
             long long i) const
  {
    typedef typename LocalMD::mapped_type partition_t;
    typedef typename partition_t::domain_type local_domain_t;

    // get the global partition id of the entry at i
    size_t gidx = part.get_local_vid(0)+i;

    // if there aren't any local partitions, set the current
    // partition to be the last global partition
    if (part.local_size()==0 ||
        part.get_local_vid(0)==index_bounds<size_t>::invalid())
      gidx = part.size()-1+i;

    // if the partition is not in our local cache, grab it
    // from the global partition
    if (local_md.find(gidx) == local_md.end())
      local_md[gidx] = part[gidx];

    // get the domain for the partition
    local_domain_t ldom = local_md[gidx].domain();

    // domain of the global view
    typename View::domain_type gdom = view.domain();

    // return the size of the partition and how far the start
    // of that partition's domain is from the global domain
    // of the view
    size_t sz = ldom.size();

    // figure out where the next gid would be in the local domain
    size_t off = gdom.distance(gdom.first(), ldom.first());
    return std::make_pair(sz, off);
  }
};


namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to initialize the temporary local information
///   required for the alignment.
//////////////////////////////////////////////////////////////////////
struct initialize_guided_offset_alignment
{
  typedef void result_type;

  //////////////////////////////////////////////////////////////////////
  /// @param x Position of the first local metadata entry.
  /// @param p The resulting metadata alignment.
  /// @param part Initial locality metadata partition.
  /// @param lmd Temporary local storage for metadata information.
  /// @param lsz Number of local metadata entries to align.
  //////////////////////////////////////////////////////////////////////
  template <typename AlignMD, typename Part, typename LocalMD>
  void operator()(long long& x, AlignMD& p, Part const& part,
                  LocalMD& lmd, size_t& lsz) const
  {
    typedef typename AlignMD::view_container_type     part_t;
    typedef typename Part::value_type                 md_t;
    x=0;
    p = AlignMD(new part_t());

    if (part.size()!=index_bounds<size_t>::highest())
    {
      size_t tmpsz = 0;

      for (size_t i=0;i < part.local_size();++i)
      {
        size_t gpos = part.get_local_vid(0)+i;
        md_t md_entry = part[gpos];
        if (!md_entry.domain().empty()) {
          lmd[gpos] = md_entry;

          ++tmpsz;
        }
      }
      lsz = tmpsz;
    }
    else {
      lmd[1] = part[0];
      lsz = 1;
    }
  }
}; // struct initialize_guided_offset_alignment


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to align metadata entries that have the
///        same offset (are left aligned).
//////////////////////////////////////////////////////////////////////
class proc_equal_offset
{
private:
  const size_t m_mblk;

public:
  typedef void result_type;

  proc_equal_offset(size_t mblk)
    : m_mblk(mblk)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Aligns metadata that have the same offset.
  /// @param sz_off Information about size and offset of the
  ///               metadata to be aligned.
  /// @param part Partitioned metadata locality.
  /// @param i Index of the metadata that is going to be aligned.
  /// @param local_md Temporary local storage for metadata entries.
  /// @param alignpart Container for metadata locality that contains the
  ///                  resulting coarsened partitioned metadata locality.
  /// @param k Returns the increment that needs to be applied to
  ///          advance to the next metadata locality.
  //////////////////////////////////////////////////////////////////////
  template <typename SzOff, typename Part,
            typename LocalMD, typename AlignPart>
  void operator()(SzOff sz_off, Part const& part, long long i,
                  LocalMD& local_md, AlignPart& alignpart, long long& k) const
  {
    typedef typename LocalMD::mapped_type             md_entry_t;
    typedef typename md_entry_t::domain_type          md_domain_t;

    // get the i'th local partition id
    size_t gpos = part.get_local_vid(0)+i;

    // if there aren't any local partitions, use the last
    // global partition + i
    if (part.local_size() == 0
        || part.get_local_vid(0)==index_bounds<size_t>::invalid())
    {
      gpos = part.size()-1+i;
    }

    // get the metadata for that partition
    md_entry_t md_entry = local_md[gpos];
    md_domain_t ldom    = md_entry.domain();

    // if that partition's domain is empty, then
    // return that we should skip this one
    if (ldom.size() == 0)
    {
      k = 1;
      return;
    }

    // if the size of the domain is equal to the size of the
    // domain that we are trying to create, then return that
    // we should skip to the next base container
    k = (sz_off.first == m_mblk ? 1 : 0);

    // current offset in the partition (the GID that we should start
    // with for the new domain)


    // create a new domain for the realigned view
    md_domain_t ndom(ldom.first(), ldom.advance(ldom.first(), m_mblk-1), ldom);

    // create a new metadata with this entry and add it to the aligned partition
    md_entry_t md(typename md_entry_t::cid_type(), ndom, md_entry.component(),
                  md_entry.location_qualifier(), md_entry.affinity(),
                  md_entry.handle(), md_entry.location());

    alignpart.push_back_here(md);

    // if we are not skipping this partition, we need to update
    // that when we next use it, we have to skip the GIDs that we just added
    // to the new realigned view
    if (k != 1)
    {
      md_domain_t udom(ldom.advance(ldom.first(), m_mblk), ldom.last(), ldom);
      md_entry_t upd_md(typename md_entry_t::cid_type(),
                  udom, md_entry.component(),
                  md_entry.location_qualifier(), md_entry.affinity(),
                  md_entry.handle(), md_entry.location());
      local_md[gpos] = upd_md;
    }
  }
}; // class proc_equal_offset


//////////////////////////////////////////////////////////////////////
/// @brief Helper functor to check if this location has all the
///        information which is required for doing local computation
///        of coarsening.
//////////////////////////////////////////////////////////////////////
class proc_unequal_offset
{
private:
  /// The distance from the beginning of the global view's domain
  /// to the first local element of the guiding view
  const size_t m_global_guide_offset;

public:
  typedef void result_type;

  proc_unequal_offset(size_t moff)
    : m_global_guide_offset(moff)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief Helper function to verify and obtain all the metadata
  ///        information required for aligning the metadata at given
  ///        position.
  ///
  /// @param view View that is aligned.
  /// @param part Partitioned metadata locality.
  /// @param i Index of the metadata that is going to be computed.
  /// @param local_md Container for metadata locality that contains the
  ///                 resulting coarsened partitioned metadata locality.
  /// @param k Returns the increment that needs to be applied to
  ///          advance to the next metadata locality.
  //////////////////////////////////////////////////////////////////////
  template <typename View, typename Part, typename LocalMD>
  void operator()(View const& view, Part const& part, long long i,
                  LocalMD& local_md, long long& k) const
  {
    typedef typename LocalMD::mapped_type        md_entry_t;
    typedef typename md_entry_t::domain_type     md_domain_t;

    // get the id of the i'th local partition
    size_t pidx = part.get_local_vid(0)+i;

    // if there aren't any local partitions then set the
    // current partition id to the last partition + i
    if (part.local_size()==0 ||
        part.get_local_vid(0)==index_bounds<size_t>::invalid())
      pidx = part.size()-1+i;

    // get the metadata entry for the partition
    md_entry_t md_entry = local_md[pidx];

    // get the domain of the partition
    md_domain_t ldom = md_entry.domain();
    size_t sz = ldom.size();

    // get the global domain of original view
    typename View::domain_type gdom = view.domain();

    // the index in the global view that has the first element
    // of the guiding view
    typename md_domain_t::index_type guiding_first =
      gdom.advance(gdom.first(), m_global_guide_offset);


    // if the first element of the local domain is the same as the
    // first element of the guiding view's local domain then we
    // don't need to skip any local partitions
    if (guiding_first == ldom.first() || sz == index_bounds<size_t>::highest())
    {
      k = 0;

      return;
    }

    // the current running offset
    long long j = 0;

    // if the domain of the partition is empty, set the local
    // partition of the previous partition to be the last partition
    if (sz == 0) {
      --j;
      local_md[pidx+j] = part[part.size()-1];
    }

    else {
      md_entry_t tmp_md;

      // try to find the local domain that contains
      // the first local element of the guiding view
      if (!ldom.contains(guiding_first)) {
        do {
          // distance between the beginning of global domain and the
          // local domain
          const size_t first_off = gdom.distance(gdom.first(), ldom.first());

          // the guiding view's first domain is before this view's,
          // so we should look at the next partition
          if (m_global_guide_offset < first_off)
            --j;
          else
            ++j;

          // if the next or prev partition is not on this location,
          // then get it from the partition
          if (local_md.find(pidx+j) == local_md.end()) {
            local_md[pidx+j] = part[pidx+j];
          }
          tmp_md = local_md[pidx+j];
        } while (!tmp_md.domain().contains(guiding_first));
      }
    }

    // we should now have the entry for the local partition that
    // has the first element of the guiding view's local dom
    md_entry = local_md[pidx+j];

    // the distance between the found domain's first
    // and the guiding view's first
    size_t tmp_off = gdom.distance(md_entry.domain().first(), guiding_first);

    // if the found partition doesn't start with the same element as the
    // guiding view's start, add a new metadata entry that has the
    // correct subdomain
    if (tmp_off > 0)
    {
      md_domain_t udom(ldom.advance(md_entry.domain().first(),tmp_off),
                       md_entry.domain().last(),md_entry.domain());
      local_md[pidx+j] = md_entry_t(
        typename md_entry_t::cid_type(), udom, md_entry.component(),
        md_entry.location_qualifier(), md_entry.affinity(),
        md_entry.handle(), md_entry.location()
      );
    }

    k = j;
  }
}; // proc_unequal_offset

} // namespace detail


//////////////////////////////////////////////////////////////////////
/// @brief Generates a metadata alignment for the given view set, using
///        the view at position @p k to guide the alignment.
//////////////////////////////////////////////////////////////////////
template<typename ViewSet,
          typename = make_index_sequence<tuple_size<ViewSet>::value>>
struct guided_offset_alignment;


template<typename ViewSet, std::size_t ...Indices>
struct guided_offset_alignment<ViewSet, index_sequence<Indices...>>
{
private:
  using md_extractor_t  = metadata_from_container<ViewSet>;
  using partitions_type = typename md_extractor_t::type;

  typedef typename result_of::transform1<
    partitions_type, convert_to_md_vec_array
  >::type coarsen_part_type;

  using coarsen_wrap_part_type =
    tuple<
      convert_to_wrap_partition<
        typename tuple_element<Indices, partitions_type>::type
       >...
    >;

public:
  using type =
    tuple<
      partitioned_mix_view<
        typename tuple_element<Indices, ViewSet>::type,
        typename tuple_element<Indices, coarsen_wrap_part_type>::type
      >...
    >;

private:
  static constexpr size_t vs_size = tuple_size<ViewSet>::value;

  typedef typename homogeneous_tuple_type<vs_size, long long>::type position_t;
  typedef typename homogeneous_tuple_type<vs_size, size_t>::type    sizes_t;

  typedef typename result_of::transform1<
    partitions_type, get_copy_md_entry_type
  >::type  md_local_copy_t;

  typedef typename homogeneous_tuple_type<
    vs_size, std::pair<size_t,size_t>>::type                    size_offset_t;

  //////////////////////////////////////////////////////////////////////
  /// @brief Determines the largest portion of data that can be aligned
  ///   based on the current local metadata entries.
  ///
  /// @param view The original views
  /// @param parts The partitions generated by the metadata extraction
  /// @param align_parts The new partitions that we are currently generating
  /// @param pos The partition IDs for each view that we are currently trying
  ///            to align
  /// @param local_mds A cache of local md information (local partitions)
  /// @param GuideIndex The index of the guiding view
  ///
  /// @return The number of partitions for each view that need to be skipped
  ///         for the next round. Note that align_parts is also populated
  ///         with new partition information.
  //////////////////////////////////////////////////////////////////////
  template<typename GuideIndex>
  static position_t
  process_local_md(ViewSet const& views, partitions_type const& parts,
                   coarsen_part_type const& align_parts, position_t const& pos,
                   md_local_copy_t& local_mds, GuideIndex)
  {
    // compute the size of each partition and the distance from
    // the partition's first GID to the global view's first GID
    size_offset_t szs_offs(vs_map(size_offset(), views, parts, local_mds, pos));

    // the smallest distance that we have to traverse from the source
    // is the distance of the guiding view's partition
    size_t minimum_offset = get<GuideIndex::value>(szs_offs).second;

    // find amongst all of the views, which has the smallest partition
    // size and which has the smallest offset from the beginning
    std::pair<size_t,size_t> sz_off = vs_map_reduce(
      element_map(), min_size_offset(), std::make_pair(-1,-1), szs_offs
    );

    // if the guiding view's partition has the smallest distance from the
    // beginning of the global view
    bool equal_offset = vs_map_reduce(
      equal_second(minimum_offset), stapl::logical_and<bool>(), true, szs_offs
    );

    if (equal_offset)
    {
      // the block size should be the smallest partition size of all
      // of the views
      size_t mblk = sz_off.first;
      position_t ks;
      vs_map(detail::proc_equal_offset(mblk),
             szs_offs, parts, pos, local_mds, align_parts, ks);

      return ks;
    }

    // at this point, a view besides the guiding view has a partition
    // whose beginning is furthest from the global view's beginning

    position_t ks;
    vs_map(detail::proc_unequal_offset(minimum_offset),
           views, parts, pos, local_mds, ks);

    return ks;
  }

public:
  //////////////////////////////////////////////////////////////////////
  /// @brief Align the views based on the view at GuideIndex.
  ///
  /// @param views The set of original views
  /// @param part_loc The metadata of the views that was generated through
  ///                 the metadata extraction process
  /// @param GuideIndex A compile time integral constant describing which
  ///                   view in the view set is guiding the alignment process
  ///
  /// @return A set of new views that are partitioned in such a way that
  ///         they can be referenced in tandem and be aligned
  //////////////////////////////////////////////////////////////////////
  template<typename GuideIndex>
  static type apply(ViewSet const& views, partitions_type& part_loc,
                    GuideIndex)
  {
    // the index of the view that is guiding alignment
    const int guide_index = GuideIndex::value;

    // a tuple of partition ids for the current local partition
    // that is being aligned
    position_t          pos;

    // the number of local partitions in part_loc
    sizes_t             local_sz;

    // the final aligned partition that is being computed
    coarsen_part_type   alignpart;

    // a local map of metadata information.
    // essentially a cache of part_loc that contains partition
    // information for local partitions
    md_local_copy_t     local_md;

    // populate local_md with metadata information for local
    // partitions found in part_loc
    // also, set local_sz to be the number of local partitions
    // and pos to be be all zeroes
    vs_map(detail::initialize_guided_offset_alignment(),
           pos, alignpart, part_loc, local_md, local_sz);

    // we only need to do alignment if there are actually any local partitions
    // in the guiding view
    const bool need_alignment = get<guide_index>(part_loc).local_size() > 0;

    if (need_alignment)
    {
      // if the next partition id to look at is larger than the number of local
      // partitions, then we are done
      bool res = (size_t(get<guide_index>(pos)) < get<guide_index>(local_sz));

      while (res)
      {
        // for each view, we look at its partition at the index given by pos
        // and determine what is the largest portion of that partition that
        // is contained within the partition of the guiding view at index pos.
        position_t incpos = process_local_md(
          views, part_loc, alignpart, pos, local_md, GuideIndex()
        );

        // increment the partition ids so we know which partitions to
        // look at in the next iteration
        vs_map(inc_position(), incpos, pos);

        // we're done if the guiding view does not have
        // any more local partitions
        res = (size_t(get<guide_index>(pos)) < get<guide_index>(local_sz));
      }
    }

    // create the new aligned views based on the partitions created
    // in the previous steps
    return metadata::transform(views, alignpart);
  }
}; // struct guided_offset_alignment

} // namespace metadata

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_COARSEN_ALIGN_GUIDED_HPP
