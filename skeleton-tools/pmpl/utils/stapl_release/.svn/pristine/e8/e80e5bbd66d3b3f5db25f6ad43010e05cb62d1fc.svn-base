/*--------------------------------------------------------------------------
 * Sweep-based solver routine.
 *--------------------------------------------------------------------------*/

#include <Kripke.h>
#include <Kripke/User_Data.h>
#include <Kripke/Kernel/Kernel_3d_DGZ.h>
#include <Kripke/Kernel/Kernel_3d_DZG.h>
#include <Kripke/Kernel/Kernel_3d_ZDG.h>
#include <Kripke/Kernel/Kernel_3d_ZGD.h>
#include <Kripke/Kernel/Kernel_3d_GDZ.h>
#include <Kripke/Kernel/Kernel_3d_GZD.h>
#include <stapl/skeletons/executors/algorithm_executor.hpp>
#include <stapl/views/metadata/coarseners/null.hpp>
#include <stapl/skeletons/functional/wavefront.hpp>
#include <sweep_filters.hpp>
#include <vector>
#include <stdio.h>



/*----------------------------------------------------------------------
 * SweepSolverSolve
 *----------------------------------------------------------------------*/

int SweepSolver (User_Data *user_data)
{
  Kernel *kernel = user_data->kernel;

  BLOCK_TIMER(user_data->timing, Solve);

  // Loop over iterations
  for (int iter = 0; iter < user_data->niter; ++iter) {

    /*
     * Compute the RHS
     */

    // Discrete to Moments transformation
    {
      BLOCK_TIMER(user_data->timing, LTimes);
      kernel->LTimes();
    }

    // This is where the Scattering kernel would go!



    // Moments to Discrete transformation
    {
      BLOCK_TIMER(user_data->timing, LPlusTimes);
      kernel->LPlusTimes();
    }

    /*
     * Sweep each Group Set
     */
    {
      BLOCK_TIMER(user_data->timing, Sweep);
      for (int group_set = 0;
           group_set < user_data->num_group_sets;
           ++ group_set)
      {
        switch(user_data->kernel->nestingPhi()){
          case NEST_DGZ:
          {
            Kernel_3d_DGZ* full_kernel =
              static_cast<Kernel_3d_DGZ*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_DZG:
          {
            Kernel_3d_DZG* full_kernel =
              static_cast<Kernel_3d_DZG*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_GDZ:
          {
            Kernel_3d_GDZ* full_kernel =
              static_cast<Kernel_3d_GDZ*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_GZD:
          {
            Kernel_3d_GZD* full_kernel =
              static_cast<Kernel_3d_GZD*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_ZDG:
          {
            Kernel_3d_ZDG* full_kernel =
              static_cast<Kernel_3d_ZDG*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_ZGD:
          {
            Kernel_3d_ZGD* full_kernel =
              static_cast<Kernel_3d_ZGD*>(user_data->kernel);
            SweepSolver_GroupSet(group_set, user_data, full_kernel);
            break;
          }
          case NEST_DNM:
            printf("Nesting DNM not supported yet.\n");
            std::exit(1);
          case NEST_NMD:
            printf("Nesting NMD not supported yet.\n");
            std::exit(1);
        }
      }
    }
  }

  return(0);
}


template <typename Kernel, typename Sigt, typename Psi, typename Rhs>
void sweep(Grid_Data_Base* grid_data, Kernel* kernel,
           int group_set, int direction_set,
           Sigt&& sigt, Psi&& psi, Rhs&& rhs)
{
  using namespace stapl;
  using namespace stapl::skeletons;

  kernel->group_set = group_set;
  kernel->direction_set = direction_set;

  auto wavefront_skeleton =
    wavefront<3>(*kernel,
      find_corner(grid_data->gd_sets()[group_set][direction_set].directions[0]),
      filter_domain_result<std::array<std::vector<double>, 3>>());

  auto size = sigt[group_set]->domain().dimensions();

  /// boundary condition views
  using stapl::skeletons::wavefront_utils::wavefront_filter_direction;
  auto bg0 = gen_bdry_ones(grid_data, group_set, direction_set,
                           wavefront_filter_direction::direction0);
  auto bg1 = gen_bdry_ones(grid_data, group_set, direction_set,
                           wavefront_filter_direction::direction1);
  auto bg2 = gen_bdry_ones(grid_data, group_set, direction_set,
                           wavefront_filter_direction::direction2);
  auto boundary_view0 = functor_view(size, bg0);
  auto boundary_view1 = functor_view(size, bg1);
  auto boundary_view2 = functor_view(size, bg2);

  // null coarsener used because view elements are coarse, as is the
  // work function.
  algorithm_executor<null_coarsener> exec;

  exec.execute<void, false>(
    wavefront_skeleton,
    *sigt[group_set],
    *psi[group_set][direction_set],
    *rhs[group_set][direction_set],
    boundary_view0,
    boundary_view1,
    boundary_view2);
}

/*----------------------------------------------------------------------
 * SweepSolverSolveDD
 *----------------------------------------------------------------------*/

template <typename FullKernel>
int SweepSolver_GroupSet (int group_set, User_Data *user_data,
                          FullKernel* full_kernel)
{
  int num_direction_sets = user_data->num_direction_sets;

  /* Use standard Diamond-Difference sweep */
  {
    BLOCK_TIMER(user_data->timing, Sweep_Kernel);
    // Sweep in each direction set
    for (int ds = 0; ds < num_direction_sets; ++ds)
    {
      sweep(full_kernel->grid_data, full_kernel, group_set, ds,
            full_kernel->grid_data->sigt(), full_kernel->grid_data->psi(),
            full_kernel->grid_data->rhs());
    }
    // cause all PARAGRAPHs to be executed
    stapl::get_executor()(stapl::execute_all);
  }
  return(0);
}

/*----------------------------------------------------------------------
 * CreateBufferInfo
 *----------------------------------------------------------------------*/

void CreateBufferInfo(User_Data *user_data)
{
  Grid_Data_Base* grid_data  = user_data->grid_data;

  int *nzones          = grid_data->nzones();
  int local_imax, local_jmax, local_kmax;
  int num_direction_sets = grid_data->gd_sets()[0].size();
  int len[6], nm[6], length;

  // get group and direction dimensionality
  int dirs_groups = user_data->num_directions_per_set
                  * user_data->num_groups_per_set;

  local_imax = nzones[0];
  local_jmax = nzones[1];
  local_kmax = nzones[2];

  /* Info for buffers used for messages sent in the x direction */
  length = local_jmax * local_kmax + 1;
  len[0] = len[1] = length * dirs_groups;

  /* Info for buffers used for messages sent in the y direction */
  length = local_imax * local_kmax + 1;
  len[2] = len[3] = length * dirs_groups;

  /* Info for buffers used for messages sent in the z direction */
  length = local_imax * local_jmax + 1;
  len[4] = len[5] = length * dirs_groups;

  for (int i=0; i<6; i++) {
    nm[i] = 0;
  }

  for (int ds=0; ds<num_direction_sets; ds++) {
    Directions *directions = grid_data->gd_sets()[0][ds].directions;
    if (directions[0].id > 0) {
      nm[0]++;
    }
    else {nm[1]++; }
    if (directions[0].jd > 0) {
      nm[2]++;
    }
    else {nm[3]++; }
    if (directions[0].kd > 0) {
      nm[4]++;
    }
    else {nm[5]++; }
  }
}
