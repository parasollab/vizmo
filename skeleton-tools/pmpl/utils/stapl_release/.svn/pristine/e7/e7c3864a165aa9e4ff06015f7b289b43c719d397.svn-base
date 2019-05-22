/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_BENCHMARKS_KRIPKE_SWEEP_FILTERS_HPP
#define STAPL_BENCHMARKS_KRIPKE_SWEEP_FILTERS_HPP

#include <vector>
#include "Grid.h"
#include "Kripke/Directions.h"
#include <stapl/skeletons/utility/position.hpp>
#include <stapl/skeletons/functional/wavefront.hpp>


// find the corner from which the sweep originates
inline
std::array<stapl::skeletons::position, 3>
find_corner(Directions const& sweep_direction)
{
  using stapl::skeletons::position;
  std::array<position, 3> corner;
  auto negative_face = position::first;
  auto positive_face = position::last;

  // If a sweep direction is positive along an axis, the sweep originates from
  // the negative face.
  corner[0] = sweep_direction.id > 0 ? negative_face : positive_face;
  corner[1] = sweep_direction.jd > 0 ? negative_face : positive_face;
  corner[2] = sweep_direction.kd > 0 ? negative_face : positive_face;

  return corner;
}


// work function to generate boundary input
// The problem boundary is initialized to 1.0
struct gen_bdry_ones
{
private:
  // number of zones in the x-dimension of a plane
  int m_dimx;

  // number of zones in the y-dimension of plane
  int m_dimy;

  // number of zones in the z-dimension of plane
  int m_dimz;

  // product of the number of groups and number of directions
  // in the groupset and direction set.
  int m_groups_dirs;

  stapl::skeletons::wavefront_utils::wavefront_filter_direction m_direction;
public:
  typedef std::tuple<std::size_t, std::size_t, std::size_t> index_type;
  typedef std::vector<double>                               result_type;

  template <typename Grid_Data>
  gen_bdry_ones(Grid_Data* grid_data, int gs, int ds,
                stapl::skeletons::wavefront_utils::wavefront_filter_direction d)
    : m_dimx(grid_data->ax()), m_dimy(grid_data->ay()), m_dimz(grid_data->az()),
      m_groups_dirs(grid_data->gd_sets()[gs][ds].num_groups *
                    grid_data->gd_sets()[gs][ds].num_directions),
      m_direction(d)
  { }

  template<typename Index>
  result_type operator()(Index&&) const
  {
    using stapl::skeletons::wavefront_utils::wavefront_filter_direction;

    if (m_direction == wavefront_filter_direction::direction0)
      return result_type(m_dimy * m_dimz * m_groups_dirs, 1.0);

    if (m_direction == wavefront_filter_direction::direction1)
      return result_type(m_dimx * m_dimz * m_groups_dirs, 1.0);

    return result_type(m_dimx * m_dimy * m_groups_dirs, 1.0);
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_dimx);
    t.member(m_dimy);
    t.member(m_dimz);
    t.member(m_groups_dirs);
    t.member(m_direction);
  }
};


template<typename IncPsi>
class filter_domain_result
{
private:
  stapl::skeletons::wavefront_utils::wavefront_filter_direction m_direction;

public:
  typedef typename IncPsi::value_type result_type;

  template<typename FullResult>
  result_type operator()(FullResult&& domain_result) const
  { return std::move(domain_result[static_cast<std::size_t>(m_direction)]); }

  void set_direction(
    stapl::skeletons::wavefront_utils::wavefront_filter_direction direction)
  { m_direction = direction; }

  void define_type(stapl::typer& t)
  { t.member(m_direction); }
};

#endif // STAPL_BENCHMARKS_KRIPKE_SWEEP_FILTERS_HPP
