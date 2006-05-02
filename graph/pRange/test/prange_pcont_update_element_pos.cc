#include <stdlib.h>
#include <iostream>
#include <utility>
#include <pVector.h>
#include <pArray.h>
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

/* Class used as the work function of the second call to UpdateElement. */
class my_negate {
  public:
    int operator()(int& orig) {
      orig *= -1;
    }

    void define_type(stapl::typer& t) { }
};

/* Class used as the work function of the second call to UpdateElement. */
class negate_and_increment {
  public:
    int operator()(int& orig, int& arg) {
      orig = arg - orig;
    }

    void define_type(stapl::typer& t) { }
};

void stapl_main(int argc, char **argv) {

  // Type definitions for the test
  typedef stapl::pArray<int> pvector;
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
    cout << "pRange pContainer UpdateElement positive with " 
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

  my_negate func1;
  vec2.UpdateElement(id, func1);
  pr.UpdateElement(id, func1);
  rmi_fence();

  bool first_corr = true;
  if (id == 0) {
    for (int i = 0; i < nThreads; ++i) {
      if ((vec1[i] > 0) || (vec2[i] > 0))
        first_corr = false;
    }
  }

  negate_and_increment func2;
  int one = 1;
  vec2.UpdateElement(id, func2, one);
  pr.UpdateElement(id, func2, one);
  rmi_fence();

  bool second_corr = true;
  if (id == 0) {
    for (int i = 0; i < nThreads; ++i) {
      if ((vec1[i] != i + 1) || (vec2[i] != i + 1))
        second_corr = false;
    }
  }


  // Report whether or not the test passed
  if (id == 0) {
    if (first_corr && second_corr)
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
