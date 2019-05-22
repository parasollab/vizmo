/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_DEFAULT_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_DEFAULT_HPP

#include <stapl/skeletons/executors/algorithm_executor.hpp>
#include <stapl/views/multiarray_view.hpp>
#include <stapl/views/array_view.hpp>

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief An @c optimizer with the default execution strategy, spawns
/// the given skeleton (creates a data flow graph).
///
/// The value 'false' in default_execution denotes that the given
/// skeleton is not reducing the input to a single value.
///
/// @see algorithm_executor
///
/// @ingroup skeletonsTransformationsNest
//////////////////////////////////////////////////////////////////////
template <typename Tag>
struct optimizer<Tag, tags::default_execution<false>>
{
  // would be discarded if a deriving optimizer redefines result_type
  template <typename R>
  struct result;

  template <typename Optimizer, typename OutputValueType>
  struct result<Optimizer(OutputValueType)>
  {
    using traits_t = stapl::skeletons::skeleton_traits<
                       Tag, tags::default_execution<false>>;
    using type = typename traits_t::template result_type<OutputValueType>;
  };

private:
  template <int i, typename V>
  static auto wrap_with_view(V&& v, std::true_type)
  STAPL_AUTO_RETURN((v))

  template <int i, typename V,
            typename = typename std::enable_if<(i>1)>::type>
  static auto wrap_with_view(V&& v, std::false_type)
  STAPL_AUTO_RETURN((stapl::make_multiarray_view(std::forward<V>(v))))

  template <int i, typename V,
            typename = typename std::enable_if<(i==1)>::type>
  static auto wrap_with_view(V&& v, std::false_type)
  STAPL_AUTO_RETURN((stapl::make_array_view(std::forward<V>(v))))

  template <typename V>
  static auto wrap_with_view(V&& v)
  STAPL_AUTO_RETURN((
    wrap_with_view<
      stapl::dimension_traits<typename std::decay<V>::type>::type::value
    >(std::forward<V>(v),
      typename stapl::is_view<typename std::decay<V>::type>::type())
  ))

  template <typename R, typename S, typename... View>
  static R apply_execute(std::true_type, S&& skeleton, View&&... view)
  {
    return algorithm_executor<null_coarsener>().execute(
             std::forward<S>(skeleton),
             wrap_with_view(view)...);
  }

  template <typename R, typename S, typename View0, typename...Views>
  static R apply_execute(std::false_type, S&& s, View0&& view0,
                         Views&&... views)
  {
    using namespace skeletons;
    using val_t  = typename std::decay<View0>::type::value_type;
    using dims_t = typename dimension_traits<
                     typename std::decay<View0>::type>::type;
    using span_t = typename std::conditional<
                     (dims_t::value == 1),
                     stapl::use_default,
                     spans::blocked<dims_t::value>>::type;

    R result(view0.domain().dimensions());

    // views should not be forwarded in this method, as they might be rvalue
    // references. This causes the views to be destroyed before returning from
    // wrap_with_view
    algorithm_executor<null_coarsener>().execute(
      skeletons::sink<val_t, span_t>(std::forward<S>(s)),
      optimizer::wrap_with_view(view0),
      optimizer::wrap_with_view(views)...,
      optimizer::wrap_with_view(result));

    return result;
  }

public:
  template <typename R, typename... Args>
  static R execute(Args&&... args)
  {
    return apply_execute<R>(
             typename std::is_same<R, void>::type(),
             std::forward<Args>(args)...);
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief An @c optimizer with the default execution strategy, spawns
/// the given skeleton (creates a data flow graph).
///
/// The value 'true' in default_execution denotes that the given
/// skeleton is reducing the input to a single value. In order to
/// provide the reduced value to all the participating locations,
/// the given skeleton needs to be composed with a @c broadcast_to_locs
/// skeleton, and should then be wrapped by a @c sink_value skeleton.
///
/// @see algorithm_executor
///
/// @ingroup skeletonsTransformationsNest
//////////////////////////////////////////////////////////////////////
template <typename Tag>
struct optimizer<Tag, tags::default_execution<true>>
{
  template <typename FlowValueType>
  struct result;

  template <typename Optimizer, typename FlowValueType>
  struct result<Optimizer(FlowValueType)>
  {
    using type = FlowValueType;
  };

  template <typename R, typename S, typename... View>
  static R execute(S&& skeleton, View&&... view)
  {
    return algorithm_executor<null_coarsener>().execute<R>(
             skeletons::sink_value<R>(
               skeletons::compose(
                 std::forward<S>(skeleton),
                 skeletons::broadcast_to_locs())),
             std::forward<View>(view)...);
  }
};

} // namespace optimizers
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_DEFAULT_HPP
