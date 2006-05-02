// In this benchmark, the overhead to create and execute an empty RMI request
// on the local thread is measured.  It compares the overhead of various
// numbers and types of arguments.  It also compares against the regular C++
// means of invoking a method, and invoking a method via a method pointer.

// 1M iterations yields the average overhead, in microseconds
const int NUM_RMI = 1000000;

#include "stdDev.h"
#include "testContainers.h"

#include <runtime.h>
#include <stdio.h>


struct userDefinedObject {
  int i;
  int j;
  double k;
  double l;
  userDefinedObject() : i( 0 ), j( 0 ), k( 0.0 ), l( 0.0 ) {}
  void define_type(stapl::typer& t) {
    t.local( i );
    t.local( j );
    t.local( k );
    t.local( l );
  }
};


struct testObject {
  int test0();
  int test1(int i);
  int test2(int i, int j);
  int testUserDefined(userDefinedObject u);
  int testUserDefinedRef(userDefinedObject& u);
  int testConstUserDefinedRef(const userDefinedObject& u);
  int testVectorRef(testVector<double>& v);
  int testConstVectorRef(const testVector<double>& v);
};

// Some compilers automatically inline methods defined within the class
// definition.  This is not a fair comparison against RMI.
int testObject::test0()                                             { return 0; }
int testObject::test1(int i)                                        { return 0; }
int testObject::test2(int i, int j)                                 { return 0; }
int testObject::testUserDefined(userDefinedObject u)                { return 0; }
int testObject::testUserDefinedRef(userDefinedObject& u)            { return 0; }
int testObject::testConstUserDefinedRef(const userDefinedObject& u) { return 0; }
int testObject::testVectorRef(testVector<double>& v)                { return 0; }
int testObject::testConstVectorRef(const testVector<double>& v)     { return 0; }


userDefinedObject globalUser;
testVector<double> globalVector( 10 );
testObject globalTest;
stapl::rmiHandle handle;


void syncTest0(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::test0 );
}

void syncTest1(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::test1, 1 );
}

void syncTest2(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::test2, 1, 2 );
}

void syncUserDefined(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::testUserDefined, globalUser );
}

void syncUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::testUserDefinedRef, globalUser );
}

void syncConstUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::testConstUserDefinedRef, globalUser );
}

void syncVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::testVectorRef, globalVector );
}

void syncConstVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::sync_rmi( id, handle, &testObject::testConstVectorRef, globalVector );
}


void asyncTest0(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::test0 );
}

void asyncTest1(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::test1, 1 );
}

void asyncTest2(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::test2, 1, 2 );
}

void asyncUserDefined(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::testUserDefined, globalUser );
}

void asyncUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::testUserDefinedRef, globalUser );
}

void asyncConstUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::testConstUserDefinedRef, globalUser );
}

void asyncVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::testVectorRef, globalVector );
}

void asyncConstVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    stapl::async_rmi( id, handle, &testObject::testConstVectorRef, globalVector );
}


void methodPtrTest0(int id) {
  int (testObject::*methodPtr)() = &testObject::test0;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)();
}

void methodPtrTest1(int id) {
  int (testObject::*methodPtr)(int) = &testObject::test1;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( 1 );
}

void methodPtrTest2(int id) {
  int (testObject::*methodPtr)(int, int) = &testObject::test2;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( 1, 2 );
}

void methodPtrUserDefined(int id) {
  int (testObject::*methodPtr)(userDefinedObject) = &testObject::testUserDefined;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( globalUser );
}

void methodPtrUserDefinedRef(int id) {
  int (testObject::*methodPtr)(userDefinedObject&) = &testObject::testUserDefinedRef;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( globalUser );
}

void methodPtrConstUserDefinedRef(int id) {
  int (testObject::*methodPtr)(const userDefinedObject&) = &testObject::testConstUserDefinedRef;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( globalUser );
}

void methodPtrVectorRef(int id) {
  int (testObject::*methodPtr)(testVector<double>&) = &testObject::testVectorRef;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( globalVector );
}

void methodPtrConstVectorRef(int id) {
  int (testObject::*methodPtr)(const testVector<double>&) = &testObject::testConstVectorRef;
  for( int i=0; i<NUM_RMI; i++ )
    (globalTest.*methodPtr)( globalVector );
}


void methodTest0(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.test0();
}

void methodTest1(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.test1( 1 );
}

void methodTest2(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.test2( 1, 2 );
}

void methodUserDefined(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.testUserDefined( globalUser );
}

void methodUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.testUserDefinedRef( globalUser );
}

void methodConstUserDefinedRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.testConstUserDefinedRef( globalUser );
}

void methodVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.testVectorRef( globalVector );
}

void methodConstVectorRef(int id) {
  for( int i=0; i<NUM_RMI; i++ )
    globalTest.testConstVectorRef( globalVector );
}


void stapl_main(int argc, char* argv[]) {
  int id = stapl::get_thread_id();
  if( argc <= 1 ) {
    printf( "Usage: %s [nIters]\n", argv[0] );
    return;
  }
  int nIters = atoi( argv[1] );
  handle = stapl::register_rmi_object( &globalTest );
  stapl::set_poll_rate( 100 * NUM_RMI ); // never poll, everything is local...

  printf( "sync_rmi  (us), 0 args:                   " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncTest0, id );

  printf( "sync_rmi  (us), 1 int arg:                " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncTest1, id );

  printf( "sync_rmi  (us), 2 int args:               " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncTest2, id );

  printf( "sync_rmi  (us), 1 UserDefined arg:        " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncUserDefined, id );

  printf( "sync_rmi  (us), 1 UserDefined& arg:       " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncUserDefinedRef, id );

  printf( "sync_rmi  (us), 1 const UserDefined& arg: " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncConstUserDefinedRef, id );

  printf( "sync_rmi  (us), 1 Vector& arg:            " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncVectorRef, id );

  printf( "sync_rmi  (us), 1 const Vector& arg:      " ); fflush( stdout );
  confidenceExperiment( id, nIters, syncConstVectorRef, id );
  printf( "\n" );


  printf( "async_rmi (us), 0 args:                   " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncTest0, id );

  printf( "async_rmi (us), 1 int arg:                " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncTest1, id );

  printf( "async_rmi (us), 2 int args:               " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncTest2, id );

  printf( "async_rmi (us), 1 UserDefined arg:        " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncUserDefined, id );

  printf( "async_rmi (us), 1 UserDefined& arg:       " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncUserDefinedRef, id );

  printf( "async_rmi (us), 1 const UserDefined& arg: " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncConstUserDefinedRef, id );

  printf( "async_rmi (us), 1 Vector& arg:            " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncVectorRef, id );

  printf( "async_rmi (us), 1 const Vector& arg:      " ); fflush( stdout );
  confidenceExperiment( id, nIters, asyncConstVectorRef, id );
  printf( "\n" );


  printf( "methodPtr (us), 0 args:                   " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrTest0, id );

  printf( "methodPtr (us), 1 int arg:                " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrTest1, id );

  printf( "methodPtr (us), 2 int args:               " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrTest2, id );

  printf( "methodPtr (us), 1 UserDefined arg:        " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrUserDefined, id );

  printf( "methodPtr (us), 1 UserDefined& arg:       " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrUserDefinedRef, id );

  printf( "methodPtr (us), 1 const UserDefined& arg: " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrConstUserDefinedRef, id );

  printf( "methodPtr (us), 1 Vector& arg:            " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrVectorRef, id );

  printf( "methodPtr (us), 1 const Vector& arg:      " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodPtrConstVectorRef, id );
  printf( "\n" );


  printf( "method    (us), 0 args:                   " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodTest0, id );

  printf( "method    (us), 1 int arg:                " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodTest1, id );

  printf( "method    (us), 2 int args:               " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodTest2, id );

  printf( "method    (us), 1 UserDefined arg:        " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodUserDefined, id );

  printf( "method    (us), 1 UserDefined& arg:       " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodUserDefinedRef, id );

  printf( "method    (us), 1 const UserDefined& arg: " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodConstUserDefinedRef, id );

  printf( "method    (us), 1 Vector& arg:            " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodVectorRef, id );

  printf( "method    (us), 1 const Vector& arg:      " ); fflush( stdout );
  confidenceExperiment( id, nIters, methodConstVectorRef, id );
}
