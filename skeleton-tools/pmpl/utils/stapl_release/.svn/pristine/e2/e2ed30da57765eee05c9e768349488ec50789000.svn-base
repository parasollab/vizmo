#ifndef KRIPKE_KERNEL_3D_ZDG_H__
#define KRIPKE_KERNEL_3D_ZDG_H__

#include<Kripke/Kernel.h>
#include<Grid.h>
#include "common_kernel_functions.h"

class Kernel_3d_ZDG
  : public Kernel
{
public:
  // Grid is needed to access metadata (e.g. gd_sets) stored on it.
  Grid_Data<NEST_ZDG>* grid_data;

  Kernel_3d_ZDG(Grid_Data_Base*);
  virtual ~Kernel_3d_ZDG();

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

template<typename SigtView, typename PsiView, typename RhsView,
         typename IPlane, typename JPlane, typename KPlane>
Kernel::result_type
Kernel_3d_ZDG::operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
                          IPlane const& i_plane_in, JPlane const& j_plane_in,
                          KPlane const& k_plane_in)
{
  // Index in to sigt, psi, and rhs is (i, j, k, d, g)
  typedef std::tuple<size_t, size_t, size_t, size_t, size_t> index_type;
  result_type result;

  std::tuple<size_t, size_t, size_t> cellset_id = stapl::index_of(sigt);

  std::vector<double> i_plane = i_plane_in;
  std::vector<double> j_plane = j_plane_in;
  std::vector<double> k_plane = k_plane_in;

  // grid_data, group_set, and direction_set are data members of the Kernel.
  Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];

  int num_directions = gd_set.num_directions;
  int num_groups = gd_set.num_groups;

  Directions *direction = gd_set.directions;

  typename SigtView::dimensions_type cellset_size = sigt.dimensions();
  int local_imax = std::get<0>(cellset_size);
  int local_jmax = std::get<1>(cellset_size);

  auto dx = grid_data->deltas(0);
  auto dy = grid_data->deltas(1);
  auto dz = grid_data->deltas(2);

  // All directions have same id,jd,kd, since these are all one Direction Set
  // So pull that information out now
  int octant = direction[0].octant;
  std::pair<Grid_Sweep_Block, Grid_Sweep_Block> extents =
    adjust_extent(grid_data->octant_extent()[octant], grid_data, cellset_id);

  index_type psi_idx, rhs_idx, sigt_idx;
  for (int k = extents.first.start_k, kcs = extents.second.start_k;
           k != extents.first.end_k;
           k += extents.first.inc_k, kcs += extents.second.inc_k) {
    double dzk = dz[k + 1];
    std::get<2>(psi_idx) = kcs;
    std::get<2>(rhs_idx) = kcs;
    std::get<2>(sigt_idx) = kcs;
    for (int j = extents.first.start_j, jcs = extents.second.start_j;
             j != extents.first.end_j;
             j += extents.first.inc_j, jcs += extents.second.inc_j) {
      double dyj = dy[j + 1];
      std::get<1>(psi_idx) = jcs;
      std::get<1>(rhs_idx) = jcs;
      std::get<1>(sigt_idx) = jcs;
      for (int i = extents.first.start_i, ics = extents.second.start_i;
               i != extents.first.end_i;
               i += extents.first.inc_i, ics += extents.second.inc_i) {
        double dxi = dx[i + 1];
        std::get<0>(psi_idx) = ics;
        std::get<0>(rhs_idx) = ics;
        std::get<0>(sigt_idx) = ics;

        // In reference, loop over d is parallel
        std::get<3>(sigt_idx) = 0;
        for (int d = 0; d < num_directions; ++d) {
          std::get<3>(psi_idx) = d;
          std::get<3>(rhs_idx) = d;

          double xcos = direction[d].xcos;
          double ycos = direction[d].ycos;
          double zcos = direction[d].zcos;

          double zcos_dzk = 2.0 * zcos / dzk;
          double ycos_dyj = 2.0 * ycos / dyj;
          double xcos_dxi = 2.0 * xcos / dxi;

          int i_plane_idx = I_PLANE_INDEX(jcs, kcs)*num_directions*num_groups +
            d*num_groups;
          double * psi_lf_z_d = &i_plane[i_plane_idx];

          int j_plane_idx = J_PLANE_INDEX(ics, kcs)*num_directions*num_groups +
            d*num_groups;
          double * psi_fr_z_d = &j_plane[j_plane_idx];

          int k_plane_idx = K_PLANE_INDEX(ics, jcs)*num_directions*num_groups +
            d*num_groups;
          double * psi_bo_z_d = &k_plane[k_plane_idx];

          for (int group = 0; group < num_groups; ++group) {
            std::get<4>(psi_idx) = group;
            std::get<4>(rhs_idx) = group;
            std::get<4>(sigt_idx) = group;

            /* Calculate new zonal flux */
            double psi_z_d_g = (rhs[rhs_idx]
                + psi_lf_z_d[group] * xcos_dxi
                + psi_fr_z_d[group] * ycos_dyj
                + psi_bo_z_d[group] * zcos_dzk)
                / (xcos_dxi + ycos_dyj + zcos_dzk +
                   sigt[sigt_idx]);
            psi[psi_idx] = psi_z_d_g;

            /* Apply diamond-difference relationships */
            psi_z_d_g *= 2.0;
            psi_lf_z_d[group] = psi_z_d_g - psi_lf_z_d[group];
            psi_fr_z_d[group] = psi_z_d_g - psi_fr_z_d[group];
            psi_bo_z_d[group] = psi_z_d_g - psi_bo_z_d[group];
          }
        }
      }
    }
  }

  result[0] = std::move(i_plane);
  result[1] = std::move(j_plane);
  result[2] = std::move(k_plane);
  return result;
}

#endif
