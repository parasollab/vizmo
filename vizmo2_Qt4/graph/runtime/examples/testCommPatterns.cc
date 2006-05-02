// This test stresses various patterns of RMI communication, focusing on
// cyclic/nested invocations, common in graph algorithms.  DO NOT BE SURPRISED
// IF AN IMMATURE IMPLEMENTATION HANGS/DEADLOCKS ON THIS TEST!

#include <runtime.h>
#include <stdio.h>


struct p_test : public stapl::parallel_task {
  int _nThreads; // number of participating threads
  int _id;       // local id
  int _lt, _rt;  // neighbor id's

  p_test() {
    _nThreads = stapl::get_num_threads();
    _id = stapl::get_thread_id();
    _rt = ( _id == _nThreads - 1 ) ? 0 : _id + 1;
    _lt = ( _id == 0 ) ? _nThreads - 1 : _id - 1;
  }

  int dummy() { return 1; }

  int sync_nest(const int nest) {
    int n = ( _id == 0 ) ? nest-1 : nest;
    if( n != 0 )
      stapl_assert( n == stapl::sync_rmi(_rt, rmiHandle, &p_test::sync_nest, n), "" );
    return nest;
  }

  void async_nest(int nest) {
    int n = ( _id == 0 ) ? nest-1 : nest;
    if( n != 0 ) {
      stapl::async_rmi( _lt, rmiHandle, &p_test::async_nest, n );
      // If only one message can be in-flight at a time, this second call
      // could block.  Although this is legal, make sure the block doesn't
      // eventually cause deadlock (i.e., block without polling).
      stapl::async_rmi( _lt, rmiHandle, &p_test::async_nest, 1 );
    }
  }

  void execute() {
    for( int i=0; i<2; i++ ) {

      // Test using default/maximum aggregation/poll_rate on the first pass,
      // and minimal settings on the second pass
      if( i == 1 ) {
	stapl::set_aggregation( 1 );
	stapl::set_poll_rate( 1 );
      }

      // Test centralized communication to a single thread, 0, mixed with
      // dummy calls to stress the internal buffers.
      stapl::async_rmi( 0, rmiHandle, &p_test::sync_nest, 1 );
      stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
      stapl::sync_rmi( 0, rmiHandle, &p_test::sync_nest, 1 );
      stapl::async_rmi( _lt, rmiHandle, &p_test::dummy );
      stapl::rmi_fence();

      // Test cyclic sync_rmi communication (i.e., sync_rmi's form a cycle:
      // 0 -> 1 -> ... -> n -> 0), mixed with dummy calls to stress the
      // internal buffers.
      if( _id == 0 ) {
	stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
	stapl::sync_rmi(  _rt, rmiHandle, &p_test::sync_nest, 1 );
	stapl::async_rmi( _lt, rmiHandle, &p_test::dummy );
	stapl::sync_rmi(  _lt, rmiHandle, &p_test::sync_nest, 2 );
	stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
	stapl::sync_rmi(  _rt, rmiHandle, &p_test::sync_nest, 10 );
	stapl::async_rmi( _lt, rmiHandle, &p_test::dummy );
	stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
      }
      else {
	stapl::async_rmi( _lt, rmiHandle, &p_test::dummy );
	stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
	stapl::sync_rmi(  _rt, rmiHandle, &p_test::sync_nest, 1 );
	stapl::async_rmi( _rt, rmiHandle, &p_test::dummy );
	stapl::async_rmi( _lt, rmiHandle, &p_test::dummy );
      }
      stapl::rmi_fence();

      // Test cyclic async_rmi communication.
      if( _id == 0 ) stapl::async_rmi( _lt, rmiHandle, &p_test::async_nest, 1 );
      stapl::rmi_fence();
      if( _id == 0 ) stapl::async_rmi( _lt, rmiHandle, &p_test::async_nest, 2 );
      stapl::rmi_fence();
      if( _id == 0 ) stapl::async_rmi( _lt, rmiHandle, &p_test::async_nest, 10 );
      stapl::rmi_fence();
    }
  }
};


void stapl_main(int argc, char *argv[]) {
  p_test pt;
  stapl::execute_parallel_task( &pt );
  printf( "%d successfully passed!\n", stapl::get_thread_id() );
}
