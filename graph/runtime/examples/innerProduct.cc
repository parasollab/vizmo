// This example implements pi_inner_product().  It basically calls
// std::inner_product() for each thread's portion of the calculation, then
// uses stapl::reduce_rmi() to combine the results.  Note, two versions are
// provided.  One, simple, performs the inner_product as described.  The other
// handles the more complicated case where a given's thread's input may be
// empty, as required by STL.

#include "stdDev.h"

#include <runtime.h>
#include <stdio.h>
#include <vector>
#include <numeric>


template<class InputIterator1, class InputIterator2, class T>
struct p_simple_inner_product : public stapl::parallel_task {
  InputIterator1 _first1, _last1;
  InputIterator2 _first2;
  T result;

  p_simple_inner_product(InputIterator1 first1, InputIterator1 last1, 
			 InputIterator2 first2)
    : _first1(first1), _last1(last1), _first2(first2), result(0) {}

  void sum(int* in, int* inout) { *inout += *in; }

  void execute() {
    T rtn = std::inner_product( ++_first1, _last1, ++_first2, ((*_first1) * (*_first2)) );
    stapl::reduce_rmi( &rtn, &result, rmiHandle, &p_simple_inner_product::sum, true );
  }
};

template<class InputIterator1, class InputIterator2, class T>
T simple_inner_product(InputIterator1 first1, InputIterator1 last1,
		       InputIterator2 first2) {
  p_simple_inner_product<std::vector<int>::iterator, std::vector<int>::iterator, int>
    pip( first1, last1, first2 );
  stapl::execute_parallel_task( &pip );
  stapl_assert( pip.result == (last1-first1)*stapl::get_num_threads(), "simple_inner_product produced the wrong answer!" );
  return pip.result;
}


template<class InputIterator1, class InputIterator2, class T>
struct p_inner_product : public stapl::parallel_task {
  InputIterator1 _first1, _last1;
  InputIterator2 _first2;
  T result;

  p_inner_product(InputIterator1 first1, InputIterator1 last1, 
		  InputIterator2 first2, T init)
    : _first1(first1), _last1(last1), _first2(first2), result(init) {}

  struct combo {
    int valid;
    T datum;
    combo() : valid(false), datum(0) {}
    void define_type(stapl::typer& t) { t.local(valid); t.local(datum); }
  };

  void sum(combo* in, combo* inout) {
    if( in->valid )
      if( inout->valid ) { inout->datum += in->datum; }
      else { inout->valid = true; inout->datum = in->datum; }
  }

  void execute() {
    combo c, rtn;
    if( _first1 != _last1 ) {
      c.valid = true;
      if( stapl::get_thread_id() == 0 )
	c.datum = std::inner_product( _first1, _last1, _first2, result );
      else
	c.datum = std::inner_product( ++_first1, _last1, ++_first2, ((*_first1) * (*_first2)) );
    }
    stapl::reduce_rmi( &c, &rtn, rmiHandle, &p_inner_product::sum, true );
    result = rtn.datum;
  }
};

template<class InputIterator1, class InputIterator2, class T>
T pi_inner_product(InputIterator1 first1, InputIterator1 last1,
		   InputIterator2 first2, T init) {
  p_inner_product<std::vector<int>::iterator, std::vector<int>::iterator, int>
    pip( first1, last1, first2, init );
  stapl::execute_parallel_task( &pip );
  stapl_assert( pip.result == (last1-first1)*stapl::get_num_threads(), "pi_inner_product produced the wrong answer!" );
  return pip.result;
}


void stapl_main(int argc, char *argv[]) {
  int id     = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if( id == 0 )
    if( argc <= 2 ) {
      printf( "Usage: %s [nElems] [nIterations]\n", argv[0] );
      return;
    }
  int nElems = atoi( argv[1] );
  int nIters = atoi( argv[2] );
  std::vector<int> input( nElems / nThreads, 1 );
  typedef std::vector<int>::iterator iter;

  confidenceExperiment( id, nIters, (int (*)(iter,iter,iter)) simple_inner_product<iter,iter,int>,
			input.begin(), input.end(), input.begin() );

  confidenceExperiment( id, nIters, (int (*)(iter,iter,iter,int)) pi_inner_product<iter,iter,int>, 
			input.begin(), input.end(), input.begin(), 0 );
}
