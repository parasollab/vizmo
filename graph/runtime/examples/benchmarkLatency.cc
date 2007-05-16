// In this benchmark, the latency to ping-pong an RMI between two threads is
// measured (i.e., one thread invokes an RMI on the other thread, which in
// turn invokes a reply RMI).  Also, to measure the effectiveness of
// aggregation, a second test measures the latency to issue multiple pings
// before the single pong reply.

#include "stdDev.h"

#include <runtime.h>
#include <stdio.h>


struct p_sync : stapl::parallel_task {
  const int _id;
  const int _nIters;
  stdDevTimer _timer;

  p_sync(const int id, const int nIters)
    : _id(id), _nIters(nIters), _timer(nIters) {}

  int ping() { return 0; }

  void execute() {
    stapl::set_poll_rate( _nIters + 1 );
    if( _id == 0 ) {
      for( int i=0; i<_nIters; i++ ) {
	_timer.startIteration();
	stapl::sync_rmi( 1, rmiHandle, &p_sync::ping );
	_timer.stopIteration();
      }
      printf( "single sync_rmi (s):         " ); _timer.print( _id );
    }
    stapl::rmi_fence();
  }
};


struct p_sync_opaque : stapl::parallel_task {
  const int _id;
  const int _nIters;
  stdDevTimer _timer;

  p_sync_opaque(const int id, const int nIters)
    : _id(id), _nIters(nIters), _timer(nIters) {}

  int ping() { return 0; }

  void execute() {
    stapl::set_poll_rate( _nIters + 1 );
    if( _id == 0 ) {
      for( int i=0; i<_nIters; i++ ) {
	stapl::OpaqueHandle<int> x;
	_timer.startIteration();
	stapl::sync_rmi( 1, rmiHandle, &p_sync_opaque::ping, &x );
	while (!x.ready()) {}
	_timer.stopIteration();
      }
      printf( "single sync_rmi_opaque (s):         " ); _timer.print( _id );
    }
    stapl::rmi_fence();
  }
};


struct p_async : stapl::parallel_task {
  const int _id;
  const int _nIters;
  stdDevTimer _timer;

  p_async(const int id, const int nIters)
    : _id(id), _nIters(nIters), _timer(nIters) {}

  void ping() { stapl::async_rmi(0, rmiHandle, &p_async::pong); }
  void pong() { _timer.stopIteration(); }

  void execute() {
    stapl::set_aggregation( 1 );
    stapl::set_poll_rate( _nIters + 1 );
    for( int i=0; i<_nIters; i++ ) {
      if( _id == 0 ) {
	_timer.startIteration();
	stapl::async_rmi( 1, rmiHandle, &p_async::ping );
      }
      stapl::rmi_wait();
    }
    if( _id == 0 ) { printf( "single async_rmi (s):        " ); _timer.print( _id ); }
    stapl::rmi_fence();
  }
};


struct p_multi_async : stapl::parallel_task {
  const int _id;
  const int _nMulti;
  const int _nIters;
  stdDevTimer _timer;

  p_multi_async(const int id, const int nMulti, const int nIters) 
    : _id(id), _nMulti(nMulti), _nIters(nIters), _timer(nIters) {}

  void ping() {}
  void ping2() { stapl::async_rmi(0, rmiHandle, &p_multi_async::pong); }
  void pong() { _timer.stopIteration(); }

  void execute() { 
    stapl::set_poll_rate( _nIters*_nMulti + 2 );
    for( int i=0; i<_nIters; i++ ) {
      if( _id == 0 ) {
	_timer.startIteration();
	for( int j=2; j<=_nMulti; j++ )
	  stapl::async_rmi( 1, rmiHandle, &p_multi_async::ping );
	stapl::async_rmi( 1, rmiHandle, &p_multi_async::ping2 );
      }
      stapl::rmi_fence();
    }
    if( _id == 0 ) { printf( "multi async_rmi (s):%8d ", _nMulti ); _timer.print( _id ); }
    stapl::rmi_fence();
  }
};


void stapl_main(int argc, char* argv[]) {
  stapl_assert( stapl::get_num_threads() == 2, "must use exactly 2 threads" );
  if( argc <= 2 ) {
    printf( "Usage: %s [nMulti] [nIters]\n", argv[0] );
    return;
  }
  int nMulti = atoi( argv[1] );
  int nIters = atoi( argv[2] );
  int id = stapl::get_thread_id();

  p_sync task1( id, nIters );
  stapl::execute_parallel_task( &task1 );

  p_sync_opaque task2( id, nIters );
  stapl::execute_parallel_task( &task2 );

  p_async task3( id, nIters );
  stapl::execute_parallel_task( &task3 );

  for( int i=10; i<=nMulti; i*=10 ) {
    p_multi_async task4( id, i, nIters );
    stapl::execute_parallel_task( &task4 );
  }
}
