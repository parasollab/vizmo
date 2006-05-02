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
    cout << "Linear pRange pContainer max_size positive with " 
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


  int pr_size1 = pr.max_size();
  int pc_size1 = vec2.max_size();

  pr.DeleteElement(local_start);
  vec2.DeleteElement(local_start);

  rmi_fence();

  int pr_size2 = pr.max_size();
  int pc_size2 = vec2.max_size();

  pr.clear();
  vec2.clear();

  rmi_fence();

  int pr_size3 = pr.max_size();
  int pc_size3 = vec2.max_size();

  pr.AddElement(100);
  pr.AddElement(101);
  pr.AddElement(102);
  pr.AddElement(103);
  pr.AddElement(104);

  vec2.AddElement(100);
  vec2.AddElement(101);
  vec2.AddElement(102);
  vec2.AddElement(103);
  vec2.AddElement(104);

  rmi_fence();

  int pr_size4 = pr.max_size();
  int pc_size4 = vec2.max_size();

  bool corr_pr = (pr_size1 == pr_size2) && (pr_size3 == pr_size4) &&
	         (pr_size1 == pr_size4) && (pr_size1 == 100);

  bool corr_pc = (pc_size1 == pc_size2) && (pc_size3 == pc_size4) &&
	         (pc_size1 == pc_size4) && (pc_size1 == 100);


  // Report whether or not the test passed
  stapl_bool test_result(corr_pr && corr_pc);
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
