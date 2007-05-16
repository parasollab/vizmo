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
    cout << "Linear pRange pContainer insert positive with " 
         << nThreads << " threads... ";
  }

  // Instantiate the pContainers 
  pvector vec1(100);
  pvector vec2(100);

  // Initialize the elements of the pContainer
  int local_start = num_local_elems * id;
  sequence seq1(local_start);
  sequence seq2(local_start);
  generate(vec1.local_begin(), vec1.local_end(), seq1);
  generate(vec2.local_begin(), vec2.local_end(), seq2);

  // Instantiate the pRange
  boundary b(vec1.local_begin(), vec1.local_end() - 1);
  prange pr(&vec1, b);


  pvector::iterator pr_it = pr.insert(pr.get_boundary().start(), 99);

  pvector::iterator pc_it = vec2.insert(vec2.local_begin(), 99);

  rmi_fence();

  bool corr_val1 = ((*pr_it == 99) && (*pc_it == 99));
  ++pr_it; ++pc_it;
  bool corr_val2 = ((*pr_it == local_start) && (*pc_it == local_start));

  bool corr_pc_size1 = (vec2.local_size() == num_local_elems + 1);
  bool corr_pc_size2 = (vec2.size() == 100 + nThreads);
  bool corr_pr_size = (pr.local_size() == num_local_elems + 1);

  // Report whether or not the test passed
  stapl_bool test_result(corr_val1 && corr_val2 && 
                         corr_pc_size1 && corr_pc_size2 && corr_pr_size);
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
