/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#include <stapl/runtime/synchronization.hpp>
#include <stapl/runtime/context.hpp>
#include <stapl/runtime/yield.hpp>
#include <stapl/runtime/collective/allreduce_object.hpp>
#include <stapl/runtime/non_rmi/rpc.hpp>
#include <boost/serialization/unordered_map.hpp>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief RMI fence implementation.
///
/// For one location, the fence succeeds iff
/// \f$ \sum_{lid=i}^{p} (sent - processed) + |f()| = 0 \f$.
///
/// For more than one locations, the fence succeeds iff
/// \f$ \sum_{lid=i}^{p} (sent - processed) + new_sent + f() = 0 \f$
/// for at least two iterations.
///
/// The fence ensures that all outstanding RMI requests have completed.
///
/// In the simple case, iterating is not necessary. However, two special cases
/// make iterating necessary for correctness:
///  -# if an RMI invokes a method that makes another RMI, which invokes a
///     method that makes another RMI, etc.
///  -# the communication layer does not make ordering guarantees between point-
///     to-point traffic and collective operations' traffic.
///
/// @warning This is an SPMD function.
///
/// @ingroup requestBuildingBlock
///
/// @todo The iteration restarts from the first phase, however it may be correct
///       to return to the second iteration.
//////////////////////////////////////////////////////////////////////
template<typename Function>
void rmi_fence_impl(context& ctx, Function&& f)
{
  ctx.flush();

  gang_md& g     = ctx.get_gang_md();
  location_md& l = ctx.get_location_md();

  // quiescence detection
  if (g.size()==1) {
    fence_section fs{l};
    yield_until(no_context,
                [&f, &l]
                {
                  return (f()                                           &&
                          l.get_gang_md().get_fence_md().none_pending() &&
                          (l.get_fence_md().pending()==0));
                });
  }
  else {
    const bool leader      = l.is_leader();
    const process_id owner = gang_md_registry::id_owner(g.get_id());
    const bool is_owner    = (runqueue::get_process_id()==owner);

    allreduce_object<int, std::plus<int>> ar{ctx, std::plus<int>{}};
    auto const& l_fence_md = l.get_fence_md();

    fence_section fs{l};

    runqueue::yield();

    for (int phase = 0, old_sent = 0; phase<2; ++phase) {
      int n = (f() ? 0 : 1);

      if (leader) {
        if (is_owner) {
          // check if pending intergang requests completed
          yield_if_not(no_context,
                       [&g, &n]
                       {
                         if (g.get_fence_md().none_pending())
                           return true;
                         // pending intergang requests
                         ++n;
                         return false;
                       });
        }
        else {
          // send counts of pending intergang requests to owner
          if (yield_until(no_context,
                          [&g, owner]
                          {
                            auto c = g.get_fence_md().retrieve();
                            if (c.empty())
                              return true;
                            rpc<
                              decltype(&gang_md::update_fence_md),
                              &gang_md::update_fence_md
                            >(owner, g.get_id(), std::move(c));
                            return false;
                          })) {
            // pending intergang requests
            ++n;
          }
        }
      }

      // number of pending intragang requests
      n += l_fence_md.pending();

      // figure out if new requests were generated since last phase
      if (phase==0) {
        old_sent = l_fence_md.get_sent();
      }
      else {
        const int curr_sent = l_fence_md.get_sent();
        STAPL_RUNTIME_ASSERT(curr_sent >= old_sent);
        n += (curr_sent - old_sent);
        old_sent = curr_sent;
      }

      // calculate global number of pending requests (synchronization point)
      ar(n);
      n = ar.get();

      // decide locally next phase
      if (n!=0) {
        phase = -1; // inconsistent state - fall back to first phase
        // phase = 0; // inconsistent state - repeat second phase
      }
    }
  }
}


// RMI fence that only takes care of RMIs
void rmi_fence(context& ctx)
{
  rmi_fence_impl(ctx, [] { return true; });
}


// RMI fence that takes care of RMIs and additional function object
void rmi_fence(context& ctx, std::function<bool(void)> f)
{
  rmi_fence_impl(ctx, std::move(f));
}

} // namespace runtime

} // namespace stapl
