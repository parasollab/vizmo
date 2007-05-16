#ifndef _MPI_PRIMITIVES_H
#define _MPI_PRIMITIVES_H

#include <new>
#include <string.h>

namespace mpi_primitives {
   extern rmiRegistry registry;
   extern int id;
   extern unsigned int pollCount;  // the current number of calls executed
   extern unsigned int pollMax;    // 1 in pollMax calls performs a poll
   void rmi_poll();

   // internal communication calls are instrumented to periodically poll for
   // incoming RMI requests
   inline void periodic_poll() {
      if( ++pollCount >= pollMax )
         rmi_poll();
   }
} //end namespace mpi_primitives


namespace stapl {
int get_thread_id();
#ifndef P_TASK
#define P_TASK
 struct parallel_task {
  virtual ~parallel_task() {};
  enum rmiHandleConstant { rmiHandle = 0 };
  virtual void execute() = 0;
 };
#endif
 void execute_parallel_task(parallel_task* const task);
}

namespace mpi_primitives {

// arg_storage provides specialized storage for fundamental/pointer/reference
// and const/non-const types, alleviating the need to duplicate code in each
// rmiRequest.  Regardless of type, entire objects are stored inside of
// arg_storage (i.e., a T* is stored as a T).  In addition, if a type needs
// packing, packed_size extra, contiguous space is necessary at the end of the
// object/buffer using arg_storage.
//
// arg_storage uses a properly aligned char array to store objects, preventing
// them from being automatically constructed and possibly allocating memory.
// The dynamically allocated sections of objects will instead be copied to the
// extra space during packing in the constructor.  An explicit call to unpack
// or unpack_copy is necessary to restore the packed object.  Since the
// aggregate arg_storage object exists entirely within a single object/buffer,
// no call to destructors is necessary.
template<class T> struct const_traits          { typedef T base; };
template<class T> struct const_traits<const T> { typedef T base; };
template<class Rtn> class OpaqueHandle;
class OpaqueBase;



template<class X> union arg_storage {
  typedef typename const_traits<X>::base T;
  double align;
  char c[sizeof(T)];
  static int packed_size(const T& t) { return typer_traits<T>::packed_size( &t ); }
  inline arg_storage(const T& t, void* rmiRequestBase, int* size) {
    memcpy( c, &t, sizeof(T) );
    typer_traits<T>::pack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase), size );
  }
  inline T unpack(void* rmiRequestBase) {
    typer_traits<T>::unpack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase) );
    return *reinterpret_cast<T*>( c );
  }
  inline T unpack_copy(void* rmiRequestBase) { return unpack( rmiRequestBase ); }
};

template<class X> union arg_storage<X&> {
  typedef typename const_traits<X>::base T;
  double align;
  char c[sizeof(T)];
  static int packed_size(const T& t) { return typer_traits<T>::packed_size( &t ); }
  inline arg_storage(const T& t, void* rmiRequestBase, int* size) {
    memcpy( c, &t, sizeof(T) );
    typer_traits<T>::pack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase), size );
  }
  inline T& unpack(void* rmiRequestBase) {
    typer_traits<T>::unpack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase) );
    return *reinterpret_cast<T*>( c );
  }
  inline T& unpack_copy(void* rmiRequestBase) { return *new T( unpack(rmiRequestBase) ); }
};

template<class X> union arg_storage<X*> {
  typedef typename const_traits<X>::base T;
  double align;
  char c[sizeof(T)];
  static int packed_size(const T* const& t) { return typer_traits<T>::packed_size( t ); }
  inline arg_storage(const T* t, void* rmiRequestBase, int* size) {
    memcpy( c, t, sizeof(T) );
    typer_traits<T>::pack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase), size );
  }
  inline T* unpack(void* rmiRequestBase) {
    typer_traits<T>::unpack( reinterpret_cast<T*>(c), 1, reinterpret_cast<char*>(rmiRequestBase) );
    return reinterpret_cast<T*>( c );
  }
  inline T* unpack_copy(void* rmiRequestBase) { return new T( *unpack(rmiRequestBase) ); }
};


// rmiRequest encapsulates an RMI request for subsequent execution via the
// exec method.  rmiRequests package all information for straightforward
// buffering (i.e., aggregation) and transfer.  The 'header' contains the size
// of the request, the rmi object handle, and the desired member function.
// The 'body' contains the required arguments (see also the arg_storage docs).
class rmiRequest {
public:
  int _size;
  void* operator new(size_t size, const int destThread);
  //void operator delete(void*, const int destThread) {}
  void operator delete(void*) {}
  explicit rmiRequest(const int s) : _size( s ) {};
  virtual ~rmiRequest() {};
  virtual void exec(const rmiRegistry& registry, void* returnBuf, int* size) = 0;
};


// rmiRequestReduce encapsulates a reduction RMI request for use with
// MPI_Reduce, which expects a function.
class rmiRequestReduce {
public:
  int       _size;
  void*     _in;
  void*     _inout;
  rmiRequestReduce(const int size, void* const in, void* const inout)
    : _size(size), _in(in), _inout(inout) {}
  virtual void exec(const rmiRegistry& registry, void* in, void* inout) = 0;
};


// rmiRequestBroadcast encapsulates a broadcast RMI request for use with
// MPI_Bcast, which expects a function.
struct rmiRequestBroadcast {
  virtual void exec(const rmiRegistry& registry, void* in) = 0;
};

void send_abort(char *str);
void send_async_rmi(const int destThread);
void send_opaque_rmi(const int destThread);
void* send_sync_rmi(const int destThread);
void* get_arg_buffer(const int size);
void send_reduce_rmi(rmiRequestReduce* const f, const bool commutative, const int rootThread);
void send_broadcast_rmi(rmiRequestBroadcast* r, void* in, const int size, const int rootThread);
#ifndef _STAPL_COMM
void finish_sync_rmi();
#endif
void rmi_fence();
void rmi_flush();
void periodic_poll();

template<class Class, class Rtn>
class rmiRequest0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle     _h;
  const MemberFuncPtr _m;

public:
  void* operator new(size_t, const int destThread) {
    size_t size = ALIGNED_SIZEOF( rmiRequest0 );
    return rmiRequest::operator new( size, destThread );
  }

  rmiRequest0(const int destThread, const rmiHandle h, const MemberFuncPtr& m) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest0) ), _h( h ), _m(m) {
    send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry, void*, int* r) {
    if (r) *r=0;
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )();
  }
};


template<class Class, class Rtn, class Arg1>
class rmiRequest1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1) {
    size_t size = ALIGNED_SIZEOF( rmiRequest1 );
    size += arg_storage<Arg1>::packed_size( a1 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1) {}
  void operator delete(void*) {}

  rmiRequest1(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
	      const typename rmi_arg<Arg1>::type& a1) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest1) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ) {
    send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry, void*, int* r) {
    if (r) *r = 0;
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( _a1.unpack(this) );
  }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequest2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;
  arg_storage<Arg2>   _a2;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1,
		     const typename rmi_arg<Arg2>::type& a2) {
    size_t size = ALIGNED_SIZEOF( rmiRequest2 );
    size += arg_storage<Arg1>::packed_size( a1 );
    size += arg_storage<Arg2>::packed_size( a2 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1,
		       const typename rmi_arg<Arg2>::type& a2) {}
  void operator delete(void*) {}

  rmiRequest2(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
	      const typename rmi_arg<Arg1>::type& a1,
	      const typename rmi_arg<Arg2>::type& a2) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest2) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ), _a2( a2, this, &_size ) {
    send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry, void*, int* r) {
    if (r) *r = 0;
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( _a1.unpack(this), _a2.unpack(this) );
  }
};


template<class Class, class Rtn>
class rmiRequestReturn0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle     _h;
  const MemberFuncPtr _m;

public:
  void* operator new(size_t, const int destThread) {
    size_t size = ALIGNED_SIZEOF( rmiRequestReturn0 );
    return rmiRequest::operator new( size, destThread );
  }

  rmiRequestReturn0(const int destThread, const rmiHandle h, const MemberFuncPtr& m) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn0) ), _h( h ), _m( m ) {}

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    *size = ALIGNED_SIZEOF( arg_storage<Rtn> );
    new( returnBuf ) arg_storage<Rtn>( (c->*_m)(), returnBuf, size );
  }

  Rtn result(const int destThread) {
    arg_storage<Rtn>* r = reinterpret_cast<arg_storage<Rtn>*>( send_sync_rmi(destThread) );
    Rtn tmp = r->unpack_copy( r );

#ifndef _STAPL_COMM
    finish_sync_rmi();
#endif
    return tmp;
  }
};

template<class Class, class Rtn, class Arg1>
class rmiRequestReturn1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1) {
    size_t size = ALIGNED_SIZEOF( rmiRequestReturn1 );
    size += arg_storage<Arg1>::packed_size( a1 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1) {}
  void operator delete(void*) {}

  rmiRequestReturn1(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn1) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ) {}

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    *size = ALIGNED_SIZEOF( arg_storage<Rtn> );
    new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this)), returnBuf, size );
  }

  Rtn result(const int destThread) {
    arg_storage<Rtn>* r = reinterpret_cast<arg_storage<Rtn>*>( send_sync_rmi(destThread) );
    Rtn tmp = r->unpack_copy( r );
#ifndef _STAPL_COMM
    finish_sync_rmi();
#endif
    return tmp;
  }
};


template<class Class, class Rtn, class Arg1>
class rmiRequestReturn1const : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1) const;
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1) {
    size_t size = ALIGNED_SIZEOF( rmiRequestReturn1const );
    size += arg_storage<Arg1>::packed_size( a1 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1) {}
  void operator delete(void*) {}

  rmiRequestReturn1const(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn1const) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ) {}

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    *size = ALIGNED_SIZEOF( arg_storage<Rtn> );
    new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this)), returnBuf, size );
  }

  Rtn result(const int destThread) {
    arg_storage<Rtn>* r = reinterpret_cast<arg_storage<Rtn>*>( send_sync_rmi(destThread) );
    Rtn tmp = r->unpack_copy( r );
#ifndef _STAPL_COMM
    finish_sync_rmi();
#endif
    return tmp;
  }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequestReturn2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;
  arg_storage<Arg2>   _a2;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1,
		     const typename rmi_arg<Arg2>::type& a2) {
    size_t size = ALIGNED_SIZEOF( rmiRequestReturn2 );
    size += arg_storage<Arg1>::packed_size( a1 );
    size += arg_storage<Arg2>::packed_size( a2 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1,
		       const typename rmi_arg<Arg2>::type& a2) {}
  void operator delete(void*) {}

  rmiRequestReturn2(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1,
		    const typename rmi_arg<Arg2>::type& a2) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn2) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ), _a2( a2, this, &_size ) {}

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    *size = ALIGNED_SIZEOF( arg_storage<Rtn> );
    new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this), _a2.unpack(this)), returnBuf, size );
  }

  Rtn result(const int destThread) {
    arg_storage<Rtn>* r = reinterpret_cast<arg_storage<Rtn>*>( send_sync_rmi(destThread) );
    Rtn tmp = r->unpack_copy( r );
#ifndef _STAPL_COMM
    finish_sync_rmi();
#endif
    return tmp;
  }
};



template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequestReturn2const : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2) const;
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  arg_storage<Arg1>   _a1;
  arg_storage<Arg2>   _a2;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1,
		     const typename rmi_arg<Arg2>::type& a2) {
    size_t size = ALIGNED_SIZEOF( rmiRequestReturn2const );
    size += arg_storage<Arg1>::packed_size( a1 );
    size += arg_storage<Arg2>::packed_size( a2 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1,
		       const typename rmi_arg<Arg2>::type& a2) {}
  void operator delete(void*) {}

  rmiRequestReturn2const(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1,
		    const typename rmi_arg<Arg2>::type& a2) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn2const) ), _h( h ), _m( m ),
    _a1( a1, this, &_size ), _a2( a2, this, &_size ) {}

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    *size = ALIGNED_SIZEOF( arg_storage<Rtn> );
    new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this), _a2.unpack(this)), returnBuf, size );
  }

  Rtn result(const int destThread) {
    arg_storage<Rtn>* r = reinterpret_cast<arg_storage<Rtn>*>( send_sync_rmi(destThread) );
    Rtn tmp = r->unpack_copy( r );
#ifndef _STAPL_COMM
    finish_sync_rmi();
#endif
    return tmp;
  }
};



template<class Class, class Rtn>
class rmiRequestOpaqueReturn0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  OpaqueHandle<Rtn> *_oh;


public:
  void* operator new(size_t, const int destThread) {
    size_t size = ALIGNED_SIZEOF( rmiRequestOpaqueReturn0 );
    return rmiRequest::operator new( size, destThread );
  }

  rmiRequestOpaqueReturn0(const int destThread, const rmiHandle h, 
			  const MemberFuncPtr& m, OpaqueHandle<Rtn> *q) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn0) ),_h( h ),_m( m ),_oh(q) {
    send_opaque_rmi(destThread);
  }

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );

    int needed_size = ALIGNED_SIZEOF(arg_storage<Rtn>) + 
      ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);

    if (needed_size > *size) {
      *size = -1;   //Signal Caller We Need More Buffer Space;
      return;

    } else { 
      *reinterpret_cast<int*> (returnBuf) = needed_size; 
      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(int);
     

      *reinterpret_cast<OpaqueBase**> (returnBuf) =  _oh;
      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(OpaqueBase*);

      new( returnBuf ) arg_storage<Rtn>( (c->*_m)(), returnBuf, size );

      *size = ALIGNED_SIZEOF( arg_storage<Rtn> ) + 
	ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);
    }
  }
};

template<class Class, class Rtn, class Arg1>
class rmiRequestOpaqueReturn1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  OpaqueHandle<Rtn> *_oh;
  arg_storage<Arg1>   _a1;


public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1) {
    size_t size = ALIGNED_SIZEOF( rmiRequestOpaqueReturn1 );
    size += arg_storage<Arg1>::packed_size( a1 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1) {}
  void operator delete(void*) {}

  rmiRequestOpaqueReturn1(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1,  OpaqueHandle<Rtn> *q) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn1) ), _h( h ), _m( m ), _oh(q),
    _a1( a1, this, &_size ) {
    send_opaque_rmi(destThread);    
  }

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );

    int needed_size = ALIGNED_SIZEOF(arg_storage<Rtn>) + 
      ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);

    if (needed_size > *size) {
      *size = -1;   //Signal Caller We Need More Buffer Space;
      return;

    } else { 
      *reinterpret_cast<int*> (returnBuf) = needed_size; 

      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(int);
      
      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(OpaqueBase*);
  
      new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this)), returnBuf, size );
      
      *size = ALIGNED_SIZEOF( arg_storage<Rtn> ) + 
	ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);
    }
  }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequestOpaqueReturn2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle     _h;
  const MemberFuncPtr _m;
  OpaqueHandle<Rtn> *_oh;
  arg_storage<Arg1>   _a1;
  arg_storage<Arg2>   _a2;

public:
  void* operator new(size_t, const int destThread, 
		     const typename rmi_arg<Arg1>::type& a1,
		     const typename rmi_arg<Arg2>::type& a2) {
    size_t size = ALIGNED_SIZEOF( rmiRequestOpaqueReturn2 );
    size += arg_storage<Arg1>::packed_size( a1 );
    size += arg_storage<Arg2>::packed_size( a2 );
    return rmiRequest::operator new( size, destThread );
  }
  void operator delete(void*, const int destThread,
		       const typename rmi_arg<Arg1>::type& a1,
		       const typename rmi_arg<Arg2>::type& a2) {}
  void operator delete(void*) {}

  rmiRequestOpaqueReturn2(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
		    const typename rmi_arg<Arg1>::type& a1,
		    const typename rmi_arg<Arg2>::type& a2,
			   OpaqueHandle<Rtn> *q) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn2) ), _h( h ), _m( m ), _oh(q),
    _a1( a1, this, &_size ), _a2( a2, this, &_size ) {
    send_opaque_rmi(destThread);
  }

  void exec(const rmiRegistry& registry, void* returnBuf, int* size) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );

    int needed_size = ALIGNED_SIZEOF(arg_storage<Rtn>) + 
      ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);

    if (needed_size > *size) {
      *size = -1;   //Signal Caller We Need More Buffer Space;
      return;

    } else { 
      *reinterpret_cast<int*> (returnBuf) = needed_size; 
      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(int);
      
      *reinterpret_cast<OpaqueBase**> (returnBuf) =  _oh;
      //fixme this hurts performance but aCC on HPUX doesn't like the obvious
      //i.e. reinterpret_cast<x*> (returnBuf) += ALIGNED_SIZEOF(y);
      *(reinterpret_cast<char**> (&returnBuf)) += ALIGNED_SIZEOF(OpaqueBase*);

      new( returnBuf ) arg_storage<Rtn>( (c->*_m)(_a1.unpack(this), _a2.unpack(this)), returnBuf, size );
 
      *size = ALIGNED_SIZEOF( arg_storage<Rtn> ) + 
	ALIGNED_SIZEOF(int) + ALIGNED_SIZEOF(OpaqueBase*);      
    }
  }
};


template<class Class, class Arg>
class rmiRequestReduce2 : public rmiRequestReduce {
protected:
  typedef void (Class::* MemberFuncPtr)(Arg*, Arg*);
  const rmiHandle     _h;
  const MemberFuncPtr _m;

public:
  rmiRequestReduce2(Arg* in, Arg* inout, const rmiHandle h, const MemberFuncPtr& m,
		    const bool commutative, const int rootThread) :
    rmiRequestReduce( sizeof(Arg), in, inout ), _h( h ), _m( m ) {
    // pack, see broadcast for an example
    send_reduce_rmi( this, commutative, rootThread );
  }

  void exec(const rmiRegistry& registry, void* in, void* inout) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    // unpack in & inout
    ( c->*_m )( reinterpret_cast<Arg*>(in), reinterpret_cast<Arg*>(inout) );
    // re-pack inout
  }

};

template<class Class, class Arg>
class rmiRequestBroadcast3 : public rmiRequestBroadcast {
private:
  typedef void (Class::* MemberFuncPtr)(Arg*, Arg*, const int);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  Arg* _local;
  const int _count;

public:
  rmiRequestBroadcast3(const rmiHandle h, const MemberFuncPtr& m, 
		       Arg* const inout, const int count, const int rootThread)
    : _h( h ), _m( m ), _local( inout ), _count( count ) {
    Arg* tmp = inout;
    int size = sizeof(Arg) * count;
    if( typer_traits<Arg>::type(inout) == PACKED ) {
      if( get_thread_id() == rootThread) {
	tmp = reinterpret_cast<Arg*>( get_arg_buffer(size) );
	memcpy( tmp, inout, size );
	typer_traits<Arg>::pack( tmp, count, reinterpret_cast<char*>(tmp), &size );
      }
      else
	size += typer_traits<Arg>::packed_size( inout, count );
    }
    send_broadcast_rmi( this, tmp, size, rootThread );
  }

  void exec(const rmiRegistry& registry, void* in) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    if( typer_traits<Arg>::type(static_cast<Arg*>(in)) == PACKED )
      typer_traits<Arg>::unpack( static_cast<Arg*>(in), _count, static_cast<char*>(in) );
    ( c->*_m )( _local, static_cast<Arg*>(in), _count );
  }
};




template<class Class, class Rtn>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)()) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     ( c->*memberFuncPtr )();
     return;
  } 

  typedef rmiRequest0<Class, Rtn> rmi;
  new( destThread ) rmi( destThread, objHandle, memberFuncPtr );
}

template<class Class, class Rtn, class Arg1>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1),
	       const typename rmi_arg<Arg1>::type& a1) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     ( c->*memberFuncPtr )((Arg1) a1);
     return;
  } 

  typedef rmiRequest1<Class, Rtn, Arg1> rmi;
  new( destThread, a1 ) rmi( destThread, objHandle, memberFuncPtr, a1 );
}

template<class Class, class Rtn, class Arg1, class Arg2>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	       const typename rmi_arg<Arg1>::type& a1,
	       const typename rmi_arg<Arg2>::type& a2) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     ( c->*memberFuncPtr )((Arg1) a1, (Arg2) a2);
     return;
  } 

  typedef rmiRequest2<Class, Rtn, Arg1, Arg2> rmi;
  new( destThread, a1, a2 ) rmi( destThread, objHandle, memberFuncPtr, a1, a2 );
}



template<class Class, class Rtn>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)()) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     return ( c->*memberFuncPtr )();
  }

  typedef rmiRequestReturn0<Class, Rtn> rmi;
  return ( new(destThread) rmi(destThread, objHandle, memberFuncPtr) )
    ->result( destThread );
}

template<class Class, class Rtn, class Arg1>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     return ( c->*memberFuncPtr )((Arg1) a1);
  }

  typedef rmiRequestReturn1<Class, Rtn, Arg1> rmi;
  return ( new(destThread, a1) rmi(destThread, objHandle, memberFuncPtr, a1) )
    ->result( destThread );
}

template<class Class, class Rtn, class Arg1, class Arg2>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2) ,
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     return ( c->*memberFuncPtr )((Arg1) a1, (Arg2) a2);
  }

  typedef rmiRequestReturn2<Class, Rtn, Arg1, Arg2> rmi;
  return ( new(destThread, a1, a2) rmi(destThread, objHandle, memberFuncPtr, a1, a2) )
    ->result( destThread );
}

template<class Class, class Rtn, class Arg1>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1) const,
	     const typename rmi_arg<Arg1>::type& a1) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     return ( c->*memberFuncPtr )((Arg1) a1);
  }

  typedef rmiRequestReturn1const<Class, Rtn, Arg1> rmi;
  return ( new(destThread, a1) rmi(destThread, objHandle, memberFuncPtr, a1) )
    ->result( destThread );
}


template<class Class, class Rtn, class Arg1, class Arg2>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2) const,
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     return ( c->*memberFuncPtr )((Arg1) a1, (Arg2) a2);
  }

  typedef rmiRequestReturn2const<Class, Rtn, Arg1, Arg2> rmi;
  return ( new(destThread, a1, a2) rmi(destThread, objHandle, memberFuncPtr, a1, a2) )
    ->result( destThread );
}



template<class Class, class Rtn>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(),
	     OpaqueHandle<Rtn> *handle) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     handle->_value = ( c->*memberFuncPtr )();
     handle->_ready = true;
     return;
  }  

  typedef rmiRequestOpaqueReturn0<Class, Rtn> rmi;
  new(destThread) rmi(destThread, objHandle, memberFuncPtr, handle);
}


template<class Class, class Rtn, class Arg1>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1,
	     OpaqueHandle<Rtn> *handle) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     handle->_value = ( c->*memberFuncPtr )((Arg1) a1);
     handle->_ready = true;
     return;
  }  

  typedef rmiRequestOpaqueReturn1<Class, Rtn, Arg1> rmi;
  new(destThread, a1) rmi(destThread, objHandle, memberFuncPtr, a1, handle);
}


template<class Class, class Rtn, class Arg1, class Arg2>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2,
	     OpaqueHandle<Rtn> *handle) {

  //The request is local
  if (id == destThread) {
     periodic_poll();
     Class* const c = reinterpret_cast<Class*>( registry[objHandle] );
     handle->_value = ( c->*memberFuncPtr )((Arg1) a1, (Arg2) a2);
     handle->_ready = true;
     return;
  }  

  typedef rmiRequestReturn2<Class, Rtn, Arg1, Arg2> rmi;
  new(destThread, a1, a2) rmi(destThread, objHandle, memberFuncPtr, a1, a2, handle);
}


template<typename Class, typename Arg> 
class do_reduce : public stapl::parallel_task {
  Class* c; 
  void (Class::* const func)(Arg*, Arg*);
  Arg *in, *out;
  int root;
 public:
  do_reduce(Class *cl, void (Class::*const f)(Arg*, Arg*),
	    Arg* i, Arg* o, int k)
    : c(cl), func(f), in(i), out(o), root(k) { }
  
  void add_mine(const Arg& remote_in) {
    Arg& remote_ref = const_cast<Arg&>(remote_in);
    (c->*func)(&remote_ref, out);
  }

  void execute() { 
    int threadID = stapl::get_thread_id();
    if (threadID == root) {
      add_mine(*in);
    } else {
      stapl::async_rmi(root, rmiHandle, &do_reduce::add_mine, *in);
      rmi_flush();
    }
    rmi_fence();
  }
};
 
template<class Class, class Arg>
void reduce_rmi(Arg* in, Arg* out, const rmiHandle objHandle,
		void (Class::* const memberFuncPtr)(Arg*, Arg*), 
		const bool commutative, const int rootThread=-1) {
  if( typer_traits<Arg>::type(in) == PACKED) {
    stapl_assert(rootThread != -1, 
		 "all_reduce not supported for dynamic data types");
    stapl_assert(commutative, 
		 "reduce operation must be commutative if for dynamic types"); 
     Class * const c = reinterpret_cast<Class*>( registry[objHandle] );
     do_reduce<Class, Arg> red(c, memberFuncPtr, in, out, rootThread); 
     stapl::execute_parallel_task( &red ); 
  } else {
    stapl::rmi_fence();
    rmiRequestReduce2<Class, Arg> rmi( in, out, objHandle, memberFuncPtr, 
				       commutative, rootThread );
  }
}		      

template<class Class, class Arg>
void broadcast_rmi(Arg* inout, const int count, const rmiHandle objHandle,
		   void (Class::* const memberFuncPtr)(Arg*, Arg*, const int),
		   const int rootThread) {
  stapl::rmi_fence();
  rmiRequestBroadcast3<Class, Arg> rmi( objHandle, memberFuncPtr, inout, 
					count, rootThread );
}


void abort_rmi(char *str); 

} // end namespace
#endif // _MPI_PRIMITIVES_H
