#include <stdlib.h>
#include <iostream>
#include <pvector.h>
#include <pRange.h>
#include <runtime.h>

using namespace stapl;
using namespace std;

template<class pRange, class T>
struct p_sum : public parallel_task {

  pRange pr;
  T result;

public:
  p_sum(pRange &p) : pr(p) { }

  void sum(int* in, int* inout) { *inout += *in; }

  T get_res() { return result; }

  void execute() {
    T rtn = 0;
    if (0 == pr.get_num_subranges()) {
      for (typename pRange::iteratorType i  = pr.get_boundary().start(); 
                                         i != pr.get_boundary().finish(); 
                                       ++i) {
        rtn += *i;
      }
    } else {
      vector<typename pRange::subrangeType>& subranges = pr.get_subranges();
      for (typename vector<typename pRange::subrangeType>::iterator i  = subranges.begin();
                                             i != subranges.end();
                                           ++i) {
        for (typename pRange::iteratorType j  = i->get_boundary().start();
                                           j != i->get_boundary().finish();
                                         ++j) {
          rtn += *j;
        }
      }
    }

    stapl::reduce_rmi( &rtn, &result, rmiHandle, &p_sum::sum, true);
  }

};

void stapl_main(int argc, char **argv) {
  typedef stapl::pvector<int> pvector;
  typedef stapl::linear_boundary<pvector::iterator> linear_boundary;
  typedef stapl::pRange<linear_boundary, pvector> prange;
//  typedef s_range<pvector, linear_boundary> srange;

  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if (id == 0) {
    cout << "Testing psum with " << nThreads << " threads... ";
    if( argc <= 1 ) {
      cout << "Failed:  Usage: " << argv[0] << " [numints]\n";
      return;
    }
  }
  int nElems = atoi( argv[1] );

  pvector vec(nElems);
  int count = 1;
  for (pvector::iterator i  = vec.local_begin(); 
                         i != vec.local_end(); 
                       ++i, ++count)
    *i = count;

  prange pr(&vec, linear_boundary(vec.local_begin(), vec.local_end()));
  p_sum<prange, int> ps(pr);
  execute_parallel_task(&ps);


  if (id == 0 ) {
    int chk = nThreads * (((nElems/nThreads) * (nElems/nThreads + 1)) / 2);
    if (ps.get_res() == chk)
      cout << "Passed\n";
    else
      cout << "Failed: " << ps.get_res() << " != " << chk << endl;
  }
}
