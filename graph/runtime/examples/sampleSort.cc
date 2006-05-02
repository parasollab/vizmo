// This example implements parallel sample sort.  Each thread generates a
// portion of an overall vector<int> composed of n random integers, then
// globally sorts this data.  Note the manual distribution of data (each thread
// allocates n/nThreads integers), which could be substituted with a pVector.

#if defined( __sgi ) || defined( __hpux )
#define _POSIX_C_SOURCE 199506L // must enable rand_r on some platforms
#endif

#include "stdDev.h"

#include <runtime.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>


struct p_sample_sort : public stapl::parallel_task {
  std::vector<int>* _input;
  std::vector<int> _tmp;

  p_sample_sort(std::vector<int>* input) : _input(input), _tmp(0) { 
    _tmp.reserve( _input->size() / 4 * 3 ); // reduce number of costly allocations
  }

  void copy_splitters(int* local, int* in, const int count) {
    std::copy( in, in+count, local );
  }

  void insert(const int i) {
    _tmp.push_back( i );
  }

  void execute() {
    int id = stapl::get_thread_id();
    int nThreads = stapl::get_num_threads();
    int size = _input->size();
    std::vector<int>::iterator input = _input->begin();

    // Sample a set of local splitters (there are oversampling of them)
    const int oversampling = ( 128 > size ) ? size : 128;
    const int sampleRate = size / oversampling;
    int* localSplitters = static_cast<int*>( malloc(sizeof(int) * nThreads * oversampling) );
    for( int i=0; i<oversampling; i++ )
      localSplitters[i+id*oversampling] = input[i*sampleRate];

    // Broadcast and sort the local splitters
    for( int i=0; i<nThreads; i++ )
      stapl::broadcast_rmi( &localSplitters[i*oversampling], oversampling,
			    rmiHandle, &p_sample_sort::copy_splitters, i );
    std::sort( &localSplitters[0], &localSplitters[nThreads*oversampling] );

    // Pick the final set of global splitters (there are nThreads-1 of them)
    int* globalSplitters = static_cast<int*>( malloc(sizeof(int) * nThreads-1) );
    for( int i=0; i<nThreads-1; i++ )
      globalSplitters[i] = localSplitters[(i+1)*oversampling];
    free( localSplitters );

    // Distribute the input based on the global splitters
    stapl::set_poll_rate( nThreads * stapl::get_aggregation() ); // ideally 1 elem per thread, per nThreads elems
    for( int i=0; i<size; i++ ) {
      int thread = std::lower_bound( &globalSplitters[0], &globalSplitters[nThreads-1], 
				     input[i], std::less<int>() ) - &globalSplitters[0];
      stapl::async_rmi( thread, rmiHandle, &p_sample_sort::insert, input[i] );
    }
    stapl::rmi_fence();

    // Sort the now distributed data back into the input vector
    std::sort( _tmp.begin(), _tmp.end() );
    //_tmp.swap( *_input ); // This swap is O(1).  If we don't do it, each iteraton
			    // will operate on the same input.  Otherwise all
			    // subsequent iterations will sort already sorted data, 
			    // which is the worst case for sample sort.
  }
};


void p_sort(std::vector<int>* input) {
  p_sample_sort pss( input );
  stapl::execute_parallel_task( &pss );
}


void stapl_main(int argc, char *argv[]) {
  int id     = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if( id == 0 )
    if( argc <= 2 ) {
      std::cout << "Usage: " << argv[0] << " [numints] [numiterations]\n";
      return;
    }
  int nElems = atoi( argv[1] );
  int nIters = atoi( argv[2] );

  std::vector<int> input( nElems / nThreads );
  unsigned int seed = ( id+1 ) * time( 0 );
  for( std::vector<int>::iterator i=input.begin(); i!=input.end(); ++i )
    *i = rand_r( &seed ) % nElems;

// A very inelegant way to display the input...
//  for( int i=0; i<nThreads; i++ ) {
//    stapl::rmi_fence();
//    if( id == i ) {
//	    std::ostream_iterator<int> out( std::cout, " " );
//	    std::cout << "thread " << id << ": ";
//	    std::copy( input.begin(), input.end(), out );
//	    std::cout << std::endl << std::flush;
//    }
//  }
//  stapl::rmi_fence();
  confidenceExperiment( id, nIters, p_sort, &input );
}
