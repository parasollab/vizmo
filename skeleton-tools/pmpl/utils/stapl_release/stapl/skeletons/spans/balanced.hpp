/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_SPANS_BALANCED_HPP
#define STAPL_SKELETONS_SPANS_BALANCED_HPP

#include <stapl/containers/partitions/block_partition.hpp>
#include <stapl/containers/type_traits/default_traversal.hpp>
#include <stapl/views/mapping_functions/linearization.hpp>
#include <stapl/containers/partitions/balanced.hpp>
#include <stapl/utility/tuple/tuple.hpp>

namespace stapl {
namespace skeletons {
namespace spans {

//////////////////////////////////////////////////////////////////////
/// @brief A balanced span which is used the most across the
/// skeletons library. It assigns a balanced <b>spawn</b> load to each
/// location. The load on each location is roughly \f$\frac{n}{p}\f$.
///
/// @ingroup skeletonsSpans
//////////////////////////////////////////////////////////////////////
template <int i>
class blocked
{
public:
  using traversal_type = typename default_traversal<i>::type;
  using partition_type = multiarray_impl::block_partition<
                           typename default_traversal<i>::type>;
  using domain_type    = typename partition_type::domain_type;
  using index_type     = typename domain_type::index_type;
  using size_type      = typename domain_type::size_type;
  using dimension_type = std::integral_constant<int, i>;

protected:
  partition_type m_partition;
  std::size_t    m_cur_pid;

public:
  blocked()
    : m_partition(domain_type()),
      m_cur_pid(0)
  { }

  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    //what we currently do is that we look at the first view and base our
    //division of work off of a balanced partition over that
    auto dom = stapl::get<
                 skeletons::first_finite_domain_index<VDomains>::value
               >(view_domains);

    m_partition = partition_type(
                    domain_type(dom.dimensions()),
                    multiarray_impl::make_multiarray_size<i>()(
                      spawner.get_num_PEs()));
    m_cur_pid = spawner.get_PE_id();
  }

  domain_type local_domain()
  {
    auto idx = m_partition.domain().first();
    idx = m_partition.domain().advance(idx, m_cur_pid);
    return m_partition.domain().contains(idx) ?
             m_partition[idx] : domain_type();
  }

  template <typename Coord>
  bool should_spawn (Coord const& skeleton_size, Coord const& coord) const
  {
    return true;
  }

  size_type dimensions() const
  {
    return m_partition.global_domain().dimensions();
  }

  std::size_t size() const
  {
    return m_partition.global_domain().size();
  }

  std::size_t linearize(index_type const& coord) const
  {
    return nd_linearize<index_type, traversal_type>(dimensions())(coord);
  }

  void define_type(typer& t)
  {
    t.member(m_partition);
    t.member(m_cur_pid);
  }
};

template <>
class blocked<1>
{
public:
  using domain_type    = indexed_domain<std::size_t, 1>;
  using index_type     = typename domain_type::index_type;
  using size_type      = typename domain_type::size_type;
  using dimension_type = std::integral_constant<int, 1>;

protected:
  balanced_partition<domain_type> m_partition;
  std::size_t                     m_cur_pid;

public:
  blocked()
    : m_partition(domain_type()),
      m_cur_pid(0)
  { }

  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    //what we currently do is that we look at the first view and base our
    //division of work off of a balanced partition over that
    auto dom = stapl::get<
                 skeletons::first_finite_domain_index<VDomains>::value
               >(view_domains);

    m_partition = balanced_partition<domain_type>(
                    domain_type(dom.dimensions()), spawner.get_num_PEs());
    m_cur_pid = spawner.get_PE_id();
  }

  domain_type local_domain()
  {
    auto idx = m_partition.domain().first();
    idx = m_partition.domain().advance(idx, m_cur_pid);
    return m_partition[idx];
  }


  template <typename Coord>
  bool should_spawn (Coord const& skeleton_size, Coord const& coord) const
  {
    return true;
  }

  size_type dimensions() const
  {
    return m_partition.global_domain().dimensions();
  }

  std::size_t size() const
  {
    return m_partition.global_domain().size();
  }

  std::size_t linearize(index_type const& coord) const
  {
    return coord;
  }

  void define_type(typer& t)
  {
    t.member(m_partition);
    t.member(m_cur_pid);
  }
};

template <int i>
class summa
  : public blocked<i>
{
public:
  using metadata_type = stapl::tuple<size_t, size_t, size_t>;

private:
  using base_t = blocked<i>;
  metadata_type m_mx_sizes;

public:
  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    // get m, k and p sizes of input views
    m_mx_sizes = make_tuple(
                  stapl::get<0>((stapl::get<0>(view_domains)).dimensions()),
                  stapl::get<1>((stapl::get<0>(view_domains)).dimensions()),
                  stapl::get<1>((stapl::get<1>(view_domains)).dimensions())
                );

    // get the size of resulting matrix C to determine number of iterations
    auto dom = stapl::get<2>(view_domains);


    this->m_partition = typename base_t::partition_type(
                          typename base_t::domain_type(dom.dimensions()),
                          multiarray_impl::make_multiarray_size<i>()(
                            spawner.get_num_PEs())
                        );

    this->m_cur_pid = spawner.get_PE_id();
  }

  metadata_type dimensions_metadata() const
  {
    return m_mx_sizes;
  }

  void define_type(typer& t)
  {
    t.base<base_t>(*this);
    t.member(m_mx_sizes);
  }
};


template <int i = 1>
class balanced;


//////////////////////////////////////////////////////////////////////
/// @brief A balanced span which is used the most across the
/// skeletons library. It assigns a balanced <b>spawn</b> load to each
/// location. The load on each location is roughly \f$\frac{n}{p}\f$.
///
/// @ingroup skeletonsSpans
//////////////////////////////////////////////////////////////////////
template <int i>
class balanced
{
public:
  using domain_type    = indexed_domain<std::size_t, i>;
  using traversal_type = typename domain_type::traversal_type;
  using index_type     = typename domain_type::index_type;
  using size_type      = typename domain_type::size_type;
  using dimension_type = std::integral_constant<int, i>;

protected:
  balanced_partition<domain_type> m_partition;
  std::size_t                     m_cur_pid;
public:
  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    //what we currently do is that we look at the first view and base our
    //division of work off of a balanced partition over that
    auto dom = stapl::get<
                 skeletons::first_finite_domain_index<VDomains>::value
               >(view_domains);

    m_partition = balanced_partition<domain_type>(
                    domain_type(dom.dimensions()), spawner.get_num_PEs());
    m_cur_pid = spawner.get_PE_id();
  }

  domain_type local_domain()
  {
    auto idx = m_partition.domain().first();
    idx = m_partition.domain().advance(idx, m_cur_pid);
    return m_partition[idx];
  }

  template <typename Coord>
  bool should_spawn (Coord const&, Coord const&) const
  {
    return true;
  }

  size_type dimensions() const
  {
    return m_partition.global_domain().dimensions();
  }

  std::size_t size() const
  {
    return m_partition.global_domain().size();
  }

  std::size_t linearize(index_type const& coord) const
  {
    return nd_linearize<index_type, traversal_type>(dimensions())(coord);
  }

  void define_type(typer& t)
  {
    t.member(m_partition);
    t.member(m_cur_pid);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A balanced span which is used the most across the
/// skeletons library. It assigns a balanced <b>spawn</b> load to each
/// location. The load on each location is roughly \f$\frac{n}{p}\f$.
///
/// @ingroup skeletonsSpans
//////////////////////////////////////////////////////////////////////
template <>
class balanced<1>
{
public:
  using domain_type    = indexed_domain<std::size_t>;
  using index_type     = domain_type::index_type;
  using size_type      = domain_type::size_type;
  using dimension_type = std::integral_constant<int, 1>;

protected:
  balanced_partition<domain_type>     m_partition;
  std::size_t                         m_cur_pid;
public:
  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    //what we currently do is that we look at the first view and base our
    //division of work off of a balanced partition over that
    auto dom = stapl::get<
                 skeletons::first_finite_domain_index<VDomains>::value
               >(view_domains);

    m_partition = balanced_partition<domain_type>(
                    domain_type(dom.dimensions()), spawner.get_num_PEs());
    m_cur_pid = spawner.get_PE_id();
  }

  domain_type local_domain()
  {
    return m_partition[m_cur_pid];
  }

  template <typename Coord>
  bool should_spawn (Coord const&, Coord const&) const
  {
    return true;
  }

  size_type dimensions() const
  {
    return m_partition.global_domain().dimensions();
  }

  std::size_t size() const
  {
    return m_partition.global_domain().size();
  }

  std::size_t linearize(index_type const& coord) const
  {
    return coord;
  }

  void define_type(typer& t)
  {
    t.member(m_partition);
    t.member(m_cur_pid);
  }
};

} // namespace spans
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_SPANS_BALANCED_HPP
