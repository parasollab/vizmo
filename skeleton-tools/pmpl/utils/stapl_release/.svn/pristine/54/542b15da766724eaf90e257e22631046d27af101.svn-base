/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_COARSE_WAVEFRONT_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_COARSE_WAVEFRONT_HPP

#include <stapl/skeletons/transformations/coarse/coarse.hpp>
#include <stapl/skeletons/functional/wavefront.hpp>
#include <stapl/skeletons/transformations/wrapped_skeleton.hpp>
#include <stapl/skeletons/transformations/optimizers/wavefront.hpp>
#include <stapl/skeletons/utility/position.hpp>

namespace stapl {
namespace skeletons {
namespace transformations {

namespace transformations_impl {

//////////////////////////////////////////////////////////////////////
/// @brief A filter operation used in the coarsened wavefront operation.
///
/// @tparam dims the dimensionality of the given wavefront.
//////////////////////////////////////////////////////////////////////
template <int dims, typename Filter>
struct wavefront_filter
{
private:
  using corner_t    = std::array<skeletons::position, dims>;
  using dir_t       = skeletons::wavefront_utils::wavefront_filter_direction;
  corner_t m_corner;
  dir_t    m_direction;
  Filter   m_filter;

public:
  template <typename F>
  struct result;

  template <typename V>
  struct result<wavefront_filter(V)>
  {
    using value_t = typename std::decay<V>::type::value_type;
    using filtered_t = typename filters::result_of<Filter,value_t>::type;
    using type    = lightweight_multiarray<filtered_t, dims>;
  };

  wavefront_filter(corner_t const& corner, Filter const& filter)
    : m_corner(corner),
      m_direction(dir_t::direction0),
      m_filter(filter)
  { }

  void set_direction(dir_t direction)
  {
    m_direction = direction;
    filters::set_direction(m_filter, m_direction);
  }

private:
  //////////////////////////////////////////////////////////////////////
  /// @brief The specialization of the filter for the 2D wavefront.
  ///
  /// @param v the input to be filtered
  ///
  /// @return the filtered portion of the input
  //////////////////////////////////////////////////////////////////////
  template <typename V>
  typename result<wavefront_filter(V)>::type
  apply(V&& v, std::integral_constant<int, 2>) const
  {
    auto&& dimensions = v.domain().dimensions();
    std::size_t dims0, dims1;
    std::tie(dims0, dims1) = dimensions;

    using namespace wavefront_utils;
    std::size_t end_i, end_j;
    std::tie(end_i, end_j) =  last_index(m_corner, dimensions);

    // Calculating the output result dimensions. For example, in a wavefront
    // starting from the top left corner, on direction0 the dimensions are
    // (1, dims1) and on direction1 they are (dims0, 1)
    //   ___________
    //  |         | |
    //  |         | | -> direction1
    //  |_________|_|
    //  |_________|_|
    //       |
    //       v
    //   direction0
    //
    std::size_t result_dim0 = (m_direction == dir_t::direction0 ? 1 : dims0);
    std::size_t result_dim1 = (m_direction == dir_t::direction1 ? 1 : dims1);

    std::size_t start_offset_i = (m_direction == dir_t::direction0 ? end_i : 0);
    std::size_t start_offset_j = (m_direction == dir_t::direction1 ? end_j : 0);

    using result_type = typename result<wavefront_filter(V)>::type;
    result_type res(make_tuple(result_dim0, result_dim1));

    using filters::apply_filter;

    // NOTE - end indices are inclusive
    for (std::size_t i = 0; i < result_dim0; ++i) {
      for (std::size_t j = 0; j < result_dim1; ++j) {

        res(i, j) = apply_filter(m_filter,
                                 v(start_offset_i + i, start_offset_j + j));
      }
    }
    return res;
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief The specialization of the filter for the 3D wavefront.
  ///
  /// @param v the input to be filtered
  ///
  /// @return the filtered portion of the input
  //////////////////////////////////////////////////////////////////////
  template <typename V>
  typename result<wavefront_filter(V)>::type
  apply(V&& v, std::integral_constant<int, 3>) const
  {
    auto&& dimensions = v.domain().dimensions();
    std::size_t dims0, dims1, dims2;
    std::tie(dims0, dims1, dims2) = dimensions;

    using namespace wavefront_utils;
    std::size_t end_i, end_j, end_k;
    std::tie(end_i, end_j, end_k) =  last_index(m_corner, dimensions);

    // Calculating the output result dimensions
    std::size_t result_dim0 = (m_direction == dir_t::direction0 ? 1 : dims0);
    std::size_t result_dim1 = (m_direction == dir_t::direction1 ? 1 : dims1);
    std::size_t result_dim2 = (m_direction == dir_t::direction2 ? 1 : dims2);

    std::size_t start_offset_i = (m_direction == dir_t::direction0 ? end_i : 0);
    std::size_t start_offset_j = (m_direction == dir_t::direction1 ? end_j : 0);
    std::size_t start_offset_k = (m_direction == dir_t::direction2 ? end_k : 0);

    using result_type = typename result<wavefront_filter(V)>::type;
    result_type res(make_tuple(result_dim0, result_dim1, result_dim2));

    using filters::apply_filter;

    // NOTE - end indices are inclusive
    for (std::size_t i = 0; i < result_dim0; ++i) {
      for (std::size_t j = 0; j < result_dim1; ++j) {
        for (std::size_t k = 0; k < result_dim2; ++k) {
          res(i, j, k) = apply_filter(m_filter,
                                      v(start_offset_i + i,
                                        start_offset_j + j,
                                        start_offset_k + k));
        }
      }
    }
    return res;
  }

public:
  template <typename V>
  typename result<wavefront_filter(V)>::type
  operator()(V&& v) const
  {
    return apply(std::forward<V>(v), std::integral_constant<int, dims>());
  }

  bool operator==(wavefront_filter const& other) const
  {
    return m_direction == other.m_direction and
           m_corner == other.m_corner;
  }

  void define_type(typer& t)
  {
    t.member(m_direction);
    t.member(m_corner);
    t.member(m_filter);
  }
};

} // transformations_impl

//////////////////////////////////////////////////////////////////////
/// @brief A coarse-grain @c wavefront skeleton on multiarrays is expressed
/// as a wavefront of wavefronts (wavefront(wavefront(op)), in which @c op
/// is the user-specified fine-grain operation).
///
/// In the default case, the nested wavefront skeleton is executed as
/// by creating a paragraph in the nested section.
///
/// @tparam S            the reduce skeleton to be coarsened
/// @tparam i            the dimensionality of the given wavefront skeleton
/// @tparam CoarseTag    a tag to specify the required specialization for
///                      coarsening
/// @tparam ExecutionTag a tag to specify the execution method used for
///                      the coarsened chunks
///
///
/// @see skeletonsTagsCoarse
/// @see skeletonsTagsExecution
///
/// @ingroup skeletonsTransformationsCoarse
//////////////////////////////////////////////////////////////////////
template<typename S, int i, typename CoarseTag, typename ExecutionTag>
struct transform<S, tags::wavefront<i>,
                 tags::coarse<CoarseTag, ExecutionTag>>
{
  static auto call(S const& skeleton)
  STAPL_AUTO_RETURN((
    skeletons::wavefront<S::number_of_inputs::value>(
      skeletons::wrap<ExecutionTag>(skeleton),
      skeleton.get_start_corner(),
      transformations_impl::wavefront_filter<S::number_of_dimensions::value,
        typename S::filter_type>(skeleton.get_start_corner(),
                                 skeleton.get_filter()))
  ))
};

} // namespace transformations
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_COARSE_WAVEFRONT_hpp
