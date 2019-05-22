#ifndef KRIPKE_KERNEL_3D_GDZ_H__
#define KRIPKE_KERNEL_3D_GDZ_H__

#include<Kripke/Kernel.h>
#include<Grid.h>
#include "common_kernel_functions.h"
#include <stapl/views/slices_view.hpp>

class Kernel_3d_GDZ
  : public Kernel
{
public:
  // Grid is needed to access metadata (e.g. gd_sets) stored on it.
  Grid_Data<NEST_GDZ>* grid_data;

  Kernel_3d_GDZ(Grid_Data_Base*);
  virtual ~Kernel_3d_GDZ();

  void LTimes(void);
  void LPlusTimes(void);

  template<typename SigtView, typename PsiView, typename RhsView,
           typename IPlane, typename JPlane, typename KPlane>
  Kernel::result_type
  operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
             IPlane const& i_plane_in, JPlane const& j_plane_in,
             KPlane const& k_plane_in);

  void define_type(stapl::typer& t)
  {
    t.base<Kernel>(*this);
    t.member(grid_data);
  }
};

/* Sweep routine for Diamond-Difference */
/* Macros for offsets with fluxes on cell faces */
#define I_PLANE_INDEX(j, k) (k)*(local_jmax) + (j)
#define J_PLANE_INDEX(i, k) (k)*(local_imax) + (i)
#define K_PLANE_INDEX(i, j) (j)*(local_imax) + (i)
#define Zonal_INDEX(i, j, k) (i) + (local_imax)*(j) \
  + (local_imax)*(local_jmax)*(k)


template<typename ResultValue>
struct Kernel_3d_GDZ_grp_loop_wf
{
private:
  // Grid is needed to access metadata (e.g. gd_sets) stored on it.
  Grid_Data<NEST_GDZ>* grid_data;

  Group_Dir_Set gd_set;

  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> extents;

  public:
  typedef ResultValue result_type;

  Kernel_3d_GDZ_grp_loop_wf(Grid_Data<NEST_GDZ>* data, Group_Dir_Set gdset,
    std::pair<Grid_Sweep_Block, Grid_Sweep_Block> grid_extents)
    : grid_data(data), gd_set(gdset), extents(grid_extents)
  { }

  template<typename GroupId, typename PsiView, typename RhsView,
           typename SigtView, typename IPlane, typename JPlane, typename KPlane>
  result_type
  operator()(GroupId g, PsiView& psi, RhsView& rhs, SigtView& sigt,
             IPlane const& i_plane_in, JPlane const& j_plane_in,
             KPlane const& k_plane_in)
  {
    // Index in to psi and rhs is (i, j, k, d)
    typedef std::tuple<size_t, size_t, size_t, size_t> index_type;

    // Index in to sigt is (i, j, k, 0, g)
    typedef std::tuple<size_t, size_t, size_t, size_t, size_t> sigt_index_type;

    result_type result;

    std::vector<double> i_plane = i_plane_in;
    std::vector<double> j_plane = j_plane_in;
    std::vector<double> k_plane = k_plane_in;

    int num_groups = gd_set.num_groups;
    int num_directions = gd_set.num_directions;

    // Variables used for indexing into the planes
    typename SigtView::dimensions_type cellset_size = sigt.dimensions();
    int local_imax = std::get<0>(cellset_size);
    int local_jmax = std::get<1>(cellset_size);

    // Because our results will be merged with results from other groups
    // we have to zero out the plane information for those groups.
    for (int curr_g = 0; curr_g < num_groups; ++curr_g)
    {
      if (curr_g != g)
      {
        for (int direction = 0; direction < num_directions; direction++) {
          for (int kcs = extents.second.start_k;
                   kcs != extents.second.end_k;
                   kcs += extents.second.inc_k) {
            for (int jcs = extents.second.start_j;
                     jcs != extents.second.end_j;
                     jcs += extents.second.inc_j) {
              int i_plane_idx = I_PLANE_INDEX(jcs, kcs) * num_directions
                * num_groups + direction * num_groups;
              i_plane[i_plane_idx+curr_g] = 0.;
              for (int ics = extents.second.start_i;
                       ics != extents.second.end_i;
                       ics += extents.second.inc_i) {
                int j_plane_idx = J_PLANE_INDEX(ics, kcs) * num_directions
                  * num_groups + direction * num_groups;
                int k_plane_idx = K_PLANE_INDEX(ics, jcs) * num_directions
                  * num_groups + direction * num_groups;
                  j_plane[j_plane_idx+curr_g] = 0.;
                  k_plane[k_plane_idx+curr_g] = 0.;
              }
            }
          }  // zones
        }  // directions
      }  // endif
    }  // groups

    auto dx = grid_data->deltas(0);
    auto dy = grid_data->deltas(1);
    auto dz = grid_data->deltas(2);

    Directions *direction = gd_set.directions;

    index_type psi_idx, rhs_idx;
    sigt_index_type sigt_idx;

    for (int d = 0; d < num_directions; ++d)
    {
      double xcos = direction[d].xcos;
      double ycos = direction[d].ycos;
      double zcos = direction[d].zcos;

      std::get<3>(psi_idx) = d;
      std::get<3>(rhs_idx) = d;
      std::get<3>(sigt_idx) = 0; // sigt_idx has a single element in direction
      std::get<4>(sigt_idx) = g;

      // In reference, loop over z is parallel
      for (int k = extents.first.start_k, kcs = extents.second.start_k;
               k != extents.first.end_k;
               k += extents.first.inc_k, kcs += extents.second.inc_k)
      {
        double zcos_dzk = 2.0 * zcos / dz[k + 1];

        std::get<2>(psi_idx) = kcs;
        std::get<2>(rhs_idx) = kcs;
        std::get<2>(sigt_idx) = kcs;

        for (int j = extents.first.start_j, jcs = extents.second.start_j;
                 j != extents.first.end_j;
                 j += extents.first.inc_j, jcs += extents.second.inc_j)
        {
          double ycos_dyj = 2.0 * ycos / dy[j + 1];

          std::get<1>(psi_idx) = jcs;
          std::get<1>(rhs_idx) = jcs;
          std::get<1>(sigt_idx) = jcs;

          int i_plane_idx = I_PLANE_INDEX(jcs, kcs) * num_directions
            * num_groups + d * num_groups + g;
          double & psi_lf_g_d_z = i_plane[i_plane_idx];

          for (int i = extents.first.start_i, ics = extents.second.start_i;
                   i != extents.first.end_i;
                   i += extents.first.inc_i, ics += extents.second.inc_i)
          {
            double xcos_dxi = 2.0 * xcos / dx[i + 1];

            std::get<0>(psi_idx) = ics;
            std::get<0>(rhs_idx) = ics;
            std::get<0>(sigt_idx) = ics;

            int j_plane_idx = J_PLANE_INDEX(ics, kcs) * num_directions
              * num_groups + d * num_groups + g;
            double & psi_fr_g_d_z = j_plane[j_plane_idx];

            int k_plane_idx = K_PLANE_INDEX(ics, jcs) * num_directions
              * num_groups + d * num_groups + g;
            double & psi_bo_g_d_z = k_plane[k_plane_idx];

            /* Calculate new zonal flux */
            double psi_g_d_z = (rhs[rhs_idx] + psi_lf_g_d_z * xcos_dxi
                + psi_fr_g_d_z * ycos_dyj + psi_bo_g_d_z * zcos_dzk)
                / (xcos_dxi + ycos_dyj + zcos_dzk + sigt[sigt_idx]);
            psi[psi_idx] = psi_g_d_z;

            /* Apply diamond-difference relationships */
            psi_g_d_z *= 2.0;
            psi_lf_g_d_z = psi_g_d_z - psi_lf_g_d_z;
            psi_fr_g_d_z = psi_g_d_z - psi_fr_g_d_z;
            psi_bo_g_d_z = psi_g_d_z - psi_bo_g_d_z;
          }
        }
      }
    }

    result[0] = std::move(i_plane);
    result[1] = std::move(j_plane);
    result[2] = std::move(k_plane);

    return result;
  }

  void define_type(stapl::typer& t)
  {
    t.member(grid_data);
    t.member(gd_set);
    t.member(extents);
  }
};


template<typename SigtView, typename PsiView, typename RhsView,
         typename IPlane, typename JPlane, typename KPlane>
Kernel::result_type
Kernel_3d_GDZ::operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
                          IPlane const& i_plane_in, JPlane const& j_plane_in,
                          KPlane const& k_plane_in)
{
  std::tuple<size_t, size_t, size_t> cellset_id = stapl::index_of(sigt);

  // grid_data, group_set, and direction_set are data members of the Kernel.
  Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];

  int num_groups = gd_set.num_groups;

  // All directions have same id,jd,kd, since these are all one Direction Set
  // So pull that information out now
  Directions *direction = gd_set.directions;
  int octant = direction[0].octant;
  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> extents =
    adjust_extent(grid_data->octant_extent()[octant], grid_data, cellset_id);

  Kernel_3d_GDZ_grp_loop_wf<result_type>
    grp_loop_body(grid_data, gd_set, extents);

  // type definitions to create a proxy backed by trivial_accessor.
  // The proxies of the incoming planes are backed by edge_accessor, which
  // isn't serializable.
  typedef Kernel::result_type::value_type  vec_t;
  typedef stapl::trivial_accessor<vec_t>   acc_t;
  typedef stapl::proxy<vec_t, acc_t>       proxy_t;

  // sigt is repeated in the call because it has a single element in the
  // group dimension.  The psi and rhs views both have elements in the
  // group dimension equal to the number of groups in the group set.
  result_type result = stapl::map_reduce<stapl::skeletons::tags::no_coarsening>(
    grp_loop_body, reduce_planes<result_type>(),
    stapl::counting_view<int>(num_groups),
    stapl::make_slices_view<4>(psi), stapl::make_slices_view<4>(rhs),
    stapl::make_repeat_view(sigt),
    stapl::make_repeat_view(proxy_t(acc_t(i_plane_in))),
    stapl::make_repeat_view(proxy_t(acc_t(j_plane_in))),
    stapl::make_repeat_view(proxy_t(acc_t(k_plane_in))));

  return result;
}
#endif

