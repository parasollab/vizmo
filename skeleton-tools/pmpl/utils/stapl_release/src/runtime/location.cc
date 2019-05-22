/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#include <stapl/runtime/location_md.hpp>
#include <stapl/runtime/executor/gang_executor.hpp>
#include <stapl/runtime/executor/scheduler/sched.hpp>
#include <stapl/runtime/type_traits/aligned_storage.hpp>

namespace stapl {

namespace runtime {

// Returns a properly aligned pointer to construct a runqueue_impl
void* location_md::get_ptr(void) noexcept
{
  const std::size_t aligned_sz = aligned_size(sizeof(location_md),
                                              runqueue::required_alignment());
  return (reinterpret_cast<char*>(this) + aligned_sz);
}

// Allocates a new gang_executor for the location_md
std::unique_ptr<executor_base> location_md::make_executor(void)
{
  using executor_type = gang_executor<default_gang_scheduler>;
  return std::unique_ptr<executor_base>{new executor_type(*this)};
}

// Allocates space for both a location_md object and a runqueue_impl object
void* location_md::operator new(std::size_t)
{
  const std::size_t aligned_sz = aligned_size(sizeof(location_md),
                                              runqueue::required_alignment());
  return ::operator new(aligned_sz + runqueue::required_size());
}

// Frees the space allocated for the location_md and the runqueue_impl object
void location_md::operator delete(void* p)
{
  ::operator delete(p);
}

} // namespace runtime

} // namespace stapl
