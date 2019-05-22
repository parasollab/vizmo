#include <Kripke/Kernel/Kernel_3d_DGZ.h>
#include "kernel_work_functions.hpp"

Kernel_3d_DGZ::Kernel_3d_DGZ(Grid_Data_Base* grid)
  : Kernel(NEST_DGZ),
    grid_data(static_cast<Grid_Data<NEST_DGZ>*>(grid))
{ }

Kernel_3d_DGZ::~Kernel_3d_DGZ()
{ }

struct dgz_ltimes_dir_wf
{
private:
  typedef Grid_Data<NEST_DGZ> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dgz_ltimes_dir_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param psi An element of the outer psi container
  ///   psi is sliced over directions, so its index is (i,j,k,g)
  /// @param phi An element of the outer phi container
  ///   phi is non-sliced, so its index is (i,j,k,m,g)
  /// @param ell An element of the outer ell container
  ///   ell is sliced over directions, so its index is (0,0,0,m)
  //////////////////////////////////////////////////////////////////////
  template <typename PsiView, typename PhiView, typename LView>
  result_type operator()(PsiView&& psi, PhiView&& phi, LView&& ell)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;

    /* 3D Cartesian Geometry */
    for (int nm_offset = 0; nm_offset < nidx; ++nm_offset) {
      double ell_d_nm = ell(0, 0, 0, nm_offset);

      // reference has parallel loop over groups and zones
      for (int group = 0; group < num_local_groups; ++group) {
        for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
          for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
            for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
              phi(i, j, k, nm_offset, group) += ell_d_nm * psi(i, j, k, group);
            }
          }
        }
      } // group - where ref parallel loop would end
    } // nm_offset
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct dgz_ltimes_wf
{
private:
  typedef Grid_Data<NEST_DGZ> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dgz_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PsiView, typename PhiView, typename LView>
  result_type operator()(PsiView&& psi, PhiView&& phi, LView&& ell)
  {
    // Loop over all Directions
    invoke_ell_op<1, index_sequence<3>>(psi, phi, ell,
      dgz_ltimes_dir_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_DGZ::LTimes(void){
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
      stapl::map_func(dgz_ltimes_wf(grid_data, gset, dset),
                      *grid_data->psi()[gset][dset],
                      *grid_data->phi()[gset],
                      *grid_data->ell()[dset]);
    }
  }
}


struct dgz_lplustimes_dir_wf
{
private:
  typedef Grid_Data<NEST_DGZ> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dgz_lplustimes_dir_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over directions, so its index is (i,j,k,g)
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is non-sliced, so its index is (i,j,k,m,g)
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is sliced over directions, so its index is (0,0,0,m)
  //////////////////////////////////////////////////////////////////////
  template <typename RhsView, typename PhiOutView, typename LPlusView>
  result_type
  operator()(RhsView&& rhs, PhiOutView&& phi_out, LPlusView&& ell_plus)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi_out.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;

    // Set Rhs to 0.
    multi_dimension_zero_fill(rhs);

    for (int nm_offset = 0;nm_offset < nidx;++nm_offset)
    {
      double ell_plus_d_n_m = ell_plus(0, 0, 0, nm_offset);

      // reference has parallel loop over groups and zones
      for (int group = 0; group < num_local_groups; ++group) {
        for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
          for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
            for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
              rhs(i, j, k, group) +=
                ell_plus_d_n_m * phi_out(i, j, k, nm_offset, group);
            }
          }
        } // zone
      } // group - ref parallel loop ended here
    } // num_offset
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct dgz_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_DGZ> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dgz_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename RhsView, typename PhiOutView, typename LPlusView>
  result_type operator()(RhsView&& rhs, PhiOutView&& phi_out,
                         LPlusView&& ell_plus)
  {
    // Loop over Directions
    invoke_ell_op<1, index_sequence<3>>(rhs, phi_out, ell_plus,
      dgz_lplustimes_dir_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_DGZ::LPlusTimes(void)
{
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute rhs
      stapl::map_func(dgz_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->rhs()[gset][dset],
                      *grid_data->phi_out()[gset],
                      *grid_data->ell_plus()[dset]);
    }
  }
}
