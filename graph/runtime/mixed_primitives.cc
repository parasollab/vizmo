//****************************************************************************
// Implements the primitives using a multi-protocol combination of MPI-1.1
// and generic threads.  Use of real thread calls are limited to accessing
// thread specific data and forking the parallel work threads.
//
// This file is a combination of mpi_primitives.cc and thread_primitives.cc.
// In general, code was copied without change.  All changes necessary have
// been marked by a '+++' comment, usually with some explaination.  The
// exception is use of mpiLock to ensure MPI calls are used safely.  This 
// file allows each shared-memory thread to use MPI calls, which often 
// causes great contention on the MPI library.  As such, polls will use
// the MPI poll much less than the shared-memory poll.  This code has been
// flagged with a '@@@'.
//
// Currently, OpenMP is used if _OPENMP is defined, otherwise Pthreads are
// used.  Many compilers that support OpenMP require a special compile-line
// flag (e.g, SGI's -mp), which defines _OPENMP and recognizes the omp
// pragmas.  If your compiler does not do this, you may need to manually
// define _OPENMP (e.g., -D_OPENMP).
//
// The number of MPI processes to use is determined by the underlying MPI
// implementation.  Usually, this is set by the 'mpirun -np' command, although
// some implementations use command line flags.  The number of threads to use
// within each MPI process is determined by the STAPL_NUM_THREADS environment
// variable.  This implementation currently assumes the same number of threads
// per MPI process, although this constraint is easily relaxed if desired
// (e.g., read a machine specification from a file).
//
// Assumptions:
// - MPI errors are fatal (i.e., abort), as specified by the standard
// - a homogeneous runtime environment (i.e., all MPI processes are running
//   on similar machines and compiled using the same compiler, such that
//   datatype and member function pointer conversion is not necessary)
//****************************************************************************

#define _COMPILING_PRIMITIVES

#include "primitives.h"
#include <math.h>
#include <string.h>
#include <sched.h> // for sched_yield()
#include <mpi.h>


// The higher this value, the less contention on the MPI layer between
// threads.  Too high of course, and no MPI work ever gets done...
#define _MPI_ITER 250 //@@@

// If equal to 0, the default, then MPI is assumed to be non-thread-safe, and
// locks are used to serialize calls to the MPI library.  If equal to 1, no
// locks are used, which, assuming a good/efficient implementation, improves
// performance.  May be defined at compile time (e.g., -D_THREAD_SAFE_MPI=???).
#ifndef _THREAD_SAFE_MPI
#define _THREAD_SAFE_MPI 0
#endif


// Size, in bytes, of the RMI request output and input buffers (actually in
// chars, which are generally 1 byte each).  _SEND_BUF_MAX is for aggregating
// small async_rmi requests (larger requests are sent individually).
// _RECV_BUF_MAX is the absolute maximum size message that may be received,
// and hence also sent (larger would require checking each incoming message's
// size, and dynamically allocating memory if necessary, which is slow!).
// May be defined at compile time (e.g., -D_SEND_BUF_MAX=???).
#ifndef _SEND_BUF_MAX
#define _SEND_BUF_MAX  1024
#endif

#ifndef _RECV_BUF_MAX
#define _RECV_BUF_MAX  ( 1024 * _SEND_BUF_MAX )
#endif


// The communication pattern for rmi_fence.  Communiation is based on a
// generic spanning tree, where children signal arrival to their parents up to
// the root, then parents signal release down to the leaves.  The default is
// HYPERCUBE_TREE, which is typically fastest.  May be defined at compile time
// to one of the three values below (e.g., -D_RMI_FENCE=FLAT_TREE).
#define HYPERCUBE_TREE 1
#define BINARY_TREE    2
#define FLAT_TREE      3
#ifndef _RMI_FENCE
#define _RMI_FENCE HYPERCUBE_TREE
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
}


// Code specific to a given thread package, implementing the functions to
// access thread specific data, fork worker threads, and a simpleLock.
#ifdef _OPENMP
#include "thread_primitives_openmp.h"
#else
#include "thread_primitives_pthreads.h"
#endif


//+++ combined the mpi & threads' poll and flush
namespace mixed_primitives {
bool poll(thread_primitives::threadInfo* const t, const bool busyWait=false);
void flush(thread_primitives::threadInfo* const t);
void periodic_poll(thread_primitives::threadInfo* const t);

void *reduceTmp = 0;
  
#if _THREAD_SAFE_MPI == 0
  typedef thread_primitives::simpleLock lock;
#else
  struct emptyLock {
    inline void lock() {}
    inline void unlock() {}
  };
  typedef emptyLock lock;
#endif
  lock mpiLock; //+++ guard MPI calls in non-thread-safe implementations
}


namespace mpi_primitives {
// This implementation uses a message header, msgHeader, in addition to MPI's
// internal header.  Although this duplicates some data (e.g., source id), it
// allows for an easier transition to mixed-mode (e.g., the MPI status->SOURCE
// equals the sending rank, not the sending thread within that rank).
//
// Each thread only accepts messages whose MPI tag matches its thread ID
// (again, to facilitate mixed-mode).  The additional header information contains
// a tag specifying the real nature of the message.
enum requestType {
  ASYNC_ONLY,      // contains a sequence of async_rmi's
  ASYNC_SYNC,      // contains a sequence of async_rmi's with one sync_rmi at the end
  SYNC_RTN,        // contains the return value for a sync_rmi
  FENCE_ARRIVAL,   // child has arrived at the rmi_fence
  FENCE_RELEASE    // parent has released from the rmi_fence
};

struct msgHeader {
  requestType tag;         // the RMI related tag
  int         source;      // the invoking thread
  void**      syncRelease; // flag for completing sync_rmi's
};

struct fenceMsg {
  msgHeader header;
  int sr;
  inline fenceMsg(requestType tag, const int src, const int sr) : sr( sr ) {
    header.tag = tag;
    header.source = src;
  }
};

// basic id information
//+++ renamed id to mpiRank and numThreads to mpiProcs
int mpiRank;       // STAPL thread ID (same as MPI rank)
int mpiProcs;      // number of threads (same as number of MPI processes)
MPI_Comm RMI_COMM; // private communicator, allows user-codes to use RMI and MPI calls in the same program

//+++: MPI portion of rmi_fence, called only by thread 0 of each shared-memory group
int mpi_fence(thread_primitives::threadInfo* const t, const int localSR);

// sendBuf provides space for outgoing rmiRequests, one buffer per destination
// thread.  Each buffer is broken into sections, such that one section can be
// filled while others are being MPI_Isent. The first few bytes of a half are
// reserved for the msgHeader.
struct perThreadSendBuffer {
  char* pos;                // position to start writing next message in the current section
  unsigned int aggCount;    // the current count, per destination thread
  char* buf;                // pointer to the current section

private:
  struct section {
    MPI_Request request;    // tracks when MPI has finished sending a section
    double kludge;          // kludge, to ensure section is properly aligned
    char buf[ALIGNED_SIZEOF(msgHeader) + _SEND_BUF_MAX];
    section() : request( MPI_REQUEST_NULL ) {}
  };
  enum initSections { MAX=2 };
  unsigned int sectionID;   // id of the current section
  unsigned int numSections; // number of available sections
  section** sections;

  // If all sections are busy, allocate a new one at the end.  This allows for
  // unlimited sections, bounded only by the total available memory.
  void increaseSections() {
    ++numSections;
    sections = static_cast<section**>( realloc(sections, sizeof(section*)*numSections) );
    stapl_assert( sections != 0, "Memory Allocation Failed" );
    //sections[sectionID] = new section;
    sections[numSections-1] = new section;
  }

public:
  perThreadSendBuffer() : sectionID( 0 ), numSections( MAX ) {
    sections = static_cast<section**>( safe_malloc(sizeof(section*)*numSections) );
    for( int i=0; i<numSections; ++i )
      sections[i] = new section;
    pos = sections[sectionID]->buf + ALIGNED_SIZEOF( msgHeader );
  }
  ~perThreadSendBuffer() {
    for( int i=0; i<numSections; ++i )
      delete sections[i];
    free( sections );
  }

  char* begin() { return sections[sectionID]->buf; }
  char* end() { return pos; }
  bool empty() const { return pos == buf; }
  MPI_Request* request() { return &sections[sectionID]->request; }

  // place the header tag information on the current section
  void tag(requestType tag, const int id, void** syncRelease=0) { //+++ added id
    msgHeader& h = *reinterpret_cast<msgHeader*>( sections[sectionID]->buf );
    h.tag = tag;
    h.source = id;
    h.syncRelease = syncRelease;
  }

  // find a temporary buffer, and optionally, a persistent MPI_Request
  char* findBuf(MPI_Request** req=0) {
    unsigned int tmpSection = 0;
    for( ; tmpSection<numSections; ++tmpSection )
      if( tmpSection != sectionID ) {
	MPI_Status status; int flag = true;
	if( sections[tmpSection]->request != MPI_REQUEST_NULL ) {
	  mpiLock.lock(); MPI_Test( &sections[tmpSection]->request, &flag, &status ); mpiLock.unlock();
	}
	if( flag )
	  break;
      }
    if (tmpSection == numSections) {
      increaseSections();
    }

    if( req != 0 )
      *req = &sections[tmpSection]->request;
    return sections[tmpSection]->buf;
  }

  // look for the first available section and reset it for filling
  void reset() {
    for( sectionID=0; sectionID<numSections; ++sectionID ) {
      MPI_Status status; int flag = true;
      if( sections[sectionID]->request != MPI_REQUEST_NULL ) {
	mpiLock.lock(); MPI_Test( &sections[sectionID]->request, &flag, &status ); mpiLock.unlock();
      }
      if( flag )
	break;
    }
    if( sectionID == numSections )
      increaseSections();
    pos = sections[sectionID]->buf + ALIGNED_SIZEOF( msgHeader );
    aggCount = 0;
    buf = sections[sectionID]->buf + ALIGNED_SIZEOF( msgHeader );
  }
};
}




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
  struct sendData {
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

  public:
    char* begin() { return buf; }
    char* end() { return &buf[offset]; }
    volatile bool& done() { return sections[sectionID]->done; }

    sendData() : offset( 0 ), aggCount( 0 ), sectionID( 0 ), numSections( MAX ) {
      sections = static_cast<section**>( safe_malloc(sizeof(section*)*numSections) );
      for( int i=0; i<numSections; ++i )
	sections[i] = new section;
      buf = sections[sectionID]->buf;
    }
    ~sendData() {
      for( int i=0; i<numSections; ++i )
	delete sections[i];
      free( sections );
    }

    // look for the first available section and reset it for filling
    void reset() {
      offset = 0;
      aggCount = 0;
      for( sectionID=0; sectionID<numSections; ++sectionID )
	if( sections[sectionID]->done == true ) {
	  buf = sections[sectionID]->buf;
	  return;
	}

      // all sections are busy, allocate a new one at the end
      // this allows for unlimited sections, bounded by the total available memory
      sectionID = numSections; ++numSections;
      sections = static_cast<section**>( realloc(sections, sizeof(section*)*numSections) );
      stapl_assert( sections != 0, "Memory Allocation Failed" );
      sections[sectionID] = new section;
      buf = sections[sectionID]->buf;
    }
  };
  sendData* data;
  perThreadSendBuffer(const int nThreads) : data( new sendData[nThreads] ) {}
  ~perThreadSendBuffer() { delete[] data; }
  sendData& operator[](const unsigned int destThread) { return data[destThread]; }
};
const unsigned int maxAggregation = _SEND_BUF_MAX / sizeof( rmiRequest0<arg_traits<int>,void> );


struct threadInfo {
  unsigned int pollCount;      // number of calls since last poll
  unsigned int pollMax;        // max number of calls before polling
  perThreadSendBuffer sendBuf; // thread's aggregation buffer
  const int threadID;          // thread's STAPL id (different from Pthreads id)
  unsigned int aggMax;         // max aggregation
  const int numThreads;        // number of shared-memory threads in this MPI process
  bool privateSense;           // used by fence for sense reversal
  rmiRegistry registry;        // the RMI object registry
  int localSR;                 // #sends - #receives
  unsigned int numRMI;         // number of RMI requests received since the last rmi_fence
  unsigned int numWaits;       // number of rmi_waits since the last rmi_fence
  unsigned int numMPI; //@@@
  unsigned int maxMPI; //@@@

  //+++ moved per-process MPI globals to per-thread threadInfo
  mpi_primitives::perThreadSendBuffer* mpiSendBuf;
  char* largeBuffer;          // scratch space for malloc'ed large messages (> _SEND_BUF_MAX)
  int   largeBufferSize;
  int __nestedRecv;           // used to restart recvBuf correctly during nested calls

  // recvBuf provides space for incoming rmiRequests.  Instead of using halves,
  // as in mpiSendBuf, recvBuf is just one large buffer.  Usually, messages are
  // Irecv'ed using recvRequest, to help overlap communication and computation.
  // If recvRequest is MPI_REQUEST_NULL, then no Irecv is currently outstanding.
  // recvBuf acts as a stack, allowing multiple rmiRequests to be received
  // (e.g., an async_rmi invocation uses sync_rmi during execution).  recvPos
  // maintains the current position on the stack.
  char* recvPos;
  MPI_Request recvRequest;
  char* recvBuf; //+++ dynamically allocated to keep stack small
  //+++

  inline threadInfo(const int id, const int nThreads) :
    pollCount( 0 ),
    pollMax( 1 ),
    sendBuf( nThreads ),
    threadID( mpi_primitives::mpiRank * nThreads + id ), //+++ global id, not shared-memory only
    aggMax( 1 ),
    numThreads( nThreads ),
    privateSense( false ),
    registry(),
    localSR( 0 ),
    numRMI( 0 ),
    numMPI( 0 ), //@@@
    maxMPI( _MPI_ITER ), //@@@
    numWaits( 0 ) {
    //+++ moved initialization code from main() to be per-thread
    mpiSendBuf = new mpi_primitives::perThreadSendBuffer[mpi_primitives::mpiProcs * nThreads]; //+++ mpiProcs
    largeBuffer = 0;
    largeBufferSize = 0;
    __nestedRecv = 0;
    recvBuf = static_cast<char*>( safe_malloc(sizeof(char) * _RECV_BUF_MAX) );
    recvPos = recvBuf;
    recvRequest = MPI_REQUEST_NULL;
  }

  //+++ moved destruction code from main() to be per-thread
  ~threadInfo() {
    delete[] mpiSendBuf;
    free( recvBuf );
  }
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


int fence(threadInfo* const t, bool doMPI=true) {
  register fenceNode* n = &fenceNodes[threadMap[t->threadID].thread]; //+++ map

  // Wait for all children to arrive
  busy_wait( n->childNotReady.volatileWhole != 0, { poll(t, true); flush(t); } );
  n->childNotReady.whole = n->haveChild.whole;

  // Calculate the parent and its children's portion of globalSR
  n->localSR = t->localSR + *n->childLocalSR[0] + *n->childLocalSR[1]
                          + *n->childLocalSR[2] + *n->childLocalSR[3];

  // Notify parent of arrival
  *n->parentPtr = false;

  // Wait for release, and propagate to children
  if( threadMap[t->threadID].thread == 0 ) { //+++ map
    if( doMPI ) fenceNode::globalSR = mpi_primitives::mpi_fence( t, n->localSR ); //+++
    else fenceNode::globalSR = n->localSR;
    fenceNode::globalSense = t->privateSense;
  }
  else
    busy_wait( fenceNode::globalSense != t->privateSense, { poll(t, true); flush(t); } );

  // Reverse the privateSense and return the globalSR
  t->privateSense = !t->privateSense;
  return fenceNode::globalSR;
}

void fence() {
  fence( get_threadInfo(), false );
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
			    perThreadSendBuffer::sendData& send) {
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
  return recvBuf[threadMap[t->threadID].thread]->dequeue( t ); //+++: map
}




//****************************************************************************
// Data structures shared between all threads, as well as the function each
// thread actually executes, for calling stapl_main.
//****************************************************************************

} namespace mixed_primitives {
//+++ added this function to make MPI_Wait with locking thread-friendly
MPI_Status mixedMPI_Wait(thread_primitives::threadInfo* const t, MPI_Request* req) {
  MPI_Status status;
  int flag = 0;
  while( !flag ) {
    mpiLock.lock(); MPI_Test( req, &flag, &status ); mpiLock.unlock();
    if( !flag )
      thread_primitives::findRequest( t );
  }
  return status;
}
} namespace thread_primitives {


// The user-specified command line arguments
int                argc;
char**             argv;


void thread_main(const int threadID, const int numThreads) {
  //+++ moved the mpi startup code from main() to per-thread
  using namespace mpi_primitives;
  threadInfo t( threadID, numThreads ); set_threadInfo( &t );
  recvBuf[threadID] = new request( numThreads );
  mpiLock.lock(); MPI_Irecv( t.recvPos, _RECV_BUF_MAX, MPI_BYTE, MPI_ANY_SOURCE,
			     t.threadID, RMI_COMM, &t.recvRequest ); mpiLock.unlock();
  stapl::rmi_fence();
  stapl_main( argc, argv );
  stapl::rmi_fence();
#ifndef _AIX  // IBM will hang if you cancel the last Irecv...
  mpiLock.lock(); MPI_Cancel( &t.recvRequest ); mpiLock.unlock();
  mixedMPI_Wait( &t, &t.recvRequest ); //+++
#endif
  delete recvBuf[threadID];
}


// Find space for async_rmi requests directly in sendBuf.  mustSend is set to
// true if a subsequent call to send_async_rmi is necessary (although this is
// ugly, it improves performance by up to 25%, by avoiding the second call to
// get_threadInfo, which is expensive if implemented with pthread_getspecific).
void* rmiRequest::operator new(size_t size, const int _destThread, bool& mustSend) {
  const int destThread = threadMap[_destThread].thread; //+++: map
  threadInfo* const t = get_threadInfo();
  periodic_poll( t );
  perThreadSendBuffer::sendData& send = t->sendBuf[destThread];
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


void send_async_rmi(const int _destThread) {
  const int destThread = threadMap[_destThread].thread; //+++: map
  threadInfo* const t = get_threadInfo();
  perThreadSendBuffer::sendData& send = t->sendBuf[destThread];
  if( t->threadID == destThread ) {
    reinterpret_cast<rmiRequest*>( send.buf )->exec( t->registry );
    ++t->numRMI;
  }
  else
    addAsyncRequest( destThread, t, send );
}


void send_sync_rmi(const int _destThread, rmiRequest* const r) {
  const int destThread = threadMap[_destThread].thread; //+++: map
  threadInfo* const t = get_threadInfo();
  if( t->threadID == destThread ) {
    periodic_poll( t );
    r->exec( t->registry );
    ++t->numRMI;
  }
  else {
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
  
  void send_reduce_rmi(rmiRequestReduce* rmi, void* in, const int rootID) {
    threadInfo* const t = get_threadInfo();
    
    for( int i=2; i<2*t->numThreads; i*=2 ) {
      if( threadMap[t->threadID].thread%i != 0 || threadMap[t->threadID].thread+i/2 >= t->numThreads ) {
	threadReduce[threadMap[t->threadID].thread] = in;
	break;
      } else {
	while( threadReduce[threadMap[t->threadID].thread+i/2] == 0 ); // busy-wait
	rmi->exec( t->registry, in, threadReduce[threadMap[t->threadID].thread+i/2] );
	in = threadReduce[threadMap[t->threadID].thread+i/2];
      }
    }
    if( threadMap[t->threadID].thread == 0 ) {
      threadReduce[0] = in;
      rmi->result( in );
      fence( t );
      threadReduce[threadMap[t->threadID].thread] = 0;
    } else {
      while( (in = threadReduce[0]) == 0 ); // busy-wait
      threadReduce[threadMap[t->threadID].thread] = 0;
      fence( t );
    }
  }

} // end namespace thread_primitives




namespace mpi_primitives {

// variables used during spanning tree based communication (e.g., rmi_fence)
bool parentRelease;    // true if the parent has signaled fence release
//+++int localSR;      // the local number of sends - receives
int globalSR;          // the global number of sends - receives, set during rmi_fence()
int numArrived;        // number of child threads arrived at rmi_fence()
int parentID;          // this thread's parent id in the tree
int numChildren;       // this thread's number of children in the tree
#define MAX_KIDS 7
int childID[MAX_KIDS]; // this thread's children id's in the tree
rmiRegistry registry;  // the RMI object registry


// Calculate the exact amount of space left in recvBuf for receiving the next
// message.  This will allow MPI to issue an error and abort if something is
// received that exceeds that bound.
inline int space(thread_primitives::threadInfo* const t) { return t->recvBuf + _RECV_BUF_MAX - t->recvPos; } //+++: added threadInfo


// recvBufProtector enforces the stack-style usage of recvBuf, by keeping RMI
// processing within its bounds, and setting recvBuf for additional messages
// should the processed requests invoke additional RMI requests.
class recvBufProtector {
  thread_primitives::threadInfo* const t; //++
  char* const tmpPos;
  int size;
public:
  inline explicit recvBufProtector(MPI_Status* status) : 
    t( thread_primitives::get_threadInfo() ), tmpPos( t->recvPos ) {
    mpiLock.lock(); MPI_Get_count( status, MPI_BYTE, &size ); mpiLock.unlock();
    t->recvPos += size;
  }
  inline ~recvBufProtector() { t->recvPos = tmpPos; }
  inline char* begin() { return tmpPos + ALIGNED_SIZEOF(msgHeader); }
  inline char* end() { return tmpPos + size; }
  inline int srcThread() { return reinterpret_cast<msgHeader*>(tmpPos)->source; }
  inline void** syncRelease() { return reinterpret_cast<msgHeader*>(tmpPos)->syncRelease; }
};


// variables and functions to implement reduce_rmi
rmiRequestReduce* rf;
MPI_Op commute;
MPI_Op no_commute;

void reduce_internal(void* in, void* inout, int*, MPI_Datatype*) {
  rf->exec( thread_primitives::get_threadInfo()->registry, in, inout );
}


// wait for an MPI_Request object while continuing to poll
inline MPI_Status request_wait(MPI_Request* req, thread_primitives::threadInfo* const t) {
  unsigned int count = 0;
  MPI_Status status;
  while( true ) {
    int flag = 0;
    if( ++count == _MPI_ITER ) { //@@@
      mpiLock.lock(); MPI_Test( req, &flag, &status ); mpiLock.unlock();
      count = 0;
    }
    if( flag )
      break;
    poll( t, true );
  }
  return status;
}


// start sending the given sendBuf half, and prepare the other half for use
inline void request_send(const int destThread, requestType tag=ASYNC_ONLY, void** syncRelease=0) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  if( t->largeBuffer == 0 ) {
    perThreadSendBuffer& send = t->mpiSendBuf[destThread];
    send.tag( tag, t->threadID, syncRelease );
    mpiLock.lock(); MPI_Isend( send.begin(), send.end()-send.begin(), MPI_BYTE, 
			       threadMap[destThread].mpiRank, destThread, RMI_COMM, send.request() ); mpiLock.unlock(); //+++: threadMap
    send.reset();
  }
  else {
    reinterpret_cast<msgHeader*>( t->largeBuffer )->tag = tag;
    reinterpret_cast<msgHeader*>( t->largeBuffer )->source = t->threadID; //+++ id
    MPI_Request req = MPI_REQUEST_NULL;
    mpiLock.lock(); MPI_Isend( t->largeBuffer, t->largeBufferSize, MPI_BYTE, threadMap[destThread].mpiRank, destThread, RMI_COMM, &req ); mpiLock.unlock(); //+++: threadMap
    request_wait( &req, t );
    free( t->largeBuffer );
    t->largeBuffer = 0;
  }
  ++t->localSR;
}


// process an aggregation group of async_rmi requests
inline void recv_async_rmi(MPI_Status* status) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  ++t->numRMI;
  recvBufProtector p( status );
  for( char* c=p.begin(); c<p.end(); c+=(reinterpret_cast<rmiRequest*>(c))->_size )
    ( reinterpret_cast<rmiRequest*>(c) )->exec( t->registry, 0, 0 );
  --t->localSR;
}


// process an aggregation group of async_rmi requests, 
// followed by a single sync_rmi request
inline void recv_sync_rmi(MPI_Status* status) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  ++t->numRMI;
  recvBufProtector p( status );
  const int srcThread = p.srcThread();
  MPI_Request* req;
  char* tmpHeader = t->mpiSendBuf[srcThread].findBuf( &req );
  char* tmpBuf = tmpHeader+ALIGNED_SIZEOF( msgHeader );
  int rtnSize = 0;

  // only the sync_rmi request uses the tmpBuf & rtnSize arguments
  for( char* c=p.begin(); c<p.end(); c+=(reinterpret_cast<rmiRequest*>(c))->_size )
    ( reinterpret_cast<rmiRequest*>(c) )->exec( t->registry, tmpBuf, &rtnSize );
  stapl_assert( rtnSize <= _SEND_BUF_MAX, "sync_rmi return exceeded buffers: increase _SEND_BUF_MAX" );

  // tag and send the return value
  msgHeader& h = *reinterpret_cast<msgHeader*>( tmpHeader );
  h.tag = SYNC_RTN;
  h.source = t->threadID;
  h.syncRelease = p.syncRelease();
  mpiLock.lock(); MPI_Isend( tmpHeader, rtnSize+ALIGNED_SIZEOF(msgHeader), MPI_BYTE,
			     threadMap[srcThread].mpiRank, srcThread, RMI_COMM, req ); mpiLock.unlock(); //+++: threadMap
  // --localSR; // one message was received...
  // ++localSR; // ... but required sending another one back
}


// process a sync_rmi return value, by setting the appropriate release flag,
// and incrementing the recvPos stack to guard the value until it is copied
// out (e.g., a sync_rmi is waiting for its return, receives another RMI that
// happens to call its own sync_rmi)
inline void recv_sync_rtn(MPI_Status* status) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  *reinterpret_cast<msgHeader*>(t->recvPos)->syncRelease = t->recvPos;
  int size;
  mpiLock.lock(); MPI_Get_count( status, MPI_BYTE, &size ); mpiLock.unlock();
  t->recvPos += size;
  *reinterpret_cast<int*>( t->recvPos ) = size; // save size so it can be subtracted from
  t->recvPos += sizeof( int );                  // recvPos upon copying the return value out
  --t->localSR;
}


// process a fence arrival request from one of this parent's children
inline void recv_fence_arrival() {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  globalSR += reinterpret_cast<fenceMsg*>( t->recvPos )->sr;
  ++numArrived;
}


// process a fence release request from this child's parent
inline void recv_fence_release() {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  globalSR = reinterpret_cast<fenceMsg*>( t->recvPos )->sr;
  parentRelease = true;
}


// return true if request was an RMI, and false if it was a control message
bool recv_request(thread_primitives::threadInfo* const t, MPI_Status* status) {
  bool rtn = false;
  switch( reinterpret_cast<msgHeader*>(t->recvPos)->tag ) {
    case ASYNC_ONLY:    recv_async_rmi( status ); rtn = true; break;
    case ASYNC_SYNC:    recv_sync_rmi( status ); rtn = true; break;
    case SYNC_RTN:      recv_sync_rtn( status ); break;
    case FENCE_ARRIVAL: recv_fence_arrival(); break;
    case FENCE_RELEASE: recv_fence_release(); break;
    default:            stapl_assert( 0, "invalid MPI tag" );
  }
  return rtn;
}


// find space for RMI request creation directly in sendBuf
void* rmiRequest::operator new(size_t size, const int destThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  stapl_assert( size <= _RECV_BUF_MAX, "request too large for transfer: increase _RECV_BUF_MAX." );
  perThreadSendBuffer& send = t->mpiSendBuf[destThread];
  if( size > _SEND_BUF_MAX ) {
    if( !send.empty() )
      request_send( destThread );
    t->largeBufferSize = size + ALIGNED_SIZEOF( msgHeader );
    t->largeBuffer = static_cast<char*>( safe_malloc(t->largeBufferSize) );
    return t->largeBuffer + ALIGNED_SIZEOF(msgHeader);
  }
  if( send.pos+size > send.buf+_SEND_BUF_MAX )
    request_send( destThread );
  void* tmpPos = send.pos;
  if( t->threadID != destThread )
    send.pos += size;
  return tmpPos;
}


void send_async_rmi(const int destThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  perThreadSendBuffer& send = t->mpiSendBuf[destThread];
  if( t->threadID == destThread ) {
    if( t->largeBuffer == 0 )
      ( reinterpret_cast<rmiRequest*>(send.buf) )->exec( t->registry, 0, 0 );
    else {
      ( reinterpret_cast<rmiRequest*>(t->largeBuffer+ALIGNED_SIZEOF(msgHeader)) )->exec( t->registry, 0, 0 );
      free( t->largeBuffer );
      t->largeBuffer = 0;
    }
  }
  else
    if( ++send.aggCount >= t->aggMax )
      request_send( destThread );
  periodic_poll( t );
}


void* send_sync_rmi(const int destThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  perThreadSendBuffer& send = t->mpiSendBuf[destThread];
  if( t->threadID == destThread ) {
    periodic_poll( t );
    ++t->__nestedRecv;
    int rtnSize = 0;
    void* tmpBuf = send.findBuf();
    if( t->largeBuffer == 0 )
      ( reinterpret_cast<rmiRequest*>(send.pos) )->exec( t->registry, tmpBuf, &rtnSize );
    else {
      ( reinterpret_cast<rmiRequest*>(t->largeBuffer+ALIGNED_SIZEOF(msgHeader)) )->exec( t->registry, tmpBuf, &rtnSize );
      free( t->largeBuffer );
      t->largeBuffer = 0;
    }
    stapl_assert( rtnSize <= _SEND_BUF_MAX, "sync_rmi return exceeded buffers: increase _SEND_BUF_MAX" );
    return tmpBuf;
  }

  // Wait for the return, while handling other incoming requests
  void* syncRelease = 0;
  request_send( destThread, ASYNC_SYNC, &syncRelease );
  if( t->recvRequest == MPI_REQUEST_NULL ) { // i.e., is this a sync_rmi within an RMI invocation?
    stapl_assert( t->recvPos+ALIGNED_SIZEOF(msgHeader)+_SEND_BUF_MAX <= t->recvBuf+_RECV_BUF_MAX, "recvBuf overflow caused by excessive nested RMI requests: increase _RECV_BUF_MAX" );
    mpiLock.lock(); MPI_Irecv( t->recvPos, space(t), MPI_BYTE, MPI_ANY_SOURCE, t->threadID, RMI_COMM, &t->recvRequest ); mpiLock.unlock();
    ++t->__nestedRecv;
  }
  do {
    MPI_Status status = mixedMPI_Wait( t, &t->recvRequest );
    recv_request( t, &status );
    if( syncRelease == 0 ) {
      mpiLock.lock(); MPI_Irecv( t->recvPos, space(t), MPI_BYTE, MPI_ANY_SOURCE, t->threadID, RMI_COMM, &t->recvRequest ); mpiLock.unlock();
    }
  } while( syncRelease == 0 );

  // Return the stack to its original value, which was incremented in recv_sync_rtn
  const int rtnSize = *reinterpret_cast<int*>( t->recvPos - sizeof(int) );
  t->recvPos -= rtnSize + sizeof( int );
  return reinterpret_cast<char*>( syncRelease ) + ALIGNED_SIZEOF( msgHeader );
}

// Trying to put this step into send_sync_rmi causes a race condition between
// MPI_Irecv-ing the next message and copying the return value from the
// buffer.  Alternating between two recvBufs (similar to sendBuf) would
// eliminate this step, but also increase the overall latency...
void finish_sync_rmi() {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  if( !t->__nestedRecv ) {
    mpiLock.lock(); MPI_Irecv( t->recvPos, space(t), MPI_BYTE, MPI_ANY_SOURCE, t->threadID, RMI_COMM, &t->recvRequest ); mpiLock.unlock();
  }
  else
    --t->__nestedRecv;
}


// Get an MPI buffer for temporary use for arguments of collective functions
void* get_arg_buffer(const int size) {
  stapl_assert( size <= _SEND_BUF_MAX, "input too large, increase _SEND_BUF_MAX" );
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  return t->mpiSendBuf[t->threadID].findBuf();
}


void send_reduce_rmi(rmiRequestReduce* const f, const bool commutative, const int rootThread) {
   rf = f;
   mpiLock.lock();
   if( rootThread == -1 )
     MPI_Allreduce( f->_in, f->_inout, f->_size, MPI_BYTE,
		    (commutative ? commute : no_commute), RMI_COMM );
   else
     MPI_Reduce( f->_in, f->_inout, f->_size, MPI_BYTE,
		 (commutative ? commute : no_commute), rootThread, RMI_COMM );
   mpiLock.unlock();
}


//+++ complete re-write compared to original MPI and thread implementations
//+++ moved out of thread_primitives to share the rmiRequestBroadcast class
void* volatile bcastTmp = 0;
void send_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int rootThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
  if( threadMap[t->threadID].thread == rootThread ) //+++ threadMap
    bcastTmp = in;
  thread_primitives::fence( t, false );
  if( threadMap[t->threadID].thread != rootThread ) //+++ threadMap
    rmi->exec( t->registry, bcastTmp );
  thread_primitives::fence( t, false );
}

void send_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int size, const int rootThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  stapl_assert( size <= _SEND_BUF_MAX, "broadcast_rmi input too large, increase _SEND_BUF_MAX" );
  if( threadMap[rootThread].mpiRank == mpiRank ) {
    send_broadcast_rmi( rmi, in, threadMap[rootThread].thread );
    if( threadMap[t->threadID].thread == 0 ) {
      mpiLock.lock(); MPI_Bcast( in, size, MPI_BYTE, threadMap[rootThread].mpiRank, RMI_COMM ); mpiLock.unlock();
    }
  }
  else {
    if( threadMap[t->threadID].thread == 0 ) {
      char* tmpBuf = t->mpiSendBuf[t->threadID].findBuf();
      mpiLock.lock(); MPI_Bcast( tmpBuf, size, MPI_BYTE, threadMap[rootThread].mpiRank, RMI_COMM ); mpiLock.unlock();
      rmi->exec( t->registry, tmpBuf );
      send_broadcast_rmi( rmi, tmpBuf, 0 );
    }
    else
      send_broadcast_rmi( rmi, in, 0 );
  }
}


// This is a generic spanning tree based fence.  Children signal arrival to
// their parents up to the root, then parents signal release down to the
// leaves.  The spanning tree layout can be easily re-defined by setting
// parentID, numChildren and childrenID as desired in 'int main(...)' above.
// The root of the tree has parentID = mpiRank.
int mpi_fence(thread_primitives::threadInfo* const t, const int localSR) {
  // send leftover rmi requests, then wait for children to arrive
  mixed_primitives::poll( t ); mixed_primitives::flush( t );
  while( numArrived != numChildren ) {
    const unsigned int tmp = t->maxMPI; t->maxMPI = 1; //@@@
    mixed_primitives::poll( t, true ); mixed_primitives::flush( t );
    t->maxMPI = tmp;
  }
  numArrived = 0;

  // signal arrival, then wait for the signal to release
  globalSR += localSR;
  if( parentID != mpiRank ) {
    fenceMsg msg( FENCE_ARRIVAL, t->threadID, globalSR );
    mpiLock.lock(); MPI_Send( &msg, ALIGNED_SIZEOF(fenceMsg), MPI_BYTE, parentID, parentID*t->numThreads, RMI_COMM ); mpiLock.unlock();
    parentRelease = false;
    while( parentRelease == false ) {
      const unsigned int tmp = t->maxMPI; t->maxMPI = 1; //@@@
      mixed_primitives::poll( t, true ); mixed_primitives::flush( t );
      t->maxMPI = tmp;
    }
  }

  // signal release to children
  for( int i=0; i<numChildren; ++i ) {
    fenceMsg msg( FENCE_RELEASE, t->threadID, globalSR );
    mpiLock.lock(); MPI_Send( &msg, ALIGNED_SIZEOF(fenceMsg), MPI_BYTE, childID[i], childID[i]*t->numThreads, RMI_COMM ); mpiLock.unlock();
  }
  const int tmp = globalSR;
  globalSR = 0;
  return tmp;
}

} // end namespace




namespace mixed_primitives {
mpi_thread* threadMap;

bool isLocal(const int threadID) {
  return mpi_primitives::mpiRank == threadMap[threadID].mpiRank;
}

bool poll(thread_primitives::threadInfo* const t, const bool busyWait) {
  using namespace mpi_primitives;
  bool rtn = false;
  t->pollCount = 0;
  rtn = thread_primitives::findRequest( t );

  if( busyWait ) //@@@
    if( ++t->numMPI <= t->maxMPI ) return rtn;
    else t->numMPI = 0;

  int flag; MPI_Status status;
  if( t->recvRequest == MPI_REQUEST_NULL ) { // i.e., is this an rmi_poll within an RMI invocation
    stapl_assert( t->recvPos+_SEND_BUF_MAX <= t->recvBuf+_RECV_BUF_MAX, "recvBuf overflow caused by excessive nested RMI requests: increase _RECV_BUF_MAX" );
    mpiLock.lock(); MPI_Iprobe( MPI_ANY_SOURCE, t->threadID, RMI_COMM, &flag, &status ); mpiLock.unlock();
    while( flag ) {
      if( t->maxMPI >= 2 ) t->maxMPI -= 2; //@@@
      rtn = true;
      mpiLock.lock(); MPI_Recv( t->recvPos, space(t), MPI_BYTE, status.MPI_SOURCE, t->threadID, RMI_COMM, &status ); mpiLock.unlock();
      recv_request( t, &status );
      mpiLock.lock(); MPI_Iprobe( MPI_ANY_SOURCE, t->threadID, RMI_COMM, &flag, &status ); mpiLock.unlock();
    }
    ++t->maxMPI; //@@@
  }
  else {
    mpiLock.lock(); MPI_Test( &t->recvRequest, &flag, &status ); mpiLock.unlock();
    while( flag ) {
      if( t->maxMPI >= 2 ) t->maxMPI -= 2; //@@@
      rtn = true;
      recv_request( t, &status );
      mpiLock.lock();
      MPI_Irecv( t->recvPos, space(t), MPI_BYTE, MPI_ANY_SOURCE, t->threadID, RMI_COMM, &t->recvRequest );
      MPI_Test( &t->recvRequest, &flag, &status );
      mpiLock.unlock();
    }
    ++t->maxMPI; //@@@
  }
  return rtn;
}

void flush(thread_primitives::threadInfo* const t) {
  for( int i=0; i<t->numThreads; ++i ) {
    thread_primitives::perThreadSendBuffer::sendData& send = t->sendBuf[i];
    if( send.offset != 0 )
      thread_primitives::addAsyncRequest( i, t, send );
  }
  for( int i=0; i<mpi_primitives::mpiProcs*t->numThreads; ++i )
    if( !t->mpiSendBuf[i].empty() )
      mpi_primitives::request_send( i );
}

inline void periodic_poll(thread_primitives::threadInfo* const t) {
  if( ++t->pollCount >= t->pollMax )
    poll( t );
}
} // namespace




namespace stapl {
//****************************************************************************
// Parallelization Primitives
//****************************************************************************

int get_num_threads() {
  return thread_primitives::get_threadInfo()->numThreads * mpi_primitives::mpiProcs;
}


int get_thread_id() {
  return thread_primitives::get_threadInfo()->threadID;
}




//****************************************************************************
// Registration Primitives
//****************************************************************************
#ifndef _ARMI_REGISTRY_CHECK_TYPE 

  rmiHandle register_rmi_object(void* const objectPointer) {
    return thread_primitives::get_threadInfo()->registry.registerObject( objectPointer );
    
  }
  
  void update_rmi_object(rmiHandle handle, void* const objectPointer) {
    thread_primitives::get_threadInfo()->registry.updateObject( handle, objectPointer );
  }
  
#else

  rmiHandle register_rmi_object(void* const objectPointer, 
				const std::type_info &objectType) {
    return thread_primitives::get_threadInfo()->
      registry.registerObject( objectPointer, objectType);
  }
  
  rmiHandle register_rmi_object(void *const objectPonter, int id) {
    return thread_primitives::get_threadInfo()->
      registry.registerObject( objectPointer, id );
  }

  void update_rmi_object(rmiHandle handle, void* const objectPointer,
			 const std::type_info &objectType) {
    thread_primitives::get_threadInfo()->
      registry.updateObject( handle, objectPointer, objectType);
  }

#endif  //_ARMI_REGISTRY_CHECK_TYPE 

  void unregister_rmi_object(rmiHandle const &handle) {
    thread_primitives::get_threadInfo()->registry.unregisterObject( handle );
  }
  
  void execute_parallel_task(parallel_task* const task) {
    stapl_assert( task != 0, "parallel_task* equals 0" );
    thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
    const unsigned int tmpAgg = t->aggMax; t->aggMax = thread_primitives::maxAggregation;
    const unsigned int tmpPoll = t->pollMax; t->pollMax = thread_primitives::maxAggregation;
    t->registry.fastRegisterTask( task );
    thread_primitives::fence( t ); // ensure all threads have registered
    task->execute();
    t->aggMax = tmpAgg;
    t->pollMax = tmpPoll;
  }




//****************************************************************************
// Communication Primitives
//****************************************************************************

unsigned int set_aggregation(const unsigned int agg) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
  t->aggMax = ( agg <= thread_primitives::maxAggregation ) ? agg : thread_primitives::maxAggregation;
  return t->aggMax;
}


unsigned int get_aggregation() {
  return thread_primitives::get_threadInfo()->aggMax;
}


int set_nesting(const int nest) {
  return -1; // ignores the suggestion, nesting remains unlimited
}


int get_nesting() {
  return -1;
}


void rmi_flush() {
  mixed_primitives::flush( thread_primitives::get_threadInfo() );
}




//****************************************************************************
// Synchronization Primitives
//****************************************************************************

void set_poll_rate(const unsigned int rate) {
  thread_primitives::get_threadInfo()->pollMax = rate;
}


unsigned int get_poll_rate() {
  return thread_primitives::get_threadInfo()->pollMax;
}


void rmi_poll() {
  mixed_primitives::poll( thread_primitives::get_threadInfo() );
}


void rmi_wait() {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
  if( t->numWaits == t->numRMI ) {
    t->pollCount = 0;
    busy_wait( 0 == mixed_primitives::poll(t, true), ; );
  }
  ++t->numWaits;
}


void rmi_fence() {
  using namespace thread_primitives;
  threadInfo* const t = get_threadInfo();
  int numCycles = 0;
  do {
    mixed_primitives::poll( t ); mixed_primitives::flush( t );
    t->numWaits = t->numRMI = 0;
    int globalSR = fence( t );

    // Iterate the fence until the global number of sends - receives = 0.
    // This ensures that all outstanding RMI requests have completed.  In the
    // simple case, iterating is not necessary.  However, if an RMI invokes a
    // method that starts another RMI, which invokes a method that starts
    // another RMI, etc, multiple iterations may be necessary.  To ensure the
    // program doesn't hang, abort after a certain number of iterations.
    if( globalSR == 0 ) break;
    stapl_assert( ++numCycles < 1000, "rmi_fence internal error: contact stapl development" );
  } while( 1 );
  t->pollCount = 0;
}


} // end namespace


int main(int argc, char *argv[]) {
  using namespace thread_primitives;
  using namespace mpi_primitives;

#if _THREAD_SAFE_MPI == 0
  MPI_Init( &argc, &argv );
#else
  int provided;
  MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &provided );
  stapl_assert( provided == MPI_THREAD_MULTIPLE, "MPI implementation not thread-safe, add -D_THREAD_SAFE_MPI=0 to compiler" );
#endif
  MPI_Comm_rank( MPI_COMM_WORLD, &mpiRank );
  MPI_Comm_size( MPI_COMM_WORLD, &mpiProcs );
  MPI_Comm_dup( MPI_COMM_WORLD, &RMI_COMM );

  // initialize variables for tree based communication
  parentRelease = false;
  //+++localSR = 0;
  globalSR = 0;
  numArrived = 0;
#if _RMI_FENCE == HYPERCUBE_TREE
  const double _mpiProcs = mpiProcs;
  stapl_assert( MAX_KIDS > log10(_mpiProcs)/log10(2.0), "MAX_KIDS exceeded: please increase" );
  parentID = 0;
  int tmpParent = 0;
  int stage = 1;
  int N = static_cast<int>( pow(2.0, ceil(log10(_mpiProcs) / log10(2.0))) );
  while( tmpParent != mpiRank ) {
    if( mpiRank >= tmpParent + N / pow(2.0, stage) ) {
      parentID = tmpParent;
      tmpParent += static_cast<int>( N / pow(2.0, stage) );
    }
    ++stage;
  }
  numChildren = 0;
  while( N/pow(2.0, stage) >= 1.0 ) {
    childID[numChildren] = mpiRank + static_cast<int>( N / pow(2.0, stage) );
    ++stage;
    if( childID[numChildren] < _mpiProcs ) ++numChildren;
  }
#elif _RMI_FENCE == BINARY_TREE
  stapl_assert( MAX_KIDS >= 2, "MAX_KIDS exceeded: please increase" );
  parentID = (mpiRank-1) / 2;
  numChildren = 2;
  childID[0] = ( 2 * (mpiRank+1) ) - 1;
  childID[1] = ( 2 * (mpiRank+1) );
  if( childID[0] >= mpiProcs ) --numChildren;
  if( childID[1] >= mpiProcs ) --numChildren;
#elif _RMI_FENCE == FLAT_TREE
  stapl_assert( MAX_KIDS >= mpiProcs-1, "MAX_KIDS exceeded: please increase" );
  parentID = 0;
  if( mpiRank == 0 ) {
    numChildren = mpiProcs-1;
    for( int i=1; i<mpiProcs; ++i )
      childID[i-1] = i;
  }
  else
    numChildren = 0;
#endif

  // register the reduce_rmi functions
  MPI_Op_create( reduce_internal, true,  &commute );
  MPI_Op_create( reduce_internal, false, &no_commute );

  // Determine the number of threads based on an environment variable.
  // Default to 1 thread if the environment isn't set.
  char* c = getenv( "STAPL_NUM_THREADS" );
  int n = ( c == 0 ) ? 1 : atoi( c );
  stapl_assert( n >= 1, "invalid number of threads requested by STAPL_NUM_THREADS environment variable" );

  //+++: Initialize the MPI-thread communication threadMap.
  threadMap = static_cast<mpi_thread*>( safe_malloc(sizeof(mpi_thread) * n * mpiProcs) );
  int index = 0;
  for( int i=0; i<mpiProcs; ++i ) {
    for( int j=0; j<n; ++j ) {
      mixed_primitives::threadMap[index].mpiRank = i;
      mixed_primitives::threadMap[index].thread = j;
      ++index;
    }
  }

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

  // Cleanup shared data structures
  free( threadMap ); //+++
  free( recvBuf );
  free( threadReduce );
  free( fenceNodes );
  destroy_threadInfo();

  // cleanup
  MPI_Op_free( &commute );
  MPI_Op_free( &no_commute );
  MPI_Comm_free( &RMI_COMM );
  MPI_Finalize();
}
