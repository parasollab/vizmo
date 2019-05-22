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
/// Test @ref stapl::runtime::allreduce_object.
//////////////////////////////////////////////////////////////////////

#include <stapl/runtime.hpp>
#include <stapl/runtime/collective/allreduce_object.hpp>
#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include "test_utils.h"

using namespace stapl;


class p_test
: public p_object
{
public:
  // test for commutative operator
  void test_commutative(void)
  {
    using namespace runtime;

    typedef unsigned int          value_type;
    typedef std::plus<value_type> operator_type;

    STAPL_RUNTIME_TEST_CHECK(is_non_commutative<operator_type>::value, false);

    operator_type op;

    value_type N = 0;
    for (value_type i = 0; i < this->get_num_locations(); ++i) {
      N = op(N, i);
    }

    allreduce_object<value_type, operator_type> r{this_context::get(), op};

    for (unsigned int i = 0; i < 100; ++i) {
      r(this->get_location_id() + i);
      value_type result = r.get();
      STAPL_RUNTIME_TEST_CHECK(result, (N + i*get_num_locations()));
    }

    rmi_fence(); // quiescence before next test
  }

  // test for commutative operator with continuation
  void test_commutative_async(void)
  {
    using namespace runtime;

    typedef unsigned int          value_type;
    typedef std::plus<value_type> operator_type;

    STAPL_RUNTIME_TEST_CHECK(is_non_commutative<operator_type>::value, false);

    operator_type op;

    value_type N = 0;
    for (value_type i = 0; i < this->get_num_locations(); ++i) {
      N = op(N, i);
    }

    allreduce_object<value_type, operator_type> r{this_context::get(), op};

    for (unsigned int i = 0; i < 100; ++i) {
      r(this->get_location_id() + i);

      value_type result = 0;
      bool done         = false;
      r.async_then([&](future<value_type> f)
                   {
                     result = f.get();
                     done   = true;
                   });
      block_until([&done] { return done; });
      STAPL_RUNTIME_TEST_CHECK(result, (N + i*get_num_locations()));
    }

    rmi_fence(); // quiescence before next test
  }

  // test a lot of allreduce_objects fired in ordered fashion
  void test_ordered(void)
  {
    using namespace runtime;

    typedef unsigned int          value_type;
    typedef std::plus<value_type> operator_type;

    const std::size_t NUM   = 100;
    const std::size_t ITERS = 1000;

    STAPL_RUNTIME_TEST_CHECK(is_non_commutative<operator_type>::value, false);

    operator_type op;

    value_type N = 0;
    for (value_type i = 0; i < this->get_num_locations(); ++i) {
      N = op(N, i);
    }

    std::deque<allreduce_object<value_type, operator_type>> d;

    for (auto i = 0u; i < NUM; ++i) {
      d.emplace_back(std::ref(this_context::get()), op);
    }

    for (auto i = 0u; i < ITERS; ++i) {
      for (auto j = 0u; j < NUM; ++j)
        d[j](this->get_location_id() + j);

      for (auto j = 0u; j < NUM; ++j) {
        value_type result = d[j].get();
        STAPL_RUNTIME_TEST_CHECK(result, (N + j*get_num_locations()));
      }
    }

    rmi_fence(); // quiescence before next test
  }

  // test a lot of allreduce_objects fired in ordered fashion with async_then()
  void test_ordered_async(void)
  {
    using namespace runtime;

    typedef unsigned int          value_type;
    typedef std::plus<value_type> operator_type;

    const std::size_t NUM   = 100;
    const std::size_t ITERS = 1000;

    STAPL_RUNTIME_TEST_CHECK(is_non_commutative<operator_type>::value, false);

    operator_type op;

    value_type N = 0;
    for (value_type i = 0; i < this->get_num_locations(); ++i) {
      N = op(N, i);
    }

    std::deque<allreduce_object<value_type, operator_type>> d;

    for (auto i = 0u; i < NUM; ++i) {
      d.emplace_back(std::ref(this_context::get()), op);
    }

    for (auto i = 0u; i < ITERS; ++i) {
      unsigned int pending = NUM;

      for (auto j = 0u; j < NUM; ++j) {
        d[j](this->get_location_id() + j);
        const auto res = (N + j*get_num_locations());
        d[j].async_then(
          [res, &pending](future<value_type> f)
          {
            STAPL_RUNTIME_TEST_CHECK(f.get(), res);
            --pending;
          });
      }

      block_until([&pending] { return (pending==0); });
    }

    rmi_fence(); // quiescence before next test
  }

  void execute(void)
  {
    test_commutative();
    test_commutative_async();
    test_ordered();
    test_ordered_async();
  }
};


exit_code stapl_main(int, char*[])
{
  p_test pt;
  pt.execute();
#ifndef _TEST_QUIET
  std::cout << get_location_id() << " successfully passed!" << std::endl;
#endif
  return EXIT_SUCCESS;
}
