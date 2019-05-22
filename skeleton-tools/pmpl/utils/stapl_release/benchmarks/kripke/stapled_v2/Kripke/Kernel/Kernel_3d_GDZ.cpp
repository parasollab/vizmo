#include<Kripke/Kernel/Kernel_3d_GDZ.h>
#include "kernel_work_functions.hpp"

Kernel_3d_GDZ::Kernel_3d_GDZ(Grid_Data_Base* grid)
  : Kernel(NEST_GDZ),
    grid_data(static_cast<Grid_Data<NEST_GDZ>*>(grid))
{ }

Kernel_3d_GDZ::~Kernel_3d_GDZ()
{ }

struct gdz_ltimes_group_wf
{
private:
  typedef Grid_Data<NEST_GDZ>   grid_type;

  stapl::rmi_handle::reference  m_grid_handle;
  grid_type*           m_grid;
  int                  m_gset;
  int                  m_dset;

public:
  typedef void result_type;

  gdz_ltimes_group_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param phi An element of the outer phi container
  ///   phi is sliced over groups, so its index is (i,j,k,m)
  /// @param ell An element of the outer ell container
  ///   ell is non-sliced, so its index is (0,0,0,d,m)
  /// @param psi An element of the outer psi container
  ///   psi is sliced over groups, so its index is (i,j,k,d)
  //////////////////////////////////////////////////////////////////////
  template <typename PhiView, typename LView, typename PsiView>
  result_type operator()(PhiView&& phi, LView&& ell, PsiView&& psi)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_directions = gd_set.num_directions;

    for (int nm_offset = 0; nm_offset < nidx; ++nm_offset) {
      // loop over directions
      for (int d = 0; d < num_local_directions; d++) {
        double ell_nm_d = ell(0, 0, 0, d, nm_offset);

        // reference has parallel loop over zones
        for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
          for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
            for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
              phi(i, j, k, nm_offset) += ell_nm_d * psi(i, j, k, d);
            }
          }
        } // zones
      } // directions
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

struct gdz_ltimes_wf
{
private:
  typedef Grid_Data<NEST_GDZ>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  gdz_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PhiView, typename LView, typename PsiView>
  result_type operator()(PhiView&& phi, LView&& ell, PsiView&& psi)
  {
    // Loop over Groups
    stapl::map_func(gdz_ltimes_group_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<4>(phi),
                    stapl::make_repeat_view(ell),
                    stapl::make_slices_view<4>(psi));
  }
  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_GDZ::LTimes(void) {
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
      stapl::map_func(gdz_ltimes_wf(grid_data, gset, dset),
                      *grid_data->phi()[gset],
                      *grid_data->ell()[dset],
                      *grid_data->psi()[gset][dset]);
    }
  }
}


struct gdz_lplustimes_group_wf
{
private:
  typedef Grid_Data<NEST_GDZ>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  gdz_lplustimes_group_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is sliced over groups, so its index is (i,j,k,m)
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is non-sliced, so its index is (0,0,0,d,m)
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over groups, so its index is (i,j,k,d)
  //////////////////////////////////////////////////////////////////////
  template <typename PhiOutView, typename LPlusView, typename RhsView>
  result_type
  operator()(PhiOutView&& phi_out, LPlusView&& ell_plus, RhsView&& rhs)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();
    auto ma_size = phi_out.dimensions();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_directions = gd_set.num_directions;

    // Set Rhs to 0.
    multi_dimension_zero_fill(rhs);

    // loop over directions
    for (int d = 0; d < num_local_directions; d++) {
      for (int nm_offset = 0;nm_offset < nidx;++nm_offset) {
        double ell_plus_d_n_m = ell_plus(0, 0, 0, d, nm_offset);

        // reference has parallel loop over zones
        for (std::size_t i = 0; i != std::get<0>(ma_size); ++i) {
          for (std::size_t j = 0; j != std::get<1>(ma_size); ++j) {
            for (std::size_t k = 0; k != std::get<2>(ma_size); ++k) {
              rhs(i, j, k, d) += ell_plus_d_n_m * phi_out(i, j, k, nm_offset);
            }
          }
        } // zone
      } // mn_offset
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


struct gdz_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_GDZ>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  gdz_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PhiOutView, typename LPlusView, typename RhsView>
  result_type
  operator()(PhiOutView&& phi_out, LPlusView&& ell_plus, RhsView&& rhs)
  {
    // Loop over Groups
    stapl::map_func(gdz_lplustimes_group_wf(m_grid, m_gset, m_dset),
                    stapl::make_slices_view<4>(phi_out),
                    stapl::make_repeat_view(ell_plus),
                    stapl::make_slices_view<4>(rhs));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


void Kernel_3d_GDZ::LPlusTimes(void){
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute rhs
      stapl::map_func(gdz_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->phi_out()[gset],
                      *grid_data->ell_plus()[dset],
                      *grid_data->rhs()[gset][dset]);
    }
  }
}
