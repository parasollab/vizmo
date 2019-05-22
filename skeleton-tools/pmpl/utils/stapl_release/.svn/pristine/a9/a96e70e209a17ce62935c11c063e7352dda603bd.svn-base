#include <Kripke/Kernel/Kernel_3d_ZGD.h>
#include "kernel_work_functions.hpp"

Kernel_3d_ZGD::Kernel_3d_ZGD(Grid_Data_Base* grid)
  : Kernel(NEST_ZGD),
    grid_data(static_cast<Grid_Data<NEST_ZGD>*>(grid))
{ }

Kernel_3d_ZGD::~Kernel_3d_ZGD()
{ }

struct zgd_ltimes_zset_wf
{
private:
  typedef Grid_Data<NEST_ZGD>   grid_type;

  stapl::rmi_handle::reference  m_grid_handle;
  grid_type*                    m_grid;
  int                           m_gset;
  int                           m_dset;

public:
  typedef void result_type;

  zgd_ltimes_zset_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid), m_gset(gset),
      m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in discrete to moments transformation
  /// @param psi An element of the outer psi container
  ///   psi is sliced over zone sets, so its index is (d,g)
  /// @param ell An element of the outer ell container
  ///   ell is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi An element of the outer phi container
  ///   phi is sliced over zone sets, so its index is (m,g)
  //////////////////////////////////////////////////////////////////////
  template <typename PsiView, typename LView, typename PhiView>
  result_type operator()(PsiView&& psi, LView&& ell, PhiView&& phi)
  {
    // Outer parameters
    int nidx = m_grid->nm_table().size();

    Group_Dir_Set &gd_set = m_grid->gd_sets()[m_gset][m_dset];

    // Get dimensioning
    int num_local_groups = gd_set.num_groups;
    int num_local_directions = gd_set.num_directions;

    // no loop over zones because the dimension size is 1
    // loop over groups
    for (int group = 0; group < num_local_groups; ++group) {
      // loop over directions
      for (int d = 0; d < num_local_directions; ++d) {
        for (int nm_offset = 0; nm_offset < nidx; ++nm_offset) {
          phi(nm_offset, group) += ell(0, 0, 0, d, nm_offset) * psi(d, group);
        }  // nm_offset
      }  // directions
    }  // groups
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};


struct zgd_ltimes_wf
{
private:
  typedef Grid_Data<NEST_ZGD>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zgd_ltimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename PsiView, typename LView, typename PhiView>
  result_type operator()(PsiView&& psi, LView&& ell, PhiView&& phi)
  {
    // loop over zone sets
    invoke_ell_op<3, index_sequence<0,1,2>>(psi, ell, phi,
      zgd_ltimes_zset_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_ZGD::LTimes(void){
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
      stapl::map_func(zgd_ltimes_wf(grid_data, gset, dset),
                      *grid_data->psi()[gset][dset],
                      *grid_data->ell()[dset],
                      *grid_data->phi()[gset]);
    }
  }
}

struct zgd_lplustimes_zset_wf
{
private:
  typedef Grid_Data<NEST_ZGD>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zgd_lplustimes_zset_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid), m_gset(gset),
      m_dset(dset)
  { }

  //////////////////////////////////////////////////////////////////////
  /// @brief work function used in moments to discrete transformation
  /// @param rhs An element of the outer rhs container
  ///   rhs is sliced over zone sets, so its index is (d,g)
  /// @param ell_plus An element of the outer ell_plus container
  ///   ell_plus is non-sliced, so its index is (0,0,0,d,m)
  /// @param phi_out An element of the outer phi_out container
  ///   phi_out is sliced over zone sets, so its index is (m,g)
  //////////////////////////////////////////////////////////////////////
  template <typename RhsView, typename LPlusView, typename PhiOutView>
  result_type
  operator()(RhsView&& rhs, LPlusView&& ell_plus, PhiOutView&& phi_out)
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
    // loop over groups
    for (int group = 0; group < num_local_groups; ++group) {
      // loop over directions
      for (int d = 0; d < num_local_directions; d++) {
        for (int nm_offset = 0;nm_offset < nidx;++nm_offset) {
          rhs(d, group) +=
            ell_plus(0, 0, 0, d, nm_offset) * phi_out(nm_offset, group);
        } // nm_offset
      } // directions
    } // groups
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

struct zgd_lplustimes_wf
{
private:
  typedef Grid_Data<NEST_ZGD>     grid_type;

  stapl::rmi_handle::reference    m_grid_handle;
  grid_type*                      m_grid;
  int                             m_gset;
  int                             m_dset;

public:
  typedef void result_type;

  zgd_lplustimes_wf(grid_type* grid, int gset, int dset)
    : m_grid_handle(grid->get_rmi_handle()), m_grid(grid),
      m_gset(gset), m_dset(dset)
  { }

  template <typename RhsView, typename LPlusView, typename PhiOutView>
  result_type operator()(RhsView&& rhs, LPlusView&& ell_plus,
                         PhiOutView&& phi_out)
  {
    // loop over zone sets
    invoke_ell_op<3, index_sequence<0,1,2>>(rhs, ell_plus, phi_out,
      zgd_lplustimes_zset_wf(m_grid, m_gset, m_dset));
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_grid_handle);
    t.transient(m_grid, stapl::resolve_handle<grid_type>(m_grid_handle));
    t.member(m_gset);
    t.member(m_dset);
  }
};

void Kernel_3d_ZGD::LPlusTimes(void){
  // loop over group sets
  int num_group_sets = grid_data->gd_sets().size();
  for (int gset = 0; gset < num_group_sets; gset++)
  {
    // loop over direction sets
    int num_direction_sets = grid_data->gd_sets()[gset].size();
    for (int dset = 0; dset < num_direction_sets; dset++)
    {
      // Compute rhs
      stapl::map_func(zgd_lplustimes_wf(grid_data, gset, dset),
                      *grid_data->rhs()[gset][dset],
                      *grid_data->ell_plus()[dset],
                      *grid_data->phi_out()[gset]);
    }
  }
}
