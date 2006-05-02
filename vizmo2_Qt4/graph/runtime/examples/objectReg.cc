// In this example, each thread declares and registers a few local containers,
// then inserts elements to the other threads' containers.  This demonstrates
// the address translation provided by the parallel primitives.  
//
// This example is similar to the action of the constructor of a pContainer,
// which creates local sub-containers on each thread.  Note, a real pContainer
// could store the rmiHandle internally, register it in the constructor,
// update it if the pContainer ever moved, and unregister it in the
// destructor.

#include <runtime.h>
#include <stdio.h>
#include <vector>
#include <list>


void stapl_main(int argc, char *argv[]) {
  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();

  // create and register local containers
  std::vector<int> vLocal( 0 );
  stapl::rmiHandle vHandle = stapl::register_rmi_object( &vLocal );
  std::list<double>* lLocal = new std::list<double>( 0 );
  stapl::rmiHandle lHandle = stapl::register_rmi_object( lLocal );

  // make sure all threads have registered before communicating
  stapl::rmi_fence();

  // communicate: insert local id to all thread's containers
  for( int i=0; i<nThreads; i++ ) {
    stapl::async_rmi( i, vHandle, &std::vector<int>::push_back, id );
    stapl::async_rmi( i, lHandle, &std::list<double>::push_front, static_cast<double>(id) );
  }
  stapl::rmi_fence();

  // move the local list and update it in the registry
  lLocal = new std::list<double>( *lLocal );
  stapl::update_rmi_object( lHandle, lLocal );
  stapl::rmi_fence();

  // communicate: insert 0 to all threads's lists
  for( int i=0; i<nThreads; i++ )
    stapl::async_rmi( i, lHandle, &std::list<double>::push_back, 0.0 );
  stapl::rmi_fence();

  // unregister containers
  stapl::unregister_rmi_object( lHandle );
  stapl::unregister_rmi_object( vHandle );

  // test results: both number of RMI's received, and their contents
  stapl_assert( vLocal.size() == nThreads, "Incorrect vector size!" );
  stapl_assert( lLocal->size() == 2*nThreads, "Incorrect list size!" );
  int correct=0;
  int vSum=0;
  double lSum=0;
  for( int i=0; i<nThreads; i++ )
    correct += i;
  for( std::vector<int>::iterator i=vLocal.begin(); i!=vLocal.end(); ++i )
    vSum += *i;
  for( std::list<double>::iterator i=lLocal->begin(); i!=lLocal->end(); ++i )
    lSum += *i;
  stapl_assert( vSum == correct, "Incorrect vector arguments passed!" );
  stapl_assert( lSum == correct, "Incorrect list  arguments passed!" );
  printf( "%d successfully passed!\n", id );
}
