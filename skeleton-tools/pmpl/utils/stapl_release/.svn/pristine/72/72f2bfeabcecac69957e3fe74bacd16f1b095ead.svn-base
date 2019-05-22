/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_FUNCTIONAL_WAVEFRONT_HPP
#define STAPL_SKELETONS_FUNCTIONAL_WAVEFRONT_HPP

#include <array>
#include <stapl/skeletons/operators/elem.hpp>
#include <stapl/skeletons/param_deps/wavefront_pd.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/skeletons/utility/position.hpp>

namespace stapl {
namespace skeletons {
namespace skeletons_impl {

//////////////////////////////////////////////////////////////////////
/// @brief This class abstracts the semantics of a wavefront skeleton
/// by exposing only the necessary information in its representation.
///
/// This abstraction not only makes the reconstruction of a
/// wavefront skeleton easier, but also provides access to the
/// underlying operation of the wavefront skeleton. Furthermore,
/// it reduces the symbol size for a wavefront skeleton, hence, reducing
/// the total compilation time.
///
/// Currently we support 2D and 3D wavefronts from all corners.
///
/// @tparam num_inputs    number of inputs not coming from the wavefront
///                       data flows.
/// @tparam dims          determines the dimensionality of the wavefront
/// @tparam Op            operation to be applied at each point.
/// @tparam Filter        the filter to be applied on the result produced by
///                       the operation.
///
/// @ingroup skeletonsFunctionalInternal
//////////////////////////////////////////////////////////////////////
template <std::size_t num_inputs, std::size_t dims,
          typename Op, typename Filter>
struct wavefront
  : public decltype(
             skeletons::elem<spans::blocked<dims>, flows::elem_f::doacross>(
               skeletons::wavefront_pd<num_inputs>(
                 std::declval<Op>(),
                 std::array<skeletons::position, dims>(),
                 std::declval<Filter>())))
{
  using skeleton_tag_type = tags::wavefront<dims>;
  using number_of_dimensions = std::integral_constant<int, dims>;
  using number_of_inputs = std::integral_constant<int, num_inputs>;
  using op_type          = Op;
  using corners_type     = std::array<skeletons::position, dims>;
  using filter_type      = Filter;
  using base_type        = decltype(
                             skeletons::elem<
                             spans::blocked<dims>, flows::elem_f::doacross
                           >(skeletons::wavefront_pd<num_inputs>(
                               std::declval<Op>(),
                               std::array<
                                 skeletons::position, dims>(),
                               std::declval<Filter>())));

  wavefront(Op const& op, corners_type const& corners, Filter const& filter)
    : base_type(
        skeletons::elem<spans::blocked<dims>, flows::elem_f::doacross>(
          skeletons::wavefront_pd<num_inputs>(
            op, corners, filter))
      )
  { }

  Op const& get_op() const
  {
    return base_type::nested_skeleton().get_op();
  }

  corners_type const& get_start_corner() const
  {
    return base_type::nested_skeleton().get_start_corner();
  }

  Filter const& get_filter() const
  {
    return base_type::nested_skeleton().get_filter();
  }

  void define_type(typer& t)
  {
    t.base<base_type>(*this);
  }
};

} // namespace skeletons_impl

namespace result_of {

template <std::size_t num_inputs, std::size_t dims,
          typename Op, typename Filter>
using wavefront = skeletons_impl::wavefront<
                    num_inputs,
                    dims,
                    typename std::decay<Op>::type,
                    typename std::decay<Filter>::type>;

} // namespace result_of


//////////////////////////////////////////////////////////////////////
/// @brief Creates an n-dimensional wavefront skeleton.
///
/// @tparam num_inputs number of inputs not coming from the wavefront
///                    data flows.
/// @param op          operation to be applied at each point.
/// @param corners     determines the starting corners of the wavefront.
/// @param boundary_values_generator the boundary values generator.
/// @param filter      the filter to be applied on the result produced by
///                    the operation.
//////////////////////////////////////////////////////////////////////
template <std::size_t num_inputs = 1,
          typename Op, std::size_t dims,
          typename Filter = skeletons::no_filter>
result_of::wavefront<num_inputs, dims, Op, Filter>
wavefront(Op&& op,
          std::array<skeletons::position, dims> const& corners,
          Filter&& filter = Filter())
{
  return result_of::wavefront<num_inputs, dims, Op, Filter>(
           std::forward<Op>(op),
           corners,
           std::forward<Filter>(filter));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_FUNCTIONAL_WAVEFRONT_HPP
