/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_P_CONTAINER_PROFILER_UTIL_HPP
#define STAPL_P_CONTAINER_PROFILER_UTIL_HPP

#include "p_container_profiler.hpp"
#include "value_type_util.h"

#include <stapl/runtime.hpp>

////////////////////////////////////////////////////////////////////////////////
/// @brief Simple print function that limits output to location 0
///
/// @param s A string of characters to print
////////////////////////////////////////////////////////////////////////////////
void stapl_print(const char* s)
{
  if (stapl::get_location_id() == 0)
    std::cout << s << std::flush;
}

// simple timer
typedef stapl::counter<stapl::default_timer> counter_type;

typedef my_variable<int,1,10> MVT;

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the sum of two MVT objects.
///
/// @param v1 The first operand
/// @param v2 The second operand
////////////////////////////////////////////////////////////////////////////////
MVT operator+(MVT const& v1, MVT const& v2)
{
  return v1 + v2;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Takes an operand and increases it by 1234.
///
/// @tparam T An arbitrary type
////////////////////////////////////////////////////////////////////////////////
template <class T>
class sum_op
{
public:
  template<typename Reference>
  void operator()(Reference elem) const
  {
    elem += 1234;
  }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the sum of an operand and the value 1234 without modifying
/// the operand.
///
/// @tparam T An arbitrary type
////////////////////////////////////////////////////////////////////////////////
template <class T>
class get_sum
{
public:
  typedef T result_type;

  template<typename Reference>
  T operator()(Reference elem) const
  {
    return elem + 1234;
  }
};

#endif
