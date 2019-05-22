/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_BENCHMARKS_KRIPKE_GRID3D_H
#define STAPL_BENCHMARKS_KRIPKE_GRID3D_H

// System includes
#include <iostream>
#include <vector>

#include "GroupDirSet.hpp"
#include "Kripke/Input_Variables.h"
#include <stapl/multiarray.hpp>
#include <stapl/skeletons/map.hpp>
#include <stapl/skeletons/map_reduce.hpp>

template <typename Traversal, typename PartitionDimensions>
struct multiarray_types
{
  /// @brief Defines order in which elements of the container are linearized.
  typedef Traversal                                        D5_traversal;

  /// @brief Type of the distribution specification that will distribute the
  /// container in a balaced manner along the three dimensions that make up the
  /// spatial domain.
  typedef typename stapl::sliced_md_distribution_spec<
    PartitionDimensions, D5_traversal>::type       D5_spec;

  /// @brief Defines the dimensions of the inner containers that will be
  /// partitioned by their distribution.
  typedef PartitionDimensions                              partition_dims;

  /// @brief Defines order in which elements of the outer container are
  /// linearized.
  typedef stapl::index_sequence<2, 1, 0>                   D3_traversal;

  /// @brief Type of the distribution specification that will distribute the
  /// container in a uniform manner along all three of its dimensions.
  typedef stapl::md_distribution_spec<D3_traversal>::type  D3_spec;

  /// @brief Type of the container representing a zoneset. The 5 dimensions
  /// represent space, direction, and energy.
  typedef stapl::multiarray<5, double, D5_spec>            zoneset_cont_t;

  /// @brief Type of the container storing the collection of zonesets. The 3
  /// dimensions represent the spatial dimensions, as a zoneset is a
  /// hierarchical representation of the spatial domain.
  typedef stapl::multiarray<3, zoneset_cont_t, D3_spec>    variable_cont_t;

  /// @brief Type of the view over the container of zonesets. This view type is
  /// used for all computations on the variables in the Grid.
  typedef stapl::multiarray_view<variable_cont_t>          variable_type;
};


//////////////////////////////////////////////////////////////////////
/// @brief Define the types of the multiarray instances used for the grid
/// variables.
///
/// Each Grid variable is a composed multiarray instance.  The outer container
/// has a single element for each zoneset.  The inner containers represent the
/// phase space that includes the zones in the zoneset and all points in the
/// energy and space domain.
///
/// The default layout of the container is the ZDG layout as it matches the
/// order in which we index the multiarray (i.e., a(i, j, k, d, g)).
///
/// Traversal order is specified in reverse order of what is expected given
/// the nesting. Recall that the dimensions of multiarray are (x,y,z,d,g).
///
//////////////////////////////////////////////////////////////////////
template<Nesting_Order>
struct grid_variable_traits;


template<>
struct grid_variable_traits<NEST_ZDG>
  : public multiarray_types<
      stapl::index_sequence<4, 3, 2, 1, 0>,
      stapl::index_sequence<0, 1, 2>>
{ };


template<>
struct grid_variable_traits<NEST_ZGD>
  : public multiarray_types<
       stapl::index_sequence<4, 3, 2, 0, 1>,
       stapl::index_sequence<0, 1, 2>>
{ };


template<>
struct grid_variable_traits<NEST_DZG>
  : public multiarray_types<
      stapl::index_sequence<3, 2, 1, 4, 0>,
      stapl::index_sequence<3>>
{ };


template<>
struct grid_variable_traits<NEST_GZD>
  : public multiarray_types<
      stapl::index_sequence<3, 2, 1, 0, 4>,
      stapl::index_sequence<4>>
{ };


template<>
struct grid_variable_traits<NEST_DGZ>
  : public multiarray_types<
      stapl::index_sequence<2, 1, 0, 4, 3>,
      stapl::index_sequence<3>>
{ };


template<>
struct grid_variable_traits<NEST_GDZ>
  : public multiarray_types<
      stapl::index_sequence<2, 1, 0, 3, 4>,
      stapl::index_sequence<4>>
{ };


class Grid_Data_Base : public stapl::p_object
{
protected:
  int m_px, m_py, m_pz;

  // Number of zones in each dimension of a zoneset
  int m_ax, m_ay, m_az;

  // Number of zonesets in each dimension for the entire spatial domain
  int m_cx, m_cy, m_cz;

  // Total Number of zones in the grid on this location
  int m_num_zones;

  // Number of zones in each dimension on this location
  int m_nzones[3];

  // Neighboring locations in each dimension
  int m_mynbr[3][2];

  // Spatial grid deltas in each dimension
  // Constructed in computeGrid.
  std::vector<double> m_deltas[3];

  // Sweep index sets for each octant
  // Computed in computeSweepIndexSets
  std::vector<Grid_Sweep_Block> m_octant_extent;

  // Group/Angle sets
  std::vector<std::vector<Group_Dir_Set> > m_gd_sets;

  int m_num_moments;

  // n, m indicies for traversing ell, ell_plus of vertices
  std::vector<int> m_nm_table;

  // Store nesting value to make it available in contexts where input_variables
  // isn't available.
  Nesting_Order    m_nesting;

  void computeSweepIndexSets(void);

public:
  /*!
    \brief Build a grid from already parsed data.
  */
  Grid_Data_Base(Input_Variables *input_vars, Directions *directions);

  int num_zones(void) const
  { return m_num_zones; }

  int px(void) const
  { return m_px; }

  int py(void) const
  { return m_py; }

  int pz(void) const
  { return m_pz; }

  int ax(void) const
  { return m_ax; }

  int ay(void) const
  { return m_ay; }

  int az(void) const
  { return m_az; }

  int cx(void) const
  { return m_cx; }

  int cy(void) const
  { return m_cy; }

  int cz(void) const
  { return m_cz; }

  int nzones(int dim) const
  { return m_nzones[dim]; }

  int (&nzones(void))[3]
  { return m_nzones; }

  int (&mynbr(void))[3][2]
  { return m_mynbr; }

  std::vector<double> const& deltas(int dim) const
  { return m_deltas[dim]; }

  std::vector<Grid_Sweep_Block> const& octant_extent(void) const
  { return m_octant_extent; }

  std::vector<std::vector<Group_Dir_Set> >& gd_sets(void)
  { return m_gd_sets; }

  std::vector<std::vector<Group_Dir_Set> > const& gd_sets(void) const
  { return m_gd_sets; }

  int num_moments(void) const
  { return m_num_moments; }

  std::vector<int> const& nm_table(void) const
  { return m_nm_table; }

  std::vector<int>& nm_table(void)
  { return m_nm_table; }

  Nesting_Order nesting(void) const
  { return m_nesting; }

  void computeGrid(int dim, int num_locs, int num_zones,
         stapl::location_type loc_index, double min_coord, double max_coord);

  virtual void randomizeData(void) = 0;

  virtual void copy(Grid_Data_Base const&) = 0;

  virtual bool compare(Grid_Data_Base const&, double, bool) = 0;
};


////////////////////////////////////////////////////////////////////////////////
///
/// @brief Represents a hexahedral grid.
///
/// @tparam VariableTraits Traits class specifying traversal orders and
/// distribution specifications for the multiarrays that are data members of
/// the class.
///
////////////////////////////////////////////////////////////////////////////////
template <Nesting_Order Nest>
class Grid_Data : public Grid_Data_Base
{
public:
  typedef grid_variable_traits<Nest>                variable_types;

  typedef typename variable_types::variable_cont_t  container_type;
  typedef typename variable_types::variable_type    variable_type;

private:
  // TODO: use std::vector<std::unique_ptr<variable_type*> > for phi,phi_out,L,L+
  std::vector<variable_type*>     m_phi;      // Moments of psi
  std::vector<variable_type*>     m_phi_out;  // Scattering source (moments)
  std::vector<variable_type*>     m_ell;      // L matrix in nm_offset coordinates
  std::vector<variable_type*>     m_ell_plus; // L+ matrix in nm_offset coordinates

  std::vector<std::vector<variable_type*>> m_psi; // Solution
  std::vector<std::vector<variable_type*>> m_rhs; // RHS, source term

  // Zonal per-group cross-section
  std::vector<variable_type*>              m_sigt;

public:
  /*!
    \brief Build a grid from already parsed data.
  */
  Grid_Data(Input_Variables *input_vars, Directions *directions)
    : Grid_Data_Base(input_vars, directions)
  { }

  ~Grid_Data()
  {
    // TODO: change the type of m_phi, m_phi_out, m_ell, m_ell_plus to
    // std::vector<std::unique_ptr<variable_type*> > to simplify the destructor
    for (size_t i = 0; i != m_psi.size(); ++i)        // loop over groups
    {
      delete m_sigt[i];
      delete m_phi[i];
      delete m_phi_out[i];
      for (size_t j = 0; j != m_psi[i].size(); ++j)  // loop over directions
      {
        delete m_psi[i][j];
        delete m_rhs[i][j];

        if (i == 0) {
          delete m_ell[j];
          delete m_ell_plus[j];
        }
      }
    }
  }

  std::vector<variable_type*>& phi(void)
  { return m_phi; }

  std::vector<variable_type*> const& phi(void) const
  { return m_phi; }

  std::vector<variable_type*>& phi_out(void)
  { return m_phi_out; }

  std::vector<variable_type*> const& phi_out(void) const
  { return m_phi_out; }

  std::vector<variable_type*>& ell(void)
  { return m_ell; }

  std::vector<variable_type*> const& ell(void) const
  { return m_ell; }

  std::vector<variable_type*>& ell_plus(void)
  { return m_ell_plus; }

  std::vector<variable_type*> const& ell_plus(void) const
  { return m_ell_plus; }

  std::vector<std::vector<variable_type*>>& psi(void)
  { return m_psi; }

  std::vector<std::vector<variable_type*>> const& psi(void) const
  { return m_psi; }

  std::vector<std::vector<variable_type*>>& rhs(void)
  { return m_rhs; }

  std::vector<std::vector<variable_type*>> const& rhs(void) const
  { return m_rhs; }

  std::vector<variable_type*>& sigt(void)
  { return m_sigt; }

  std::vector<variable_type*> const& sigt(void) const
  { return m_sigt; }

  void randomizeData(void);

  bool compare(Grid_Data_Base const&, double tolerance, bool verbose);

  void copy(Grid_Data_Base const&);
};


// Work function for assigning random values to elements of inner multiarrays
struct randomize_element
{
  // value set before each map_func call to keep variables from having
  // the same values
  int unique;

  typedef void result_type;

  template<typename Multiarray>
  result_type operator()(Multiarray m)
  {
    std::tuple<int, int, int>           eidx = stapl::index_of(m);
    std::tuple<int, int, int, int, int> size = m.dimensions();

    std::mt19937_64
      gen(unique*std::get<0>(eidx)*std::get<1>(eidx)*std::get<2>(eidx));
    std::uniform_real_distribution<double> dist(0., 1.);

    std::tuple<int, int, int, int, int> idx(0,0,0,0,0);
    for (int i = 0; i != std::get<0>(size); ++i){
      std::get<0>(idx) = i;
      for (int j = 0; j != std::get<1>(size); ++j){
        std::get<1>(idx) = j;
        for (int k = 0; k != std::get<2>(size); ++k){
          std::get<2>(idx) = k;
          for (int d = 0; d != std::get<3>(size); ++d){
            std::get<3>(idx) = d;
            for (int g = 0; g != std::get<4>(size); ++g){
              std::get<4>(idx) = g;
              m[idx] = dist(gen);
            }
          }
        }
      }
    }
  }

  void define_type(stapl::typer& t)
  { t.member(unique); }
};

template <Nesting_Order Nesting>
void Grid_Data<Nesting>::randomizeData(void)
{
  unsigned int loc_id = stapl::get_location_id();
  unsigned int nlocs  = stapl::get_num_locations();

  // Generator for random values
  std::mt19937_64                gen(loc_id);
  // Distribution for random values
  std::uniform_real_distribution<double> dist(0.,1.);

  for (int d = 0; d != 3; ++d)
    for (int i = 0; i != m_deltas[d].size(); ++i)
      m_deltas[d][i] = dist(gen);

  randomize_element rand_wf;
  for (int gs = 0; gs != m_gd_sets.size(); ++gs) {
    for (int ds = 0; ds != m_gd_sets[gs].size(); ++ds) {
      rand_wf.unique += nlocs;
      stapl::map_func(rand_wf,  *m_psi[gs][ds]);
      rand_wf.unique += nlocs;
      stapl::map_func(rand_wf,  *m_rhs[gs][ds]);

      // L & L+ are stored on a per directionset basis, so they only need to
      // be processed once.
      if (gs == 0) {
        rand_wf.unique += nlocs;
        stapl::map_func(rand_wf, *m_ell[ds]);
        rand_wf.unique += nlocs;
        stapl::map_func(rand_wf, *m_ell_plus[ds]);
      }
    }
    rand_wf.unique += nlocs;
    stapl::map_func(rand_wf, *m_sigt[gs]);

    rand_wf.unique += nlocs;
    stapl::map_func(rand_wf, *m_phi[gs]);
    rand_wf.unique += nlocs;
    stapl::map_func(rand_wf, *m_phi_out[gs]);
  }
}


// work function to copy cellset multiarray
struct compare_element
{
private:
  double m_tol;
  bool   m_verbose;

public:
  std::string  name;

  typedef bool result_type;

  compare_element(double const& tol, bool verbose)
    : m_tol(tol), m_verbose(verbose)
  { }

  template<typename Multiarray, typename RefMultiarray>
  result_type operator()(Multiarray m, RefMultiarray ref)
  {
    double err       = 0.;
    int    num_wrong = 0;
    auto   size      = m.dimensions();

    std::tuple<int, int, int, int, int> idx(0,0,0,0,0);
    for (int i = 0; i != std::get<0>(size); ++i){
      std::get<0>(idx) = i;
      for (int j = 0; j != std::get<1>(size); ++j){
        std::get<1>(idx) = j;
        for (int k = 0; k != std::get<2>(size); ++k){
          std::get<2>(idx) = k;
          for (int d = 0; d != std::get<3>(size); ++d){
            std::get<3>(idx) = d;
            for (int g = 0; g != std::get<4>(size); ++g){
              std::get<4>(idx) = g;

              err = std::abs(m[idx] - ref[idx]);
              if (err > m_tol) {
                if (m_verbose) {
                  printf("%s[x=%d, y=%d, z=%d, d=%d, g=%d]: |%e - %e| = %e\n",
                         name.c_str(), i, j, k, d, g,
                         (double)m[idx], (double)ref[idx], err);
                  ++num_wrong;
                  if (num_wrong > 100)
                    return true;
                }
              }
            }
          }
        }
      }
    }
    return num_wrong != 0;
  }

  void define_type(stapl::typer& t)
  {
    t.member(m_tol);
    t.member(m_verbose);
    t.member(name);
  }
};

template <Nesting_Order Nesting>
bool Grid_Data<Nesting>::compare(Grid_Data_Base const& b_base, double tol,
                                 bool verbose)
{
  Grid_Data<Nesting> const& b(static_cast<Grid_Data<Nesting> const&>(b_base));
  bool is_diff = false;
  is_diff |= compareVector("deltas[0]", m_deltas[0], b.deltas(0), tol, verbose);
  is_diff |= compareVector("deltas[1]", m_deltas[1], b.deltas(1), tol, verbose);
  is_diff |= compareVector("deltas[2]", m_deltas[2], b.deltas(2), tol, verbose);

  compare_element comp_wf(tol, verbose);
  stapl::logical_or<bool> or_wf;
  for (unsigned int gs = 0;gs < gd_sets().size();++ gs){
    for (unsigned int ds = 0;ds < gd_sets()[gs].size();++ ds){
      std::stringstream namess;
      namess << "gdset[" << gs << "][" << ds << "]";
      comp_wf.name = namess.str()+".psi";
      is_diff |= stapl::map_reduce(comp_wf, or_wf,
                   linear_view(*m_psi[gs][ds]), linear_view(*b.psi()[gs][ds]));
      comp_wf.name = namess.str()+".rhs";
      is_diff |= stapl::map_reduce(comp_wf, or_wf,
                   linear_view(*m_rhs[gs][ds]), linear_view(*b.rhs()[gs][ds]));

      // L & L+ are stored on a per directionset basis, so they only need to
      // be processed once.
      if (gs == 0) {
        namess << "dset[" << ds << "]";
        comp_wf.name = namess.str()+".ell";
        is_diff |= stapl::map_reduce(comp_wf, or_wf,
          linear_view(*m_ell[ds]), linear_view(*b.ell()[ds]));
        comp_wf.name = namess.str()+".ell_plus";
        is_diff |= stapl::map_reduce(comp_wf, or_wf,
          linear_view(*m_ell_plus[ds]), linear_view(*b.ell_plus()[ds]));
      }
    }
    std::stringstream namess;
    namess << "gset[" << gs << "]";
    comp_wf.name = namess.str()+".sigt";
    is_diff |= stapl::map_reduce(comp_wf, or_wf,
                 linear_view(*m_sigt[gs]), linear_view(*b.sigt()[gs]));

    comp_wf.name = namess.str()+".phi";
    is_diff |= stapl::map_reduce(comp_wf, or_wf,
      linear_view(*m_phi[gs]), linear_view(*b.phi()[gs]));
    comp_wf.name = namess.str()+".phi_out";
    is_diff |= stapl::map_reduce(comp_wf, or_wf,
      linear_view(*m_phi_out[gs]), linear_view(*b.phi_out()[gs]));
  }
  return is_diff;
}


// work function to copy cellset multiarray
struct copy_element
{
  typedef void result_type;

  template<typename SourceMultiarray, typename DestinationMultiarray>
  result_type operator()(SourceMultiarray source, DestinationMultiarray dest)
  {
    auto size = source.dimensions();
    std::tuple<int, int, int, int, int> idx(0,0,0,0,0);
    for (int i = 0; i != std::get<0>(size); ++i){
      std::get<0>(idx) = i;
      for (int j = 0; j != std::get<1>(size); ++j){
        std::get<1>(idx) = j;
        for (int k = 0; k != std::get<2>(size); ++k){
          std::get<2>(idx) = k;
          for (int d = 0; d != std::get<3>(size); ++d){
            std::get<3>(idx) = d;
            for (int g = 0; g != std::get<4>(size); ++g){
              std::get<4>(idx) = g;
              dest[idx] = source[idx];
            }
          }
        }
      }
    }
  }
};

template <Nesting_Order Nesting>
void Grid_Data<Nesting>::copy(Grid_Data_Base const& b_base)
{
  Grid_Data<Nesting> const& b(static_cast<Grid_Data<Nesting> const&>(b_base));
  for (int d = 0; d != 3; ++d)
    this->m_deltas[d] = b.deltas(d);

  copy_element copy_wf;
  for (unsigned int gs = 0; gs != gd_sets().size(); ++gs) {
    for (unsigned int ds = 0; ds != gd_sets()[gs].size(); ++ds){
      stapl::map_func(copy_wf, *b.psi()[gs][ds], *m_psi[gs][ds]);
      stapl::map_func(copy_wf, *b.rhs()[gs][ds], *m_rhs[gs][ds]);

      // L & L+ are stored on a per directionset basis, so they only need to
      // be processed once.
      if (gs == 0) {
        stapl::map_func(copy_wf, *b.ell()[ds], *m_ell[ds]);
        stapl::map_func(copy_wf, *b.ell_plus()[ds], *m_ell_plus[ds]);
      }
    }
    stapl::map_func(copy_wf, *b.sigt()[gs], *m_sigt[gs]);

    stapl::map_func(copy_wf, *b.phi()[gs], *m_phi[gs]);
    stapl::map_func(copy_wf, *b.phi_out()[gs], *m_phi_out[gs]);
  }
}
#endif
