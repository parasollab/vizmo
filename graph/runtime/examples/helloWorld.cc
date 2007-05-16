// In this example, all threads send an RMI request to a master thread (thread
// 0) to display a common greeting message.
//
// p_hello is the parallel_task.  When it is passed to
// execute_parallel_task(), its execute() method is executed in parallel by
// all threads.  Additionally, its local address is registered with all
// threads participating, facilitating calls to its methods (i.e., a call from
// within a remote p_hello to another p_hello object does not need to register
// itself, or otherwise perform address translation, it simply uses
// 'stapl::parallel_task::rmiHandle', and lets the primitives perform
// translation internally as necessary).
//
// async_rmi() sends an RMI request to an object on the specified destination
// thread given that objects's handle on the target thread.  The member
// function (C++ terminology for method) and arguments are also supplied.
// Note that the master thread can also use async_rmi() to access its own
// local objects.
//
// rmi_fence() is used to ensure all RMI requests have completed before
// proceeding.  Threads that arrive process outstanding requests.  After all
// threads arrive and complete processing, all threads release from the fence.
//
// Note the change of 'int main(...)' to 'void stapl_main(...)'.  This allows
// the primitives to use main to do initialization and cleanup internally.

#include <runtime.h>
#include <stdio.h>

struct p_hello : public stapl::parallel_task {
  const int _masterID;
  const int _localID;

  p_hello(const int id) : _masterID(0), _localID(id) {}

  void hello(int requestID) {
    printf( "thread %d RMI's 'hello world' to thread %d\n", requestID, _localID );
  }

  void execute() {
    stapl::async_rmi( _masterID, rmiHandle, &p_hello::hello, _localID );
    stapl::rmi_fence();
  }
};


void stapl_main(int argc, char *argv[]) {
  p_hello ph( stapl::get_thread_id() );
  stapl::execute_parallel_task( &ph );
}
