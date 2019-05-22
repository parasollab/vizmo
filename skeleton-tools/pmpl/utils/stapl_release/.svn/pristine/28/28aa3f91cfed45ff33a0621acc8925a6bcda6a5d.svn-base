/*
 // Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
 // component of the Texas A&M University System.

 // All rights reserved.

 // The information and source code contained herein is the exclusive
 // property of TEES and may not be disclosed, examined or reproduced
 // in whole or in part without explicit written authorization from TEES.
 */

#include <stapl/runtime.hpp>

#include <iostream>
#include <cstdlib>

#include <stapl/containers/array/array.hpp>
#include <stapl/views/counting_view.hpp>
#include <stapl/views/array_view.hpp>
#include <stapl/views/reverse_view.hpp>
#include <stapl/algorithms/algorithm.hpp>

#include "../../test_report.hpp"

using namespace std;
using namespace stapl;

struct print
{

  template <typename T>
  void operator()(T x)
  {
    std::cout << x << " ";
  }

  template <typename T0, typename T1>
  void operator()(T0 t0, T1 t1)
  {
    std::cout << "(" << t0 << "," << t1 << ") ";
  }
};


stapl::exit_code stapl_main(int argc, char* argv[])
{
  typedef array<int> p_array_int;
  typedef array_view<p_array_int> parrayView;


  typedef parrayView::reference ref_t;

  int nelem = 9;
  int number_permutations = 3;

  bool test_result;

  if (argc > 1)
    nelem = atoi(argv[1]);
  if (argc > 2)
    nelem = argc - 2;

  // p_array min_element
  p_array_int int_array(nelem);
  parrayView int_array_view(int_array);

  copy(counting_view<int> (nelem, 1), int_array_view);
  if (!argc > 1) {
    int_array[8] = 1;
    int_array[7] = 2;
    int_array[6] = 3;
  }
  if (argc > 2) {
    number_permutations = atoi(argv[1]);
    for (int i = 0; i < nelem; i++) {
      int_array[i] = atoi(argv[i + 2]);
    }
  }

  do_once([&](void) { cout << argv[0])); }

  if (get_location_id() == 0) {
    cout << endl << "Original Array :" << endl;
    for (size_t i = 0; i < int_array_view.size(); i++) {
      std::cout << int_array[i] << " ";
    }
    cout << endl << "Testing next_permutation on a p_array of int : ";
  }

  test_result = false;

  // ref_t myRef=min_element(int_array_view);

  std::cout << "# permutations: " << number_permutations << std::endl;
  int k = 1;
  do {
    test_result = next_permutation(int_array_view);

    if (get_location_id() == 0) {
      std::cout << endl;
#if 0
      cout << " permutation n°" << k << " : ";
     cout << endl << "Array after permutation "<<k<<" :("<<test_result<<")\n";
        for (size_t i = 0; i < int_array_view.size(); i++)
          std::cout << int_array[i] << " ";

     cout << endl << "Array again from view :" << endl;
           for (size_t i = 0; i < int_array_view.size(); i++)
             std::cout << (int_array_view.container())[i] << " ";
    }
#endif

    serial(print(), int_array_view);
    k++;
  } while (test_result == true && k <= number_permutations);

  if (get_location_id() == 0) {
    if (test_result)
      cout << "[PASSED]" << endl;
    else
      cout << "[FAILED]" << endl;

    cout << endl << "Array after all " << endl;
    for (size_t i = 0; i < int_array_view.size(); i++)
      std::cout << int_array[i] << " ";
    cout << endl;
  }

#if 0
  if (get_location_id() == 0) {
    cout << endl << "Original Array :" << endl;
    for (size_t i = 0; i < int_array_view.size(); i++) {
      std::cout << int_array[i] << " ";
    }
    cout << endl << "Testing prev_permutation on a p_array of  int : ";
  }
  test_result = false;
  k = 1;
  do {
    test_result = prev_permutation(int_array_view);

    if (get_location_id() == 0) {
      cout << " permutation n°" << k << " : ";
      map_func(print(), int_array_view);
      std::cout << endl;
    }
    k++;
  } while (test_result == true && k <= number_permutations);

  if (get_location_id() == 0) {
    cout << endl << "Time taken : " << total_time << endl;
    if (test_result)
      cout << "[PASSED]" << endl;
    else
      cout << "[FAILED]" << endl;

    cout << endl << "Array after all " << endl;
    for (size_t i = 0; i < int_array_view.size(); i++)
      std::cout << int_array[i] << " ";
    cout << endl;
  }
#endif

  return EXIT_SUCCESS;
}
