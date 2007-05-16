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

  int num_local_elems = 64 / nThreads;
  if (id < 64 % nThreads) ++num_local_elems;

  // Initial output to show what test is running
  if (id == 0) {
    cout << "pRange get_elem_ddg positive with " 
         << nThreads << " threads... ";
  }

  // Instantiate the pContainer 
  pvector vec(64);
  boundary bound(vec.local_begin(), vec.local_end() - 1);

  // Instantiate the pRange
  prange pr(&vec, bound);

  // Initialize the elements of the pContainer
  int local_start = num_local_elems * id;
  sequence seq(local_start);
  generate(vec.local_begin(), vec.local_end(), seq);

  // Initialize the partial order to be used in the test.
  prange::ElemDDGType* g = new prange::ElemDDGType(); // A partial order that 
                                                      // organizes the elements 
                                                      // into a 2d mesh.

  prange::ElemDDGType* l = new prange::ElemDDGType(); // A linear partial order 
                                                      // of the elements.


  for (int i = local_start; i < local_start + num_local_elems; ++i) {
    DDGTask t(i);
    g->AddVertex(t, i);
    l->AddVertex(t, i);
  }

  rmi_fence();

  for (int i = local_start; i < local_start + num_local_elems - 1; ++i) {
    l->add_dependence(i, i + 1);
  }

  if (id == 0) {
    g->add_dependence( 0,  1); g->add_dependence( 1,  2); g->add_dependence( 2,  3); g->add_dependence( 3,  4);
    g->add_dependence( 4,  5); g->add_dependence( 5,  6); g->add_dependence( 6,  7);

    g->add_dependence( 8,  9); g->add_dependence( 9, 10); g->add_dependence(10, 11); g->add_dependence(11, 12);
    g->add_dependence(12, 13); g->add_dependence(13, 14); g->add_dependence(14, 15);
  } else if (id == 1) {
    g->add_dependence(16, 17); g->add_dependence(17, 18); g->add_dependence(18, 19); g->add_dependence(19, 20);
    g->add_dependence(20, 21); g->add_dependence(21, 22); g->add_dependence(22, 23);

    g->add_dependence(24, 25); g->add_dependence(25, 26); g->add_dependence(26, 27); g->add_dependence(27, 28);
    g->add_dependence(28, 29); g->add_dependence(29, 30); g->add_dependence(30, 31);
  } else if (id == 2) {

    g->add_dependence(32, 33); g->add_dependence(33, 34); g->add_dependence(34, 35); g->add_dependence(35, 36);
    g->add_dependence(36, 37); g->add_dependence(37, 38); g->add_dependence(38, 39);

    g->add_dependence(40, 41); g->add_dependence(41, 42); g->add_dependence(42, 43); g->add_dependence(43, 44);
    g->add_dependence(44, 45); g->add_dependence(45, 46); g->add_dependence(46, 47);
  } else {
    g->add_dependence(48, 49); g->add_dependence(49, 50); g->add_dependence(50, 51); g->add_dependence(51, 52);
    g->add_dependence(52, 53); g->add_dependence(53, 54); g->add_dependence(54, 55);

    g->add_dependence(56, 57); g->add_dependence(57, 58); g->add_dependence(58, 59); g->add_dependence(59, 60);
    g->add_dependence(60, 61); g->add_dependence(61, 62); g->add_dependence(62, 63);
  }

  l->initialize_base_num_predecessors();
  l->reset();
  pr.set_element_ddg(l);

  prange::ElemDDGType& ddg_ref1 = pr.get_element_ddg();
  ddg_ref1.processed(0);
  prange::ElemDDGType& ddg_ref2 = pr.get_element_ddg();

  bool eq = (ddg_ref1 == ddg_ref2);



  // Report whether or not the test passed
  stapl_bool test_result(eq);
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
