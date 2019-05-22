/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_SPANS_MISC_HPP
#define STAPL_SKELETONS_SPANS_MISC_HPP

#include <stapl/utility/tuple/tuple.hpp>
#include <stapl/utility/tuple/front.hpp>
#include <stapl/domains/indexed.hpp>
#include "balanced.hpp"

namespace stapl {
namespace skeletons {
namespace spans {

//////////////////////////////////////////////////////////////////////
/// @brief A balanced span (@see balanced) in which each location
/// will have exactly one element to spawn.
///
/// @ingroup skeletonsSpans
//////////////////////////////////////////////////////////////////////
class per_location
  : public balanced<>
{
public:
  using size_type      = balanced<>::size_type;
  using dimension_type = balanced<>::dimension_type;

  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool forced = false)
  {
    using dom_t = skeletons::domain_type<indexed_domain<std::size_t>>;
    if (forced &&
        tuple_ops::front(view_domains).size() < spawner.get_num_PEs()) {
      balanced::set_size(spawner, view_domains);
    }
    else {
      balanced::set_size(
        spawner,
        stapl::make_tuple(
          dom_t(indexed_domain<std::size_t>(spawner.get_num_PEs()))
        )
      );
    }
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief A span that will just have one element on the location it
/// was created on.
///
/// @ingroup skeletonsSpans
//////////////////////////////////////////////////////////////////////
class only_here
  : public balanced<>
{
public:
  using size_type      = std::size_t;
  using dimension_type = typename balanced<>::dimension_type;

  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    using dom_t = skeletons::domain_type<indexed_domain<std::size_t>>;
    balanced::set_size(
      spawner,
        stapl::make_tuple(
          dom_t(indexed_domain<std::size_t>(1))
        ));
  }
};


/////////////////////////////////////////////////////////////////////////
/// @brief In some skeletons it is required to spawn as many elements as
/// the closest smaller power of two. This span defines that behavior.
///
/// For example, in a reduction tree of 20 elements, one needs to use a
/// reduction tree that has 16 leaves. In order to do that the size of
/// the input should be changed to the closest smaller power of two.
///
/// @tparam OnSpan the span on which this tree is defined
///
/// @ingroup skeletonsSpans
/////////////////////////////////////////////////////////////////////////
template <typename OnSpan>
struct reduce_to_pow_two
  : public OnSpan
{
public:
  using size_type      = typename OnSpan::size_type;
  using dimension_type = typename OnSpan::dimension_type;

private:
  size_type m_nearest_pow_2;

public:
  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    std::size_t n =
      stapl::get<
        skeletons::first_finite_domain_index<VDomains>::type::value
      >(view_domains).size();
    m_nearest_pow_2 = 1;
    while (n != 1)
    {
      n >>= 1;
      m_nearest_pow_2 <<= 1;
    }
    OnSpan::set_size(spawner, view_domains);
  }

  template <typename Coord>
  bool should_spawn (Coord const&, Coord const& coord) const
  {
    return ((std::size_t)tuple_ops::front(coord) < m_nearest_pow_2);
  }
};


/////////////////////////////////////////////////////////////////////////
/// @brief In some skeletons the number of elements to be spawned is less
/// then the elements in the input size. One good example is in reduction
/// skeleton where the size of the leaves of the tree is less than or
/// equal to the number of elements in the input.
///
/// Typically, a skeleton with this span comes after a skeleton with
/// @c reduce_to_pow_two. The main difference between the two is that in
/// this span all the elements are spawned (no should_spawn is defined).
/// The reason behind this is that the size of the span is important to
/// the elementary skeleton using it, hence needs to be stored.
///
/// @tparam OnSpan the span on which this tree is defined
///
/// @ingroup skeletonsSpans
/////////////////////////////////////////////////////////////////////////
template <typename OnSpan>
struct nearest_pow_2
  : public OnSpan
{
  using size_type      = typename OnSpan::size_type;
  using dimension_type = typename OnSpan::dimension_type;

  template <typename Spawner, typename VDomains>
  void set_size(Spawner const& spawner, VDomains const& view_domains,
                bool /*forced*/ = false)
  {
    // we need to do this just to make sure we get the correct size for
    // the next step if something like spans::per_location is used.
    OnSpan::set_size(spawner, view_domains);
    std::size_t n = OnSpan::size();
    std::size_t nearest_pow_2 = 1;
    while (n != 1)
    {
      n >>= 1;
      nearest_pow_2 <<= 1;
    }

    using dom_t = skeletons::domain_type<indexed_domain<std::size_t>>;
    //now we adjust the size
    OnSpan::set_size(
      spawner,
      stapl::make_tuple(dom_t(indexed_domain<std::size_t>(nearest_pow_2))),
      //for now this is the only place that we are forcing the changes
      true
    );
  }
};

} // namespace spans
} // namespace skeletons
} // namepsace stapl

#endif // STAPL_SKELETONS_SPANS_MISC_HPP
