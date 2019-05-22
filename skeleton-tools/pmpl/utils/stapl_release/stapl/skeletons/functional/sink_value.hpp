/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_SINK_VALUE_HPP
#define STAPL_SKELETONS_FUNCTIONAL_SINK_VALUE_HPP

#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/operators/compose.hpp>
#include <stapl/skeletons/operators/elem.hpp>
#include <stapl/skeletons/param_deps/zip_pd.hpp>
#include <stapl/algorithms/functional.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief This class abstracts the semantics of a sink_value skeleton
/// by exposing only the necessary information in its representation.
///
/// A sink_value skeleton pipes the data produced from a skeleton to
/// a set of identity tasks which are labeled as results tasks. Result
/// tasks in a @c PARAGRAPH environment (@ref taskgraph_env) allow
/// values to be returned from a @c PARAGRAPH.
///
/// @note This skeleton is not designed to be used with other environments
/// unless the same functionality (defining result tasks) is defined in
/// the given environment.
///
/// This abstraction not only makes the reconstruction of a
/// a sink_value skeleton easier, but also reduces the symbol size for a
/// sink_value skeleton, hence, reducing the total compilation time.
///
/// @tparam Skeleton  the skeleton to be piped into result tasks.
/// @tparam ValueType the value type for the result tasks.
/// @tparam Span      the iteration space for the elements in the skeleton.
///
/// @ingroup skeletonsFunctionalInternal
//////////////////////////////////////////////////////////////////////
template <typename Skeleton, typename ValueType,
          typename Span>
struct sink_value
  : public decltype(
             skeletons::compose(
               std::declval<Skeleton>(),
               skeletons::elem<Span>(
                 skeletons::zip_pd<1, true>(stapl::identity<ValueType>()))))
{
  using skeleton_tag_type = tags::sink_value;
  using base_type = decltype(
                      skeletons::compose(
                        std::declval<Skeleton>(),
                        skeletons::elem<Span>(
                          skeletons::zip_pd<1, true>(
                            stapl::identity<ValueType>()))));
  using value_type = ValueType;

  sink_value(Skeleton const& skeleton)
    : base_type(
        skeletons::compose(
          skeleton,
          skeletons::elem<Span>(
            skeletons::zip_pd<1, true>(stapl::identity<ValueType>())
          )
        )
      )
  { }

  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
};

} // namespace skeletons_impl

namespace result_of {

template <typename ValueType,
          typename Span,
          typename Skeleton>
using sink_value = skeletons_impl::sink_value<
                     typename std::decay<Skeleton>::type, ValueType, Span>;

} // namespace result_of

//////////////////////////////////////////////////////////////////////
/// @brief This sink skeleton assumes a default destination skeleton and
/// a default span for the created skeleton
///
/// @tparam ValueType    the type of elements to be copied
/// @tparam Span         the span of result tasks
/// @param  skeleton     the skeleton to be plugged into result tasks
/// @return a sink_value skeleton with a default @c span
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <typename ValueType,
          typename Span = spans::per_location,
          typename Skeleton>
result_of::sink_value<ValueType, Span, Skeleton>
sink_value(Skeleton&& skeleton)
{
  return result_of::sink_value<ValueType, Span, Skeleton>(
           std::forward<Skeleton>(skeleton));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_SINK_VALUE_HPP
