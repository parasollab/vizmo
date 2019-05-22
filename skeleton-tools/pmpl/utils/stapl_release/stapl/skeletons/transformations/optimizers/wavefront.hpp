/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_WAVEFRONT_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_WAVEFRONT_HPP

#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/utility/lightweight_multiarray.hpp>
#include <stapl/skeletons/utility/filters.hpp>
#include <stapl/skeletons/transformations/optimizer.hpp>
#include <stapl/skeletons/transformations/optimizers/default.hpp>

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief A sequential optimizer of a 2D wavefront is used in the cases
/// that all the arguments are local.
///
/// @ingroup skeletonsTransformationsCoarse
//////////////////////////////////////////////////////////////////////
template <>
struct optimizer<tags::wavefront<2>, tags::sequential_execution>
{
  template <typename R>
  struct result;

  template <typename Optimizer, typename OutputValueType>
  struct result<Optimizer(OutputValueType)>
  {
    using type = lightweight_multiarray<OutputValueType, 2>;
  };

  template <typename R, typename S,
            typename... Views>
  static R execute(S&& skeleton, Views&&... views)
  {
    return apply_reorder<R>(std::forward<S>(skeleton),
                            std::forward_as_tuple(views...),
                            make_index_sequence<
                              std::decay<S>::type::number_of_inputs::value>());
  }

private:
  template <typename R, typename S,
            typename ViewsTuple, std::size_t... Indices>
  static R apply_reorder(S&& skeleton, ViewsTuple&& views,
                         index_sequence<Indices...>&&)
  {
    return apply_execute<R>(std::forward<S>(skeleton),
                            get<sizeof...(Indices)>(views),
                            get<sizeof...(Indices)+1>(views),
                            get<Indices>(views)...);
  }

  template <typename R, typename S,
            typename BoundaryView0,
            typename BoundaryView1,
            typename View0,
            typename... Views>
  static R apply_execute(S&& skeleton,
                         BoundaryView0&& bv0,
                         BoundaryView1&& bv1,
                         View0&& view0,
                         Views&&... views)
  {
    using namespace skeletons;
    using namespace wavefront_utils;
    using index_t = typename std::decay<View0>::type::index_type;

    auto op = skeleton.get_op();
    auto f_d0 = get_wavefront_filter(skeleton.get_filter(),
                                     wavefront_filter_direction::direction0);
    auto f_d1 = get_wavefront_filter(skeleton.get_filter(),
                                     wavefront_filter_direction::direction1);

    auto&& start_corner = skeleton.get_start_corner();

    auto&& dimensions = view0.domain().dimensions();
    auto&& v0_first  = view0.domain().first();

    auto&& bv0_first = bv0.domain().first();
    auto&& bv1_first = bv1.domain().first();

    auto const bv0_first_i = get<0>(bv0_first);
    auto const bv1_first_i = get<0>(bv1_first);
    auto const bv0_first_j = get<1>(bv0_first);
    auto const bv1_first_j = get<1>(bv1_first);

    int d0, d1;
    std::size_t start_i, start_j,
                  end_i,   end_j;
    std::tie(     d0,      d1) = wavefront_direction(start_corner);
    std::tie(start_i, start_j) = first_index(start_corner, dimensions);
    std::tie(  end_i,   end_j) =  last_index(start_corner, dimensions);

    // the end indices are inclusive. We need to adjust them a priori
    end_i -= d0;
    end_j -= d1;

    using filters::apply_filter;
    R result(dimensions);
    // the rest of the domain
    for (std::size_t i = start_i; i != end_i; i -= d0)
    {
      for (std::size_t j = start_j; j != end_j; j -= d1)
      {
        auto dep0 = (i == start_i) ?
                         bv0(bv0_first_i, bv0_first_j + j)
                       : apply_filter(f_d0, result(i + d0, j));
        auto dep1 = (j == start_j) ?
                         bv1(bv1_first_i + i, bv1_first_j)
                       : apply_filter(f_d1, result(i, j + d1));
        result(i, j) =
          op( view0( i + get<0>(v0_first),
                     j + get<1>(v0_first) ),
              views( i + get<0>(views.domain().first()),
                     j + get<1>(views.domain().first()) )...,
              dep0, dep1 );
      }
    }

    return result;
  }

};

//////////////////////////////////////////////////////////////////////
/// @brief A sequential optimizer of a 3D wavefront is used in the cases
/// that all the arguments are local.
///
/// @ingroup skeletonsTransformationsCoarse
//////////////////////////////////////////////////////////////////////
template <>
struct optimizer<tags::wavefront<3>, tags::sequential_execution>
{
  template <typename R>
  struct result;

  template <typename Optimizer, typename OutputValueType>
  struct result<Optimizer(OutputValueType)>
  {
    using type = lightweight_multiarray<OutputValueType, 3>;
  };

  template <typename R, typename S,
            typename... Views>
  static R execute(S&& skeleton, Views&&... views)
  {
    return apply_reorder<R>(std::forward<S>(skeleton),
                            std::forward_as_tuple(views...),
                            make_index_sequence<
                              std::decay<S>::type::number_of_inputs::value>());
  }

private:
  template <typename R, typename S,
            typename ViewsTuple, std::size_t... Indices>
  static R apply_reorder(S&& skeleton, ViewsTuple&& views,
                         index_sequence<Indices...>&&)
  {
    return apply_execute<R>(std::forward<S>(skeleton),
                            get<sizeof...(Indices)>(views),
                            get<sizeof...(Indices)+1>(views),
                            get<sizeof...(Indices)+2>(views),
                            get<Indices>(views)...);
  }

  template <typename R, typename S,
            typename BoundaryView0,
            typename BoundaryView1,
            typename BoundaryView2,
            typename View0,
            typename... Views>
  static R apply_execute(S&& skeleton,
                         BoundaryView0&& bv0,
                         BoundaryView1&& bv1,
                         BoundaryView2&& bv2,
                         View0&& view0,
                         Views&&... views)
  {
    using namespace skeletons;
    using namespace wavefront_utils;
    using index_t = typename std::decay<View0>::type::index_type;

    auto op = skeleton.get_op();
    auto f_d0 = get_wavefront_filter(skeleton.get_filter(),
                                     wavefront_filter_direction::direction0);
    auto f_d1 = get_wavefront_filter(skeleton.get_filter(),
                                     wavefront_filter_direction::direction1);
    auto f_d2 = get_wavefront_filter(skeleton.get_filter(),
                                     wavefront_filter_direction::direction2);

    auto&& start_corner = skeleton.get_start_corner();

    auto&& dimensions = view0.domain().dimensions();
    auto&& v0_first  = view0.domain().first();

    auto&& bv0_first = bv0.domain().first();
    auto&& bv1_first = bv1.domain().first();
    auto&& bv2_first = bv2.domain().first();

    auto const bv0_first_i = get<0>(bv0_first);
    auto const bv1_first_i = get<0>(bv1_first);
    auto const bv2_first_i = get<0>(bv2_first);

    auto const bv0_first_j = get<1>(bv0_first);
    auto const bv1_first_j = get<1>(bv1_first);
    auto const bv2_first_j = get<1>(bv2_first);

    auto const bv0_first_k = get<2>(bv0_first);
    auto const bv1_first_k = get<2>(bv1_first);
    auto const bv2_first_k = get<2>(bv2_first);

    int d0, d1, d2;
    std::size_t start_i, start_j, start_k,
                  end_i,   end_j,   end_k;
    std::tie(     d0,      d1,      d2) = wavefront_direction(start_corner);
    std::tie(start_i, start_j, start_k) = first_index(start_corner, dimensions);
    std::tie(  end_i,   end_j,   end_k) =  last_index(start_corner, dimensions);

    // the end indices are inclusive. We need to adjust them a priori
    end_i -= d0;
    end_j -= d1;
    end_k -= d2;

    using filters::apply_filter;
    R result(dimensions);
    // the rest of the domain
    for (std::size_t i = start_i; i != end_i; i -= d0)
    {
      for (std::size_t j = start_j; j != end_j; j -= d1)
      {
        for (std::size_t k = start_k; k != end_k; k -= d2)
        {
          auto dep0 =
            (i == start_i) ?
              bv0(bv0_first_i, bv0_first_j + j, bv0_first_k + k)
            : apply_filter(f_d0, result(i+d0,    j,    k));
          auto dep1 =
            (j == start_j) ?
              bv1(bv1_first_i + i, bv1_first_j, bv1_first_k + k)
            : apply_filter(f_d1, result(   i, j+d1,    k));
          auto dep2 =
            (k == start_k) ?
              bv2(bv2_first_i + i, bv2_first_j + j, bv2_first_k)
            : apply_filter(f_d2, result(   i,    j, k+d2));

          result(i, j, k) =
            op( view0( i + get<0>(v0_first),
                       j + get<1>(v0_first),
                       k + get<2>(v0_first) ),
                views( i + get<0>(views.domain().first()),
                       j + get<1>(views.domain().first()),
                       k + get<2>(views.domain().first()) )...,
                dep0, dep1, dep2 );
        }
      }
    }

    return result;
  }
};

} // namespace optimizers


template <typename SkeletonTag, typename ExecutionTag>
struct skeleton_traits;

template <int i>
struct skeleton_traits<tags::wavefront<i>, tags::default_execution<false>>
{
  template <typename OutputValueType>
  using result_type = lightweight_multiarray<OutputValueType, i>;
};

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_OPTIMIZERS_WAVEFRONT_hpp
