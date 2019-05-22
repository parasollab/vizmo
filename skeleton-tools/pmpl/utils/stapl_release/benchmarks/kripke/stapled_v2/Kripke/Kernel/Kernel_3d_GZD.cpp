#include<Kripke/Kernel/Kernel_3d_GZD.h>
#include "kernel_work_functions.hpp"

Kernel_3d_GZD::Kernel_3d_GZD(Grid_Data_Base* grid)
  : Kernel(NEST_GZD),
    grid_data(static_cast<Grid_Data<NEST_GZD>*>(grid))
{ }

Kernel_3d_GZD::~Kernel_3d_GZD()
{ }

struct gzd_ltimes_group_wf
{
private:
  typedef Grid_Data<NEST_GZD>  grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  gzd_ltimes_group_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param psi An element of the outer psi container
  ///   psi is sliced over groups, so its index is (i,j,k,d)
  /// @param ell An element of the outer ell container
  ///   ell is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi An element of the outer phi container
  ///   phi is sliced over groups, so its index is (i,j,k,m)
  //////////////////////////////////////////////////////////////////////
  template <typename PsiView, typename LView, typename PhiView>
  result_type operator()(PsiView&& psi, LView&& ell, PhiView&& phi)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_directions = gd_set.num_directions;

    //  loop over zones
    for (size_t i = 0; i != std::get<0>(ma_size); ++i) {
      for (size_t j = 0; j != std::get<1>(ma_size); ++j) {
        for (size_t k = 0; k != std::get<2>(ma_size); ++k) {

          // loop over directions
          for (int d = 0; d < num_local_directions; d++) {
            for (int nm_offset = 0;nm_offset < nidx;++nm_offset){
              double ell_nm_d = ell(0, 0, 0, d, nm_offset);
              phi(i, j, k, nm_offset) += ell_nm_d * psi(i, j, k, d);
            }
          }  // directions
        }
      }
    }  // zones
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct gzd_ltimes_wf
{
private:
  typedef Grid_Data<NEST_GZD>  grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  gzd_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PsiView, typename LView, typename PhiView>
  result_type operator()(PsiView&& psi, LView&& ell, PhiView&& phi)
  {
    // Loop over Groups
    invoke_ell_op<1, index_sequence<4>>(psi, ell, phi,
      gzd_ltimes_group_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_GZD::LTimes(void) {
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
      stapl::map_func(gzd_ltimes_wf(grid_data, gset, dset),
                      *grid_data->psi()[gset][dset],
                      *grid_data->ell()[dset],
                      *grid_data->phi()[gset]);
    }
  }
}


struct gzd_lplustimes_group_wf
{
private:
  typedef Grid_Data<NEST_GZD>  grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  gzd_lplustimes_group_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over groups, so its index is (i,j,k,d)
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is sliced over groups, so its index is (i,j,k,m)
  //////////////////////////////////////////////////////////////////////
  template <typename RhsView, typename LPlusView, typename PhiOutView>
  result_type operator()(RhsView&& rhs, LPlusView&& ell_plus,
                         PhiOutView&& phi_out)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi_out.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_directions = gd_set.num_directions;

    // Set Rhs to 0.
    multi_dimension_zero_fill(rhs);

    // loop over zones
    for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
      for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
        for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {

          // loop over directions
          for (int d = 0; d < num_local_directions; d++) {
            for (int nm_offset = 0;nm_offset < nidx;++nm_offset){
              double ell_plus_d_nm = ell_plus(0, 0, 0, d, nm_offset);
              rhs(i, j, k, d) += ell_plus_d_nm * phi_out(i, j, k, nm_offset);
            } // nm_offset
          } // directions
        }
      }
    } // zones
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct gzd_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_GZD>  grid_type;

  stapl::rmi_handle::reference m_grid_handle;
  grid_type*                   m_grid;
  int                          m_gset;
  int                          m_dset;

public:
  typedef void result_type;

  gzd_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename RhsView, typename LPlusView, typename PhiOutView>
  result_type operator()(RhsView&& rhs, LPlusView&& ell_plus,
                         PhiOutView&& phi_out)
  {
    // Loop over Groups
    invoke_ell_op<1, index_sequence<4>>(rhs, ell_plus, phi_out,
      gzd_lplustimes_group_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_GZD::LPlusTimes(void){
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute rhs
      stapl::map_func(gzd_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->rhs()[gset][dset],
                      *grid_data->ell_plus()[dset],
                      *grid_data->phi_out()[gset]);
    }
  }
}
