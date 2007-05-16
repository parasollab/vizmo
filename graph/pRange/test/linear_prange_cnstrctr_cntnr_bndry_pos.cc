#include <stdlib.h>
#include <iostream>
#include <pVector.h>
#include <pRange.h>
#include <runtime.h>

#include "stapl_bool.h"

using namespace stapl;
using namespace std;

/* Simple struct used in negative tests */
struct dumb {
  int x;
  int y;
};

/* Class used to initialize pContainer elements with an integer sequence. */
class sequence {
private:
  int counter;
public:
  sequence() : counter(-1) { }
  sequence(int start) : counter(start-1) { }

  int operator()() {
    return ++counter;
  }
};

void stapl_main(int argc, char **argv) {

  // Type definitions for the test
  typedef stapl::pvector<int> pvector;
  typedef stapl::linear_boundary<pvector::iterator> boundary;
  typedef stapl::linearpRange<boundary, pvector> prange;

  // Finding the thread id, number of threads, and the number of 
  // elements per thread assuming there are 64 elements total.
  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();

  int num_local_elems = 100 / nThreads;
  if (id < 100 % nThreads) ++num_local_elems;

  // Initial output to show what test is running
  if (id == 0) {
    cout << "Linear pRange container/boundary constructor positive with " 
         << nThreads << " threads... ";
  }

  // Instantiate the pContainer 
  pvector vec(100);

  // Initialize the elements of the pContainer
  int local_start = num_local_elems * id;
  sequence seq(local_start);
  generate(vec.local_begin(), vec.local_end(), seq);


  boundary b(vec.local_begin(), vec.local_end() - 1);
  
  prange pr1(&vec, b);

  int cont_elem = vec.GetElement(0);
  int pr_elem = pr1.GetElement(0);

  bool corr_bound1 = (pr1.get_subranges()[0].get_boundary() == b);
  bool corr_elem1 = (pr_elem == cont_elem);
  bool corr_offset1 = (pr1.get_gid_producer().get_offset() == 10000);
  bool corr_subranges1 = (pr1.get_subranges().size() == 1);


  prange pr2(&vec, b, 99, &vec);

  int cont_elem2 = vec.GetElement(0);
  int pr_elem2 = pr1.GetElement(0);

  bool corr_bound2 = (pr2.get_boundary() == b);
  bool corr_elem2 = (pr_elem2 == cont_elem2);
  bool corr_offset2 = (pr2.get_gid_producer().get_offset() == 99);
  bool corr_subranges2 = (pr2.get_subranges().size() == 0);


  // Report whether or not the test passed
  stapl_bool test_result(corr_bound1 && corr_elem1 &&
		         corr_offset1 && corr_subranges1 &&
			 corr_bound2 && corr_elem2 &&
			 corr_offset2 && corr_subranges2);
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
