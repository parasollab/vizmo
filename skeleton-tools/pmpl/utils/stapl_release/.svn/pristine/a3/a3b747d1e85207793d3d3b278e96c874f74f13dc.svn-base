/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/

#include <stapl/array.hpp>
#include <stapl/algorithm.hpp>

using namespace stapl;

template<typename T>
void test_minmax(std::size_t n)
{
  typedef array<T>         container_type;
  typedef array_view<container_type> view_type;

  container_type c(n);
  view_type v(c);

  iota(v, 0);

  auto mm = minmax_value(v);

  bool passed = mm.first == 0 && mm.second == static_cast<T>(n-1);

  stapl::do_once([&]() {
    std::cout << "Testing minmax (" << typeid(T).name() << "): ";

    if (passed)
      std::cout << "PASSED" << std::endl;
    else
      std::cout << "FAILED" << std::endl;
  });
}


stapl::exit_code stapl_main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "usage: " << argv[0] << " n" << std::endl;
    exit(1);
  }

  const std::size_t n = atol(argv[1]);

  test_minmax<char>(n);
  test_minmax<unsigned char>(n);
  test_minmax<int>(n);
  test_minmax<unsigned int>(n);
  test_minmax<long>(n);
  test_minmax<unsigned long>(n);
  test_minmax<float>(n);
  test_minmax<double>(n);

  return EXIT_SUCCESS;
}
