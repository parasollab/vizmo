/************************************************************************
Copyright (c) 2003, Lawrence Rauchwerger
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the Texas A&M University nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************/

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
//
#define ERROR_HANDLER(func) { int v = func; stapl_assert( v==0, strerror(v) ); }

#define _COMPILING_PRIMITIVES
#include "primitives.h"
#include <math.h>
#include <string.h>
#include <sched.h> // for sched_yield()
#include <mpi.h>
#include <list>
#include <vector>
#include <utility>
#include <sys/types.h>
#include <unistd.h>

//using namespace std;

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
    if( ++count == _BUSY_WAIT_MAX ) { count = 0; } \
  } \
}
//    if( ++count == _BUSY_WAIT_MAX ) { count = 0; sched_yield(); } \



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
namespace comm_primitives {
  bool poll(thread_primitives::threadInfo* const t, const bool busyWait=false);
  void flush(thread_primitives::threadInfo* const t);
  void periodic_poll(thread_primitives::threadInfo* const t);
  void comm_thread(int numThreads);
  
  volatile int fenceFlag;
  volatile int SRcomm;
  void* reduceTmp = 0;  //maybe volatile
  
#if _THREAD_SAFE_MPI == 0
  typedef thread_primitives::simpleLock lock;
#else
  struct emptyLock {
    inline void lock() {}
    inline void unlock() {}
  };
  typedef emptyLock lock;
#endif

  lock syncLock;       //used for various synchronizations among threads

  struct reduceInfo {
    volatile bool flag; 
    lock reduceLock;
    mpi_primitives::rmiRequestReduce *rmirequest;
    bool commutative;
    int  rootThread;     
  } reduceInfo;
  
  struct bcastInfo {
    volatile bool flag;
    lock bcastLock;
    int rootThread;
    void *buffer;
    int size;
  } bcastInfo; 
  
} //end namespace comm_primitives


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
  volatile void**      syncRelease; // flag for completing sync_rmi's
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
  comm_primitives::lock sectionLock;
  char* pos;                // position to start writing next message in the current section
  unsigned int aggCount;    // the current count, per destination thread
  char* buf;                // pointer to the current section

public:
  struct section {
    MPI_Request request;    // tracks when MPI has finished sending a section
    double kludge;          // kludge, to ensure section is properly aligned
    char buf[ALIGNED_SIZEOF(msgHeader) + _SEND_BUF_MAX];
    int size;
    volatile int sendFlag;
    section() : request( MPI_REQUEST_NULL ), sendFlag(0) {}
  };
  enum initSections { MAX=1000 };
  unsigned int sectionID;   // id of the current section
  unsigned int numSections; // number of available sections
  section** sections;

  // If all sections are busy, allocate a new one at the end.  This allows for
  // unlimited sections, bounded only by the total available memory.
  void increaseSections() {
    //    printf("increasing sections\n"); fflush(stdout);
    ++numSections;
    sectionLock.lock();
    sections = static_cast<section**>( realloc(sections, sizeof(section*)*numSections) );
    sectionLock.unlock();
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
  void set_size(int i) { sections[sectionID]->size = i; }
  int current_section() { return sectionID; }
  void set_sendFlag() { sections[sectionID]->sendFlag = 1; }
  void clear_sendFlag() { sections[sectionID]->sendFlag = 0; }

  // place the header tag information on the current section
  void tag(requestType tag, const int id, volatile void** syncRelease=0) { //+++ added id
    msgHeader& h = *reinterpret_cast<msgHeader*>( sections[sectionID]->buf );
    h.tag = tag;
    h.source = id;
    h.syncRelease = syncRelease;
  }

  // find a temporary buffer, and optionally, a persistent MPI_Request
  //  char* findBuf(MPI_Request** req=0) {
  char* findBuf() {
    unsigned int tmpSection = 0;
    for( ; tmpSection<numSections; ++tmpSection )
      if( (tmpSection != sectionID) && (!sections[tmpSection]->sendFlag) )
	break;

    if (tmpSection == numSections)
      increaseSections();
    return sections[tmpSection]->buf;
  }

  // look for the first available section and reset it for filling
  void reset() {
    for( sectionID=0; sectionID<numSections; ++sectionID ) {
      if (!sections[sectionID]->sendFlag)
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
    enum initSections { MAX=100 };
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
      unsigned int count = 0;
      for (sectionID=0; sections[sectionID]->done!=true; sectionID=(sectionID+1)%MAX) {
	
	if (sectionID == 0)
	  if ( ++count == _BUSY_WAIT_MAX ) { 
	    count = 0; 
//	    printf("Yielding\n"); fflush(stdout);
	    sched_yield(); 

	  }
      }
      
      //For some reason this causes a crash
      
  //   for( sectionID=0; sectionID<numSections; ++sectionID )
// 	if( sections[sectionID]->done == true ) {
// 	  buf = sections[sectionID]->buf;
// 	  return;
// 	}

//       // all sections are busy, allocate a new one at the end
//       // this allows for unlimited sections, bounded by the total available memory
//       sectionID = numSections; ++numSections;
//       sections = static_cast<section**>( realloc(sections, sizeof(section*)*numSections) );
//       stapl_assert( sections != 0, "Memory Allocation Failed" );
//       sections[sectionID] = new section;
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
  comm_primitives::lock sendLock;
  std::vector<std::pair<int,int> > sendSections;
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
  void *stackPtr;

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
    numWaits( 0 ) {
    //+++ moved initialization code from main() to be per-thread
    mpiSendBuf = new mpi_primitives::perThreadSendBuffer[mpi_primitives::mpiProcs * nThreads]; //+++ mpiProcs
    largeBuffer = 0;
    largeBufferSize = 0;
    __nestedRecv = 0;
    recvBuf = static_cast<char*>( safe_malloc(sizeof(char) * _RECV_BUF_MAX) );
    recvPos = recvBuf;
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
	stapl_assert( sizeof(bool[4]) <= sizeof(unsigned int), 
		      "packedBoolArray union incorrectly sized" );
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
    int id = stapl::get_thread_id();
    //printf("%d:  In fence\n", id);
//     fflush(stdout);
    register fenceNode* n = &fenceNodes[map[t->threadID].thread]; //+++ map
    
    // Wait for all children to arrive
    busy_wait( n->childNotReady.volatileWhole != 0, { poll(t, true); flush(t); } );
    n->childNotReady.whole = n->haveChild.whole;
    
    // Calculate the parent and its children's portion of globalSR
    n->localSR = t->localSR + *n->childLocalSR[0] + *n->childLocalSR[1]
      + *n->childLocalSR[2] + *n->childLocalSR[3];
    
    // Notify parent of arrival
    *n->parentPtr = false;
    
//      printf("%d: about to poll in fence\n", id);
//     fflush(stdout);
    // Wait for release, and propagate to children
    if( map[t->threadID].thread == 0 ) { //+++ map
      if( doMPI ) fenceNode::globalSR = mpi_primitives::mpi_fence( t, n->localSR ); //+++
      else fenceNode::globalSR = n->localSR;
      fenceNode::globalSense = t->privateSense;
    }
    else {
//     printf("%d:  immediate poll\n", id);
       busy_wait( fenceNode::globalSense != t->privateSense, { poll(t, true); flush(t); } );
    }

    // Reverse the privateSense and return the globalSR
//     printf("%d:  Past Wait\n", id);
//     fflush(stdout);
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
  enum maxEntries { EXPANSION=1000 };

  struct entry {
    char* _begin;
    char* _end;
    char* rbuf;
    int* rsize;
    int source;
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
       if (!stapl::get_thread_id()) {
 	printf("Failed to Enqueue, full\n"); fflush(stdout);
       }
      return false;
    }
    return true;
  }

  void enqueue(char* begin, char* end, volatile bool* done, int source, 
  	       char* returnBuf, int *returnSize) {
    busy_wait( !trylockEntry(), ; ); // no polls, to prevent nested RMI invocation
    requests[last]._begin = begin;
    requests[last]._end = end;
    requests[last]._done = done;
     requests[last].rbuf = returnBuf;
      requests[last].rsize = returnSize;
      requests[last].source = source;
    last = (last + 1) % size;
    lock.unlock();
  }

  bool dequeue(threadInfo* const t) {
    bool rtn = false;
    while( current != last ) {
      int id = stapl::get_thread_id();
//       if (id == 1) {
// 	printf("%d in dequeue\n", id); fflush(stdout);
//       }
      lock.lock();
      char* begin = requests[current]._begin;
      char* end = requests[current]._end;
      volatile bool* done  = requests[current]._done;
      char *rbuf = requests[current].rbuf;
      int *rsize = requests[current].rsize;
      int source = requests[current].source;
      current = (current + 1) % size;
      lock.unlock();
//       if (id == 1) {
// 	printf("%d decremented it's queue\n", id); fflush(stdout);
//       }
      
      if (source == 0) {
	for( char* c=begin; c!=end; c+=((rmiRequest*)c)->_size )
	  reinterpret_cast<rmiRequest*>( c )->exec( t->registry );
      } else {
	
	for( char* c=begin; c!=end; c+=((rmiRequest*)c)->_size ) {
	  reinterpret_cast<mpi_primitives::rmiRequest*>( c )->
	    exec( t->registry, rbuf, rsize);
	}
	
	if (rbuf != 0)
	  ++t->localSR;  //a send will be sent to source by comm_thread		
      }
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
    int id = stapl::get_thread_id();
    send.done() = false;
//     if (!id) {
//       printf("%d Start enqueue\n", id); fflush(stdout);
//     }
    recvBuf[destThread]->enqueue( send.begin(), send.end(), &send.done(), 0, 0, 0);
 //    if (!id) {
//       printf("%d Past enqueue\n", id); fflush(stdout);
//     }

    send.reset();
//     if (!id) {
//       printf("%d Past reset\n", id); fflush(stdout);
//     }
    ++t->localSR;
  }
  
  
  void addSyncRequest(const int destThread, threadInfo* const t,
		    char* const begin, char* const end) {
    volatile bool tmp = false;
    recvBuf[destThread]->enqueue( begin, end, &tmp, 0, 0, 0);
    busy_wait( tmp == false, poll(t) );
     ++t->localSR;
  }
  

  bool findRequest(threadInfo* const t) {
    return recvBuf[map[t->threadID].thread]->dequeue( t ); //+++: map
  }


  //****************************************************************************
  // Data structures shared between all threads, as well as the function each
  // thread actually executes, for calling stapl_main.
  //****************************************************************************
  threadInfo **allThreadInfo;

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
// 	printf("checking indices %d / %d\n", index1, index2);
 	stapl_assert(index1==index2,"Automatic Registration Failure - nonSPMD registration");
      }
    }
  }    

  volatile int threads_started=0;
  volatile int threads_finished=0;
  

  // The user-specified command line arguments
  int                argc;
  char**             argv;

  
  void thread_main(const int threadID, const int numThreads) {
    threadInfo *t = new threadInfo(threadID, numThreads); set_threadInfo(t);
    allThreadInfo[map[threadID].thread] = t;
    recvBuf[threadID] = new request( numThreads );

#ifdef _ARMI_REGISTRY_VERIFY_TRACE
    t->stackPtr = (void *) getStackPtr();
#endif //_ARMI_REGISTRY_VERIFY_TRACE
    

    syncLock.lock();
    ++threads_started;
    syncLock.unlock();
    
    stapl::rmi_fence();
    stapl_main( argc, argv );
    stapl::rmi_fence();

    syncLock.lock();
      ++threads_finished;
    syncLock.unlock();

    delete recvBuf[threadID];
  }


  // Find space for async_rmi requests directly in sendBuf.  mustSend is set to
  // true if a subsequent call to send_async_rmi is necessary (although this is
  // ugly, it improves performance by up to 25%, by avoiding the second call to
  // get_threadInfo, which is expensive if implemented with pthread_getspecific).
  void* rmiRequest::operator new(size_t size, const int _destThread, bool& mustSend) {
    int id = stapl::get_thread_id();
//     if (!id) {
//       printf("%d Enter New\n", id); fflush(stdout);
//     }
    const int destThread = map[_destThread].thread; //+++: map
    threadInfo* const t = get_threadInfo();
    periodic_poll( t );
    perThreadSendBuffer::sendData& send = t->sendBuf[destThread];
//     if (!id) {
//       printf("%d Past Poll\n", id); fflush(stdout);
//     }
    if( send.offset+size > _SEND_BUF_MAX ) {
      stapl_assert( size <= _SEND_BUF_MAX, 
	 "argument size exceeded: increase _SEND_BUF_MAX or pass argument as a pointer");
      addAsyncRequest( destThread, t, send );
    }
//     if (!id) {
//       printf("%d Past Async\n", id); fflush(stdout);
//     }
    void* const tmpPos = &send.buf[send.offset];
    if( t->threadID == destThread )
      mustSend = true;
    else {
      send.offset += size;
      if( ++send.aggCount >= t->aggMax )
	mustSend = true;
    }
//     if (!id) {
//       printf("%d Exit New\n", id); fflush(stdout);
//     }
    return tmpPos;
  }


  void send_async_rmi(const int _destThread) {
    const int destThread = map[_destThread].thread; //+++: map
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
    const int destThread = map[_destThread].thread; //+++: map
    threadInfo* const t = get_threadInfo();
    if( t->threadID == destThread ) {
      periodic_poll( t );
      r->exec( t->registry );
      ++t->numRMI;
    }
    else {
      if( t->sendBuf[destThread].offset != 0 )
	addAsyncRequest( destThread, t, t->sendBuf[destThread] );
      addSyncRequest( destThread, t, reinterpret_cast<char*>(r), 
		      reinterpret_cast<char*>(r) + r->_size );
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
      if( map[t->threadID].thread%i != 0 || map[t->threadID].thread+i/2 >= t->numThreads ) {
	threadReduce[map[t->threadID].thread] = in;
	break;
      }
      else {
	while( threadReduce[map[t->threadID].thread+i/2] == 0 ); // busy-wait
	rmi->exec( t->registry, in, threadReduce[map[t->threadID].thread+i/2] );
	in = threadReduce[map[t->threadID].thread+i/2];
      }
    }
    if( map[t->threadID].thread == 0 ) {
      threadReduce[0] = in;
      rmi->result( in );
      fence( t );
      threadReduce[map[t->threadID].thread] = 0;
    } else {
      while( (in = threadReduce[0]) == 0 ); // busy-wait
      threadReduce[map[t->threadID].thread] = 0;
      fence( t );
    }
  }
  
} //end namespace thread_primitives




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

// variables and functions to implement reduce_rmi
rmiRequestReduce* rf;
MPI_Op commute;
MPI_Op no_commute;

void reduce_internal(void* in, void* inout, int*, MPI_Datatype*) {
  //  rf->exec( thread_primitives::get_threadInfo()->registry, in, inout );
  rf->exec(thread_primitives::allThreadInfo[0]->registry, in, inout);
}


// start sending the given sendBuf half, and prepare the other half for use
inline void request_send(const int destThread, requestType tag=ASYNC_ONLY, volatile void** syncRelease=0) {
  //printf("Enter Request Send\n"); fflush(stdout);
 thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  if( t->largeBuffer == 0 ) {
    perThreadSendBuffer& send = t->mpiSendBuf[destThread];
    send.tag( tag, t->threadID, syncRelease );

    if ((tag != ASYNC_ONLY) && (tag != ASYNC_SYNC)) {      
      stapl_assert(0, "request_send tag != ASYNC_ONLY or ASYNC_SYNC?");
    } else {
      send.set_size(send.end() - send.begin());
      send.set_sendFlag();
      std::pair<int,int> temp;
      temp.first = destThread;
      temp.second = send.current_section();
      t->sendLock.lock();
      t->sendSections.push_back(temp);
      t->sendLock.unlock();
      send.reset();
    }
  } else {
    stapl_assert(0, "Large Message Send\n");
//     reinterpret_cast<msgHeader*>( t->largeBuffer )->tag = tag;
//     reinterpret_cast<msgHeader*>( t->largeBuffer )->source = t->threadID; //+++ id
//     MPI_Request req = MPI_REQUEST_NULL;
//     mpiLock.lock(); MPI_Isend( t->largeBuffer, t->largeBufferSize, MPI_BYTE, map[destThread].mpiRank, destThread, RMI_COMM, &req ); mpiLock.unlock(); //+++: map
//     request_wait( &req, t );
//     free( t->largeBuffer );
//     t->largeBuffer = 0;
  }
  //    printf("%d request_send ++ lsr\n", t->threadID); fflush(stdout);
  ++t->localSR;
  //  printf("ExitRequest Send\n"); fflush(stdout);

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
  //printf("%d Entering send_sync_rmi\n", mpiRank);
  fflush(stdout);

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

    //printf("%d Exiting send_sync_rmi1\n", mpiRank);
    fflush(stdout);
    return tmpBuf;
  }

  // Wait for the return, while handling other incoming requests
  volatile void* syncRelease = 0;
  request_send( destThread, ASYNC_SYNC, &syncRelease );
  while (!syncRelease) {
    thread_primitives::findRequest( t );
  }

  //printf("%d Exiting send_sync_rmi2\n", mpiRank);
  fflush(stdout);
  --t->localSR;
  return reinterpret_cast<char*>( (void *) syncRelease ) + ALIGNED_SIZEOF( msgHeader );
}


// Get an MPI buffer for temporary use for arguments of collective functions
void* get_arg_buffer(const int size) {
  stapl_assert( size <= _SEND_BUF_MAX, "input too large, increase _SEND_BUF_MAX" );
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  return t->mpiSendBuf[t->threadID].findBuf();
}


void send_reduce_rmi(rmiRequestReduce* const f, const bool commutative, const int rootThread) {
    using comm_primitives::reduceInfo;
    reduceInfo.reduceLock.lock();
    reduceInfo.flag = true;
    reduceInfo.rmirequest = f;
    reduceInfo.commutative = commutative;
    reduceInfo.rootThread = rootThread;
    reduceInfo.reduceLock.unlock();
    //todo:change to busy wait (i.e. do something useful)
    while (reduceInfo.flag) { rmi_poll(); }
}


//+++ complete re-write compared to original MPI and thread implementations
//+++ moved out of thread_primitives to share the rmiRequestBroadcast class
void send_mpi_broadcast_rmi(void *inout, const int size, const int rootThread) {
  using comm_primitives::bcastInfo;  
  bcastInfo.bcastLock.lock();
  if (rootThread == -1) 
    bcastInfo.rootThread = -1;
  else
    bcastInfo.rootThread = map[rootThread].mpiRank;;
  bcastInfo.buffer = inout;
  bcastInfo.size = size;
  bcastInfo.flag = true;
  bcastInfo.bcastLock.unlock();
  while (bcastInfo.flag) { rmi_poll(); } //todo: do something useful here
}

void* volatile bcastTmp = 0;
void send_thread_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int rootThread) {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
  if( map[t->threadID].thread == rootThread ) {  
    bcastTmp = in;
  }

  thread_primitives::fence( t, false );
  if( map[t->threadID].thread != rootThread ) //+++ map
    rmi->exec( t->registry, bcastTmp );
  thread_primitives::fence( t, false );
}

void send_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int size, const int rootThread) {
 thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo(); //+++
  stapl_assert( size <= _SEND_BUF_MAX, "broadcast_rmi input too large, increase _SEND_BUF_MAX" );

  if( map[rootThread].mpiRank == mpiRank ) {   
    if (t->threadID == rootThread) 
      send_mpi_broadcast_rmi(in, size, rootThread);
    send_thread_broadcast_rmi( rmi, in, map[rootThread].thread ); 

  } else {

    if( map[t->threadID].thread == 0 ) {
      char* tmpBuf = t->mpiSendBuf[t->threadID].findBuf();
      send_mpi_broadcast_rmi(tmpBuf, size, rootThread);
       rmi->exec( t->registry, tmpBuf );
      send_thread_broadcast_rmi( rmi, tmpBuf, 0 );
    }
    else
      send_thread_broadcast_rmi( rmi, in, 0 );
  }
}


// This is a generic spanning tree based fence.  Children signal arrival to
// their parents up to the root, then parents signal release down to the
// leaves.  The spanning tree layout can be easily re-defined by setting
// parentID, numChildren and childrenID as desired in 'int main(...)' above.
// The root of the tree has parentID = mpiRank.
  int mpi_fence(thread_primitives::threadInfo* const t, const int localSR) {
    //printf("%d Entered mpi-fence\n", t->threadID); fflush(stdout);
    comm_primitives::SRcomm = localSR;
    comm_primitives::fenceFlag = 1;
    busy_wait(fenceFlag, { poll(t, true); flush(t); });
    //printf("%d Exit mpi-fence\n", t->threadID); fflush(stdout);
    return comm_primitives::SRcomm;
  }


  void checkRegistryTrace(void) {
    //fixme - nthomas this probably won't work with sizeof(void*) != sizeof(int)
    stapl_assert(sizeof(void*) == sizeof(int), "sizeof(void*) != sizeof(int)");

    rmiRegistry &registry = thread_primitives::allThreadInfo[0]->registry;
    
    int *sizeBuf;
    if (mpiRank == 0) 
      sizeBuf = (int *) malloc(mpiProcs*sizeof(int));
    int traceSize = registry.getTrace().size();
    
    //Check Registration Traces
    MPI_Gather (&traceSize, 1, MPI_INT, sizeBuf, 1, MPI_INT, 0, RMI_COMM);
    
    //Check That All Traces Are Same Size
    if (mpiRank == 0) {
      bool sameSize = true;
      for (int i = 1; i < mpiProcs; i++)
	sameSize = sameSize && (sizeBuf[i-1] == sizeBuf[i]); 
      stapl_assert(sameSize,"Automatic Registration Failure - registry size not equal");
    }  
    
    //Check That Traces Are Equivalent
    void **traceBuf = (void**) malloc(mpiProcs*sizeof(void*)*traceSize);
    void *trace = (void *) &(*registry.getTrace().begin());
    MPI_Gather(trace, traceSize, MPI_INT, traceBuf, traceSize, MPI_INT, 0, RMI_COMM);
    if (mpiRank == 0) {
      for (int i = 0; i < traceSize; i++) {
	bool sameValue = true;
	for (int j = 1; j < mpiProcs; j++) {
	  sameValue = sameValue && (traceBuf[i] == traceBuf[j*traceSize + i]);
// 	  printf("mpi check - %d == %d \n", traceBuf[i], traceBuf[j*traceSize + i]);
	}
	stapl_assert(sameValue,"Automatic Registration Failure - nonSPMD registration");  
      }
    }
  }
} // end namespace mpi_primitives




namespace comm_primitives {

  mpi_thread* map;
  
  bool isLocal(const int threadID) {
    return mpi_primitives::mpiRank == map[threadID].mpiRank;
  }

  
  bool poll(thread_primitives::threadInfo* const t, const bool busyWait) {
    using namespace mpi_primitives;
    bool rtn = false;
    t->pollCount = 0;
    rtn = thread_primitives::findRequest( t );
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

} // namespace comm_primitives



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
    return thread_primitives::get_threadInfo()->
      registry.registerObject( objectPointer );
  }
  
  
  void update_rmi_object(rmiHandle handle, void* const objectPointer) {
    thread_primitives::get_threadInfo()->
      registry.updateObject( handle, objectPointer );
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
    thread_primitives::get_threadInfo()->
      registry.unregisterObject( handle );
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
  comm_primitives::flush( thread_primitives::get_threadInfo() );
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
  comm_primitives::poll( thread_primitives::get_threadInfo() );
}


void rmi_wait() {
  thread_primitives::threadInfo* const t = thread_primitives::get_threadInfo();
  if( t->numWaits == t->numRMI ) {
    t->pollCount = 0;
    busy_wait( 0 == comm_primitives::poll(t, true), ; );
  }
  ++t->numWaits;
}


void rmi_fence() {
  using namespace thread_primitives;
  threadInfo* const t = get_threadInfo();
  int numCycles = 0;
  do {
    comm_primitives::poll( t ); comm_primitives::flush( t );
    t->numWaits = t->numRMI = 0;
    int globalSR = fence( t );
    
    // Iterate the fence until the global number of sends - receives = 0.
    // This ensures that all outstanding RMI requests have completed.  In the
    // simple case, iterating is not necessary.  However, if an RMI invokes a
    // method that starts another RMI, which invokes a method that starts
    // another RMI, etc, multiple iterations may be necessary.  To ensure the
    // program doesn't hang, abort after a certain number of iterations.
    if( globalSR == 0 ) break;
//       printf("%d looping on rmi_fence, globalSR = %d\n",
// 	     mpi_primitives::mpiRank, globalSR); fflush(stdout);
    stapl_assert( ++numCycles < 1000, "rmi_fence internal error: contact stapl development" );
  } while( 1 );
  t->pollCount = 0;
}


} // end namespace


int main(int argc, char *argv[]) {
  using namespace thread_primitives;
  using namespace mpi_primitives;

  fenceFlag = 0;

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

  thread_primitives::allThreadInfo = 
    static_cast<thread_primitives::threadInfo**> 
    ( safe_malloc(sizeof(thread_primitives::threadInfo*) * n ));


  //+++: Initialize the MPI-thread communication map.
  map = static_cast<mpi_thread*>( safe_malloc(sizeof(mpi_thread) * n * mpiProcs) );
  int index = 0;
  for( int i=0; i<mpiProcs; ++i ) {
    for( int j=0; j<n; ++j ) {
      comm_primitives::map[index].mpiRank = i;
      comm_primitives::map[index].thread = j;
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
  //I know this shouldn't go here, but thread_primitives_pthreads.h doesn't seem
  //right either - it won't work if it's shared with the straight thread implementation
  pthread_attr_t threadAttrs;
  ERROR_HANDLER( pthread_attr_init(&threadAttrs) );
  ERROR_HANDLER( pthread_attr_setdetachstate(&threadAttrs, PTHREAD_CREATE_JOINABLE) );
  pthread_t commThread;
  ERROR_HANDLER(pthread_create(&commThread, &threadAttrs,
			       (void*(*)(void*)) comm_primitives::comm_thread,
 			       (void*) n ));
  execute_threads( n );

  //printf("%d execute_threads completed\n", mpiRank); fflush(stdout);

  pthread_join(commThread, 0);

#ifdef _ARMI_REGISTRY_VERIFY_TRACE
  thread_primitives::checkRegistryTrace();
  mpi_primitives::checkRegistryTrace();
#endif //_ARMI_REGISTRY_VERIFY_TRACE


  // Cleanup shared data structures
  free( map ); //+++
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



namespace comm_primitives {

  bool parentSignalled = false;  //Signal I've signalled parent during a fence

  typedef struct sync_return {
    MPI_Request request;         //request to check against - sync_rtn send asynchronously
    bool *rmi_done;              //pointer to done for rmiRequest chunk w/ sync call
    char retbuf[_SEND_BUF_MAX];  //buffer for return value
    int retsize;                 //byte size of return value
    int src_thread;              //who am I sending this sync_rtn to
    int dest_thread;             //which one of my threads got the sync request
    bool rtn_sent;               //sent the sync_rtn yet?(just need isend to complete)
    volatile void **syncRelease; //this is pretty much just a unique id for the sync_rmi
  } sync_return;

  std::list<sync_return*> syncs;      //keep track of remote sync_rmi to send a return value

  //buffer sections of local computation threads that I'm currently sending
  std::list<mpi_primitives::perThreadSendBuffer::section *> sendReqs; 


  //Sends Messages to Children, releasing them from a fence
  inline void release_children(int numThreads, int threadID) {
    using namespace mpi_primitives;
    //printf("Release children\n"); fflush(stdout);
    for( int j = 0; j<numChildren; ++j ) {
      fenceMsg msg( FENCE_RELEASE, threadID, globalSR );
      MPI_Send( &msg, ALIGNED_SIZEOF(fenceMsg), MPI_BYTE, 
		childID[j], childID[j]*numThreads, RMI_COMM ); 
    }
  }


  //check to see if progress has been made toward a fence and take action if
  //ready to finish the fence
  inline bool check_fence(int numThreads, int threadID) {
    using namespace mpi_primitives;
    static MPI_Request request;
    int j;
    if (mpiRank == 0) {
      //printf("Checking Fence -- arrived %d / children %d / fenceFlag %d / parentsig %d\n",
      //numArrived, numChildren, fenceFlag, parentSignalled); fflush(stdout);
    }
    if ( (numArrived == numChildren) && (fenceFlag) && !(parentSignalled)) {
      //printf("%d Check_fence in if\n",mpiRank); fflush(stdout);
      numArrived = 0;	 
      //fixme do I need to decrement by size of syncs? (account for assumed already
      //sent by individual thread increment b/c of sync_rtn)
      //globalSR += SRcomm - syncs.size();
      //probably not b/c if not received yet,globalSR won't be 0,rmi_fence will iterate
      globalSR += SRcomm;

      //If I'm not the root, signal arrival to parent, else signal release to children
      if( parentID != mpiRank ) {
	//printf("%d Sending Arrival to %d\n", mpiRank, parentID); fflush(stdout);
	fenceMsg msg( FENCE_ARRIVAL, threadID, globalSR );
	MPI_Isend( &msg, ALIGNED_SIZEOF(fenceMsg), MPI_BYTE, 
		  parentID, parentID*numThreads, RMI_COMM, &request ); 
	parentSignalled = true;
        //printf("%d Past Sending Arrival to %d\n", mpiRank, parentID); fflush(stdout);

      } else {
// 	printf("%d Releasing Children\n", mpiRank); fflush(stdout);
	release_children(numThreads, threadID);
	SRcomm = globalSR;
	globalSR = 0;
	fenceFlag = 0;
//         printf("%d Released FenceFlag\n", mpiRank); fflush(stdout);

      }
      return true;
    }
    return false;
  }
  

  //Check to see if the lead computation thread has requested a broadcast
  inline bool check_broadcast(void) {
    bcastInfo.bcastLock.lock();
    if (bcastInfo.flag) {
      MPI_Bcast( bcastInfo.buffer, bcastInfo.size, MPI_BYTE, 
		 bcastInfo.rootThread, mpi_primitives::RMI_COMM );
      bcastInfo.flag = false;
      bcastInfo.bcastLock.unlock();
      return true;
    }
    
    bcastInfo.bcastLock.unlock();
    return false;
  }


  //Check to see if the lead computation thread has requested a reduce
  inline bool check_reduce(void) {
    reduceInfo.reduceLock.lock();
    if (reduceInfo.flag) {
      mpi_primitives::rf = reduceInfo.rmirequest;
      
      if (reduceInfo.rootThread == -1) {
	
	MPI_Allreduce( reduceInfo.rmirequest->_in, reduceInfo.rmirequest->_inout, 
		       reduceInfo.rmirequest->_size, MPI_BYTE,
		       (reduceInfo.commutative ? mpi_primitives::commute : mpi_primitives::no_commute), mpi_primitives::RMI_COMM );

      } else {
	MPI_Reduce( reduceInfo.rmirequest->_in, reduceInfo.rmirequest->_inout, 
		    reduceInfo.rmirequest->_size, MPI_BYTE,
		    (reduceInfo.commutative ? mpi_primitives::commute : mpi_primitives::no_commute), 
		    map[reduceInfo.rootThread].mpiRank, mpi_primitives::RMI_COMM );
      }
      reduceInfo.flag = false;
      reduceInfo.reduceLock.unlock();
      return true;
    }
 
    reduceInfo.reduceLock.unlock();
    return false;
  }


  //Check to see if any local computation thread has finished executing a remote
  //sync request.  If so ship off the return value.  Subsequent invocation will
  //then check for message arrival before removing it from the list.
  inline bool check_sync_rtn(void) {
    static int count = 0;
    int flag;
    bool rtn = false;
    bool inc;
    using namespace mpi_primitives;
    MPI_Status status;
    
    std::list<sync_return*>::iterator jiter = syncs.begin();
    
    while (jiter != syncs.end()) {
      inc = true;
      sync_return &syn_rtn = **jiter;
      
      if (*syn_rtn.rmi_done) {
	
	if (!syn_rtn.rtn_sent) {	     
	  rtn = true;
	  msgHeader &hdr = *reinterpret_cast<msgHeader*> (syn_rtn.retbuf);	   
	  hdr.tag = SYNC_RTN;
	  hdr.source = syn_rtn.dest_thread;
	  hdr.syncRelease = syn_rtn.syncRelease;
	  int dest = map[syn_rtn.src_thread].mpiRank;
	  int size = syn_rtn.retsize + ALIGNED_SIZEOF(msgHeader);
	  syn_rtn.rtn_sent = true;
 	  fflush(stdout);
	  MPI_Isend(syn_rtn.retbuf, size, MPI_BYTE, dest, syn_rtn.src_thread, 
		    RMI_COMM, &syn_rtn.request);
	} else {
	  MPI_Test(&syn_rtn.request, &flag, &status);
	  if (flag) {
	    std::list<sync_return*>::iterator titer = jiter;
	    ++jiter;
	    inc = false;
	    free(*titer);
	    syncs.erase(titer);
	    rtn = true;
	  }
	}
      }
      if (inc)
	++jiter;
    }
    return rtn;
  }


  //Check a computation thread's send queue for messages to send
  inline bool check_send(int thread) {
    using thread_primitives::allThreadInfo;
    bool rtn = false;
    int i;
    int dest, sectionid;

    std::vector<std::pair<int,int> > &sends = allThreadInfo[thread]->sendSections;
    allThreadInfo[thread]->sendLock.lock();     

      int numSends = sends.size();
      if (numSends) {
	rtn = true;
	
	for (i=0; i<numSends; i++) {
	  dest = sends[i].first;
	  sectionid = sends[i].second;
	  mpi_primitives:: perThreadSendBuffer &send = 
	    allThreadInfo[thread]->mpiSendBuf[dest];

	   //We need to lock to prevent comp thread from doing a realloc on send.sections
	  send.sectionLock.lock(); 
	  mpi_primitives::perThreadSendBuffer::section &section = 
	    *(send.sections[sectionid]);
	  send.sectionLock.unlock();

	  sendReqs.push_back(send.sections[sectionid]); 
	  MPI_Isend( section.buf, section.size, MPI_BYTE, map[dest].mpiRank, 
		     dest, mpi_primitives::RMI_COMM, &section.request);	
	  
	}
	sends.clear();
      }
      allThreadInfo[thread]->sendLock.unlock();
    
    return rtn;
  }



  //Check all outstanding SYNC/ASYNC sends to see if they have completed
  //notify the computation thread that the buffer is available for use
  inline bool check_completed_sends() {
    bool rtn = false;
    int flag;
    MPI_Status status;
    std::list<mpi_primitives::perThreadSendBuffer::section*>::iterator jiter=sendReqs.begin();
    //printf("%d: sendReqs size = %d\n", mpiRank, sendReqs.size());
    //fflush(stdout);
 
    while (jiter != sendReqs.end()) {
      MPI_Test(&((*jiter)->request), &flag, &status);
      if (flag) {
	std::list<mpi_primitives::perThreadSendBuffer::section*>::iterator titer = jiter;
	(*jiter)->sendFlag = 0;
	++jiter;
	sendReqs.erase(titer);
	rtn = true;
      } else {
	++jiter;
      }
    }
    return rtn;
  }
  

  //allocate space for a return value and set syncRelease to signal waiting thread
  inline void recv_sync_rtn(char *buf, int size, bool *done) {
    using mpi_primitives::msgHeader;
    using stapl::safe_malloc;
    msgHeader &hdr = *reinterpret_cast<msgHeader*> (buf);	   
    void *temp = (void *) safe_malloc(size);
    memcpy(temp, buf, size);
    *hdr.syncRelease = temp;
    *done = true;    
  }

  
  //receive arrival from a child, update numarrived and my globalSR
  inline void recv_fence_arrival(char *buf, bool *done) {
    using mpi_primitives::fenceMsg;
    using mpi_primitives::globalSR;
    using mpi_primitives::numArrived;
    //printf("%d Received Fence Arrival\n", mpiRank); fflush(stdout);
    globalSR += reinterpret_cast<fenceMsg*>(buf)->sr;
    ++numArrived;
    *done = true;       
  }
  
  
  //update my local globalSR, and flag local threads to release, also send release
  //to my mpi children
  inline void recv_fence_release(char *buf, int numThreads, int threadID, bool *done) {
    using mpi_primitives::globalSR;
    using mpi_primitives::fenceMsg;

    globalSR = reinterpret_cast<fenceMsg*>(buf)->sr;	      
    release_children(numThreads, threadID);
    *done = true;
    parentSignalled = false;
    SRcomm = globalSR;
    globalSR = 0;
    fenceFlag = 0;
    //printf("%d Released FenceFlag\n", mpiRank); fflush(stdout);
  }


  //enqueue this request on the local thread's request queue
  inline void recv_async_only(char *buf, int size, int thread, bool *done) {
    using mpi_primitives::msgHeader;
    using thread_primitives::recvBuf;
    
    *done = false;
    recvBuf[thread]->enqueue(buf+ALIGNED_SIZEOF(msgHeader), buf+size, done, 1, 0, 0);
  }

  
  //enqueue this request on the local thread's request queue and setup
  //mechanism for propagating the return value back to the callee
  inline void recv_async_sync(char *buf, int size, int thread, bool *done) {
    using mpi_primitives::msgHeader;
    using stapl::safe_malloc;
    using thread_primitives::recvBuf;

    msgHeader &hdr = *reinterpret_cast<msgHeader*> (buf);	   
    sync_return *syn_rtn = static_cast<sync_return*>
      (safe_malloc(sizeof(sync_return)));
    
    *done = false;  
    recvBuf[thread]->enqueue(buf+ALIGNED_SIZEOF(msgHeader), buf+size, done, 1, 
			     (char*)&syn_rtn->retbuf + ALIGNED_SIZEOF(msgHeader),
			     &(syn_rtn->retsize));
    
    syn_rtn->request = MPI_REQUEST_NULL;
    syn_rtn->rmi_done = done;
    syn_rtn->src_thread = hdr.source;
    syn_rtn->dest_thread = thread;
    syn_rtn->rtn_sent = 0;
    syn_rtn->syncRelease = hdr.syncRelease;
    syncs.push_back(syn_rtn);
  }


  //Check if there's the communication thread is doing thing, if not yield to other threads
  inline void check_work(bool work_done) {
    static int workless_iters = 0;

    if (work_done) {
      workless_iters = 0;
    } else {
      workless_iters++;
      if (workless_iters == _BUSY_WAIT_MAX) {
	workless_iters = 0;
//	printf("%d: comm_thread yielding\n", mpi_primitives::mpiRank); fflush(stdout);
	sched_yield();	  	  
      }
    }
  }


  //This is the main function for the communication thread.  It's basically
  //a big loop sees to all remote communication responsibilities.
  void comm_thread(int numThreads) {

    using namespace mpi_primitives;   
    using thread_primitives::allThreadInfo;

    MPI_Status status;
    MPI_Request *requests;

    int flag;
    int size;
    int i,j,k=0;
    int sent=0;
    bool work_done;     
    const int expansion = 1000;
    
    char **buffers = static_cast<char **> ( safe_malloc(sizeof(char*) * numThreads));
    int *bufPos = static_cast<int *> ( safe_malloc(sizeof(int) * numThreads));
    char **curBuf =  static_cast<char **> ( safe_malloc(sizeof(char*) * numThreads));
    bool *done = static_cast<bool *> 
      ( safe_malloc(sizeof(bool) * numThreads * expansion));
    int *stats = static_cast<int*> (safe_malloc(sizeof(int) * numThreads));
 
    requests = static_cast<MPI_Request *> ( safe_malloc(sizeof(MPI_Request) * numThreads));

    for (i = 0; i < numThreads * expansion; i++) {
      done[i] = true;
    }
    
    int bufsize = _SEND_BUF_MAX + ALIGNED_SIZEOF(msgHeader);
    for (i = 0; i < numThreads; i++) {
      requests[i] = MPI_REQUEST_NULL;
      stats[i] = 0;
      buffers[i] = static_cast<char*> ( safe_malloc(sizeof(char) * bufsize * expansion));
      bufPos[i] = 0;
      curBuf[i] = buffers[i];
    }

    int tim = 0; k = 0;
    while (thread_primitives::threads_started  != numThreads) { }   
    while (thread_primitives::threads_finished != numThreads) {
  
     work_done = false;      
      for (i=0; i<numThreads; i++) {
	//printf("%d: Looping\n", mpiRank); fflush(stdout);
 	if (!done[i*expansion + bufPos[i]]) {
	  //	  printf("%d Buffers needed for %d\n", mpiRank, i); fflush(stdout);
	} 

	thread_primitives::threadInfo *t = thread_primitives::allThreadInfo[i];
	if (done[i*expansion + bufPos[i]]) {
	  
	  if (requests[i] == MPI_REQUEST_NULL) 	  
	    MPI_Irecv(curBuf[i], bufsize, MPI_BYTE, MPI_ANY_SOURCE, 
		      t->threadID, RMI_COMM, &requests[i] );
	  
	  MPI_Test(&requests[i], &flag, &status); 
	  
	  while (flag) {     
	    work_done = true;
	    MPI_Get_count(&status, MPI_BYTE, &size ); 
	    bool *donePtr =  &done[i*expansion + bufPos[i]];
            ++stats[i];
	    msgHeader &hdr = *reinterpret_cast<msgHeader*> (curBuf[i]);	   	    
	    switch(hdr.tag) {
	    case ASYNC_ONLY:
	      recv_async_only(curBuf[i], size, i, donePtr); break;
	    case ASYNC_SYNC:
	      recv_async_sync(curBuf[i], size, i, donePtr); break;
	    case SYNC_RTN:
	      recv_sync_rtn(curBuf[i], size, donePtr); break;
	    case FENCE_ARRIVAL: 
	      recv_fence_arrival(curBuf[i], donePtr); break;
	    case FENCE_RELEASE:
	      recv_fence_release(curBuf[i],numThreads,allThreadInfo[0]->threadID,donePtr);
	      break;
	    default: stapl_assert(0, "Unknown message header tag type");
	    }

	    bufPos[i] += 1;
	    bufPos[i] %= expansion;
	    curBuf[i] = buffers[i] + bufsize * bufPos[i];
	    
	    if (!done[i*expansion + bufPos[i]]) {
	      requests[i] = MPI_REQUEST_NULL;
	      break;	      
	    }

	    MPI_Irecv(curBuf[i], bufsize, MPI_BYTE, MPI_ANY_SOURCE, 
		      t->threadID, RMI_COMM, &requests[i] );
	    MPI_Test(&requests[i], &flag, &status );	
	  }
	}
	//        printf("%d Spin 1\n",mpiRank);        
	work_done |= check_send(i);          
	//        printf("%d Spin 3\n",mpiRank);        
	work_done |= check_sync_rtn(); 
	//        printf("%d Spin 4\n",mpiRank);        
	work_done |= check_fence(numThreads, allThreadInfo[0]->threadID);
	//        printf("%d Spin 5\n",mpiRank);        
	work_done |= check_broadcast();
	//        printf("%d Spin 6\n",mpiRank);        
	work_done |= check_reduce();
	//        printf("%d Spin 7\n",mpiRank);        
	check_work(work_done);
	//        printf("%d Spin 8\n",mpiRank);        

      }
          
      //        printf("%d Spin 2\n",mpiRank);      
	++k;
	if (k > 500) {
	  k = 0;
	  work_done |= check_completed_sends();
	}
    } //end while !thread_finished
//     printf("%d: Exiting Comm Thread Stats are:\n",mpiRank); fflush(stdout);
//     for (i =0; i< numThreads; i++) {
//       printf("%d: %d received %d\n",mpiRank, i, stats[i]); fflush(stdout); 
//     }
 } // end comm_thread
} //end namespace comm_primitives



