/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#ifndef STAPL_TEST_REPORT_HPP
#define STAPL_TEST_REPORT_HPP

#include <iostream>

#include <stapl/utility/do_once.hpp>

#define STAPL_TEST_MESSAGE(str)                       \
  stapl::do_once([&](void) {                          \
    std::cout << str << "\n";                         \
  });

#ifdef REPORT_WITH_COLOR
#  define STAPL_TEST_REPORT(val,str)                  \
    stapl::do_once([&](void) {                        \
      std::cout << str;                               \
      if (val)                                        \
        std::cout << ": \033[;32mPASSED\033[0m\n";    \
      else                                            \
        std::cout << ": \033[;31mFAILED\033[0m\n";    \
    });
#else
#  define STAPL_TEST_REPORT(val,str)                  \
    stapl::do_once([&](void) {                        \
      std::cout << str;                               \
      if (val)                                        \
        std::cout << ": PASSED\n";                    \
      else                                            \
        std::cout << ": FAILED\n";                    \
    });
#  endif
#endif // STAPL_TEST_REPORT_HPP
