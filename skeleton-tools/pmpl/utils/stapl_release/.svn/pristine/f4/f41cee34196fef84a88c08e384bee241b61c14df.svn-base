/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_TRANSFORMATIONS_COARSE_STENCIL_HPP
#define STAPL_SKELETONS_TRANSFORMATIONS_COARSE_STENCIL_HPP

#include <type_traits>
#include <stapl/utility/utility.hpp>
#include <stapl/skeletons/utility/tags.hpp>
#include <stapl/skeletons/functional/stencil.hpp>
#include <stapl/skeletons/transformations/wrapped_skeleton.hpp>
#include <stapl/skeletons/transformations/transform.hpp>
#include <stapl/skeletons/transformations/optimizers/stencil.hpp>

namespace stapl {
namespace skeletons {
namespace transformations {

namespace stencil_utils {

/////////////////////////////////////////////////////////////////////
/// @brief The filter function used for accessing the boundary values
/// for coarse-grain stencil.
///
/// @tparam skeleton_tag skeleton tag that filter is specialized
///                      based on that
/////////////////////////////////////////////////////////////////////
template <typename skeleton_tag>
struct stencil_filter;

/////////////////////////////////////////////////////////////////////
/// @brief The filter function used for accessing the boundary values
/// for coarse-grain stencil.
///
/////////////////////////////////////////////////////////////////////
template <>
struct stencil_filter<tags::stencil<1, 3> >
{
private:
  int m_direction;

public:

  template <typename F>
  struct result;

  template <typename V>
  struct result<stencil_filter(V)>
  {
    using value_t = typename std::decay<V>::type::value_type;
    using type    = std::vector<value_t>;
  };

  template <typename V>
  typename result<stencil_filter(V)>::type
  operator()(V const& v) const
  {
    using  result_type = typename result<stencil_filter(V)>::type;
    return result_type({v[m_direction == 1 ? 0 : v.size()-1]});
  }

  template <int i>
  void set_direction(tags::direction<i>)
  {
    m_direction = i;
  }

  bool operator==(stencil_filter const& other) const
  {
    return m_direction == other.m_direction;
  }

  void define_type(typer& t)
  {
    t.member(m_direction);
  }
};


/////////////////////////////////////////////////////////////////////
/// @brief The filter function used for accessing the boundary values
/// for coarse-grain stencil.
///
/////////////////////////////////////////////////////////////////////
template <>
struct stencil_filter<tags::stencil<2, 5> >
{
private:
  int m_direction0;
  int m_direction1;

public:
  template <typename F>
  struct result;

  template <typename V>
  struct result<stencil_filter(V)>
  {
    using value_t = typename std::decay<V>::type::value_type;
    using type    = lightweight_multiarray<value_t, 2>;
  };

  template <typename V>
  typename result<stencil_filter(V)>::type
  operator()(V const& v) const
  {
    using result_type = typename result<stencil_filter(V)>::type;
    using index_type  = typename V::index_type;

    auto dims = v.dimensions();
    const std::size_t m = std::get<0>(dims);
    const std::size_t n = std::get<1>(dims);

    std::size_t start_i, start_j;
    std::size_t end_i, end_j;
    std::size_t nx, ny;
    std::size_t offset_i, offset_j;

    start_i = m_direction0 == -1 ? m - 1 : 0 ;
    start_j = m_direction1 == -1 ? n - 1 : 0 ;

    end_i = m_direction0 == 1 ? 1 : m ;
    end_j = m_direction1 == 1 ? 1 : n ;

    offset_i = m_direction0 == -1 ? m - 1 : 0 ;
    offset_j = m_direction1 == -1 ? n - 1 : 0 ;

    nx = m_direction0 == 0 ? m : 1 ;
    ny = m_direction1 == 0 ? n : 1 ;

    result_type result(nx, ny);

    for (size_t i = start_i; i < end_i ; ++i)
    {
      for (size_t j = start_j; j < end_j ; ++j)
      {
        auto r_idx = index_type(i, j);
        auto w_idx = index_type(i - offset_i, j - offset_j);
        result[w_idx] = v[r_idx];
      }
    }

    return result;
  }

  template <int i, int j>
  void set_direction(tags::direction<i, j>)
  {
    m_direction0 = i;
    m_direction1 = j;
  }

  bool operator==(stencil_filter const& other) const
  {
    return (m_direction0 == other.m_direction0)
            and
           (m_direction1 == other.m_direction1) ;
  }

  void define_type(typer& t)
  {
    t.member(m_direction0);
    t.member(m_direction1);
  }
};

} // namespace stencil_utils


template <typename S, typename SkeletonTag, typename CoarseTag>
struct transform;


/////////////////////////////////////////////////////////////////////
/// @brief Coarse-grain stencil can be created by creating a
/// @c stencil of @c stencils. The nested stencils are non-periodic even
/// if the outer one is periodic.
///
/// @tparam S          the stencil skeleton to be coarsened
/// @tparam dimensions specifies the dimension of stencil skeleton inputs
/// @tparam CoarseTag    a tag to specify the required specialization for
///                      coarsening
/// @tparam ExecutionTag a tag to specify the execution method used for
///                      the coarsened chunks
///
/// @see skeletonsTagsCoarse
/// @see skeletonsTagsExecution
/// @ingroup skeletonsTransformationsCoarse
/////////////////////////////////////////////////////////////////////
template<typename S, int dim, int numPoints,
         typename CoarseTag, typename ExecutionTag>
struct transform<S, tags::stencil<dim, numPoints, true>,
                 tags::coarse<CoarseTag, ExecutionTag>>
{
public:
  static auto call (S const& skeleton)
  STAPL_AUTO_RETURN((
    stencil<tags::stencil<dim, numPoints>>(
      skeletons::wrap<ExecutionTag>(
        stencil<tags::stencil<dim, numPoints, false>>(
          skeleton.get_op(), skeleton.get_filter())),
      stencil_utils::stencil_filter<typename S::skeleton_tag_type>())
  ))
};

template<typename S, int dim, int numPoints,
         typename CoarseTag, typename ExecutionTag>
struct transform<S, tags::stencil<dim, numPoints, false>,
                 tags::coarse<CoarseTag, ExecutionTag>>
{
public:
  static auto call (S const& skeleton)
  STAPL_AUTO_RETURN((
    stencil<tags::stencil<dim, numPoints, false>>(
      skeletons::wrap<ExecutionTag>(
        stencil<tags::stencil<dim, numPoints, false>>(
          skeleton.get_op(), skeleton.get_filter()))
    )
  ))
};


} // namespace transformations
} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_TRANSFORMATIONS_COARSE_STENCIL_HPP
