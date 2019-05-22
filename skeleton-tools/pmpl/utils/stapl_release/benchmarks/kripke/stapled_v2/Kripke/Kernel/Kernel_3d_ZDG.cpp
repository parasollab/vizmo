#include<Kripke/Kernel/Kernel_3d_ZDG.h>
#include<Kripke/User_Data.h>
#include "kernel_work_functions.hpp"

Kernel_3d_ZDG::Kernel_3d_ZDG(Grid_Data_Base* grid)
  : Kernel(NEST_ZDG),
    grid_data(static_cast<Grid_Data<NEST_ZDG>*>(grid))
{ }

Kernel_3d_ZDG::~Kernel_3d_ZDG()
{ }

struct zdg_ltimes_zset_wf
{
private:
  typedef Grid_Data<NEST_ZDG>   grid_type;

  stapl::rmi_handle::reference  m_grid_handle;
  grid_type*                    m_grid;
  int                           m_gset;
  int                           m_dset;

public:
  typedef void result_type;

  zdg_ltimes_zset_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid), m_gset(gset),
      m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param ell An element of the outer ell container
  ///   ell is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi An element of the outer phi container
  ///   phi is sliced over zone sets, so its index is (m,g)
  /// @param psi An element of the outer psi container
  ///   psi is sliced over zone sets, so its index is (d,g)
  //////////////////////////////////////////////////////////////////////
  template <typename PhiView, typename LView, typename PsiView>
  result_type operator()(PhiView&& phi, LView&& ell, PsiView&& psi)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;
    int num_local_directions = gd_set.num_directions;

    // no loop over zones because the dimension size is 1
    // loop over directions
    for (int d = 0; d < num_local_directions; ++d) {
      for (int nm_offset = 0; nm_offset < nidx; ++nm_offset) {
        double ell_d_nm = ell(0, 0, 0, d, nm_offset);

        // loop over groups
        for (int group = 0; group < num_local_groups; ++group) {
          phi(nm_offset, group) += ell_d_nm * psi(d, group);
        }  // groups
      }  // nm_offset
    }  // directions
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct zdg_ltimes_wf
{
private:
  typedef Grid_Data<NEST_ZDG>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zdg_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PhiView, typename LView, typename PsiView>
  result_type operator()(PhiView&& phi, LView&& ell, PsiView&& psi)
  {
    // loop over zone sets
    // TODO: use coarsening map_func when the multi-dimension slices_view and
    // repeat_view are correctly processed
    stapl::map_func<stapl::skeletons::tags::no_coarsening>(
                    zdg_ltimes_zset_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<0,1,2>(phi),
                    stapl::make_repeat_view_nd<3>(ell),
                    stapl::make_slices_view<0,1,2>(psi));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_ZDG::LTimes(void) {
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // Clear phi
    stapl::map_func(zero_fill(), *grid_data->phi()[gset]);

    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute L
      stapl::map_func(zdg_ltimes_wf(grid_data, gset, dset),
                      *grid_data->phi()[gset],
                      *grid_data->ell()[dset],
                      *grid_data->psi()[gset][dset]);
    }
  }
}


struct zdg_lplustimes_zset_wf
{
private:
  typedef Grid_Data<NEST_ZDG>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zdg_lplustimes_zset_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid), m_gset(gset),
      m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is sliced over zone sets, so its index is (m,g)
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over zone sets, so its index is (d,g)
  //////////////////////////////////////////////////////////////////////
  template <typename PhiOutView, typename LPlusView, typename RhsView>
  result_type
  operator()(PhiOutView&& phi_out, LPlusView&& ell_plus, RhsView&& rhs)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;
    int num_local_directions = gd_set.num_directions;

    // Set Rhs to 0.
    multi_dimension_zero_fill(rhs);

    // no loop over zones because the dimension size is 1
    // loop over directions
    for (int d = 0; d < num_local_directions; d++) {
      for (int nm_offset = 0;nm_offset < nidx;++nm_offset) {
        double ell_plus_d_n_m = ell_plus(0, 0, 0, d, nm_offset);

        // loop over groups
        for (int group = 0; group < num_local_groups; ++group) {
          rhs(d, group) += ell_plus_d_n_m * phi_out(nm_offset, group);
        } // groups
      } // nm_offset
    } // directions
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct zdg_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_ZDG>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zdg_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }


  template <typename PhiOutView, typename LPlusView, typename RhsView>
  result_type
  operator()(PhiOutView&& phi_out, LPlusView&& ell_plus, RhsView&& rhs)
  {
    // loop over zone sets
    // TODO: use coarsening map_func when the multi-dimension slices_view and
    // repeat_view are correctly processed
    stapl::map_func<stapl::skeletons::tags::no_coarsening>(
                    zdg_lplustimes_zset_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<0,1,2>(phi_out),
                    stapl::make_repeat_view_nd<3>(ell_plus),
                    stapl::make_slices_view<0,1,2>(rhs));

  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_ZDG::LPlusTimes(void) {
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute rhs
      stapl::map_func(zdg_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->phi_out()[gset],
                      *grid_data->ell_plus()[dset],
                      *grid_data->rhs()[gset][dset]);

    }
  }
}
