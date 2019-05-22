#ifndef KRIPKE_KERNEL_3D_DGZ_H__
#define KRIPKE_KERNEL_3D_DGZ_H__

#include<Kripke/Kernel.h>
#include<Grid.h>
#include "common_kernel_functions.h"
#include <stapl/views/slices_view.hpp>
#include <stapl/skeletons/transformations/optimizers/default.hpp>
#include <stapl/skeletons/transformations/optimizers/wavefront.hpp>
#include "../../sweep_filters.hpp"

class Kernel_3d_DGZ
  : public Kernel
{
public:
  // Grid is needed to access metadata (e.g. gd_sets) stored on it.
  Grid_Data<NEST_DGZ>* grid_data;

  Kernel_3d_DGZ(Grid_Data_Base*);
  virtual ~Kernel_3d_DGZ();

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


template<int OuterIdx, int InnerIdx>
class dgz_z_boundary_gen
{
private:
  typedef stapl::immutable_shared<std::vector<double>> is_t;

  is_t m_plane;
  int  m_idx_max;
  int  m_num_directions;
  int  m_num_groups;
  int  m_direction;
  int  m_group;

public:
  typedef std::tuple<std::size_t, std::size_t, std::size_t> index_type;
  typedef double                                            result_type;

  dgz_z_boundary_gen(is_t const& plane, int idx_max,
                     int num_directions, int num_groups,
                     int direction, int group)
    : m_plane(plane), m_idx_max(idx_max),
      m_num_directions(num_directions), m_num_groups(num_groups),
      m_direction(direction), m_group(group)
  { }

  result_type operator()(index_type const& idx) const
  {
    const int plane_idx =
     ((std::get<InnerIdx>(idx) * (m_idx_max)) + std::get<OuterIdx>(idx))
       * m_num_directions * m_num_groups
     + (m_direction * m_num_groups)
     + m_group;

    return m_plane.get()[plane_idx];
  }

  void define_type(typer &t)
  {
    t.member(m_plane);
    t.member(m_idx_max);
    t.member(m_num_directions);
    t.member(m_num_groups);
    t.member(m_direction);
    t.member(m_group);
  }
}; // class dgz_z_boundary_gen


template<int OuterIdx, int InnerIdx>
class dgz_g_boundary_gen
{
private:
  typedef stapl::immutable_shared<std::vector<double>> is_t;

  is_t                                              m_plane;
  int                                               m_idx_max;
  std::tuple<std::size_t, std::size_t, std::size_t> m_dimensions;
  int                                               m_num_directions;
  int                                               m_num_groups;
  int                                               m_direction;

public:
  // index is the group
  using index_type  = std::size_t;
  using result_type =
    typename functor_view_type<dgz_z_boundary_gen<OuterIdx, InnerIdx>, 3>::type;

  dgz_g_boundary_gen(is_t const& plane, int idx_max,
                     std::tuple<std::size_t, std::size_t, std::size_t> dimensions,
                     int num_directions, int num_groups, int direction)
    : m_plane(plane), m_idx_max(idx_max), m_dimensions(dimensions),
      m_num_directions(num_directions), m_num_groups(num_groups),
      m_direction(direction)
  { }

  result_type operator()(index_type const& idx) const
  {
    return functor_view(
      m_dimensions,
      dgz_z_boundary_gen<OuterIdx, InnerIdx>
        (m_plane, m_idx_max, m_num_directions, m_num_groups, m_direction, idx)
    );
  }

  void define_type(typer &t)
  {
    t.member(m_plane);
    t.member(m_idx_max);
    t.member(m_dimensions);
    t.member(m_num_directions);
    t.member(m_num_groups);
    t.member(m_direction);
  }
}; // class dgz_g_boundary_gen


template<int OuterIdx, int InnerIdx>
class dgz_d_boundary_gen
{
private:
  typedef stapl::immutable_shared<std::vector<double>> is_t;

  is_t                                              m_plane;
  int                                               m_idx_max;
  std::tuple<std::size_t, std::size_t, std::size_t> m_dimensions;
  int                                               m_num_directions;
  int                                               m_num_groups;

public:
  // index is the direction
  using index_type  = std::size_t;
  using result_type =
    typename functor_view_type<dgz_g_boundary_gen<OuterIdx, InnerIdx>, 1>::type;

  dgz_d_boundary_gen(is_t const& plane, int idx_max,
                     std::tuple<std::size_t, std::size_t, std::size_t> dimensions,
                     int num_directions, int num_groups)
    : m_plane(plane),  m_idx_max(idx_max), m_dimensions(dimensions),
      m_num_directions(num_directions), m_num_groups(num_groups)
  { }

  result_type operator()(index_type const& idx) const
  {
    return functor_view(
      m_num_groups,
      dgz_g_boundary_gen<OuterIdx, InnerIdx>
        (m_plane, m_idx_max, m_dimensions, m_num_directions, m_num_groups, idx)
    );
  }

  void define_type(typer& t)
  {
    t.member(m_plane);
    t.member(m_idx_max);
    t.member(m_dimensions);
    t.member(m_num_directions);
    t.member(m_num_groups);
  }
}; // class dgz_d_boundary_gen

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief The optimizer used for the wavefront skeleton in the DGZ
/// kernel.
///
/// @tparam Tag the type of the zip skeleton
/// @tparam arity the number of inputs to the zip skeleton
//////////////////////////////////////////////////////////////////////
template <>
struct optimizer<tags::wavefront<3>,
                 ::tags::kripke<NEST_DGZ, Nesting::Zone>>
{
  using result_type = std::array<std::vector<std::vector<double>>, 3>;

  template<typename R, typename S, typename Arg0, typename... Args>
  static result_type
  execute(S&& s, Arg0&& arg0, Args&&... args)
  {

    using boundary_type = std::array<double, 3>;
    using opt_t = optimizer<tags::wavefront<3>, tags::sequential_execution>;

    auto&& out_vals =
      opt_t::template execute<lightweight_multiarray<boundary_type, 3>>(
        std::forward<S>(s),
        std::forward<Arg0>(arg0), std::forward<Args>(args)...);

    // putting the result in the outgoing boundaries
    auto size = arg0.dimensions();
    auto&& out_vals_vw = make_multiarray_view(out_vals);

    result_type result;

    // extract i outbound face
    result[0].resize(std::get<1>(size));

    for (int j = 0; j < std::get<1>(size); ++j)
    {
      result[0][j].resize(std::get<2>(size));

      for (int k = 0; k < std::get<2>(size); ++k)
        result[0][j][k] = out_vals_vw(std::get<0>(size)-1, j, k)[0];
    }

    // extract j outbound face
    result[1].resize(std::get<0>(size));

    for (int i = 0; i < std::get<0>(size); ++i)
    {
      result[1][i].resize(std::get<2>(size));

      for (int k = 0; k < std::get<2>(size); ++k)
        result[1][i][k] = out_vals_vw(i, std::get<1>(size)-1, k)[1];
    }

    // extract k outbound face
    result[2].resize(std::get<0>(size));

    for (int i = 0; i < std::get<0>(size); ++i)
    {
      result[2][i].resize(std::get<1>(size));

      for (int j = 0; j < std::get<1>(size); ++j)
        result[2][i][j] = out_vals_vw(i, j, std::get<2>(size)-1)[2];
    }

    return result;
  }
};

//////////////////////////////////////////////////////////////////////
/// @brief The optimizer used for the zip skeleton in the DGZ kernel is
/// invoked by calling the @c default_execution execution strategy.
///
/// The value 'false' in default_execution denotes that the given
/// skeleton is not reducing the input to a single value.
///
/// @tparam Tag the type of the zip skeleton
/// @tparam arity the number of inputs to the zip skeleton
//////////////////////////////////////////////////////////////////////
template <typename Tag, int arity>
struct optimizer<tags::zip<Tag, arity>,
                 ::tags::kripke<NEST_DGZ, Nesting::Group>>
  : public optimizer<tags::zip<Tag, arity>, tags::sequential_execution>
{
  using result_type = std::vector<
                         std::array<std::vector<std::vector<double>>, 3>>;
};

} // namespace optimizers
} // namespace stapl
} // namespace skeletons

struct dgz_wf
{
  template<typename Sigt, typename Psi, typename Rhs,
           typename IPlane, typename JPlane, typename KPlane,
           typename GridData, typename Result>
  void operator()(Sigt& sigt_sliced, Psi& psi_sliced, Rhs& rhs_sliced,
                IPlane const& i_plane, JPlane const& j_plane,
                KPlane const& k_plane, GridData* grid_data,
                int direction_set, int group_set,
                std::tuple<size_t, size_t, size_t> cellset_id,
                promise<Result> p)
  {
    Group_Dir_Set& gd_set = grid_data->gd_sets()[group_set][direction_set];

    const int num_directions  = gd_set.num_directions;
    const int num_groups      = gd_set.num_groups;
    const auto dims           = psi_sliced[0][0].dimensions();

    dgz_d_boundary_gen<1,2> bg0(i_plane, std::get<1>(dims),
                                dims, num_directions, num_groups);

    dgz_d_boundary_gen<0,2> bg1(j_plane, std::get<0>(dims),
                                dims, num_directions, num_groups);

    dgz_d_boundary_gen<0,1> bg2(k_plane, std::get<0>(dims),
                                dims, num_directions, num_groups);

    using stapl::skeletons::wrap;
    auto dgz_skeleton =
      zip<6>(wrap<tags::kripke<NEST_DGZ, Nesting::Group>>(
        zip<6>(wrap<tags::kripke<NEST_DGZ, Nesting::Zone>>(
          wavefront<3>(
           make_functional_diamond_difference_wf(
             grid_data, psi_sliced[0][0], group_set, direction_set, cellset_id),
           find_corner(gd_set.directions[0]),
           filter_domain_result<std::array<double, 3>>())))));

    using boundary_type =
      std::vector<std::array<std::vector<std::vector<double>>, 3>>;

    stapl::array<boundary_type> out_vals_ct(sigt_sliced.size());

    auto out_vals_vw = make_array_view(out_vals_ct);
    auto size = psi_sliced.size();

    algorithm_executor<stapl::multiview_coarsener<true>>().execute(
     stapl::skeletons::coarse(sink<boundary_type>(dgz_skeleton)),
       psi_sliced, rhs_sliced, sigt_sliced,
       functor_view(size, bg0), functor_view(size, bg1), functor_view(size, bg2),
       out_vals_vw);

    stapl::do_once([&] {
      Result result;

      result[0].resize(i_plane.get().size());
      result[1].resize(j_plane.get().size());
      result[2].resize(k_plane.get().size());

      const int local_imax      = std::get<0>(dims);
      const int local_jmax      = std::get<1>(dims);

      // outgoing i_plane
      for (int j = 0; j < std::get<1>(dims); ++j)
        for (int k = 0; k < std::get<2>(dims); ++k)
          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
            {
             const int plane_idx =
               (k * local_jmax + j) * num_directions * num_groups
                + (direction * num_groups) + group;

             result[0][plane_idx] = out_vals_vw[direction][group][0][j][k];
            }

      // outgoing j_plane
      for (int i = 0; i < std::get<0>(dims); ++i)
        for (int k = 0; k < std::get<2>(dims); ++k)
          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
            {
             const int plane_idx =
               (k * local_imax + i) * num_directions * num_groups
                + (direction * num_groups) + group;

              result[1][plane_idx] = out_vals_vw[direction][group][1][i][k];
            }

      // outgoing k_plane
      for (int i = 0; i < std::get<0>(dims); ++i)
        for (int j = 0; j < std::get<1>(dims); ++j)
          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
            {
              const int plane_idx =
                (j * local_imax + i) * num_directions * num_groups
                + (direction * num_groups) + group;

              result[2][plane_idx] = out_vals_vw[direction][group][2][i][j];
            }

      p.set_value(std::move(result));
    });
  }
}; // struct dgz_wf


template<typename SigtView, typename PsiView, typename RhsView,
         typename IPlane, typename JPlane, typename KPlane>
Kernel::result_type
Kernel_3d_DGZ::operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
                          IPlane const& i_plane_in,
                          JPlane const& j_plane_in,
                          KPlane const& k_plane_in)
{
  return invoke_kernel<
    Kernel::result_type, index_sequence<3,4>, index_sequence<0>
  >(sigt, psi, rhs, i_plane_in, j_plane_in, k_plane_in,
    this->direction_set, this->group_set, this->grid_data,
    index_of(psi), dgz_wf());
}

#endif
