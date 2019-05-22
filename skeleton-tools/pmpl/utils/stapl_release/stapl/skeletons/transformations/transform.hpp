/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_TRANSFORM_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_TRANSFORM_HPP

namespace stapl {
namespace skeletons {
namespace transformations {

//////////////////////////////////////////////////////////////////////
/// @brief In order to define a new transformation, you need to define
/// a partial template specialization of this class. Since there is
/// no default transformation, this class is only declared here.
///
/// @tparam S            the skeleton to be transformed
/// @tparam SkeletonTag  should be used for partial specialization
/// @tparam TransformTag determines the type of transformation to be
///                      applied
//////////////////////////////////////////////////////////////////////
template <typename S, typename SkeletonTag, typename TransformTag>
struct transform;

namespace transformations_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A wrapper of the transformed skeleton that reduces symbol
/// sizes used for a transformed skeleton. The real transformations
/// are defined by specializing the @c transformations::transform
/// class.
///
/// @tparam S            the skeleton to be transformed
/// @tparam TransformTag determines the type of transformation to be
///                      applied
//////////////////////////////////////////////////////////////////////
template <typename S, typename TransformTag, typename... Args>
struct transform
  : public decltype(
              skeletons::transformations::transform<
                S, typename S::skeleton_tag_type, TransformTag
              >::call(std::declval<S>(), std::declval<Args>()...))
{
  using base_type = decltype(
                      skeletons::transformations::transform<
                        S, typename S::skeleton_tag_type, TransformTag
                      >::call(std::declval<S>(), std::declval<Args>()...));

  transform(S const& s, Args const&... args)
    : base_type(
        skeletons::transformations::transform<
          S, typename S::skeleton_tag_type, TransformTag
        >::call(s, args...))
  { }
};

} // namespace transformations_impl
} // namespace transformations

//////////////////////////////////////////////////////////////////////
/// @brief Applies a transformation specified by the @c TransformTag
/// on a given skeleton in order to generate a new skeleton (e.g., nesting)
///
/// @tparam TransformTag determines the type of transformation to be
///                      applied
/// @param  skeleton     the skeleton to be transformed
//////////////////////////////////////////////////////////////////////
template <typename TransformTag,
          typename S,
          typename =
            typename std::enable_if<
              is_skeleton<typename std::decay<S>::type>::value>::type,
          typename... Args>
auto
transform(S&& skeleton, Args&&... args)
STAPL_AUTO_RETURN((
  skeletons::transformations::transformations_impl::transform<
    typename std::decay<S>::type,
    TransformTag,
    typename std::decay<Args>::type...>(
      std::forward<S>(skeleton), std::forward<Args>(args)...)
))

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_TRANSFORM_HPP