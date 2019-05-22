/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_STENCIL_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_STENCIL_HPP

#include <numeric>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/transformations/optimizer.hpp>
#include <stapl/skeletons/utility/filters.hpp>
#include <stapl/skeletons/transformations/optimizers/default.hpp>

namespace stapl {
namespace skeletons {

template <typename SkeletonTag, typename ExecutionTag>
struct skeleton_traits;

template <>
struct skeleton_traits<tags::stencil<1, 3, false>,
                       tags::default_execution<false>>
{
  template <typename OutputValueType>
  using result_type = std::vector<OutputValueType>;
};


template <>
struct skeleton_traits<tags::stencil<2, 5, false>,
                       tags::default_execution<false>>
{
  template <typename OutputValueType>
  using result_type = lightweight_multiarray<OutputValueType, 2>;
};

namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

/////////////////////////////////////////////////////////////////////
/// @brief A @c stencil_optimizer executes the non-periodic stencil
/// 1D, 3 point stencil skeleton on given input and boundary values.
/// This specialization is for executing the nested stencil
/// sequentially.
///
/// @ingroup skeletonsTransformationsCoarse
/////////////////////////////////////////////////////////////////////
template <>
struct optimizer<tags::stencil<1, 3, false>, tags::sequential_execution>
{
  template <typename R>
  struct result;

  template <typename Skeleton, typename FlowValueType>
  struct result<Skeleton(FlowValueType)>
  {
    typedef std::vector<FlowValueType> type;
  };

  template <typename ReturnType, typename S,
            typename Boundary, typename Center>
  static ReturnType
  execute(S&& skeleton,
          Center&& center,
          Boundary&& l, Boundary&& r)
  {
    const std::size_t n = center.size();
    ReturnType result(n);

    using filters::apply_filter;

    auto f_left  =
      stencil_utils::get_stencil_filter<
        tags::direction<-1>>(skeleton.get_filter());
    auto f_right =
      stencil_utils::get_stencil_filter<
        tags::direction<+1>>(skeleton.get_filter());

    auto stencil_op = skeleton.get_op();

    if (n == 1)
    {
      result[0] = stencil_op(apply_filter(f_left, l[0]) ,
                             center[0],
                             apply_filter(f_right, r[0]));
    }
    else
    {
      // left and right boundary values
      result[0]   = stencil_op(apply_filter(f_left, l[0]),
                               center[0],
                               apply_filter(f_right, center[1]));

      result[n-1] = stencil_op(apply_filter(f_left, center[n-2]),
                               center[n-1],
                               apply_filter(f_right, r[0]));
    }

    for (std::size_t i = 1; i < n-1; ++i)
    {
      result[i] = stencil_op(apply_filter(f_left,  center[i-1]),
                             center[i],
                             apply_filter(f_right, center[i+1]));
    }

    return result;
  }

};


/////////////////////////////////////////////////////////////////////
/// @brief A @c stencil_optimizer executes the non-periodic
/// stencil 2D, 5 point skeleton on given input and boundary values.
/// This specialization is for executing the nested stencil sequentially.
///
///
///   (first, first)........(first, middle).........(first, last)
///   ...........................................................
///   ...........................................................
///   (middle, first).......(middle, middle).........(middle, last)
///   ...........................................................
///   ...........................................................
///   (last, first).........(last, middle).............(last, last)
///
/// @ingroup skeletonsTransformationsCoarse
/////////////////////////////////////////////////////////////////////
template <>
struct optimizer<tags::stencil<2, 5, false>, tags::sequential_execution>
{
  template <typename R>
  struct result;

  template <typename Skeleton, typename FlowValueType>
  struct result<Skeleton(FlowValueType)>
  {
    typedef lightweight_multiarray<FlowValueType, 2> type;
  };

  template <typename ReturnType, typename S,
            typename Boundary, typename Center>
  static ReturnType
  execute(S&& skeleton,
          Center && center,
          Boundary && u, Boundary && l,
          Boundary && d, Boundary && r)
  {

    auto dims = center.dimensions();
    auto stencil_op = skeleton.get_op();

    ReturnType out_container(dims);

    const std::size_t nx = std::get<0>(dims);
    const std::size_t ny = std::get<1>(dims);

    auto up_first    = u.domain().first();
    auto left_first  = l.domain().first();
    auto down_first  = d.domain().first();
    auto right_first = r.domain().first();

    using index_t = typename std::decay<Boundary>::type::index_type;
    auto&& view_index = [](std::size_t i, std::size_t j,
                           index_t const& first)
                        { return index_t(i + std::get<0>(first),
                                         j + std::get<1>(first)); };
    auto f_up    =
      stencil_utils::get_stencil_filter<
        tags::direction<-1, 0>>(skeleton.get_filter());
    auto f_down  =
      stencil_utils::get_stencil_filter<
        tags::direction<+1, 0>>(skeleton.get_filter());
    auto f_left  =
      stencil_utils::get_stencil_filter<
        tags::direction<0, -1>>(skeleton.get_filter());
    auto f_right =
      stencil_utils::get_stencil_filter<
        tags::direction<0, +1>>(skeleton.get_filter());

    using filters::apply_filter;


    // (first, first) point
    out_container[stapl::make_tuple(0, 0) ] =
      stencil_op(
        apply_filter(f_up,   u[view_index(0, 0, up_first)]),
        apply_filter(f_left, l[view_index(nx-1, 0, left_first)]),
        center[stapl::make_tuple(0, 0)],
        center[stapl::make_tuple(1, 0)],
        center[stapl::make_tuple(0, 1)]);

    // (last, first) point
    out_container[stapl::make_tuple(nx-1,0)] =
      stencil_op(
        center[stapl::make_tuple(nx-2, 0)],
        apply_filter(f_left, l[view_index(nx-1, 0, left_first)]),
        center[stapl::make_tuple(nx-1, 0)],
        apply_filter(f_down, d[stapl::make_tuple(0, 0)]),
        center[stapl::make_tuple(nx-1, 1)]);

    // (last, last) point
    out_container[stapl::make_tuple(nx-1,ny-1)] =
      stencil_op(
        center[stapl::make_tuple(nx-2, ny-1)],
        center[stapl::make_tuple(nx-1, ny-2)],
        center[stapl::make_tuple(nx-1, ny-1)],
        apply_filter(f_down, d[view_index(0, ny-1, down_first)]),
        apply_filter(f_right, r[view_index(0, 0, right_first)]));

    // (first, last) point
    out_container[stapl::make_tuple(0,ny-1)] =
      stencil_op(
        apply_filter(f_up, u[view_index(0, ny-1, up_first)]),
        center[stapl::make_tuple(0, ny-2)],
        center[stapl::make_tuple(0, ny-1)],
        center[stapl::make_tuple(1, ny-1)],
        apply_filter(f_right, r[view_index(0, 0, right_first)]));

    // (middle, first) , (middle, last) points
    for (std::size_t i = 1; i < nx-1; ++i)
    {
      out_container[stapl::make_tuple(i,0)] =
        stencil_op(
          center[stapl::make_tuple(i-1, 0)],
          apply_filter(f_left, l[view_index(i , 0, left_first)]),
          center[stapl::make_tuple(i,0)],
          center[stapl::make_tuple(i+1, 0)],
          center[stapl::make_tuple(i, 1)]);

      out_container[stapl::make_tuple(i,ny-1)] =
        stencil_op(
          center[stapl::make_tuple(i-1, ny-1)],
          center[stapl::make_tuple(i, ny-2)],
          center[stapl::make_tuple(i, ny-1)],
          center[stapl::make_tuple(i+1, ny-1)],
          apply_filter(f_right, r[view_index(i, 0, right_first)]));
    }

    // (first, middle) , (last, middle) points
    for (std::size_t j = 1; j < ny-1; ++j)
    {
      out_container[stapl::make_tuple(0, j)] =
        stencil_op(
          apply_filter(f_up, u[view_index(0, 0, up_first)]),
          center[stapl::make_tuple(0, j-1)],
          center[stapl::make_tuple(0, j)],
          center[stapl::make_tuple(1, j)],
          center[stapl::make_tuple(0, j+1)]);

      out_container[stapl::make_tuple(nx-1, j)] =
        stencil_op(
          center[stapl::make_tuple(nx-2, j)],
          center[stapl::make_tuple(nx-1, j-1)],
          center[stapl::make_tuple(nx-1, j)],
          apply_filter(f_down, d[view_index(0, j, down_first)]),
          center[stapl::make_tuple(nx-1, j+1)]);
    }

    // (middle, middle) points
    for (std::size_t i = 1; i < nx-1; ++i)
    {
      for (std::size_t j = 1; j < ny-1; ++j)
      {
        out_container[stapl::make_tuple(i,j)] =
          stencil_op(
            center[stapl::make_tuple(i-1, j)],
            center[stapl::make_tuple(  i, j-1)],
            center[stapl::make_tuple(  i, j  )],
            center[stapl::make_tuple(i+1, j)],
            center[stapl::make_tuple(  i, j+1)]);
      }
    }
    return out_container;
  }
};

} // namespace optimizers
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_STENCIL_HPP
