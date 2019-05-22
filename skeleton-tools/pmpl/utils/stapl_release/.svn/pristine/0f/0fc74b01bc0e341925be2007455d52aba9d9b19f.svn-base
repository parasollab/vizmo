/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_BENCHMARKS_KERNEL_WORK_FUNCTIONS_HPP
#define STAPL_BENCHMARKS_KERNEL_WORK_FUNCTIONS_HPP

#include <stapl/views/slices_view.hpp>
#include <stapl/views/extended_view.hpp>
#include <stapl/views/cross_view.hpp>
#include <stapl/views/functor_view.hpp>
#include <stapl/algorithm.hpp>
#include <stapl/utility/integer_sequence.hpp>

#include <stapl/skeletons/functional/wavefront.hpp>
#include <stapl/skeletons/transformations/wrapped_skeleton.hpp>
#include <stapl/skeletons/transformations/coarse.hpp>
#include <stapl/skeletons/transformations/coarse/compose.hpp>
//
// Container
//
using stapl::multiarray;


//
// Algorithm Specification
//
using stapl::skeletons::wavefront_utils::wavefront_filter_direction;
using stapl::default_coarsener;
using stapl::skeletons::algorithm_executor;
using stapl::skeletons::sink;
using stapl::skeletons::spans::blocked;
using stapl::map_func;
using stapl::skeletons::wavefront;
using stapl::skeletons::wrap;
using stapl::skeletons::zip;

//
// View Creation, Element Access
//
using stapl::make_array_view;
using stapl::make_repeat_view;
using stapl::make_repeat_view_nd;
using stapl::make_slices_view;
using stapl::make_extended_view;
using stapl::index_of;
using stapl::functor_view;
using stapl::functor_view_type;
using stapl::index_sequence;
using stapl::promise;

// Object Serialization
using stapl::typer;

struct zero_fill
{
  typedef void result_type;

  template <typename View>
  result_type operator()(View vw)
  {
    stapl::fill(vw, 0.0);
  }
};


class functional_diamond_difference_wf
{
private:
  int                         m_num_directions;
  Directions*                 m_directions;
  std::vector<double> const*  m_dx;
  std::vector<double> const*  m_dy;
  std::vector<double> const*  m_dz;

public:
  functional_diamond_difference_wf(int num_directions, Directions* directions,
                                   std::vector<double> const* dx,
                                   std::vector<double> const* dy,
                                   std::vector<double> const* dz)
    : m_num_directions(num_directions), m_directions(directions),
      m_dx(dx), m_dy(dy), m_dz(dz)
  { }

  typedef std::array<double, 3> result_type;

  template<typename PSI, typename RHS, typename SIGT,
           typename IBndry, typename JBndry, typename KBndry>
  result_type
  operator()(PSI&& psi, RHS&& rhs, SIGT&& sigt,
             IBndry&& lf_bndry, JBndry&& fr_bndry, KBndry&& bo_bndry) const
  {
     auto psi_idx = index_of(psi);

     int d       = std::get<3>(psi_idx);

     // Adding one to the index is needed to account for boundary cell width.
     double dzk  = (*m_dz)[std::get<2>(psi_idx) + 1];
     double dyj  = (*m_dy)[std::get<1>(psi_idx) + 1];
     double dxi  = (*m_dx)[std::get<0>(psi_idx) + 1];

     double zcos_dzk = 2.0 * m_directions[d].zcos / dzk;
     double ycos_dyj = 2.0 * m_directions[d].ycos / dyj;
     double xcos_dxi = 2.0 * m_directions[d].xcos / dxi;

     // Calculate new zonal flux
     double new_psi = (rhs + lf_bndry * xcos_dxi
                           + fr_bndry * ycos_dyj
                           + bo_bndry * zcos_dzk) /
                      ((xcos_dxi + ycos_dyj + zcos_dzk + sigt));
     psi = new_psi;

     // Apply diamond-difference relationships
     new_psi *= 2.0;
     return {{ new_psi - lf_bndry, new_psi - fr_bndry, new_psi - bo_bndry }};
  }

  void define_type(typer& t)
  {
    t.member(m_num_directions);
    t.member(m_directions, m_num_directions);
    t.member(m_dx);
    t.member(m_dy);
    t.member(m_dz);
  }
}; // class functional_diamond_difference_wf


class diamond_difference_wf
{
private:
  int                         m_num_directions;
  Directions*                 m_directions;
  std::vector<double> const*  m_dx;
  std::vector<double> const*  m_dy;
  std::vector<double> const*  m_dz;

public:
  diamond_difference_wf(int num_directions, Directions* directions,
                        std::vector<double> const* dx,
                        std::vector<double> const* dy,
                        std::vector<double> const* dz)
    : m_num_directions(num_directions), m_directions(directions),
      m_dx(dx), m_dy(dy), m_dz(dz)
  { }

  typedef void result_type;

  template<typename PSI, typename RHS, typename SIGT,
           typename IBndry, typename JBndry, typename KBndry>
  void
  operator()(PSI&& psi, RHS&& rhs, SIGT&& sigt,
             IBndry&& lf_bndry, JBndry&& fr_bndry, KBndry&& bo_bndry) const
  {
     auto psi_idx = index_of(psi);

     int d       = std::get<3>(psi_idx);

     // Adding one to the index is needed to account for boundary cell width.
     double dzk  = (*m_dz)[std::get<2>(psi_idx) + 1];
     double dyj  = (*m_dy)[std::get<1>(psi_idx) + 1];
     double dxi  = (*m_dx)[std::get<0>(psi_idx) + 1];

     double zcos_dzk = 2.0 * m_directions[d].zcos / dzk;
     double ycos_dyj = 2.0 * m_directions[d].ycos / dyj;
     double xcos_dxi = 2.0 * m_directions[d].xcos / dxi;

     // Calculate new zonal flux
     double new_psi = (rhs + lf_bndry * xcos_dxi
                           + fr_bndry * ycos_dyj
                           + bo_bndry * zcos_dzk) /
                      ((xcos_dxi + ycos_dyj + zcos_dzk + sigt));
     psi = new_psi;

     // Apply diamond-difference relationships
     new_psi *= 2.0;
     lf_bndry = new_psi - lf_bndry;
     fr_bndry = new_psi - fr_bndry;
     bo_bndry = new_psi - bo_bndry;
  }

  void define_type(typer& t)
  {
    t.member(m_num_directions);
    t.member(m_directions, m_num_directions);
    t.member(m_dx);
    t.member(m_dy);
    t.member(m_dz);
  }
}; // class diamond_difference_wf


template<typename GridData, typename PSI>
diamond_difference_wf
make_diamond_difference_wf(GridData* grid_data, PSI&&,
                           int group_set, int direction_set,
                           std::tuple<size_t, size_t, size_t> cellset_id)
{
  Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];
  Directions *direction = gd_set.directions;

#ifndef STAPL_NDEBUG
  int octant = direction[0].octant;
  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> extents =
    adjust_extent(grid_data->octant_extent()[octant],
                  grid_data, cellset_id);
#endif

  std::vector<double> const* dx = &grid_data->deltas(0);
  std::vector<double> const* dy = &grid_data->deltas(1);
  std::vector<double> const* dz = &grid_data->deltas(2);

  stapl_assert(grid_data->deltas(0).size() ==
                 std::max(extents.second.start_i+1,extents.second.end_i)+2,
               "size and index of dx results in out-of-bounds access");
  stapl_assert(grid_data->deltas(1).size() ==
                 std::max(extents.second.start_j+1,extents.second.end_j)+2,
               "size and index of dy results in out-of-bounds access");
  stapl_assert(grid_data->deltas(2).size() ==
                 std::max(extents.second.start_k+1,extents.second.end_k)+2,
               "size and index of dz results in out-of-bounds access");

  return diamond_difference_wf(
    gd_set.num_directions, direction, dx, dy, dz);
}


template<typename GridData, typename PSI>
functional_diamond_difference_wf
make_functional_diamond_difference_wf(GridData* grid_data, PSI&&,
                                      int group_set, int direction_set,
                                      std::tuple<size_t, size_t, size_t>
                                        cellset_id)
{
  Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];
  Directions *direction = gd_set.directions;

#ifndef STAPL_NDEBUG
  int octant = direction[0].octant;
  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> extents =
    adjust_extent(grid_data->octant_extent()[octant],
                  grid_data, cellset_id);
#endif

  std::vector<double> const* dx = &grid_data->deltas(0);
  std::vector<double> const* dy = &grid_data->deltas(1);
  std::vector<double> const* dz = &grid_data->deltas(2);

  stapl_assert(grid_data->deltas(0).size() ==
                 std::max(extents.second.start_i+1,extents.second.end_i)+2,
               "size and index of dx results in out-of-bounds access");
  stapl_assert(grid_data->deltas(1).size() ==
                 std::max(extents.second.start_j+1,extents.second.end_j)+2,
               "size and index of dy results in out-of-bounds access");
  stapl_assert(grid_data->deltas(2).size() ==
                 std::max(extents.second.start_k+1,extents.second.end_k)+2,
               "size and index of dz results in out-of-bounds access");
  return functional_diamond_difference_wf(
    gd_set.num_directions, direction, dx, dy, dz);
}


template<typename GridData, typename Result,
         typename InnerSlice, typename OuterSlice>
struct kernel_spawner;


template<typename GridData, typename Result,
         std::size_t ...InnerIndices, std::size_t ...OuterIndices>
struct kernel_spawner<
  GridData, Result,
  index_sequence<InnerIndices...>, index_sequence<OuterIndices...>>
  : public stapl::p_object
{
  typedef stapl::immutable_shared<std::vector<double>> is_t;

  template<typename SigtPkg, typename PsiPkg, typename RhsPkg,
           typename KernelWF>
  kernel_spawner(SigtPkg&& sigt_pkg, PsiPkg&& psi_pkg, RhsPkg&& rhs_pkg,
                 is_t i_plane, is_t j_plane, is_t k_plane,
                 stapl::rmi_handle::reference grid_data_ref,
                 int direction_set, int group_set,
                 std::tuple<size_t, size_t, size_t> cellset_id,
                 KernelWF kernel_wf, stapl::promise<Result> p)
  {
    auto sigt = sigt_pkg();
    auto psi  = psi_pkg();
    auto rhs  = rhs_pkg();

    GridData* grid_data   = stapl::resolve_handle<GridData>(grid_data_ref);
    stapl_assert(grid_data != nullptr, "grid_data failed to resolve");
    Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];

    auto psi_sliced_inner  = make_slices_view<InnerIndices...>(psi);
    auto psi_sliced_outer  = make_slices_view<OuterIndices...>(psi_sliced_inner);
    auto rhs_sliced_inner  = make_slices_view<InnerIndices...>(rhs);
    auto rhs_sliced_outer  = make_slices_view<OuterIndices...>(rhs_sliced_inner);
    auto sigt_ext          = make_extended_view<3>(sigt, gd_set.num_directions);
    auto sigt_sliced_inner = make_slices_view<InnerIndices...>(sigt_ext);
    auto sigt_sliced_outer = make_slices_view<OuterIndices...>(sigt_sliced_inner);

    kernel_wf(sigt_sliced_outer, psi_sliced_outer, rhs_sliced_outer,
              i_plane, j_plane, k_plane, grid_data,
              direction_set, group_set, cellset_id, std::move(p));
  }
}; // struct kernel_spawner

//////////////////////////////////////////////////////////////////////
/// @brief Spawn the sweep kernel over the set of location on which the
/// containers for a zoneset are stored.
///
/// This function is used to create the nested parallel section in which
/// the inner sweep over a zoneset executes.
//////////////////////////////////////////////////////////////////////
template<typename Result, typename InnerSlice, typename OuterSlice,
         typename SigtView, typename PsiView, typename RhsView,
         typename IPlane, typename JPlane, typename KPlane,
         typename GridData, typename KernelWF>
Result
invoke_kernel(SigtView& sigt, PsiView& psi, RhsView& rhs,
              IPlane const& i_plane_in, JPlane const& j_plane_in,
              KPlane const& k_plane_in, int direction_set, int group_set,
              GridData* grid_data,
              std::tuple<size_t, size_t, size_t> cellset_id,
              KernelWF kernel_wf)
{
  using value_type = typename Result::value_type;

  auto i_plane = stapl::make_immutable_shared<value_type>(i_plane_in);
  auto j_plane = stapl::make_immutable_shared<value_type>(j_plane_in);
  auto k_plane = stapl::make_immutable_shared<value_type>(k_plane_in);

  stapl::promise<Result> p;
  auto f = p.get_future();

  using spawner_t = kernel_spawner<GridData, Result, InnerSlice, OuterSlice>;

  stapl::async_construct<spawner_t>(
    [](spawner_t* s) { delete s; },
    psi.container().get_rmi_handle(), stapl::all_locations,
    stapl::transporter_packager()(sigt),
    stapl::transporter_packager()(psi),
    stapl::transporter_packager()(rhs),
    i_plane, j_plane, k_plane, grid_data->get_rmi_handle(),
    direction_set, group_set, cellset_id, kernel_wf, std::move(p)
  );

  Result val = f.get();

  return std::move(val);
}


template<int RepeatDim, typename Slice>
struct ell_op_spawner;

template<int RepeatDim, std::size_t ...Indices>
struct ell_op_spawner<RepeatDim, index_sequence<Indices...>>
  : public stapl::p_object
{
  template<typename SlicedPkg0, typename RepeatedPkg, typename SlicedPkg1,
           typename WF>
  ell_op_spawner(SlicedPkg0&& slice_pkg0, RepeatedPkg&& repeat_pkg,
                 SlicedPkg1&& slice_pkg1, WF wf, stapl::promise<void> p)
  {
    auto to_slice0 = slice_pkg0();
    auto to_repeat = repeat_pkg();
    auto to_slice1 = slice_pkg1();

    auto sliced0  = make_slices_view<Indices...>(to_slice0);
    auto repeated = make_repeat_view_nd<RepeatDim>(to_repeat);
    auto sliced1  = make_slices_view<Indices...>(to_slice1);

    map_func(wf, sliced0, repeated, sliced1);

    stapl::do_once([&] { p.set_value(); });
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Spawn the LTimes or LPlusTimes computation over the set of
/// locations on which the containers for a zoneset are stored.
///
/// This function is used to create the nested parallel section in which
/// the inner map_func over the elements associated with a zoneset is executed.
/// The function differs from invoke_kernel in that it doesn't have to handle
/// boundary views in the input and there is no return from the nested
/// computation.
//////////////////////////////////////////////////////////////////////
template<int RepeatDim, typename Slice,
         typename SlicedView0, typename RepeatedView, typename SlicedView1,
         typename WF>
void
invoke_ell_op(SlicedView0& sliced0, RepeatedView& repeated,
              SlicedView1& sliced1, WF wf)
{
  stapl::promise<void> p;
  auto f = p.get_future();

  using spawner_t = ell_op_spawner<RepeatDim, Slice>;

  stapl::async_construct<spawner_t>(
    [](spawner_t* s) { delete s; },
    sliced0.container().get_rmi_handle(), stapl::all_locations,
    stapl::transporter_packager()(sliced0),
    stapl::transporter_packager()(repeated),
    stapl::transporter_packager()(sliced1),
    wf, std::move(p)
  );

  // wait on signal that nested PARAGRAPH has finished execution
  f.get();
}


template <typename View>
void multi_dimension_zero_fill(View vw) {
  typedef typename stapl::dimension_traits<View>::type dim_t;
  typedef stapl::skeletons::spans::balanced<dim_t::value>  span_t;

  // coarsening on SLICED_view (element of slices_view) not working
  // due to mismatch in dimensionality of indices from container metadata
  // and view domain.
  stapl::skeletons::algorithm_executor<stapl::null_coarsener> exec;
  auto skel = stapl::skeletons::zip<1, span_t>(
    stapl::bind1st(stapl::assign<double>(), 0.0));
  exec.execute(skel, vw);
}

#endif
