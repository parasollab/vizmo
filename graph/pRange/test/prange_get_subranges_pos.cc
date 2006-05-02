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
  typedef stapl::pRange<boundary, pvector> prange;

  // Finding the thread id, number of threads, and the number of 
  // elements per thread assuming there are 64 elements total.
  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();

  int num_local_elems = 100 / nThreads;
  if (id < 100 % nThreads) ++num_local_elems;

  // Initial output to show what test is running
  if (id == 0) {
    cout << "pRange get_subranges positive with " 
         << nThreads << " threads... ";
  }

  // Instantiate the pContainer 
  pvector vec(100);

  // Initialize the elements of the pContainer
  int local_start = num_local_elems * id;
  sequence seq(local_start);
  generate(vec.local_begin(), vec.local_end(), seq);

  // Instantiate the pRange
  boundary b(vec.local_begin(), vec.local_end() - 1);
  prange pr(&vec, b);


  vector<prange::subrangeType>& subranges = pr.get_subranges();

  bool corr_bound = (subranges[0].get_boundary() == b);
  bool corr_size = (subranges.size() == 1);
  bool corr_gid = (subranges[0].get_global_id() == 10000 * id + 1);

  // Report whether or not the test passed
  stapl_bool test_result(corr_bound && corr_size && corr_gid);
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
