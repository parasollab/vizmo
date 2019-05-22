/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_ZIP_HPP
#define STAPL_SKELETONS_FUNCTIONAL_ZIP_HPP

#include <type_traits>
#include <utility>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/operators/elem.hpp>
#include <stapl/skeletons/param_deps/zip_pd.hpp>
#include <stapl/skeletons/transformations/transform.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief This class abstracts the semantics of a filtered zip skeleton
/// by exposing only the necessary information in its representation.
///
/// A filtered zip skeleton is simply a parametric dependency that
/// combines @c Arity elements in each fine-grain computation and
/// applies the filter on the produced result.
///
/// This skeleton is the most used skeleton used in several composed
/// skeletons such as map (which is simply zip<1>), sink, etc.
///
/// This abstraction not only makes the reconstruction of a
/// a zip skeleton easier, but also reduces the symbol size for a
/// zip skeleton, hence, reducing the total compilation time.
///
/// @tparam Op     the underlying operation to combine the input element.
/// @tparam Arity  the arity of the computation.
/// @tparam Span   the iteration space for the elements in the skeleton.
/// @tparam Flows  the flow between the elements in the skeleton.
/// @tparam Filter the filter to be applied on the result produced by
///                the operation.
///
/// @ingroup skeletonsFunctionalInternal
//////////////////////////////////////////////////////////////////////
template <typename Op, int Arity, typename Span, typename Flows,
          typename Filter>
struct zip
  : public decltype(
             skeletons::elem<Span, Flows>(
               skeletons::zip_pd<Arity>(
                 std::declval<Op>(), std::declval<Filter>())))
{
  using skeleton_tag_type = tags::zip<stapl::use_default, Arity>;
  using zip_op_type = Op;
  using op_type     = Op;
  using span_type   = Span;

  using base_type = decltype(
                      skeletons::elem<Span, Flows>(
                        skeletons::zip_pd<Arity>(
                          std::declval<Op>(), std::declval<Filter>())));

  zip(Op const& op, Filter const& filter)
    : base_type(
        skeletons::elem<Span, Flows>(skeletons::zip_pd<Arity>(op, filter)))
  { }

  Op get_op(void) const
  {
    return base_type::nested_skeleton().get_op();
  }

  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
};

}

namespace result_of {

template <int Arity,
          typename Span,
          typename Flows,
          typename Op,
          typename Filter>
using zip = skeletons_impl::zip<
              typename std::decay<Op>::type, Arity, Span, Flows,
              typename std::decay<Filter>::type>;

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief A filtered zip is similar to @c zip skeleton but it applies
/// a filter function on the producer side before sending data along
/// the edges to each parametric dependency.
///
/// @tparam Arity  the arity of zip
/// @tparam Span   the iteration space for the elements in this
///                skeleton
/// @tparam Flows  the flow to be used for this skeleton
/// @param  op     the workfunction to be used in each zip parametric
///                dependency.
/// @param  filter the filter function to be used on the producer side
///                before sending data to a parametric dependency
/// @return a zip skeleton with a filter on the incoming edges
///
/// @see zip
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <int Arity       = 2,
          typename Span   = stapl::use_default,
          typename Flows  = stapl::use_default,
          typename Op,
          typename Filter = skeletons::no_filter,
          typename =
            typename std::enable_if<
              !is_skeleton<typename std::decay<Op>::type>::value>::type>
result_of::zip<Arity, Span, Flows, Op, Filter>
zip(Op&& op, Filter&& filter = Filter())
{
  return result_of::zip<Arity, Span, Flows, Op, Filter>(
           std::forward<Op>(op),
           std::forward<Filter>(filter));
}

//////////////////////////////////////////////////////////////////////
/// @brief Creates a zip skeleton over a nested skeleton composition
/// by transforming the inner skeleton to a suitable skeleton for
/// nested execution.
///
/// @tparam Arity  the arity of zip
/// @param  skeleton the skeleton to be used in the nested section
/// @param  filter   the filter function to be used on the producer side
///                  before sending data to a parametric dependency
///
/// @see zip
///
/// @ingroup skeletonsFunctional
//////////////////////////////////////////////////////////////////////
template <int Arity, typename S,
          typename Filter = skeletons::no_filter,
          typename =
            typename std::enable_if<
              is_skeleton<typename std::decay<S>::type>::value>::type>
result_of::zip<
  Arity, stapl::use_default, stapl::use_default,
  decltype(skeletons::transform<tags::nest>(std::declval<S>())),
  Filter>
zip(S&& skeleton, Filter&& filter = Filter())
{
  return skeletons::zip<Arity>(
           skeletons::transform<tags::nest>(std::forward<S>(skeleton)),
           std::forward<Filter>(filter));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_ZIP_HPP
