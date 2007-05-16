#include <pthread.h>

// Assume the following are defined for using various compilers:
// __HP_aCC - Hewlett Packard aCC B3910B A.03.27
// __sgi    - SGI MIPSpro C++ 7.3.1.3m


// Traps any Pthreads errors that may occur.
#define ERROR_HANDLER(func) { int v = func; stapl_assert( v==0, strerror(v) ); }


namespace thread_primitives {

// If possible use private per-thread globals, which are usually much faster
// than using pthread_getspecific, to access threadInfo.  This is the
// equivalent of declaring something threadprivate in OpenMP.
#if defined( __HP_aCC )
  __thread threadInfo* threadInfo_local;
#else
  pthread_key_t threadInfo_key;
#endif

inline void declare_threadInfo() {
#if defined( __HP_aCC )
  ;
#else
  ERROR_HANDLER( pthread_key_create(&threadInfo_key, 0) );
#endif
}

inline void destroy_threadInfo() {
#if defined( __HP_aCC )
  ;
#else
  ERROR_HANDLER( pthread_key_delete(threadInfo_key) );
#endif
}

inline void set_threadInfo(threadInfo* t) {
#if defined( __HP_aCC )
  threadInfo_local = t;
#else
  ERROR_HANDLER( pthread_setspecific(threadInfo_key, t) );
#endif
}

inline threadInfo* get_threadInfo() {
#if defined( __HP_aCC )
  return threadInfo_local;
#else
  return static_cast<threadInfo*>( pthread_getspecific(threadInfo_key) );
#endif
}




int numThreads;

void pthread_main(int id) {
  thread_main( id, numThreads );
}

void execute_threads(const int n) {
  numThreads = n;

  // Create the rest of the threads.  Threads are SYSTEM_SCOPE so they each
  // execute on top of a kernel object, and hence can run on parallel processors.
  pthread_attr_t threadAttrs;
  ERROR_HANDLER( pthread_attr_init(&threadAttrs) );
  ERROR_HANDLER( pthread_attr_setdetachstate(&threadAttrs, PTHREAD_CREATE_JOINABLE) );
#if defined( __sgi )
  // SYSTEM_SCOPE requires special permissions on SGI, but this scope does
  // not, and it still runs each thread on a kernel object.
  ERROR_HANDLER( pthread_attr_setscope(&threadAttrs, PTHREAD_SCOPE_BOUND_NP) );
#else
  ERROR_HANDLER( pthread_attr_setscope(&threadAttrs, PTHREAD_SCOPE_SYSTEM) );
#endif
  pthread_t* pthreadID = static_cast<pthread_t*>( safe_malloc(sizeof(pthread_t)*n) );
  for( int threadID=1; threadID<n; ++threadID )
    ERROR_HANDLER( pthread_create(&pthreadID[threadID], &threadAttrs,
				  (void*(*)(void*)) pthread_main, (void*) threadID) );
  pthread_main( 0 );
  for( int threadID=1; threadID<n; ++threadID )
    pthread_join( pthreadID[threadID], 0 );
  free( pthreadID );
}




class simpleLock {
  pthread_mutex_t _lock;
public:
  simpleLock()                  { ERROR_HANDLER( pthread_mutex_init(&_lock, 0) ); }
  simpleLock(const simpleLock&) { ERROR_HANDLER( pthread_mutex_init(&_lock, 0) ); }
  ~simpleLock()                 { ERROR_HANDLER( pthread_mutex_destroy(&_lock) ); }
  void lock()                   { ERROR_HANDLER( pthread_mutex_lock(&_lock) ); }
  void unlock()                 { ERROR_HANDLER( pthread_mutex_unlock(&_lock) ); }
};


} // end namespace
