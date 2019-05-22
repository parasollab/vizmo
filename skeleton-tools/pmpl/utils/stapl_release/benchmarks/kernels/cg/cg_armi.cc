/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


//////////////////////////////////////////////////////////////////////
/// @file
/// Benchmark for nested parallelism pattern found in NAS CG using STAPL-RTS
/// primitives and the @ref stapl::terminator.
//////////////////////////////////////////////////////////////////////

#include <stapl/runtime.hpp>
#include <stapl/runtime/executor/terminator.hpp>
#include <stapl/utility/do_once.hpp>
#include <iostream>
#include <cmath>

struct dummy_init_p_object
: public stapl::p_object
{
  int m_count;

  dummy_init_p_object(void)
    : m_count(0)
  { }

  void notify_complete(void)
  { ++m_count; }

  int termination_value(void)
  { return m_count; }
};


struct dummy_p_object
: public stapl::p_object
{
  dummy_p_object(stapl::pointer_wrapper<stapl::p_object> parent,
                 stapl::location_type parent_location)
  {
    if (this->get_location_id() == 0)
    {
      stapl_assert(parent != nullptr, "Failed parent pg handle resolution");
      stapl::gang g(*parent);
      stapl::async_rmi(parent_location, parent->get_rmi_handle(),
                       &dummy_init_p_object::notify_complete);
      stapl::rmi_flush();
    }
    delete this;
  }
};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  using timer_type = stapl::counter<stapl::default_timer>;

  const auto N                     = 100;
  const auto nlocs                 = stapl::get_num_locations();
  const decltype(nlocs) col_length = std::sqrt(nlocs);
  const auto row_width =
    col_length * col_length == nlocs ? col_length : col_length * 2;

  stapl_assert(row_width * col_length == nlocs,
    "# locations must be a square or 2*square");

  const bool row_creator = (stapl::get_location_id()%row_width == 0);

  stapl::rmi_handle::reference o;
  stapl::p_object_delete<dummy_init_p_object> d;

  // create row objects
  if (row_creator) {
    std::vector<stapl::location_type> locs;
    locs.reserve(row_width);
    for (auto i = stapl::get_location_id();
           i < (stapl::get_location_id() + row_width);
             ++i)
      locs.push_back(i);

    auto f = stapl::construct<dummy_init_p_object>(stapl::location_range(locs));
    o = f.get();
  }

  timer_type benchmark_timer;

  stapl::rmi_fence();

  benchmark_timer.start();

  for (int i = 0; i < N; ++i)
  {
    bool done = false;
    dummy_init_p_object outer_pg;

    auto terminator_ptr =
      new stapl::terminator<int,std::plus<int>>(
        std::plus<int>(),
        [&outer_pg] { return outer_pg.termination_value(); },
        col_length);

    terminator_ptr->set_notifier(
      [&done] { done = true; });

    outer_pg.advance_epoch();

    if (row_creator) {
       stapl::async_construct<dummy_p_object>(
         [](dummy_p_object*) { },
         o, stapl::all_locations,
         stapl::pointer(&outer_pg), outer_pg.get_location_id()
       );
    }

    terminator_ptr->operator()();

    stapl::block_until([&done] { return done; });

    if (!done)
      stapl::abort("It should have been done!!");

    delete terminator_ptr;
  }

  const double benchmark_elapsed = benchmark_timer.stop();

  // delete row objects
  if (row_creator) {
    d(o);
  }

  stapl::do_once([benchmark_elapsed] {
    std::cout << "Benchmark completed\n";
    std::cout << "Time in seconds = " << benchmark_elapsed << "\n";
  });

  return EXIT_SUCCESS;
}
