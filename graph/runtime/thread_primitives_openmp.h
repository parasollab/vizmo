#include <omp.h>


namespace thread_primitives {

threadInfo* threadInfo_local; // declared threadprivate below...

inline void declare_threadInfo() {}

inline void destroy_threadInfo() {}

inline void set_threadInfo(threadInfo* t) { threadInfo_local = t; }

inline threadInfo* get_threadInfo() { return threadInfo_local; }




void execute_threads(const int n) {
  omp_set_num_threads( n );
  omp_set_dynamic( 0 );
  omp_set_nested( 0 ); // No OpenMP vendors implement nested parallelism yet!
                       // 0 prevents warning messages from being displayed...
#pragma omp parallel
  {
    thread_main( omp_get_thread_num(), n );
  }
}




class simpleLock {
  omp_lock_t _lock;
public:
  simpleLock()                  { omp_init_lock( &_lock ); }
  simpleLock(const simpleLock&) { omp_init_lock( &_lock ); }
  ~simpleLock()                 { omp_destroy_lock( &_lock ); }
  void lock()                   { omp_set_lock( &_lock ); }
  void unlock()                 { omp_unset_lock( &_lock ); }
};


} // end namespace
#pragma omp threadprivate( thread_primitives::threadInfo_local )
