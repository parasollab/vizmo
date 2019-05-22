/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/paragraph/paragraph_fwd.h>
#include <stapl/runtime.hpp>
#include <stapl/runtime/location_specific_storage.hpp>

namespace stapl {

#if 0

/// @todo This code was associated with prototype RDMA support that needs to be
/// integrated into the runtime before using it again in the PARAGRAPH.
namespace runtime {

namespace detail {

struct messager_base;

messager_base* no_armi_messager = 0;

void set_no_armi_messager(messager_base* ptr)
{
  stapl_assert(no_armi_messager == 0, "get_no_armi_messager found set ptr");

  no_armi_messager = ptr;
}

messager_base* get_no_armi_messager(void)
{
  // stapl_assert(no_armi_messager != 0,
  //   "get_no_armi_messager found null ptr");

  return no_armi_messager;
}

} // namespace detail

} // namespace runtime

#endif


//////////////////////////////////////////////////////////////////////
/// @brief Object held in @ref location_specific_storage (one instance per
/// location), tracking various pieces of low level information about the
/// PARAGRAPHs executing on the location.
//////////////////////////////////////////////////////////////////////
struct paragraph_location_metadata
{
  /// @brief True if a PARAGRAPH on this location is being initialized.
  bool b_initializing_tg;

  paragraph_location_metadata(void)
    : b_initializing_tg(false)
  { }
};


static location_specific_storage<paragraph_location_metadata> stack;


tg_initializer::tg_initializer(void)
{
//  stapl_assert(!b_initializing_tg,
//    "start_tg_initialization found b_initializing_tg == true");
  stack.get().b_initializing_tg = true;
}


tg_initializer::~tg_initializer(void)
{
//  stapl_assert(b_initializing_tg,
//    "stop_tg_initialization found b_initializing_tg == false");
  stack.get().b_initializing_tg = false;
}


bool tg_initializer::is_initializing(void)
{
  return stack.get().b_initializing_tg;
}

} // namespace stapl
