#include <Kripke/Kernel/Kernel_3d_DZG.h>
#include "kernel_work_functions.hpp"

Kernel_3d_DZG::Kernel_3d_DZG(Grid_Data_Base* grid)
  : Kernel(NEST_DZG),
    grid_data(static_cast<Grid_Data<NEST_DZG>*>(grid))
{ }

Kernel_3d_DZG::~Kernel_3d_DZG()
{ }

struct dzg_ltimes_dir_wf
{
private:
  typedef Grid_Data<NEST_DZG> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dzg_ltimes_dir_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param ell An element of the outer ell container
  ///   ell is sliced over directions, so its index is (0,0,0,m)
  /// @param phi An element of the outer phi container
  ///   phi is non-sliced, so its index is (i,j,k,m,g)
  /// @param psi An element of the outer psi container
  ///   psi is sliced over directions, so its index is (i,j,k,g)
  //////////////////////////////////////////////////////////////////////
  template <typename LView, typename PhiView, typename PsiView>
  result_type operator()(LView&& ell, PhiView&& phi, PsiView&& psi)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;

    for (int nm_offset = 0; nm_offset < nidx; ++nm_offset) {
      double ell_d_nm = ell(0, 0, 0, nm_offset);

      // Reference has parallel loop over zones and groups
      for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
        for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
          for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
            for (int group = 0; group < num_local_groups; ++group) {
              phi(i, j, k, nm_offset, group) += ell_d_nm * psi(i, j, k, group);
            } // group
          }
        }
      } // zone - where ref parallel loop would end
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


struct dzg_ltimes_wf
{
private:
  typedef Grid_Data<NEST_DZG> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dzg_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PhiView, typename LView, typename PsiView>
  result_type operator()(PhiView&& phi, LView&& ell, PsiView&& psi)
  {
    // Loop over all Directions
    stapl::map_func(dzg_ltimes_dir_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<3>(ell),
                    stapl::make_repeat_view_nd<1>(phi),
                    stapl::make_slices_view<3>(psi));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_DZG::LTimes(void){
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
      stapl::map_func(dzg_ltimes_wf(grid_data, gset, dset),
                      *grid_data->phi()[gset],
                      *grid_data->ell()[dset],
                      *grid_data->psi()[gset][dset]);
    }
  }
}


struct dzg_lplustimes_dir_wf
{
private:
  typedef Grid_Data<NEST_DZG> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dzg_lplustimes_dir_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is sliced over directions, so its index is (0,0,0,m)
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is non-sliced, so its index is (i,j,k,m,g)
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over directions, so its index is (i,j,k,g)
  //////////////////////////////////////////////////////////////////////
  template <typename LPlusView, typename PhiOutView, typename RhsView>
  result_type
  operator()(LPlusView&& ell_plus, PhiOutView&& phi_out, RhsView&& rhs)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi_out.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;

    // Set Rhs to 0.
    multi_dimension_zero_fill(rhs);

    for (int nm_offset = 0;nm_offset < nidx;++nm_offset) {
      double ell_plus_d_n_m = ell_plus(0, 0, 0, nm_offset);

      // Reference has parallel loop over zones
      for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
        for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
          for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
            for (int group = 0; group < num_local_groups; ++group) {
              rhs(i, j, k, group) +=
                ell_plus_d_n_m * phi_out(i, j, k, nm_offset, group);
            } // group
          }
        }
      } // zone - where ref parallel loop would end
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


struct dzg_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_DZG> grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  dzg_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PhiOutView, typename LPlusView, typename RhsView>
  result_type operator()(PhiOutView&& phi_out, LPlusView&& ell_plus, RhsView&& rhs)
  {
    // Loop over Directions
    stapl::map_func(dzg_lplustimes_dir_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<3>(ell_plus),
                    stapl::make_repeat_view_nd<1>(phi_out),
                    stapl::make_slices_view<3>(rhs));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_DZG::LPlusTimes(void)
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
      stapl::map_func(dzg_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->phi_out()[gset],
                      *grid_data->ell_plus()[dset],
                      *grid_data->rhs()[gset][dset]);
    }
  }
}
