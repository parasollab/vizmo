// In this benchmark, the latency to negotiate the rmi_fence protocol is
// measured.

#include "stdDev.h"

#include <runtime.h>
#include <stdio.h>


void stapl_main(int argc, char* argv[]) {
  int id = stapl::get_thread_id();
  if( argc <= 1 ) {
    printf( "Usage: %s [nIters]\n", argv[0] );
    return;
  }
  int nIters = atoi( argv[1] );

  printf( "rmi_fence overhead (s): " );
  confidenceExperiment( id, nIters, stapl::rmi_fence );
}
