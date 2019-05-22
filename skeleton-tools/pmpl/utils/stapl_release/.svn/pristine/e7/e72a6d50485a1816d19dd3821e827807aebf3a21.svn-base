#ifndef KRIPKE_KERNEL_3D_ZGD_H__
#define KRIPKE_KERNEL_3D_ZGD_H__

#include <Kripke/Kernel.h>
#include <Grid.h>
#include "common_kernel_functions.h"
#include "kernel_work_functions.hpp"
#include <stapl/skeletons/transformations/optimizers/default.hpp>
#include <stapl/skeletons/transformations/coarse/wavefront.hpp>
#include "../../sweep_filters.hpp"

#include <vector>
#include <array>


class Kernel_3d_ZGD
  : public Kernel
{
public:
  // Grid is needed to access metadata (e.g. gd_sets) stored on it.
  Grid_Data<NEST_ZGD>* grid_data;

  Kernel_3d_ZGD(Grid_Data_Base* grid);

  virtual ~Kernel_3d_ZGD();

  void LTimes(void);
  void LPlusTimes(void);

  template<typename SigtView, typename PsiView, typename RhsView,
           typename IPlane, typename JPlane, typename KPlane>
  Kernel::result_type
  operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
             IPlane const& i_plane_in, JPlane const& j_plane_in,
             KPlane const& k_plane_in);

  void define_type(typer& t)
  {
    t.base<Kernel>(*this);
    t.member(grid_data);
  }
};

namespace stapl {
namespace skeletons {
namespace optimizers {

template <typename SkeletonTag, typename ExecutionTag>
struct optimizer;

//////////////////////////////////////////////////////////////////////
/// @brief The optimizer used for the zip skeleton in the ZGD kernel is
/// invoked by calling the @c sequential execution strategy.
///
/// @tparam Tag the type of the zip skeleton
/// @tparam arity the number of inputs to the zip skeleton
//////////////////////////////////////////////////////////////////////
template <typename Tag, int arity>
struct optimizer<tags::zip<Tag, arity>,
                 ::tags::kripke<NEST_ZGD, Nesting::Direction>>
{
  using result_type = void;

  template<typename R, typename S,
           typename PSI, typename RHS, typename SIGT,
           typename IBoundary, typename JBoundary, typename KBoundary>
  static result_type
  execute(S&& s,
          PSI&& psi, RHS&& rhs, SIGT&& sigt, IBoundary&& i_bndry,
          JBoundary&& j_bndry, KBoundary&& k_bndry)
  {
    using base_type = optimizer<
                        tags::zip<Tag, arity>,
                        tags::sequential_execution>;

    base_type::template execute<void>(
      std::forward<S>(s),
      psi, rhs, sigt,
      make_array_view(i_bndry),
      make_array_view(j_bndry),
      make_array_view(k_bndry));
  }
};


template <typename Tag, int arity>
struct optimizer<tags::zip<Tag, arity>,
                 ::tags::kripke<NEST_ZGD, Nesting::Group>>
{
  using result_type = std::array<std::vector<std::vector<double>>, 3>;

  template<typename R, typename S,
           typename PSI, typename RHS, typename SIGT,
           typename IBndry, typename JBndry, typename KBndry>
  static result_type
  execute(S&& s,
          PSI&& psi, RHS&& rhs, SIGT&& sigt,
          IBndry&& i_bndry, JBndry&& j_bndry, KBndry&& k_bndry)
  {
    result_type out_bndry = {{i_bndry, j_bndry, k_bndry}};

    using base_type = optimizer<
                        tags::zip<Tag, arity>,
                        tags::sequential_execution>;

    base_type::template execute<void>(
      std::forward<S>(s),
      psi, rhs, sigt,
      make_array_view(out_bndry[0]),
      make_array_view(out_bndry[1]),
      make_array_view(out_bndry[2]));

    return out_bndry;
  }
};

} // namespace optimizers
} // namespace stapl
} // namespace skeletons

template<int OuterIdx, int InnerIdx>
class zgd_boundary_gen
{
private:
  typedef stapl::immutable_shared<std::vector<double>> is_t;

  std::vector<double> const& m_plane;
  int                        m_num_directions;
  int                        m_num_groups;
  int                        m_idx_max;

public:
  using index_type  = std::tuple<std::size_t, std::size_t, std::size_t>;
  using result_type = std::vector<std::vector<double>>;

  zgd_boundary_gen(is_t const& plane, Group_Dir_Set& gd_set, int idx_max)
    : m_plane(plane), m_num_directions(gd_set.num_directions),
      m_num_groups(gd_set.num_groups), m_idx_max(idx_max)
  { }

  result_type operator()(index_type const& idx) const
  {
    result_type result(m_num_groups);

    // Calculating the offset outside of the loop in order to stride by
    // number of directions * number of groups from idx
    const int offset = ((std::get<InnerIdx>(idx) * m_idx_max)
        + std::get<OuterIdx>(idx)) * m_num_directions * m_num_groups;

    for (int group = 0; group < m_num_groups; ++group)
    {
      result[group].resize(m_num_directions);

      for (int direction = 0; direction < m_num_directions; ++direction)
        result[group][direction]
          = m_plane[(direction * m_num_groups) + group + offset];
    }

    return result;
  }

  void define_type(typer& t)
  {
    t.member(m_plane);
    t.member(m_num_directions);
    t.member(m_num_groups);
    t.member(m_idx_max);
  }
}; // class zgd_boundary_gen


struct zgd_wf
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

    typedef std::array<std::vector<std::vector<double>>, 3> boundary_type;

    using stapl::skeletons::wrap;
    auto zgd_skeleton =
      wavefront<3>(wrap<tags::kripke<NEST_ZGD, Nesting::Group>>(
        zip<6>(wrap<tags::kripke<NEST_ZGD, Nesting::Direction>>(
          zip<6>(
           make_diamond_difference_wf(grid_data, psi_sliced,
                                      group_set, direction_set, cellset_id))))),
      find_corner(gd_set.directions[0]),
      filter_domain_result<boundary_type>());

    // making the boundary views
    const auto dims = sigt_sliced.dimensions();
    auto size       = psi_sliced.domain().dimensions();

    zgd_boundary_gen<1,2> bg0(i_plane, gd_set, std::get<1>(dims));
    zgd_boundary_gen<0,2> bg1(j_plane, gd_set, std::get<0>(dims));
    zgd_boundary_gen<0,1> bg2(k_plane, gd_set, std::get<0>(dims));

    multiarray<3, boundary_type> out_vals_ct(sigt_sliced.dimensions());
    auto out_vals_vw = make_multiarray_view(out_vals_ct);

    algorithm_executor<stapl::multiview_coarsener<true>>().execute(
      stapl::skeletons::coarse(sink<boundary_type, blocked<3>>(zgd_skeleton)),
      psi_sliced, rhs_sliced, sigt_sliced,
      functor_view(size, bg0), functor_view(size, bg1), functor_view(size, bg2),
      out_vals_vw);

    stapl::do_once([&] {
      Result result;
      result[0].resize(i_plane.get().size());
      result[1].resize(j_plane.get().size());
      result[2].resize(k_plane.get().size());

      const int num_directions  = gd_set.num_directions;
      const int num_groups      = gd_set.num_groups;
      const int local_imax      = std::get<0>(size);
      const int local_jmax      = std::get<1>(size);
      const int local_kmax      = std::get<2>(size);

      // Determines the stride to move between cells, which are of dimension
      // num groups * num directions
      const int multiplier      = num_directions * num_groups;

      // outgoing i_plane
      for (int j = 0; j < std::get<1>(size); ++j)
        for (int k = 0; k < std::get<2>(size); ++k)
        {
          // Selecting appropriate direction and group cell for j and k
          const int offset = (k * local_jmax + j) * multiplier;

          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
              result[0][(direction * num_groups) + group + offset]
                = out_vals_vw(local_imax - 1, j, k)[0][group][direction];
        }

      // outgoing j_plane
      for (int i = 0; i < std::get<0>(size); ++i)
        for (int k = 0; k < std::get<2>(size); ++k)
        {
          // Selecting appropriate direction and group cell for i and k
          const int offset = (k * local_imax + i) * multiplier;

          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
              result[1][(direction * num_groups) + group + offset]
                = out_vals_vw(i, local_jmax - 1, k)[1][group][direction];
        }

      // outgoing k_plane
      for (int i = 0; i < std::get<0>(size); ++i)
        for (int j = 0; j < std::get<1>(size); ++j)
        {
          // Selecting appropriate direction and group cell for i and j
          const int offset = (j * local_imax + i) * multiplier;

          for (int group = 0; group < num_groups; ++group)
            for (int direction = 0; direction < num_directions; ++direction)
              result[2][(direction * num_groups) + group + offset]
                = out_vals_vw(i, j, local_kmax - 1)[2][group][direction];
        }

      p.set_value(std::move(result));
    });
  }
}; // struct zgd_wf


template<typename SigtView, typename PsiView, typename RhsView,
         typename IPlane, typename JPlane, typename KPlane>
Kernel::result_type
Kernel_3d_ZGD::operator()(SigtView& sigt, PsiView& psi, RhsView& rhs,
                          IPlane const& i_plane_in,
                          JPlane const& j_plane_in,
                          KPlane const& k_plane_in)
{
  return invoke_kernel<
    Kernel::result_type, index_sequence<0,1,2,4>, index_sequence<0,1,2>
  >(sigt, psi, rhs, i_plane_in, j_plane_in, k_plane_in,
    this->direction_set, this->group_set, this->grid_data,
    index_of(psi), zgd_wf());
}

#endif
