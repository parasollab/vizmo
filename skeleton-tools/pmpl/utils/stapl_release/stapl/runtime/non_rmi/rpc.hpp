/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_NON_RMI_RPC_HPP
#define STAPL_RUNTIME_NON_RMI_RPC_HPP

#include "../aggregator.hpp"
#include "../request/arguments.hpp"
#include "../request/rpc_request.hpp"
#include <type_traits>
#include <utility>

namespace stapl {

namespace runtime {

//////////////////////////////////////////////////////////////////////
/// @brief Remote Procedure Call (RPC) request that discards return values.
///
/// @tparam FunPtr   Function pointer type.
/// @tparam Function Function pointer.
/// @tparam T        Argument types.
///
/// This request encodes the function pointer in the template argument so that
/// any additional indirection is avoided.
///
/// RPC requests are only inter-process, therefore no shared-memory
/// optimizations are applicable.
///
/// @ingroup requestBuildingBlock
//////////////////////////////////////////////////////////////////////
template<typename FunPtr, FunPtr Function, typename... T>
class async_rpc_request final
: public rpc_request,
  private arguments_t<FunPtr, T...>
{
private:
  using args_type = arguments_t<FunPtr, T...>;
  using seq_type  = make_index_sequence<sizeof...(T)>;

public:
  template<typename... U>
  static std::size_t expected_size(U&&... u) noexcept
  {
    return (sizeof(async_rpc_request) +
            dynamic_size<args_type>(seq_type{}, std::forward<U>(u)...));
  }

  template<typename... U>
  explicit async_rpc_request(U&&...u) noexcept
  : rpc_request(sizeof(*this)),
    args_type(std::forward_as_tuple(std::forward<U>(u),
                                    static_cast<void*>(this),
                                    this->size())...)
  { }

  void operator()(message_shared_ptr&) final
  {
    invoke(Function, static_cast<args_type&>(*this),
           static_cast<void*>(this), seq_type{});
    this->~async_rpc_request();
  }
};


//////////////////////////////////////////////////////////////////////
/// @brief Remote Procedure Call (RPC) to process primitive.
///
/// This function calls @p Function on the destination processes.
///
/// @param pids Destination processes.
/// @param t    Arguments to pass to the function.
///
/// @ingroup requestBuildingBlock
//////////////////////////////////////////////////////////////////////
template<typename FunPtr, FunPtr Function, typename Range, typename... T>
void rpc(Range&& pids, T&&... t)
{
  rpc_aggregator a{std::forward<Range>(pids)};
  using request_type = async_rpc_request<
                         FunPtr,
                         Function,
                         typename std::remove_reference<T>::type...>;
  const std::size_t size = request_type::expected_size(std::forward<T>(t)...);
  new(a.allocate(size)) request_type{std::forward<T>(t)...};
}

} // namespace runtime

} // namespace stapl

#endif
