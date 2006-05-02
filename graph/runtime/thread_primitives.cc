 //****************************************************************************
// Implements the primitives using generic threads.  Use of real thread calls
// are limited to accessing thread specific data and forking the parallel
// work threads.
//
// This implementation performs "owner-computes", meaning when a thread issues
// an RMI, it must enqueue the request to the destination/owner, such that
// only the owner actually executes the request.  There is another
// implementation, called "caller-computes", that possibly takes more
// advantage of shared-memory by allowing threads to perform requests
// directly, but also breaks somewhat from the traditional view of RMI.  It is
// in CVS under the "threadCallerComputes" tag.
//
// Currently, OpenMP is used if _OPENMP is defined, otherwise Pthreads are
// used.  Many compilers that support OpenMP require a special compile-line
// flag (e.g, SGI's -mp), which defines _OPENMP and recognizes the omp
// pragmas.  If your compiler does not do this, you may need to manually
// define _OPENMP (e.g., -D_OPENMP).
//
// The number of threads to use is determined by the STAPL_NUM_THREADS
// environment variable.
//****************************************************************************

#define _COMPILING_PRIMITIVES

#include "primitives.h"
#include <string.h>
#include <sched.h> // for sched_yield()


// Size, in bytes, of the outgoing RMI request buffers (actually in chars,
// which are generally 1 byte each).  _SEND_BUF_MAX is for aggregating
// async_rmi requests.  May be defined at compile time (e.g., -D_SEND_BUF_MAX=???).
#ifndef _SEND_BUF_MAX
#define _SEND_BUF_MAX 8192
#endif


// The size of a cache line.  Used to pad shared variables to prevent false
// sharing.  May be defined at compile time (e.g., -D_CACHE_LINE_SIZE=???).
#ifndef _CACHE_LINE_SIZE
#define _CACHE_LINE_SIZE 64
#endif


// The number of iterations to busy-wait before yielding the processor to
// another available thread.  0 means busy-wait without yielding.  May be
// defined at compile time (e.g., -D_BUSY_WAIT_MAX=???).
#ifndef _BUSY_WAIT_MAX
#define _BUSY_WAIT_MAX 100
#endif

#define busy_wait( waitTest, waitOperation ) { \
  unsigned int count = 0; \
  while( waitTest ) { \
    waitOperation; \
    if( ++count == _BUSY_WAIT_MAX ) { count = 0; sched_yield(); } \
  } \
}


// Forward declarations
namespace thread_primitives {
  struct threadInfo;
  void thread_main(const int threadID, const int numThreads);
  void poll(threadInfo* const t);
  void flush(threadInfo* const t);
  void periodic_poll(threadInfo* const t);
}


// Code specific to a given thread package, implementing the functions to
// access thread specific data, fork worker threads, and a simpleLock.
#ifdef _OPENMP
#include "thread_primitives_openmp.h"
#else
#include "thread_primitives_pthreads.h"
#endif




namespace thread_primitives {

//****************************************************************************
// Per-Thread Private Data
// - some platforms enable thread local storage (i.e. per-thread globals)
// whereas others only work through get/set_key functions.  Inline function 
// calls are used to efficiently choose the best available option from the
// implementing thread package.
// - variables have been ordered by use, which can improve cache performance,
// but somewhat obfuscates readability
//****************************************************************************

// Each thread has a set of private buffers, one per destination thread, for
// aggregating async_rmi requests.  The buffer has several sections, such that
// one section may be filled while others are being sent and processed.
struct perThreadSendBuffer {
  unsigned int offset;     // offset for filling the current section
  unsigned int aggCount;   // number of requests aggregated in the current section
  char* buf;               // pointer to the current section

private:
  struct section {
    volatile bool done;  // true when section is available, false when its enqueued at the destination
    double kludge;       // kludge, to ensure section is properly aligned
    char buf[_SEND_BUF_MAX];
    section() : done( true ) {}
  };
  enum initSections { MAX=10 };
  unsigned int sectionID;   // id of the current section
  unsigned int numSections; // number of available sections
  section** sections;       // growable array of sections

  // If all sections are busy, allocate a new one at the end.  This allows for
  // unlimited sections, bounded only by the total available memory.
  void increaseSections() {
    ++numSections;
    sections = static_cast<section**>( realloc(sections, sizeof(section*)*numSections) );
    stapl_assert( sections != 0, "Memory Allocation Failed" );
    sections[sectionID] = new section;
  }

public:
  perThreadSendBuffer() : offset( 0 ), aggCount( 0 ), sectionID( 0 ), numSections( MAX ) {
    sections = static_cast<section**>( safe_malloc(sizeof(section*)*numSections) );
    for( int i=0; i<numSections; ++i )
      sections[i] = new section;
    buf = sections[sectionID]->buf;
  }
  ~perThreadSendBuffer() {
    for( int i=0; i<numSections; ++i )
      delete sections[i];
    free( sections );
  }

  char* begin() { return buf; }
  char* end() { return &buf[offset]; }
  volatile bool& done() { return sections[sectionID]->done; }

  // look for the first available section and reset it for filling
  void reset() {
    for( sectionID=0; sectionID<numSections; ++sectionID )
      if( sections[sectionID]->done == true )
	break;
    if( sectionID == numSections )
      increaseSections();
    offset = 0;
    aggCount = 0;
    buf = sections[sectionID]->buf;
  }
};
const unsigned int maxAggregation = _SEND_BUF_MAX / sizeof( rmiRequest0<arg_traits<int>,void> );


struct threadInfo {
  unsigned int pollCount;       // number of calls since last poll
  unsigned int pollMax;         // max number of calls before polling
  perThreadSendBuffer* sendBuf; // thread's aggregation buffer
  const int threadID;           // thread's STAPL id (different from Pthreads id)
  unsigned int aggMax;          // max aggregation
  const int numThreads;         // number of threads
  bool privateSense;            // used by fence for sense reversal
  rmiRegistry registry;         // the RMI object registry
  int localSR;                  // #sends - #receives
  unsigned int numRMI;          // number of RMI requests received since the last rmi_fence
  unsigned int numWaits;        // number of rmi_waits since the last rmi_fence
  void *stackPtr;               // my stack pointer;

  threadInfo(const int id, const int nThreads) :
    pollCount( 0 ),
    pollMax( 1 ),
    sendBuf( new perThreadSendBuffer[nThreads] ),
    threadID( id ),
    aggMax( 1 ),
    numThreads( nThreads ),
    privateSense( false ),
    registry(),
    localSR( 0 ),
    numRMI( 0 ),
    numWaits( 0 ) {}
  ~threadInfo() { delete[] sendBuf; }
};




//****************************************************************************
// An implementation of the tree-based barrier, modified to poll for incoming
// RMI requests and sum all localSRs, described as algorithm 11 in:
// J. Mellor-Crummey and M. Scott. Algorithms for Scalable Synchronization
// on Shared-Memory Multiprocessors. ACM Transactions on Computer Systems,
// 9(1):21-65, 1991.
//****************************************************************************

class fenceNodeBase {
  union packedBoolArray {
    volatile unsigned int volatileWhole;
    unsigned int whole;
    bool part[4];
    packedBoolArray() {
      stapl_assert( sizeof(bool[4]) <= sizeof(unsigned int), "packedBoolArray union incorrectly sized" );
    }
  };
public:
  packedBoolArray childNotReady;
  packedBoolArray haveChild;
  volatile int localSR;
  volatile int* childLocalSR[4];
  volatile bool* parentPtr;
  bool boolDummy;
  int intDummy;
};
class fenceNode : public fenceNodeBase {
  char _padding[_CACHE_LINE_SIZE - sizeof(fenceNodeBase)%_CACHE_LINE_SIZE];
public:
  static volatile int globalSR;
  static volatile bool globalSense;
};
volatile int  fenceNode::globalSR = 0;
volatile bool fenceNode::globalSense = true;
fenceNode* fenceNodes;


void init_fenceNode(fenceNode* n, const int threadID, const int numThreads) {
  for( int i=0; i<4; ++i )
    n->haveChild.part[i] = ( 4*threadID+i+1 < numThreads ) ? true : false;
  n->childNotReady.whole = n->haveChild.whole;

  n->localSR = 0;
  n->childLocalSR[0] = ( 4*threadID+1 >= numThreads ) ? &n->intDummy : &fenceNodes[4*threadID+1].localSR;
  n->childLocalSR[1] = ( 4*threadID+2 >= numThreads ) ? &n->intDummy : &fenceNodes[4*threadID+2].localSR;
  n->childLocalSR[2] = ( 4*threadID+3 >= numThreads ) ? &n->intDummy : &fenceNodes[4*threadID+3].localSR;
  n->childLocalSR[3] = ( 4*threadID+4 >= numThreads ) ? &n->intDummy : &fenceNodes[4*threadID+4].localSR;

  if( threadID == 0 )
    n->parentPtr = &n->boolDummy;
  else
    n->parentPtr = &fenceNodes[int((threadID-1)/4)].childNotReady.part[(threadID-1)%4];

  n->boolDummy = false;
  n->intDummy = 0;
}


int fence(threadInfo* const t) {
  register fenceNode* n = &fenceNodes[t->threadID];

  // Wait for all children to arrive
  busy_wait( n->childNotReady.volatileWhole != 0, { poll(t); flush(t); } );
  n->childNotReady.whole = n->haveChild.whole;

  // Calculate the parent and its children's portion of globalSR
  n->localSR = t->localSR + *n->childLocalSR[0] + *n->childLocalSR[1]
                          + *n->childLocalSR[2] + *n->childLocalSR[3];

  // Notify parent of arrival
  *n->parentPtr = false;

  // Wait for release, and propagate to children
  if( t->threadID == 0 ) {
    fenceNode::globalSR = n->localSR;
    fenceNode::globalSense = t->privateSense;
  }
  else
    busy_wait( fenceNode::globalSense != t->privateSense, { poll(t); flush(t); } );

  // Reverse the privateSense and return the globalSR
  t->privateSense = !t->privateSense;
  return fenceNode::globalSR;
}




//****************************************************************************
// Internal buffer management functions
//****************************************************************************

// Each thread has a request queue with a fixed number of entries (i.e., the
// maximum number of requests that may be received and un-processed).  The
// queue is a circular buffer implementation of the producer-consumer problem.
// Completion is signaled by setting entry::_done to true.
struct requestBase {
  enum maxEntries { EXPANSION=21 };

  struct entry {
    char* _begin;
    char* _end;
    volatile bool* _done;
  };

  simpleLock lock;
  unsigned int current;
  unsigned int last;
  const int size;
  entry* requests;

  requestBase(const int nThreads) : current( 0 ), last( 0 ), size( nThreads*EXPANSION ) {
    requests = static_cast<entry*>( safe_malloc(sizeof(entry) * size) );
  }

  bool trylockEntry() {
    lock.lock();
    if( (last+1)%size == current ) {
      lock.unlock();
      return false;
    }
    return true;
  }

  void enqueue(char* begin, char* end, volatile bool* done) {
    busy_wait( !trylockEntry(), ; ); // no polls, to prevent nested RMI invocation
    requests[last]._begin = begin;
    requests[last]._end = end;
    requests[last]._done = done;
    last = (last + 1) % size;
    lock.unlock();
  }

  bool dequeue(threadInfo* const t) {
    bool rtn = false;
    while( current != last ) {
      lock.lock();
      char* begin = requests[current]._begin;
      char* end = requests[current]._end;
      volatile bool* done  = requests[current]._done;
      current = (current + 1) % size;
      lock.unlock();
      for( char* c=begin; c!=end; c+=((rmiRequest*)c)->_size )
	reinterpret_cast<rmiRequest*>( c )->exec( t->registry );
      *done = true;
      --t->localSR;
      ++t->numRMI;
      rtn = true;
    }
    return rtn;
  }
};

struct request : public requestBase {
  char _padding[_CACHE_LINE_SIZE - (sizeof(requestBase))%_CACHE_LINE_SIZE];
  request(const int nThreads) : requestBase( nThreads ) {}
};
request** recvBuf;


inline void addAsyncRequest(const int destThread, threadInfo* const t, 
			    perThreadSendBuffer& send) {
  send.done() = false;
  recvBuf[destThread]->enqueue( send.begin(), send.end(), &send.done() );
  send.reset();
  ++t->localSR;
}


void addSyncRequest(const int destThread, threadInfo* const t,
		    char* const begin, char* const end) {
  volatile bool tmp = false;
  recvBuf[destThread]->enqueue( begin, end, &tmp );
  busy_wait( tmp == false, poll(t) );
  ++t->localSR;
}


bool findRequest(threadInfo* const t) {
  return recvBuf[t->threadID]->dequeue( t );
}




//****************************************************************************
// Data structures shared between all threads, as well as the function each
// thread actually executes, for calling stapl_main.
//****************************************************************************
 threadInfo **allThreadInfo;
#ifdef _ARMI_REGISTRY_VERIFY_TRACE 
  unsigned int getStackPtr(void) {
    unsigned int howdy = 0;
    asm("mov %%esp, %0" : "=r" (howdy));
    return howdy;
  }

  void checkRegistryTrace(void) {
    const std::vector<void*> &compVector = allThreadInfo[0]->registry.getTrace();
    int compSize = compVector.size();

   //Check That All Registration Traces Are Same Size
    for (int i = 1; i < numThreads; i++) {
      int mySize = allThreadInfo[i]->registry.getTrace().size();
      stapl_assert(compSize == mySize,"Automatic Registration Failure - registry size not equal");
    }

    //Check That All Registration Traces Are Equivalent
    for (int i = 1; i < numThreads; i++) {
      for (int j = 0; j < compSize; j++) {
	const std::vector<void*> &myVector = allThreadInfo[i]->registry.getTrace();
       	int index1 = (int) compVector[j]  - (int) allThreadInfo[0]->stackPtr;
	int index2 = (int) myVector[j] - (int) allThreadInfo[i]->stackPtr;
	//printf("checking indices %d / %d\n", index1, index2);
 	stapl_assert(index1==index2,"Automatic Registration Failure - nonSPMD registration");
      }
    }
  }    
#endif // _ARMI_REGISTRY_VERIFY_TRACE 
  // The user-specified command line arguments
  int                argc;
  char**             argv;
  
  // The work executed by each thread
  void thread_main(const int threadID, const int numThreads) {
    threadInfo *t = new threadInfo(threadID, numThreads); set_threadInfo(t);
    allThreadInfo[threadID] = t;
    
#ifdef _ARMI_REGISTRY_VERIFY_TRACE
    t->stackPtr = (void *) getStackPtr();
#endif //_ARMI_REGISTRY_VERIFY_TRACE

    recvBuf[threadID] = new request( numThreads );
    rmi_fence();
    stapl_main( argc, argv );
    rmi_fence();
    delete recvBuf[threadID];
  }




//****************************************************************************
// Parallelization Primitives
//****************************************************************************

int get_num_threads() {
  return get_threadInfo()->numThreads;
}


int get_thread_id() {
  return get_threadInfo()->threadID;
}




//****************************************************************************
// Registration Primitives
//****************************************************************************
#ifndef _ARMI_REGISTRY_CHECK_TYPE 

  rmiHandle register_rmi_object(void* const objectPointer) {
    return get_threadInfo()->registry.registerObject( objectPointer );
  }
    
  void update_rmi_object(rmiHandle handle, void* const objectPointer) {
    get_threadInfo()->registry.updateObject( handle, objectPointer );
  }
  
#else
  
  rmiHandle register_rmi_object(void* const objectPointer, 
				const std::type_info &objectType) {
    return get_threadInfo()->registry.registerObject( objectPointer, objectType);
  }
  
  rmiHandle register_rmi_object(void *const objectPonter, int id) {
    return get_threadInfo()->registry.registerObject( objectPointer, id );
  }

  void update_rmi_object(rmiHandle handle, void* const objectPointer,
			 const std::type_info &objectType) {
    get_threadInfo()->registry.updateObject( handle, objectPointer, objectType);
  }
  
#endif //_ARMI_REGISTRY_CHECK_TYPE 

  void unregister_rmi_object(rmiHandle const &handle) {
    get_threadInfo()->registry.unregisterObject( handle );
  }
  
  
  void execute_parallel_task(parallel_task* const task) {
    stapl_assert( task != 0, "parallel_task* equals 0" );
    threadInfo* const t = get_threadInfo();  
    const unsigned int tmpAgg = t->aggMax; t->aggMax = maxAggregation;
    const unsigned int tmpPoll = t->pollMax; t->pollMax = maxAggregation;
    t->registry.fastRegisterTask( task );
    fence( t ); // ensure all threads have registered
    task->execute();
    t->aggMax = tmpAgg;
    t->pollMax = tmpPoll;
  }




//****************************************************************************
// Communication Primitives
//****************************************************************************

// Find space for async_rmi requests directly in sendBuf.  mustSend is set to
// true if a subsequent call to send_async_rmi is necessary (although this is
// ugly, it improves performance by up to 25%, by avoiding the second call to
// get_threadInfo, which is expensive if implemented with pthread_getspecific).
void* rmiRequest::operator new(size_t size, const int destThread, bool& mustSend) {
  dassert((destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");
  threadInfo* const t = get_threadInfo();
  periodic_poll( t );
  perThreadSendBuffer& send = t->sendBuf[destThread];
  if( send.offset+size > _SEND_BUF_MAX ) {
    stapl_assert( size <= _SEND_BUF_MAX, "argument size exceeded: increase _SEND_BUF_MAX or pass argument as a pointer" );
    addAsyncRequest( destThread, t, send );
  }
  void* const tmpPos = &send.buf[send.offset];
  if( t->threadID == destThread )
    mustSend = true;
  else {
    send.offset += size;
    if( ++send.aggCount >= t->aggMax )
      mustSend = true;
  }
  return tmpPos;
}


void send_async_rmi(const int destThread) {
  threadInfo* const t = get_threadInfo();
  perThreadSendBuffer& send = t->sendBuf[destThread];
  if( t->threadID == destThread ) {
    reinterpret_cast<rmiRequest*>( send.buf )->exec( t->registry );
    ++t->numRMI;
  }
  else
    addAsyncRequest( destThread, t, send );
}


void send_sync_rmi(const int destThread, rmiRequest* const r) {
  threadInfo* const t = get_threadInfo();
  if( t->threadID == destThread ) {
    periodic_poll( t );
    r->exec( t->registry );
    ++t->numRMI;
  }
  else {
    //Send Any Pending, Aggregated ASYNCS before SYNC
    if( t->sendBuf[destThread].offset != 0 )
      addAsyncRequest( destThread, t, t->sendBuf[destThread] );
    addSyncRequest( destThread, t, reinterpret_cast<char*>(r), reinterpret_cast<char*>(r) + r->_size );
  }
}


// Implements binary tree communication during the calculation of the
// reduction, with 0 getting the final result.  Since 0 could, theoretically,
// overwrite the result after this call, we force all threads to wait (e.g.,
// rmi_fence) until they have correctly copied the result before proceeding.
// If rootID = -1 all threads copy results, otherwise only the given root
// copies results (although this case doesn't perform any optimization).
class cacheline_pointer {
  void* volatile _ptr;
  char _padding[_CACHE_LINE_SIZE - sizeof(void* volatile)%_CACHE_LINE_SIZE];
public:
  void operator=(void* ptr) { _ptr = ptr; }
  operator void*() const volatile { return _ptr; }
};
cacheline_pointer* threadReduce;

void send_reduce_rmi(rmiRequestReduce* rmi, void* in, const int rootThread) {
  threadInfo* const t = get_threadInfo();
  for( int i=2; i<2*t->numThreads; i*=2 ) {
    if( t->threadID%i != 0 || t->threadID+i/2 >= t->numThreads ) {
      threadReduce[t->threadID] = in;
      break;
    }
    else {
      busy_wait( threadReduce[t->threadID+i/2] == 0, poll(t) );
      rmi->exec( t->registry, in, threadReduce[t->threadID+i/2] );
      in = threadReduce[t->threadID+i/2];
    }
  }
  if( t->threadID == 0 ) {
    threadReduce[0] = in;
    if( rootThread == -1 || rootThread == t->threadID )
      rmi->result( in );
    fence( t );
    threadReduce[t->threadID] = 0;
  }
  else {
    busy_wait( (in = threadReduce[0]) == 0, poll(t) );
    if( rootThread == -1 || rootThread == t->threadID )
      rmi->result( in );
    threadReduce[t->threadID] = 0;
    fence( t );
  }
}


void* volatile bcastTmp = 0;

void send_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int rootThread) {
  threadInfo* const t = get_threadInfo();
  if( t->threadID == rootThread )
    bcastTmp = in;
  fence( t );
  if( t->threadID != rootThread )
    rmi->exec( t->registry, bcastTmp );
  fence( t );
}


unsigned int set_aggregation(const unsigned int agg) {
  threadInfo* const t = get_threadInfo();
  t->aggMax = ( agg <= maxAggregation ) ? agg : maxAggregation;
  return t->aggMax;
}


unsigned int get_aggregation() {
  return get_threadInfo()->aggMax;
}


int set_nesting(const int nest) {
  return -1; // ignores the suggestion, nesting remains unlimited
}


int get_nesting() {
  return -1;
}


void flush(threadInfo* const t) {
  for( int i=0; i<t->numThreads; ++i ) {
    perThreadSendBuffer& send = t->sendBuf[i];
    if( send.offset != 0 )
      addAsyncRequest( i, t, send );
  }
}

void rmi_flush() {
  flush( get_threadInfo() );
}




//****************************************************************************
// Synchronization Primitives
//****************************************************************************

void set_poll_rate(const unsigned int rate) {
  get_threadInfo()->pollMax = rate;
}


unsigned int get_poll_rate() {
  return get_threadInfo()->pollMax;
}


inline void periodic_poll(threadInfo* const t) {
  if( ++t->pollCount >= t->pollMax )
    poll( t );
}

void poll(threadInfo* const t) {
  t->pollCount = 0;
  findRequest( t );
}

void rmi_poll() {
  poll( get_threadInfo() );
}


void rmi_wait() {
  threadInfo* const t = get_threadInfo();
  t->pollCount = 0;
  if( t->numWaits == t->numRMI )
    busy_wait( !findRequest(t), ; );
  ++t->numWaits;
}


void rmi_fence() {
  threadInfo* const t = get_threadInfo();
  int globalSR = 0;
  int numCycles = 0;
  do {
    poll( t ); flush( t );
    t->numWaits = t->numRMI = 0;
    globalSR = fence( t );

    // Iterate the fence until the global number of sends - receives = 0.
    // This ensures that all outstanding RMI requests have completed.  In the
    // simple case, iterating is not necessary.  However, if an RMI invokes a
    // method that starts another RMI, which invokes a method that starts
    // another RMI, etc, multiple iterations may be necessary.  To ensure the
    // program doesn't hang, abort after a certain number of iterations.
    stapl_assert( ++numCycles < 1000, "rmi_fence internal error: contact stapl development" );
  } while( globalSR != 0 );
  t->pollCount = 0;
}


} // end namespace


int main(int argc, char *argv[]) {
  using namespace thread_primitives;
  
  // Determine the number of threads based on an environment variable.
  // Default to 1 thread if the environment isn't set.
  char* c = getenv( "STAPL_NUM_THREADS" );
  int n = ( c == 0 ) ? 1 : atoi( c );
  stapl_assert( n >= 1, "invalid number of threads requested by STAPL_NUM_THREADS environment variable" );

  allThreadInfo = static_cast<threadInfo**> (safe_malloc(sizeof(threadInfo*) * n));
  
  // Initialize shared data structures
  thread_primitives::argc = argc;
  thread_primitives::argv = argv;
  declare_threadInfo();
  fenceNodes = static_cast<fenceNode*>( safe_malloc(sizeof(fenceNode)*n) );
  threadReduce = static_cast<cacheline_pointer*>( safe_malloc(sizeof(cacheline_pointer)*n) );
  for( int i=0; i<n; ++i ) {
    init_fenceNode( &fenceNodes[i], i, n );
    threadReduce[i] = 0;
  }
  recvBuf = static_cast<request**>( safe_malloc(sizeof(request*)*n) );
  stapl_assert( maxAggregation != 0, "maxAggregation equals 0: increase _SEND_BUF_MAX" );
  
  execute_threads( n );
  
#ifdef _ARMI_REGISTRY_VERIFY_TRACE
  checkRegistryTrace();
#endif //_ARMI_REGISTRY_VERIFY_TRACE
  
  // Cleanup shared data structures
  free( recvBuf );
  free( threadReduce );
  free( fenceNodes );
  destroy_threadInfo();
}
