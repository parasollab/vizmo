/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_VIEWS_METADATA_ALIGNMENT_ARE_ALIGNED_HPP
#define STAPL_VIEWS_METADATA_ALIGNMENT_ARE_ALIGNED_HPP

#include <stapl/containers/type_traits/index_bounds.hpp>
#include <stapl/utility/tuple.hpp>
#include <stapl/utility/integer_sequence.hpp>
#include <stapl/utility/pack_ops.hpp>

namespace stapl {

namespace metadata {

namespace detail {

//////////////////////////////////////////////////////////////////////
/// @brief Compares if two variables values are equivalent, allowing inequality
///  if either view is set to highest bound of value, signifying an
///  infinite domain.
//////////////////////////////////////////////////////////////////////
inline
bool compare_size_func(size_t t0, size_t t1)
{
  return t0 == t1 ||
    t0 == index_bounds<size_t>::highest() ||
    t1 == index_bounds<size_t>::highest();
}


//////////////////////////////////////////////////////////////////////
/// @brief Compares if two views' sizes are equivalent, allowing inequality
///  if either view is set to highest bound of value, signifying an
///  infinite domain.
//////////////////////////////////////////////////////////////////////
template<typename T0,typename T1>
bool compare_size_func(T0 const& t0, T1 const& t1)
{
  const size_t t0size = t0.size();
  const size_t t1size = t1.size();

  return t0size == t1size ||
    t0size == index_bounds<size_t>::highest() ||
    t1size == index_bounds<size_t>::highest();
}


//////////////////////////////////////////////////////////////////////
/// @brief Static functor to check if (1) all views have the same
///   number of local partitions on all locations and (2) all
///   corresponding partitions between views are of the same size.
//////////////////////////////////////////////////////////////////////
template<typename Views, int GuideIdx,
         typename IndexList = make_index_sequence<tuple_size<Views>::value>>
struct check_equal_size;


template<int GuideIdx, typename ...Args, std::size_t ...Indices>
struct check_equal_size<tuple<Args...>, GuideIdx, index_sequence<Indices...>>
{
  static_assert(sizeof...(Args) != 1,
    "all_metadata_aligned shouldn't be called for one view");

  static bool apply(tuple<Args...> const& views)
  {
    // Check the number of partitions for this location...
    const bool b_same_num_local_partitions =
      pack_ops::functional::and_(GuideIdx == Indices ? true :
        compare_size_func(get<GuideIdx>(views).local_size(),
          get<Indices>(views).local_size())...);

    if (!b_same_num_local_partitions)
      return false;

    // Check the size of each local partition is same...
    auto dom = get<GuideIdx>(views).domain();

    using index_type = decltype(dom.first());


    bool b_partitions_same_size = true;

    // Check the size of each local partition is same...
    const size_t local_size = get<GuideIdx>(views).local_size();

    for (std::size_t i = 0; i < local_size; ++i)
    {
      b_partitions_same_size &=
        pack_ops::functional::and_(GuideIdx == Indices ? true :
          compare_size_func(
            get<GuideIdx>(views)[
              get<GuideIdx>(views).get_local_vid(i)],
            get<Indices>(views)[get<Indices>(views).get_local_vid(i)]
          )...
        );
    };

    return b_partitions_same_size;
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Functor used to determine if all the partitioned metadata
///        is aligned
///
/// @fixme Use a skeleton instead of allreduce_rmi.
//////////////////////////////////////////////////////////////////////
class all_metadata_aligned
  : public p_object
{
  bool reflect_value(bool b) const
  { return b; }

public:
  template<int GuideIdx, typename Partitions>
  bool operator()(Partitions const& partitions) const
  {
    const bool res = check_equal_size<Partitions, GuideIdx>::apply(partitions);

    if (this->get_num_locations() == 1)
      return res;

    return allreduce_rmi(std::logical_and<bool>(),
                         this->get_rmi_handle(),
                         &all_metadata_aligned::reflect_value,
                         res).get();
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of @see entries_are_aligned.
///        Specialization for non-multidimensional containers.
///
/// @param md Tuple of metadata containers
/// @tparam Guide Index in the tuple with which to compare entry sizes
//////////////////////////////////////////////////////////////////////
template<int Guide = 0, typename Containers>
bool entries_are_aligned_impl(Containers const& md, std::false_type)
{
  return detail::all_metadata_aligned().template operator()<Guide>(md);
}

//////////////////////////////////////////////////////////////////////
/// @brief Implementation of @see entries_are_aligned.
///        Specialization for multidimensional containers where we always
///        assume it is not aligned.
///
/// @param md Tuple of metadata containers
/// @tparam Guide Index in the tuple with which to compare entry sizes
//////////////////////////////////////////////////////////////////////
template<int Guide = 0, typename Containers>
bool entries_are_aligned_impl(Containers const&, std::true_type)
{
  return false;
}

} // namespace detail

//////////////////////////////////////////////////////////////////////
/// @brief Compute whether or not a set of metadata containers have
///        their entries aligned.
///
/// @param md Tuple of metadata containers
/// @tparam Guide Index in the tuple with which to compare entry sizes
//////////////////////////////////////////////////////////////////////
template<int Guide = 0, typename Containers>
bool entries_are_aligned(Containers const& md)
{
  using index_type =
    typename tuple_element<Guide, Containers>::type::index_type;

  constexpr int num_dimensions = dimension_traits<index_type>::type::value;

  using is_multidimensional = std::integral_constant<bool,
    num_dimensions != 1
  >;

  return detail::entries_are_aligned_impl(md, is_multidimensional());
}

} // namespace metadata

} // namespace stapl

#endif // STAPL_VIEWS_METADATA_ALIGNMENT_ARE_ALIGNED_HPP
