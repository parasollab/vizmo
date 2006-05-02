#ifndef _COMM_PRIMITIVES_H
#define _COMM_PRIMITIVES_H


namespace comm_primitives {
  struct mpi_thread {
    int mpiRank;
    int thread;
  };
  extern mpi_thread* map;
  extern void *reduceTmp;
  bool isLocal(const int threadID);
}


namespace thread_primitives { using namespace stapl; using namespace comm_primitives; }
#include "thread_primitives.h"

namespace mpi_primitives { using namespace stapl; using namespace comm_primitives; }
#include "mpi_primitives.h"


namespace stapl {
using mpi_primitives::typer;

template<class Class, class Rtn>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)()) {
  if( comm_primitives::isLocal(destThread) )
    thread_primitives::async_rmi( destThread, objHandle, memberFuncPtr );
  else			   
    mpi_primitives::async_rmi( destThread, objHandle, memberFuncPtr );
}

template<class Class, class Rtn, class Arg1>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1),
	       const typename rmi_arg<Arg1>::type& a1) {
  if( comm_primitives::isLocal(destThread) )
    thread_primitives::async_rmi( destThread, objHandle, memberFuncPtr, a1 );
  else			   
    mpi_primitives::async_rmi( destThread, objHandle, memberFuncPtr, a1 );
}

template<class Class, class Rtn, class Arg1, class Arg2>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	       const typename rmi_arg<Arg1>::type& a1,
	       const typename rmi_arg<Arg2>::type& a2) {
  if( comm_primitives::isLocal(destThread) )
    thread_primitives::async_rmi( destThread, objHandle, memberFuncPtr, a1, a2 );
  else			   
    mpi_primitives::async_rmi( destThread, objHandle, memberFuncPtr, a1, a2 );
}


template<class Class, class Rtn>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)()) {
  if( comm_primitives::isLocal(destThread) )
    return thread_primitives::sync_rmi( destThread, objHandle, memberFuncPtr );
  else			   
    return mpi_primitives::sync_rmi( destThread, objHandle, memberFuncPtr );
}

template<class Class, class Rtn, class Arg1>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1) {
  if( comm_primitives::isLocal(destThread) )
    return thread_primitives::sync_rmi( destThread, objHandle, memberFuncPtr, a1 );
  else			   
    return mpi_primitives::sync_rmi( destThread, objHandle, memberFuncPtr, a1 );
}

template<class Class, class Rtn, class Arg1, class Arg2>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2) {
  if( comm_primitives::isLocal(destThread) )
    return thread_primitives::sync_rmi( destThread, objHandle, memberFuncPtr, a1, a2 );
  else			   
    return mpi_primitives::sync_rmi( destThread, objHandle, memberFuncPtr, a1, a2 );
}

}

namespace thread_primitives {
  void fence();
}

namespace stapl {

template<class Class, class T>
void reduce_rmi(T* in, T* out, const rmiHandle objHandle,
		void (Class::* const memberFuncPtr)(T*, T*), 
		const bool commutative, const int rootThread=-1) {
  using comm_primitives::map;
  using comm_primitives::reduceTmp;
  const int id = stapl::get_thread_id();

  thread_primitives::reduce_rmi( in, out, objHandle, memberFuncPtr, commutative, 0);

  if (map[id].thread == 0) {
    reduceTmp = reinterpret_cast<T*>(safe_malloc(sizeof(T)));
    mpi_primitives::reduce_rmi (out, (T*) reduceTmp, objHandle, memberFuncPtr,
				commutative, rootThread);
  }
    
  thread_primitives::fence();

   if ((rootThread == -1) || (id == rootThread))
     *out = *reinterpret_cast<T*>( reduceTmp );
   
   thread_primitives::fence();
   if (map[id].thread == 0)
     free(reduceTmp);
}




template<class Class, class Arg>
void broadcast_rmi(Arg* inout, const int count, const rmiHandle objHandle,
		   void (Class::* const memberFuncPtr)(Arg*, Arg*, const int),
		   const int rootThread) {
  mpi_primitives::broadcast_rmi( inout, count, objHandle, memberFuncPtr, rootThread );
}


} // end namespace
#endif // _COMM_PRIMITIVES_H
