/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_EXECUTORS_PARAGRAPH_SKELETON_MANAGER_HPP
#define STAPL_SKELETONS_EXECUTORS_PARAGRAPH_SKELETON_MANAGER_HPP

#include "skeleton_manager.hpp"
#include <stapl/skeletons/environments/taskgraph_env.hpp>
#include <stapl/skeletons/environments/combined_env.hpp>
#include <stapl/paragraph/incremental_wf.hpp>
#include <stapl/paragraph/factory_wf.hpp>
#include "paragraph_skeleton_manager_fwd.hpp"

namespace stapl {
namespace skeletons {

//////////////////////////////////////////////////////////////////////
/// @brief This is a wrapper over the core @c skeleton_manager
/// that allows the creating of tasks in STAPL task graphs.
///
/// @tparam Skeleton   the big skeleton that should be used in order to
///                    execute the algorithm.
/// @tparam ExtraEnv   if the given skeleton is going to be spawned in
///                    more than one environment. A good example is when
///                    one wants to visualize the dependence graph using
///                    @c graphviz_env.
/// @tparam OptionalParams
///         1. Coarsener  the coarsener to be used on the input views
///                       before passing them to the algorithm. The
///                       default case is a @c null_coarsener that leaves
///                       the views untouched.
///         2. ResultType the value type for the result of this skeleton
///                    execution.
///
/// @see skeleton_manager
/// @see combined_env
/// @see graphviz_env
/// @see local_env
///
/// @ingroup skeletonsExecutors
//////////////////////////////////////////////////////////////////////
template <typename Skeleton,
          typename ExtraEnv,
          typename... OptionalParams>
class paragraph_skeleton_manager
  : public skeleton_manager,
    public incremental_wf,
    public factory_wf
{
  using default_value_types = stapl::tuple<
                                stapl::use_default, // coarsener_type
                                void                // result_type
                              >;

  using param_types         = typename compute_type_parameters<
                                default_value_types, OptionalParams...>::type;

public:
  using coarsener_type = typename tuple_element<0, param_types>::type;
  using result_type    = typename tuple_element<1, param_types>::type;
  using tag_type       = typename Skeleton::skeleton_tag_type;
private:
  /// @brief checks if it is the first time that @c PARAGRAPH is calling this
  /// factory.
  bool     m_is_first_iter;
  Skeleton m_skeleton;
  ExtraEnv m_env;

public:
  paragraph_skeleton_manager(Skeleton const& skeleton, ExtraEnv const& env)
    : m_is_first_iter(true),
      m_skeleton(skeleton),
      m_env(env)
  { }

  constexpr coarsener_type get_coarsener() const
  {
    return coarsener_type();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief A simple optimization to avoid the @c empty_env from being
  /// called during the spawning process.
  //////////////////////////////////////////////////////////////////////
  template <typename TGV>
  taskgraph_env<TGV> create_envs(TGV tgv, std::true_type)
  {
    return taskgraph_env<TGV>(tgv);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief If the extra environment passed to the paragraph skeleton
  /// manager is not an empty environment, it should be bundled with
  /// the taskgraph environment before starting the spawning process.
  //////////////////////////////////////////////////////////////////////
  template <typename TGV>
  combined_env<taskgraph_env<TGV>, ExtraEnv>
  create_envs(TGV tgv, std::false_type)
  {
    return make_combined_env(taskgraph_env<TGV>(tgv), m_env);
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief This function combines a @c taskgraph_env with a
  /// user-provided environment (if any) and then executes the skeleton
  /// in the combined environment.
  ///
  /// This method will be called by @c PARAGRAPH until this factory
  /// specifies that it is done with task creation (@c m_is_done).
  /// Upon each invocation it asks the @c skeleton_manager to continue
  /// spawning the enclosed skeleton.
  ///
  /// @param tgv      task graph view is passed to this method by
  ///                 @c PARAGRAPH using this argument
  /// @param view     set of input views to the algorithm
  //////////////////////////////////////////////////////////////////////
  template <typename TGV, typename ...V>
  void operator()(TGV tgv, V&... view)
  {
    if (this->m_is_first_iter) {
      this->m_is_first_iter = false;

      auto envs = create_envs(tgv,
                              std::is_same<ExtraEnv, stapl::use_default>());
      envs.init_location_info(tgv.graph().get_num_locations(),
                              tgv.graph().get_location_id());
      this->execute(m_skeleton, envs, view...);
    } else {
      stapl_assert(!this->m_is_done,
        "paragraph skeleton manager was reinvoked after it was done");
      this->resume();
    }
  }

  bool finished(void) const
  {
    return this->m_is_done;
  }

  void define_type(typer& t)
  {
    t.base<skeleton_manager>(*this);
    t.base<incremental_wf>(*this);
    t.base<factory_wf>(*this);
    t.member(m_is_first_iter);
    t.member(m_skeleton);
    t.member(m_env);
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Creates a @c paragraph_skeleton_manager using the given
/// parameters. The default values for the arguments are provided in
/// paragraph_skeleton_manager_fwd.hpp.
///
/// @tparam OptionalParams
///         1. Coarsener  the coarsener to be used on the input views
///                       before passing them to the algorithm. The
///                       default case is a @c null_coarsener that leaves
///                       the views untouched.
///         2. ResultType the value type for the result of this skeleton
///                       execution.
/// @param skeleton       the skeleton to be spawned.
/// @param extraenv       the extra environemnt to be used in the spawning
///                       process. The typical environment passed here
///                       is a graphviz_env.
///
/// @return a skeleton manager over the given skeleton
///
/// @ingroup skeletonsExecutors
//////////////////////////////////////////////////////////////////////
template <typename... OptionalParams, typename S, typename ExtraEnv>
paragraph_skeleton_manager<
  typename std::decay<S>::type,
  typename std::decay<ExtraEnv>::type,
  OptionalParams...>
make_paragraph_skeleton_manager(S&& skeleton, ExtraEnv&& extra_env)
{
  return paragraph_skeleton_manager<
           typename std::decay<S>::type,
           typename std::decay<ExtraEnv>::type,
           OptionalParams...
         >(std::forward<S>(skeleton), std::forward<ExtraEnv>(extra_env));
}

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_EXECUTORS_PARAGRAPH_SKELETON_MANAGER_HPP
