//****************************************************************************
// Implements the primitives using MPI-1.1.
//
// The number of threads to use is determined by the underlying MPI
// implementation.  Usually, this is set by the 'mpirun -np' command, although
// some implementations use command line flags.
//
// Assumptions:
// - MPI errors are fatal (i.e., abort), as specified by the standard
// - a homogeneous runtime environment (i.e., all MPI processes are running
//   on similar machines and compiled using the same compiler, such that
//   datatype and member function pointer conversion is not necessary)
//****************************************************************************
#ifndef _BYTE_ALIGNMENT
#define _BYTE_ALIGNMENT 8
#endif

#define _COMPILING_PRIMITIVES

#include "pointer.h"
#include "primitives.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>

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




namespace mpi_primitives {



// Forward declarations
MPI_Status request_wait(MPI_Request* req);

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
  OPAQUE_RTN,      // One or more returns from opaque fetches
  SYNC_RTN,        // contains the return value for a sync_rmi
  FENCE_ARRIVAL,   // child has arrived at the rmi_fence
  FENCE_RELEASE,   // parent has released from the rmi_fence
  ABORT_REQ        // abort all threads after printing error message
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
int id;                // STAPL thread ID (same as MPI rank)
int numThreads;        // number of threads (same as number of MPI processes)
rmiRegistry registry;  // the RMI object registry
MPI_Comm RMI_COMM;     // private communicator, allows user-codes to use RMI and MPI calls in the same program


// variables used during spanning tree based communication (e.g., rmi_fence)
bool parentRelease;    // true if the parent has signaled fence release
int localSR;           // the local number of sends - receives
int globalSR;          // the global number of sends - receives, set during rmi_fence()
int numArrived;        // number of child threads arrived at rmi_fence()
int parentID;          // this thread's parent id in the tree
int numChildren;       // this thread's number of children in the tree
#define MAX_KIDS 12
int childID[MAX_KIDS]; // this thread's children id's in the tree


// agg (i.e, aggregation) is the number of small (i.e., < _SEND_BUF_MAX) RMI
// requests to buffer before sending the buffer to the destination thread.
// Requests are buffered up to agg or to the size of the buffer, whichever
// occurs first.
const unsigned int maxAggregation = _SEND_BUF_MAX / sizeof( rmiRequest0<arg_storage<int>,void> );
unsigned int aggMax;     // the max count before sending

// poll is the number of communication calls that should execute before
// actually performing a poll for incoming requests
unsigned int pollCount;  // the current number of calls executed
unsigned int pollMax;    // 1 in pollMax calls performs a poll

// variables to ensure the proper synchronization of rmi_wait
unsigned int numRMI = 0;   // number of RMI requests received since the last rmi_fence
unsigned int numWaits = 0; // number of rmi_waits since the last rmi_fence


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
    bool sent;              // if not current sectio but used for return values,
                            // prevent use by others until msg is sent (i.e., req != MPI_REQUEST_NULL)
    char buf[ALIGNED_SIZEOF(msgHeader) + _SEND_BUF_MAX];
    section() : request( MPI_REQUEST_NULL ), sent(true) {}
  };
  enum initSections { MAX=2 };
  unsigned int sectionID;   // id of the current section
  unsigned int numSections; // number of available sections
  section** sections;

  // If all sections are busy, allocate a new one at the end.  This allows for
  // unlimited sections, bounded only by the total available memory.
  void increaseSections() {
    ++numSections;
    sections = static_cast<section**> 
      (realloc(sections, sizeof(section*)*numSections));

    stapl_assert( sections != NULL, "realloc() failed" );
    //    sections[sectionID] = new section; //wrong wrong wrong
    sections[numSections-1] = new section;
  }

public:
  perThreadSendBuffer() : sectionID( 0 ), numSections( MAX ) {
    sections = static_cast<section**>( safe_malloc(sizeof(section*)*numSections) );
    stapl_assert( sections != NULL, "safe_malloc() failed");
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
  void tag(requestType tag, void** syncRelease=0) {
    msgHeader& h = *reinterpret_cast<msgHeader*>( sections[sectionID]->buf );
    h.tag = tag;
    h.source = id;
    h.syncRelease = syncRelease;
  }

  // find a temporary buffer, and optionally, a persistent MPI_Request
  char* findBuf(MPI_Request** req=0, bool snt = true) {
    unsigned int tmpSection = 0;
    for( ; tmpSection<numSections; ++tmpSection )
      if( tmpSection != sectionID  && sections[tmpSection]->sent ) {
	MPI_Status status; int flag = true;
	if( sections[tmpSection]->request != MPI_REQUEST_NULL )
	  MPI_Test( &sections[tmpSection]->request, &flag, &status );
	if( flag )
	  break;
      }
    if (tmpSection == numSections) {
      increaseSections();
    }
    if( req != 0 )
      *req = &sections[tmpSection]->request;
    sections[tmpSection]->sent = snt;
    return sections[tmpSection]->buf;
  }

  void mark_sent(const char *c) {
    unsigned int tmpSection = 0;
    for( ; tmpSection<numSections; ++tmpSection ) {
       if ((char*) sections[tmpSection]->buf == c) {
          sections[tmpSection]->sent = true;
          return;
       }
    }
    stapl_assert(0, "mark_sent() failed to mark any buffer!");
  }

  // look for the first available section and reset it for filling
  void reset() {
    for( sectionID=0; sectionID<numSections; ++sectionID ) {
      if (!sections[sectionID]->sent)
         continue;

      MPI_Status status; int flag = true;
      if( sections[sectionID]->request != MPI_REQUEST_NULL )
	MPI_Test( &sections[sectionID]->request, &flag, &status );
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
perThreadSendBuffer* sendBuf;
char* largeBuffer = 0;     // scratch space for malloc'ed large messages (> _SEND_BUF_MAX)
int   largeBufferSize = 0;


// recvBuf provides space for incoming rmiRequests.  Instead of using
// sections, as in sendBuf, recvBuf is just one large buffer.  Usually,
// messages are Irecv'ed using recvRequest, to help overlap communication and
// computation.  If recvRequest is MPI_REQUEST_NULL, then no Irecv is
// currently outstanding.  recvBuf acts as a stack, allowing multiple
// rmiRequests to be received (e.g., an async_rmi invocation uses sync_rmi
// during execution).  recvPos maintains the current position on the stack.
char* recvPos;
MPI_Request recvRequest;
char recvBuf[_RECV_BUF_MAX+_BYTE_ALIGNMENT];


// Calculate the exact amount of space left in recvBuf for receiving the next
// message.  This will allow MPI to issue an error and abort if something is
// received that exceeds that bound.
inline int space() { return recvBuf + _RECV_BUF_MAX - recvPos; }


// recvBufProtector enforces the stack-style usage of recvBuf, by keeping RMI
// processing within its bounds, and setting recvBuf for additional messages
// should the processed requests invoke additional RMI requests.
class recvBufProtector {
public:
  char* const tmpPos;
  int size;
public:
  inline explicit recvBufProtector(MPI_Status* status) : tmpPos( recvPos ) {
    stapl_assert( status != NULL, "invalid status");
    MPI_Get_count( status, MPI_BYTE, &size );
    recvPos += size;
  }
  inline ~recvBufProtector() { recvPos = tmpPos; }
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
  rf->exec( registry, in, inout );
}




//****************************************************************************
// Parallelization Primitives
//****************************************************************************

int get_num_threads() {
  return numThreads;
}


int get_thread_id() {
  return id;
}




//****************************************************************************
// Registration Primitives
//****************************************************************************
#ifndef _ARMI_REGISTRY_CHECK_TYPE 

  rmiHandle register_rmi_object(void* const objectPointer) {
    return registry.registerObject( objectPointer );
  }
 
  void update_rmi_object(rmiHandle handle, void* const objectPointer) {
    registry.updateObject( handle, objectPointer );
  }
  
#else

  rmiHandle register_rmi_object(void* const objectPointer, 
				const std::type_info &objectType) {
    return registry.registerObject(objectPointer, objectType);
  }
  
  rmiHandle register_rmi_object(void *const objectPonter, int id) {
    return registry.registerObject( objectPointer, id);
  }

  void update_rmi_object(rmiHandle const &handle, void* const objectPointer, 
			 std::type_info const &objectType) {
    registry.updateObject(handle, objectPointer, objectType);
  }

#endif //_ARMI_REGISTRY_CHECK_TYPE 


  void unregister_rmi_object(rmiHandle const &handle) {
    rmiHandle j = handle;
    registry.unregisterObject( j );
  }


  void execute_parallel_task(parallel_task* const task) {
    stapl_assert( task != 0, "invalid parallel_task*" );
    registry.fastRegisterTask( task );
    const unsigned int tmpAgg = aggMax; aggMax = maxAggregation;
    const unsigned int tmpPoll = pollMax; pollMax = maxAggregation;
    rmi_fence(); // ensure all threads have registered
    task->execute();
    aggMax = tmpAgg;
    pollMax = tmpPoll;
  }




//****************************************************************************
// Communication Primitives
//****************************************************************************

// wait for an MPI_Request object while continuing to poll
MPI_Status request_wait(MPI_Request* req) {
  MPI_Status status;
  while( true ) {
    int flag;
    MPI_Test( req, &flag, &status );
    if( flag )
      break;
    rmi_poll();
  }
  return status;
}


// start sending the given sendBuf section, and prepare the next section for use
inline void request_send(const int destThread, requestType tag=ASYNC_ONLY, void** syncRelease=0) {
  stapl_assert( (destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");
  if( largeBuffer == 0 ) {
    perThreadSendBuffer& send = sendBuf[destThread];
    send.tag( tag, syncRelease );
    MPI_Isend( send.begin(), send.end()-send.begin(), MPI_BYTE, destThread, destThread, RMI_COMM, send.request() );
    send.reset();
  }
  else {
    reinterpret_cast<msgHeader*>( largeBuffer )->tag = tag;
    reinterpret_cast<msgHeader*>( largeBuffer )->source = id;
    reinterpret_cast<msgHeader*>( largeBuffer )->syncRelease = syncRelease;
    MPI_Request req = MPI_REQUEST_NULL;
    MPI_Isend( largeBuffer, largeBufferSize, MPI_BYTE, destThread, destThread, RMI_COMM, &req );
    request_wait( &req );
    free( largeBuffer );
    largeBuffer = 0;
  }
  ++localSR;
}


// // process an aggregation group of async_rmi requests
// inline void recv_async_rmi(MPI_Status* status) {
//   ++numRMI;
//   recvBufProtector p( status );
//   for( char* c=p.begin(); c<p.end(); c+=(reinterpret_cast<rmiRequest*>(c))->_size )
//     ( reinterpret_cast<rmiRequest*>(c) )->exec( registry, 0, 0 );
//   --localSR;
// }

// process an aggregation group of async_rmi requests
inline void recv_async_rmi(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
  ++numRMI;
  recvBufProtector p( status );

  const int srcThread = p.srcThread();
  bool OpaqueProcessed = false;
  MPI_Request* req; 
  char* tmpHeader = sendBuf[srcThread].findBuf( &req, false );
  char* tmpBuf = tmpHeader+ALIGNED_SIZEOF( msgHeader );
  int tmpOffset = 0;
  int rtnSize;

  for( char* c=p.begin(); c<p.end(); 
       c+=(reinterpret_cast<rmiRequest*>(c))->_size ) {
    rtnSize = _SEND_BUF_MAX-tmpOffset;
    (reinterpret_cast<rmiRequest*>(c) )->exec( registry, tmpBuf, &rtnSize);
    //If rtnSize = 0 -> It was async, do nothing
    //If rtnSize > 0 -> It was opaque, increment queue pos
    //If rtnSize < 0 -> It was opaque, not enough buffer room, send this, and re-exec  
    if ( rtnSize > 0 ) { 
      OpaqueProcessed = true;
      tmpOffset += rtnSize;
    } else if (rtnSize < 0) {
      msgHeader& h = *reinterpret_cast<msgHeader*>( tmpHeader );
      h.tag = OPAQUE_RTN;
      h.source = id;
      MPI_Isend( tmpHeader, tmpOffset+ALIGNED_SIZEOF(msgHeader), MPI_BYTE,
		srcThread, srcThread, RMI_COMM, req );
      ++localSR;
      tmpHeader = sendBuf[srcThread].findBuf( &req );
      tmpBuf = tmpHeader+ALIGNED_SIZEOF( msgHeader );
      tmpOffset = 0;
      OpaqueProcessed = false;
    }
  }    
 
  if (OpaqueProcessed) {
    msgHeader& h = *reinterpret_cast<msgHeader*>( tmpHeader );
    h.tag = OPAQUE_RTN;
    h.source = id;
    MPI_Isend( tmpHeader, tmpOffset+ALIGNED_SIZEOF(msgHeader), MPI_BYTE, 
 	       srcThread, srcThread, RMI_COMM, req );    
    ++localSR;    
  } else {
    *req = MPI_REQUEST_NULL;  //Release, so findBuf can recycle buffer immediately
  }
  sendBuf[srcThread].mark_sent(tmpHeader);
  --localSR;
}

inline void recv_abort(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
  recvBufProtector p( status );
  int source = p.srcThread();
  printf("%d: Error - ARMI Abort received from thread %d. (%s)\n", 
  	 stapl::get_thread_id(), source, p.begin());
  --localSR;
  exit(1);
}

// process an aggregation group of async_rmi requests, 
// followed by a single sync_rmi request
inline void recv_sync_rmi(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
  ++numRMI;
  recvBufProtector p( status );
  const int srcThread = p.srcThread();
  MPI_Request* req; 
  char* tmpHeader = sendBuf[srcThread].findBuf( &req,false );
  char* tmpBuf = tmpHeader+ALIGNED_SIZEOF( msgHeader );
  int rtnSize = 0;

  // only the sync_rmi request uses the tmpBuf & rtnSize arguments
  //printf("%d: Entered recv_sync_rmi, about to loop\n", stapl::get_thread_id());    fflush(stdout);
  //printf("%d: begin=%x, end=%x\n", stapl::get_thread_id(), p.begin(), p.end());  fflush(stdout);
  //printf("%d: recvPos = %x , tempPos=%x, size=%d\n", stapl::get_thread_id(), recvPos, p.tmpPos, p.size);  fflush(stdout);
  //printf("%d: recvBuf=%x,recvBuf[end]=%x\n", stapl::get_thread_id(), recvBuf, &(recvBuf[_RECV_BUF_MAX-1]));fflush(stdout);


  
  for( char* c=p.begin(); c<p.end(); c+=(reinterpret_cast<rmiRequest*>(c))->_size )
  {
//	  printf("%d: Entered inside loop in mpi_primitives, rmiRequest size = %d\n", stapl::get_thread_id(), (reinterpret_cast<rmiRequest*>(c))->_size); fflush(stdout);
      	  ( reinterpret_cast<rmiRequest*>(c) )->exec( registry, tmpBuf, &rtnSize );
  }
	  stapl_assert( rtnSize <= _SEND_BUF_MAX, "sync_rmi return exceeded buffers: increase _SEND_BUF_MAX");

  // tag and send the return value
  msgHeader& h = *reinterpret_cast<msgHeader*>( tmpHeader );
  h.tag = SYNC_RTN;
  h.source = id;
  h.syncRelease = p.syncRelease();
  MPI_Isend( tmpHeader, rtnSize+ALIGNED_SIZEOF(msgHeader), MPI_BYTE, srcThread, srcThread, RMI_COMM, req );
  sendBuf[srcThread].mark_sent(tmpHeader);
  // --localSR; // one message was received...
  // ++localSR; // ... but required sending another one back
}


// process a sync_rmi return value, by setting the appropriate release flag,
// and incrementing the recvPos stack to guard the value until it is copied
// out (e.g., a sync_rmi is waiting for its return, receives another RMI that
// happens to call its own sync_rmi)
inline void recv_sync_rtn(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
// nthomas 4/2/03 an inelegant, but effective fix for now.
//  *reinterpret_cast<msgHeader*>(recvPos)->syncRelease = recvPos;
//  int size;
//  MPI_Get_count( status, MPI_BYTE, &size );
//  recvPos += size;
//  *reinterpret_cast<int*>( recvPos ) = size; // save size so it can be subtracted from
//  recvPos += sizeof( int );                  // recvPos upon copying the return value out
//  --localSR; 
  int size; 
  MPI_Get_count( status, MPI_BYTE, &size );
  stapl_assert( size >= 0, "invalid size");
  *reinterpret_cast<msgHeader*>(recvPos)->syncRelease = malloc(size);
  stapl_assert( *reinterpret_cast<msgHeader*>(recvPos)->syncRelease != NULL, "malloc() failed"); 
  memcpy(*reinterpret_cast<msgHeader*>(recvPos)->syncRelease, recvPos, size);
  --localSR; 

}

inline void recv_opaque_rtn(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
  --localSR;
  int size; 
  MPI_Get_count( status, MPI_BYTE, &size );   
  char *tptr = recvPos + ALIGNED_SIZEOF(msgHeader);
  do {
    int retsize;
    retsize = *reinterpret_cast<int*>(tptr); 
    tptr += ALIGNED_SIZEOF(int);
    OpaqueBase *p = *reinterpret_cast<OpaqueBase**>(tptr);
    tptr += ALIGNED_SIZEOF(OpaqueBase**);
    p->setRtn(tptr);
    tptr += retsize - ALIGNED_SIZEOF(int) - ALIGNED_SIZEOF(OpaqueBase**);
  } while (tptr < (recvPos + size));
  stapl_assert( tptr == (recvPos + size), "IN recv_opaque_rtn, tptr != recvPos+size");
}


// process a fence arrival request from one of this parent's children
inline void recv_fence_arrival() {
  globalSR += reinterpret_cast<fenceMsg*>( recvPos )->sr;
  ++numArrived;
}


// process a fence release request from this child's parent
inline void recv_fence_release() {
  globalSR = reinterpret_cast<fenceMsg*>( recvPos )->sr;
  parentRelease = true;
}


// return true if request was an RMI, and false if it was a control message
bool recv_request(MPI_Status* status) {
  stapl_assert( status != NULL, "invalid status");
  bool rtn = false;
  switch( reinterpret_cast<msgHeader*>(recvPos)->tag ) {
    case ASYNC_ONLY:    recv_async_rmi( status ); rtn = true; break;
    case ASYNC_SYNC:    recv_sync_rmi( status ); rtn = true; break;
    case OPAQUE_RTN:    recv_opaque_rtn( status); break;
    case SYNC_RTN:      recv_sync_rtn( status ); break;
    case FENCE_ARRIVAL: recv_fence_arrival(); break;
    case FENCE_RELEASE: recv_fence_release(); break;
    case ABORT_REQ:     recv_abort( status ); break;
    default:            stapl_assert( 0, "invalid MPI tag" );
  }
  return rtn;
}


// find space for RMI request creation directly in sendBuf
void* rmiRequest::operator new(size_t size, const int destThread) {
  stapl_assert( size <= _RECV_BUF_MAX, "request too large for transfer: increase _RECV_BUF_MAX." );
  stapl_assert( (destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");

  perThreadSendBuffer& send = sendBuf[destThread];
  if( size > _SEND_BUF_MAX ) {
    if( !send.empty() )
      request_send( destThread );
    largeBufferSize = size + ALIGNED_SIZEOF( msgHeader );
    largeBuffer = static_cast<char*>( safe_malloc(largeBufferSize) );
    stapl_assert( largeBuffer != NULL, "safe_malloc() failed");
    return largeBuffer + ALIGNED_SIZEOF(msgHeader);
  }
  if( send.pos+size > send.buf+_SEND_BUF_MAX )
    request_send( destThread );

  void* tmpPos = send.pos;
  if( id != destThread )
    send.pos += size;
  return tmpPos;
}


void send_async_rmi(const int destThread) {
  stapl_assert( (destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");
  stapl_assert( (destThread != id), "send_async_rmi(): shouldn't get local requests");

  perThreadSendBuffer& send = sendBuf[destThread];
  if( ++send.aggCount >= aggMax || largeBuffer)
    request_send( destThread );
  periodic_poll();
}

  //for mpi_primitives, same is send_async_rmi
void send_opaque_rmi(const int destThread) {
  stapl_assert( (destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");
  stapl_assert( (destThread != id), "send_opaque_rmi(): shouldn't get local requests");
  send_async_rmi(destThread);
}


int __nestedRecv = 0; // used to restart recvBuf correctly during nested calls

void* send_sync_rmi(const int destThread) {
  stapl_assert( (destThread >= 0) && (destThread < get_num_threads()), "invalid destThread");
  stapl_assert( (destThread != id), "send_sync_rmi(): shouldn't get local requests");

  perThreadSendBuffer& send = sendBuf[destThread];
  // Wait for the return, while handling other incoming requests
  void* syncRelease = 0;
  request_send( destThread, ASYNC_SYNC, &syncRelease );
  if( recvRequest == MPI_REQUEST_NULL ) { // i.e., is this a sync_rmi within an RMI invocation?
    stapl_assert( recvPos+ALIGNED_SIZEOF(msgHeader)+_SEND_BUF_MAX <= recvBuf+_RECV_BUF_MAX, "recvBuf overflow caused by excessive nested RMI requests: increase _RECV_BUF_MAX" );
    MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
    ++__nestedRecv;
  }
  do {
    MPI_Status status; MPI_Wait( &recvRequest, &status );
    recv_request( &status );
    if( syncRelease == 0 )
      MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
  } while( syncRelease == 0 );

  // Return the stack to its original value, which was incremented in recv_sync_rtn
  // nthomas 4/2/03 an inelegant, but effective fix for now.
  //const int rtnSize = *reinterpret_cast<int*>( recvPos - sizeof(int) );
  //recvPos -= rtnSize + sizeof( int );
  return reinterpret_cast<char*>( syncRelease ) + ALIGNED_SIZEOF( msgHeader );
}

// Trying to put this step into send_sync_rmi causes a race condition between
// MPI_Irecv-ing the next message and copying the return value from the
// buffer.  Alternating between two recvBufs (similar to sendBuf) would
// eliminate this step, but also increase the overall latency...
void finish_sync_rmi() {
  if( !__nestedRecv )
    MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
  else
    --__nestedRecv;
}


// Get an MPI buffer for temporary use for arguments of collective functions
void* get_arg_buffer(const int size) {
  stapl_assert( size <= _SEND_BUF_MAX, "input too large, increase _SEND_BUF_MAX" );
  return sendBuf[id].findBuf();
}



void send_reduce_rmi(rmiRequestReduce* const f, const bool commutative, const int rootThread) {
  stapl_assert( (rootThread >= -1) && (rootThread < get_num_threads()), "invalid destThread");

  rf = f;
  if( rootThread == -1 )
    MPI_Allreduce( f->_in, f->_inout, f->_size, MPI_BYTE,
		   (commutative ? commute : no_commute), RMI_COMM );
  else
    MPI_Reduce( f->_in, f->_inout, f->_size, MPI_BYTE,
		(commutative ? commute : no_commute), rootThread, RMI_COMM );
}


void send_broadcast_rmi(rmiRequestBroadcast* rmi, void* in, const int size, 
			const int rootThread) {
  stapl_assert( size <= _SEND_BUF_MAX, "broadcast_rmi input too large, increase _SEND_BUF_MAX" );
  stapl_assert( (rootThread >= 0) && (rootThread < get_num_threads()), "invalid destThread");

  if( id != rootThread ) {
    char* tmpBuf = sendBuf[id].findBuf();
    MPI_Bcast( tmpBuf, size, MPI_BYTE, rootThread, RMI_COMM );
    rmi->exec( registry, tmpBuf );
  }
  else
    MPI_Bcast( in, size, MPI_BYTE, rootThread, RMI_COMM );
}


void send_abort(char *str) {
  int msgSize = ALIGNED_SIZEOF(msgHeader) + strlen(str) + 1;
  char *abortBuf = (char *) malloc(msgSize);
  stapl_assert( abortBuf != NULL, "malloc() failed");
  ((msgHeader *) abortBuf)->tag = ABORT_REQ; 
  ((msgHeader *) abortBuf)->source = id; 
  strcpy((char*)(abortBuf + ALIGNED_SIZEOF(msgHeader)), str);  
  for (int i=0; i<numThreads; i++) {
    if (i != id) {
      MPI_Send( abortBuf, msgSize, MPI_BYTE, i, i, RMI_COMM ); 
      ++localSR;
    }
  }
}

void abort_rmi(char *str) {
  printf("%d: Error - Initiating ARMI Abort. (%s)\n", 
	 stapl::get_thread_id(), str);
  send_abort(str);
  exit(1);
}
  
unsigned int set_aggregation(const unsigned int agg) {
  stapl_assert( agg>0, "set_aggregation: aggegration must be >0 ");
  aggMax = ( agg <= maxAggregation ) ? agg : maxAggregation;
  return aggMax;
}


unsigned int get_aggregation() {
  return aggMax;
}


int set_nesting(const int nest) {
  return -1; // ignores the suggestion, nesting remains unlimited
}


int get_nesting() {
  return -1;
}


void rmi_flush() {
  for( int i=0; i<numThreads; ++i )
    if( !sendBuf[i].empty() )
      request_send( i );
}




//****************************************************************************
// Synchronization Primitives
//****************************************************************************

void set_poll_rate(const unsigned int rate) {
  stapl_assert( rate>0, "set_poll_rate: poll rate must be >0 ");
  pollMax = rate;
}


unsigned int get_poll_rate() {
  return pollMax;
}


void rmi_poll() {
  pollCount = 0;
  int flag; MPI_Status status;
  if( recvRequest == MPI_REQUEST_NULL ) { // i.e., is this an rmi_poll within an RMI invocation
    stapl_assert( recvPos+ALIGNED_SIZEOF(msgHeader)+_SEND_BUF_MAX <= recvBuf+_RECV_BUF_MAX, "recvBuf overflow caused by excessive nested RMI requests: increase _RECV_BUF_MAX" );
    MPI_Iprobe( MPI_ANY_SOURCE, id, RMI_COMM, &flag, &status );
    while( flag ) {
      MPI_Recv( recvPos, space(), MPI_BYTE, status.MPI_SOURCE, id, RMI_COMM, &status );
      recv_request( &status );
      MPI_Iprobe( MPI_ANY_SOURCE, id, RMI_COMM, &flag, &status );
    }
  }
  else {
    MPI_Test( &recvRequest, &flag, &status );
    while( flag ) {
      recv_request( &status );
      MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
      MPI_Test( &recvRequest, &flag, &status );
    }
  }
}


// if release is true, wait for any message, otherwise just wait for RMI requests
void rmi_wait(bool release) {
  pollCount = 0; 

  // Wait for a single incoming RMI request
  bool nestedRecv = false;
  if( recvRequest == MPI_REQUEST_NULL ) { // i.e., is this an rmi_wait within an RMI invocation?
    stapl_assert( recvPos+ALIGNED_SIZEOF(msgHeader)+_SEND_BUF_MAX <= recvBuf+_RECV_BUF_MAX, "recvBuf overflow caused by excessive nested RMI requests: increase _RECV_BUF_MAX" );
    MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
    nestedRecv = true;
  }
  do {
    MPI_Status status; MPI_Wait( &recvRequest, &status );
    if( recv_request(&status) )
      release = true;
    if( !release || !nestedRecv )
      MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
  } while( !release );
}

void rmi_wait() {
  if( numWaits == numRMI )
    rmi_wait( false );
  ++numWaits;
}


void rmi_fence() {
  // This is a generic spanning tree based fence.  Children signal arrival to
  // their parents up to the root, then parents signal release down to the
  // leaves.  The spanning tree layout can be easily re-defined by setting
  // parentID, numChildren and childrenID as desired in 'int main(...)' above.
  // The root of the tree has parentID = id.
  int numCycles = 0;
  while( true ) {
    // send leftover rmi requests, then wait for children to arrive
    rmi_poll(); rmi_flush();
    while( numArrived != numChildren ) { rmi_wait( true ); rmi_flush(); }
    numArrived = 0;
    numWaits = numRMI = 0;

    // signal arrival, then wait for the signal to release
    globalSR += localSR;
    if( parentID != id ) {
      fenceMsg msg( FENCE_ARRIVAL, id, globalSR );
      MPI_Send( &msg, sizeof(fenceMsg), MPI_BYTE, parentID, parentID, RMI_COMM );
      parentRelease = false;
      while( parentRelease == false ) { rmi_wait( true ); rmi_flush(); }
    }

    // signal release to children
    for( int i=0; i<numChildren; ++i ) {
      fenceMsg msg( FENCE_RELEASE, id, globalSR );
      MPI_Send( &msg, sizeof(fenceMsg), MPI_BYTE, childID[i], childID[i], RMI_COMM );
    }

    // Iterate the fence until the global number of sends - receives = 0
    // This ensures that all outstanding RMI requests have completed.  In
    // the simple case, iterating is not necessary.  However, two special
    // cases make iterating necessary for correctness.  First, if an RMI
    // invokes a method that starts another RMI, which invokes a method that
    // starts another RMI, etc.  Second, MPI does not make any fairness
    // guarantee, meaning one threads' messages could be favored over
    // another.  Even in the simple case, the fence release message could
    // potentially overtake an actual RMI request.  To ensure the program
    // doesn't hang, abort after a certain number of iterations.
    if( globalSR == 0 ) break;
    globalSR = 0;
    stapl_assert( ++numCycles < 1000, "rmi_fence internal error: contact stapl development" );
  }
}

  void checkRegistryTrace(void) {
    //fixme - nthomas this probably won't work with sizeof(void*) != sizeof(int)
    stapl_assert(sizeof(void*) == sizeof(int), "sizeof(void*) != sizeof(int)");
    
    int *sizeBuf;
    if (id == 0) 
      sizeBuf = (int *) malloc(numThreads*sizeof(int));
    stapl_assert( sizeBuf != NULL, "malloc() failed");
    int traceSize = registry.getTrace().size();
    
    //Check Registration Traces
    MPI_Gather (&traceSize, 1, MPI_INT, sizeBuf, 1, MPI_INT, 0, RMI_COMM);
    
    //Check That All Traces Are Same Size
    if (id == 0) {
      bool sameSize = true;
      for (int i = 1; i < numThreads; i++)
	sameSize = sameSize && (sizeBuf[i-1] == sizeBuf[i]); 
      stapl_assert(sameSize,"Automatic Registration Failure - registry size not equal");
    }  
    
    //Check That Traces Are Equivalent
    void **traceBuf = (void**) malloc(numThreads*sizeof(void*)*traceSize);
    stapl_assert( traceBuf != NULL, "malloc() failed");
    void *trace = (void *) &(*registry.getTrace().begin());
    MPI_Gather(trace, traceSize, MPI_INT, traceBuf, traceSize, MPI_INT, 0, RMI_COMM);
    if (id == 0) {
      for (int i = 0; i < traceSize; i++) {
	bool sameValue = true;
	for (int j = 1; j < numThreads; j++) 
	  sameValue = sameValue && (traceBuf[i] == traceBuf[j*traceSize + i]);
	stapl_assert(sameValue,"Automatic Registration Failure - nonSPMD registration");  
      }
    }
  }
} // end namespace mpi_primitives






int main(int argc, char *argv[]) {
  using namespace mpi_primitives;
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id );
  MPI_Comm_size( MPI_COMM_WORLD, &numThreads );
  MPI_Comm_dup( MPI_COMM_WORLD, &RMI_COMM );

  // initialize variables for tree based communication
  parentRelease = false;
  localSR = 0;
  globalSR = 0;
  numArrived = 0;
#if _RMI_FENCE == HYPERCUBE_TREE
  const double _numThreads = numThreads;
  stapl_assert( MAX_KIDS > log10(_numThreads)/log10(2.0), "MAX_KIDS exceeded: please increase" );
  parentID = 0;
  int tmpParent = 0;
  int stage = 1;
  int N = static_cast<int>( pow(2.0, ceil(log10(_numThreads) / log10(2.0))) );
  while( tmpParent != id ) {
    if( id >= tmpParent + N / pow(2.0, stage) ) {
      parentID = tmpParent;
      tmpParent += static_cast<int>( N / pow(2.0, stage) );
    }
    ++stage;
  }
  numChildren = 0;
  while( N/pow(2.0, stage) >= 1.0 ) {
    childID[numChildren] = id + static_cast<int>( N / pow(2.0, stage) );
    ++stage;
    if( childID[numChildren] < _numThreads ) ++numChildren;
  }
#elif _RMI_FENCE == BINARY_TREE
  stapl_assert( MAX_KIDS >= 2, "MAX_KIDS exceeded: please increase" );
  parentID = (id-1) / 2;
  numChildren = 2;
  childID[0] = ( 2 * (id+1) ) - 1;
  childID[1] = ( 2 * (id+1) );
  if( childID[0] >= numThreads ) --numChildren;
  if( childID[1] >= numThreads ) --numChildren;
#elif _RMI_FENCE == FLAT_TREE
  stapl_assert( MAX_KIDS >= numThreads-1, "MAX_KIDS exceeded: please increase" );
  parentID = 0;
  if( id == 0 ) {
    numChildren = numThreads-1;
    for( int i=1; i<numThreads; ++i )
      childID[i-1] = i;
  }
  else
    numChildren = 0;
#endif

  // initialize the aggregation and polling variables
  stapl_assert( maxAggregation != 0, "maxAggregation equals 0: increase _SEND_BUF_MAX" );
  //aggMax = 1;
  set_aggregation(100);  
  pollCount = 0;
  pollMax = 1;

  // initialize the RMI request send/receive buffers
  typedef pointer_to_integral<char*>::integral my_int;
  sendBuf = new perThreadSendBuffer[numThreads];
  recvPos = recvBuf + _BYTE_ALIGNMENT - (reinterpret_cast<my_int>(recvBuf) % _BYTE_ALIGNMENT);
  recvRequest = MPI_REQUEST_NULL;

  // register the reduce_rmi functions
  MPI_Op_create( reduce_internal, true,  &commute );
  MPI_Op_create( reduce_internal, false, &no_commute );

  // all threads start executing the same user code in parallel
  MPI_Irecv( recvPos, space(), MPI_BYTE, MPI_ANY_SOURCE, id, RMI_COMM, &recvRequest );
  stapl_main( argc, argv );
  rmi_fence();
#ifndef _AIX  // IBM will hang if you cancel the last Irecv...
  MPI_Cancel( &recvRequest );
  MPI_Status status; MPI_Wait( &recvRequest, &status );
#endif

  //Verify SPMD order registration of statically allocated objects
#ifdef _ARMI_REGISTRY_VERIFY_TRACE
  checkRegistryTrace();  
#endif //_ARMI_REGISTRY_VERIFY_TRACE

  // cleanup
  MPI_Op_free( &commute );
  MPI_Op_free( &no_commute );
  delete[] sendBuf;
  MPI_Comm_free( &RMI_COMM );
  MPI_Finalize();
}
