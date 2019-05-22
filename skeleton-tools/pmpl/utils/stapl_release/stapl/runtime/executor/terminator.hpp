/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_RUNTIME_EXECUTOR_TERMINATOR_HPP
#define STAPL_RUNTIME_EXECUTOR_TERMINATOR_HPP

#include "terminator_base.hpp"
#include "../collective/allreduce_object.hpp"
#include <functional>
#include <utility>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

namespace stapl {

void rmi_flush(void);


//////////////////////////////////////////////////////////////////////
/// @brief General termination detection.
///
/// @tparam T               Object type of the termination detection.
/// @tparam BinaryOperation Binary operation function object type to be applied.
///
/// This terminator does a reduction and compares the resulting value against a
/// known termination value. If they are the same, it calls the notify function.
///
/// It implements a phased termination consisting of 3 states:
/// -# State A: Do allreduce. If the return value is the same as the termination
///    value, goto State B, otherwise goto State A.
/// -# State B: Do allreduce. If the return value is the same as the termination
///    value, goto State C, otherwise goto State A.
/// -# State C: Termination detection succeeded.
///
/// @see terminator_base
/// @ingroup executors
///
/// @todo The termination detection should be correct if in State B we remain in
///       it if the allreduce has failed to converge, instead going to State A.
//////////////////////////////////////////////////////////////////////
template<typename T, typename BinaryOperation>
class terminator
  : public terminator_base
{
public:
  using value_type     = T;
private:
  // Using boost::function instead of std::function to avoid mallocs.
  using function_type  = boost::function<value_type(void)>;
  using allreduce_type = runtime::allreduce_object<value_type, BinaryOperation>;


  //////////////////////////////////////////////////////////////////////
  /// @brief Implements State A.
  //////////////////////////////////////////////////////////////////////
  struct state_a_wf
  {
    terminator& m_t;

    explicit state_a_wf(terminator& t) noexcept
    : m_t(t)
    { }

    void operator()(future<T> f)
    {
      auto& ar = *(m_t.m_ar);
      if (f.get()==m_t.m_termination_value)
        ar.async_then(state_b_wf{m_t});
      else
        ar.async_then(state_a_wf{m_t});
      ar(m_t.m_value_function());
    }
  };

  //////////////////////////////////////////////////////////////////////
  /// @brief Implements State B.
  //////////////////////////////////////////////////////////////////////
  struct state_b_wf
  {
    terminator& m_t;

    explicit state_b_wf(terminator& t) noexcept
    : m_t(t)
    { }

    void operator()(future<T> f)
    {
      if (f.get()==m_t.m_termination_value) {
        m_t.m_started = false;
        m_t.call_notifier();
      }
      else {
        auto& ar = *(m_t.m_ar);
        ar(m_t.m_value_function());
        ar.async_then(state_a_wf{m_t});
      }
    }
  };


  const function_type             m_value_function;
  const value_type                m_termination_value;
  boost::optional<allreduce_type> m_ar;
  bool                            m_started;

public:
  template<typename ValueFunction>
  terminator(BinaryOperation op,
             ValueFunction&& value_fun,
             T const& termination_value = {})
    : m_value_function(std::forward<ValueFunction>(value_fun)),
      m_termination_value(termination_value),
      m_started(false)
  {
    using namespace runtime;

    auto& ctx = this_context::get();
    if (ctx.get_gang_md().size()>1) {
      STAPL_RUNTIME_CHECK(ctx.is_base(), "Only allowed when SPMD");
      m_ar = boost::in_place<allreduce_type>(std::ref(ctx), std::move(op));
    }
  }

  void operator()(void) override
  {
    if (m_ar) {
      if (m_started)
        return;
      m_started = true;
      (*m_ar)(m_value_function());
      (*m_ar).async_then(state_a_wf{*this});
    }
    else {
      // only one location
      if (m_value_function()==m_termination_value) {
        call_notifier();
      }
    }
  }
};

} // namespace stapl

#endif
