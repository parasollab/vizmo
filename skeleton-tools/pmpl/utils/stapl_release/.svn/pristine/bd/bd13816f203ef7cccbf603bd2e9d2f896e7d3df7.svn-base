/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_SKELETONS_ALGORITHM_EXECUTOR_HPP
#define STAPL_SKELETONS_ALGORITHM_EXECUTOR_HPP

#include <stapl/utility/use_default.hpp>
#include <stapl/skeletons/utility/utility.hpp>
#include <stapl/skeletons/utility/skeleton.hpp>
#include <stapl/runtime/executor/scheduler/sched.hpp>

namespace stapl {
namespace skeletons {

//////////////////////////////////////////////////////////////////////
/// @brief Prepares a skeleton with the given views for execution in
/// a @c PARAGRAPH. It uses the @c Coarsener to coarsen the view
/// before passing it down to the skeleton.
///
/// @tparam Coarsener the data coarsener to be used over the given
///                   views. By default no coarsening will be applied.
/// @tparam ExtraEnv  this executor will run the skeleton in a
///                   @c PARAGRAPH environment (@c taskgraph_env). An
///                   additional environment can be passed here.
///
/// @todo This class should be merged with @c paragraph_skeleton_manager
///
/// @ingroup skeletonsExecutors
//////////////////////////////////////////////////////////////////////
template <typename Coarsener = stapl::use_default,
          typename ExtraEnv  = stapl::use_default,
          typename Sched     = default_scheduler>
class algorithm_executor
{
private:
  ExtraEnv m_env;
  Sched    m_sched;
public:
  algorithm_executor(Sched const& sched)
    : m_sched(sched)
  { }

  algorithm_executor(ExtraEnv env)
    : m_env(env)
  { }

  algorithm_executor()
    : m_env(ExtraEnv())
  { }

  void set_extra_env(ExtraEnv env)
  {
    m_env = env;
  }

private:
  template <typename ResultType, typename S, typename... Views>
  ResultType operator()(std::true_type, S&& skeleton, Views&&... views);

  template <typename ResultType, typename S, typename... Views>
  ResultType operator()(std::false_type, S&& skeleton, Views&&... views);

public:
  template <typename ResultType = void,
            bool isBlocking = true,
            typename S, typename... Views>
  ResultType
  execute(S&& skeleton, Views&&... views)
  {
    return this->operator()<ResultType>(
              std::integral_constant<bool, isBlocking>(),
              std::forward<S>(skeleton),
              std::forward<Views>(views)...);
  }
};

} // namespace skeletons
} // namespace stapl

#include <stapl/paragraph/paragraph.hpp>
#include "paragraph_skeleton_manager.hpp"


namespace stapl {
namespace skeletons {

  //////////////////////////////////////////////////////////////////////
  /// @brief Specialization for the blocking PARAGRAPHs.
  ///
  /// @param skeleton the skeleton to be evaluated.
  /// @param views    inputs to the given skeleton compositions.
  ///
  /// @return the result value of skeleton execution if @c ResultType is
  ///         not void.
  //////////////////////////////////////////////////////////////////////
  template <typename Coarsener, typename ExtraEnv, typename Sched>
  template <typename ResultType, typename S, typename ...Views>
  ResultType
  algorithm_executor<Coarsener, ExtraEnv, Sched>::operator()(
    std::true_type, S&& skeleton, Views&&... views)
  {
    auto pmg = make_paragraph_skeleton_manager<
                 Coarsener, ResultType>(std::forward<S>(skeleton), m_env);

    return paragraph<Sched, decltype(pmg), typename std::decay<Views>::type...>(
             pmg, std::forward<Views>(views)..., m_sched)();
  }

  //////////////////////////////////////////////////////////////////////
  /// @brief Specialization for the non-blocking PARAGRAPHs.
  ///
  /// @param skeleton the skeleton to be evaluated.
  /// @param views    inputs to the given skeleton compositions.
  ///
  /// @return the result value of skeleton execution if @c ResultType is
  ///         not void.
  //////////////////////////////////////////////////////////////////////
  template <typename Coarsener, typename ExtraEnv, typename Sched>
  template <typename ResultType, typename S, typename... Views>
  ResultType
  algorithm_executor<Coarsener, ExtraEnv, Sched>::operator()(
    std::false_type, S&& skeleton, Views&&... views)
  {
    auto pmg = make_paragraph_skeleton_manager<
                 Coarsener, ResultType>(std::forward<S>(skeleton), m_env);

    using pmg_t = paragraph<Sched, decltype(pmg),
                            typename std::decay<Views>::type...>;
    pmg_t* pg = new pmg_t(pmg, std::forward<Views>(views)..., m_sched);
    (*pg)(0);
  }

} // namespace skeletons
} // namespace stapl

#endif // STAPL_SKELETONS_ALGORITHM_EXECUTOR_HPP
