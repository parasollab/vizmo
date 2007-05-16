#ifndef _THREAD_PRIMITIVES_H
#define _THREAD_PRIMITIVES_H

#include <new>


namespace thread_primitives {


// return_store provides space for the return value of a sync_rmi, for use
// inside rmiRequests.  Instead of using the type T directly, it uses a
// properly aligned char array.  This prevents initialization via T's
// constructor, which could allocate memory, etc, before the return value is
// available.  It becomes correctly initialized via the copy constructor and
// placement new (and hence must explicitly call the destructor) via operator=
// for regular types.  For pointers and references, only the address is
// stored, but copying is still necessary to preserve the semantics described
// primitives.h.
template<class T> union return_store {
  double align;
  char c[sizeof(T)];
  inline void operator=(T t) { new( c ) T( t ); }
  inline operator T() { return *reinterpret_cast<T*>(c); }
  ~return_store() { reinterpret_cast<T*>(c)->~T(); }
};
template<class T> union return_store<T&> {
  T* ref;
  inline void operator=(T& t) { ref = new T( t ); }
  inline operator T&() { return *ref; }
};
template<class T> union return_store<T*> {
  T* ptr;
  inline void operator=(T* const t) { ptr = new T( *t ); }
  inline operator T*() { return ptr; }
};


// arg_traits define useful typedefs and methods to use on the arguments of
// rmiRequests.  They allow one rmiRequest codebase to handle all combinations
// of const/non-const and basic/pointer/reference types.
//
// Destroying RMI requests is more complicated than regular objects.
// async_rmi requests are allocated directly into an outgoing buffer using
// placement new.  As such, placement_destroy must be used to explicitly call
// argument destructors.  sync_rmi requests are allocated directly on the
// stack, and hence only need to worry about freeing dynamically allocated
// arguments.
template<class T> struct arg_traits {
  typedef T storage;
  typedef return_store<T> return_storage;
  inline static const T& store(const T& t) { return t; }
  inline static void destroy(const T& t) {}
  inline static void placement_destroy(const T& t) { t.~T(); }
};
template<class T> struct arg_traits<const T> {
  typedef const T storage;
  typedef return_store<T> return_storage;
  inline static const T& store(const T& t) { return t; }
  inline static void destroy(const T& t) {}
  inline static void placement_destroy(const T& t) { t.~T(); }
};

template<class T> struct arg_traits<T&> {
  typedef T storage;
  typedef return_store<T&> return_storage;
  inline static const T& store(const T& t) { return t; }
  inline static void destroy(const T& t) {}
  inline static void placement_destroy(const T& t) { t.~T(); }
};
template<class T> struct arg_traits<const T&> {
  typedef const T storage;
  typedef return_store<const T&> return_storage;
  inline static const T& store(const T& t) { return t; }
  inline static void destroy(const T& t) {}
  inline static void placement_destroy(const T& t) { t.~T(); }
};

template<class T> struct arg_traits<T*> {
  typedef T* storage;
  typedef return_store<T*> return_storage;
  inline static T* store(const T* const& t) { return new T(*t); }
  inline static void destroy(const T* const& t) { delete t; }
  inline static void placement_destroy(const T* const& t) { delete t; }
};
template<class T> struct arg_traits<const T*> {
  typedef const T* storage;
  typedef return_store<const T*> return_storage;
  inline static const T* store(const T* const& t) { return new T(*t); }
  inline static void destroy(const T* const& t) { delete t; }
  inline static void placement_destroy(const T* const& t) { delete t; }
};




// rmiRequest encapsulates an RMI request for subsequent execution via the
// exec method.  rmiRequests package all information for straightforward
// buffering (i.e., aggregation) and transfer.  The header contains the size
// of the request, the rmi object handle, and the desired member function.
// The body contains the required arguments.
class rmiRequest {
public:
  const int _size;
  void* operator new(size_t size, const int destThread, bool& mustSend);
  void operator delete(void*, const int destThread, bool& mustSend) {}
  void operator delete(void*) {}
  explicit rmiRequest(const int s) : _size( s ) {}
  virtual ~rmiRequest() {}
  virtual void exec(const rmiRegistry& registry) = 0;

};


// rmiRequestReduce encapsulates a reduction RMI request.
struct rmiRequestReduce {
  virtual void exec(const rmiRegistry& registry, void* in, void* inout) = 0;
  virtual void result(void* result) = 0;
};


// rmiRequestBroadcast encapsulates a broadcast RMI request.
struct rmiRequestBroadcast {
  virtual void exec(const rmiRegistry& registry, void* in) = 0;
};


void send_async_rmi(const int destThread);
void send_sync_rmi(const int destThread, rmiRequest* const r);
void send_reduce_rmi(rmiRequestReduce* r, void* in, const int rootThread);
void send_broadcast_rmi(rmiRequestBroadcast* r, void* in, const int rootThread);
template<class Rtn> class OpaqueHandle;

template<class Class, class Rtn>
class rmiRequest0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle _h;
  const MemberFuncPtr _m;

public:
  void* operator new(size_t, const int destThread, bool& mustSend) {
    return rmiRequest::operator new( ALIGNED_SIZEOF(rmiRequest0), destThread, mustSend );
  }

  rmiRequest0(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
	      const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest0) ), _h( h ), _m( m ) {
    if( mustSend ) send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )();
  }
};


template<class Class, class Rtn, class Arg1>
class rmiRequest1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;

public:
  void* operator new(size_t, const int destThread, bool& mustSend) {
    return rmiRequest::operator new( ALIGNED_SIZEOF(rmiRequest1), destThread, mustSend );
  }

  rmiRequest1(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
	      const typename rmi_arg<Arg1>::type& a1, const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest1) ), _h( h ), _m( m ),
    _a1( arg_traits<Arg1>::store(a1) ) {
    if( mustSend ) send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( _a1 );
    arg_traits<Arg1>::placement_destroy( _a1 );
  }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequest2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;
  typename arg_traits<Arg2>::storage _a2;

public:
  void* operator new(size_t, const int destThread, bool& mustSend) {
    return rmiRequest::operator new( ALIGNED_SIZEOF(rmiRequest2), destThread, mustSend );
  }

  rmiRequest2(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
	      const typename rmi_arg<Arg1>::type& a1,
	      const typename rmi_arg<Arg2>::type& a2, const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequest2) ), _h( h ), _m( m ),
    _a1( arg_traits<Arg1>::store(a1) ), _a2( arg_traits<Arg2>::store(a2) ) {
    if( mustSend ) send_async_rmi( destThread );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( _a1, _a2 );
    arg_traits<Arg1>::placement_destroy( _a1 );
    arg_traits<Arg2>::placement_destroy( _a2 );
  }
};


template<class Class, class Rtn>
class rmiRequestReturn0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Rtn>::return_storage rtn;

public:
  rmiRequestReturn0(const int destThread, const rmiHandle h, const MemberFuncPtr& m) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn0) ), _h( h ), _m( m ) {
    send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    rtn = ( c->*_m )();
  }
  Rtn result() { return rtn; }
};


template<class Class, class Rtn, class Arg1>
class rmiRequestReturn1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;
  typename arg_traits<Rtn>::return_storage rtn;

public:
  rmiRequestReturn1(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
		    const typename rmi_arg<Arg1>::type& a1) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn1) ), _h( h ), _m( m ),
    _a1( arg_traits<Arg1>::store(a1) ) {
    send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    rtn = ( c->*_m )( _a1 );
    arg_traits<Arg1>::destroy( _a1 );
  }

  Rtn result() { return rtn; }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequestReturn2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;
  typename arg_traits<Arg2>::storage _a2;
  typename arg_traits<Rtn>::return_storage rtn;

public:
  rmiRequestReturn2(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
		    const typename rmi_arg<Arg1>::type& a1,
		    const typename rmi_arg<Arg2>::type& a2) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestReturn2) ), _h( h ), _m( m ),
    _a1( arg_traits<Arg1>::store(a1) ), _a2( arg_traits<Arg2>::store(a2) ) {
    send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    rtn = ( c->*_m )( _a1, _a2 );
    arg_traits<Arg1>::destroy( _a1 );
    arg_traits<Arg2>::destroy( _a2 );
  }

  Rtn result() { return rtn; }
};


template<class Class, class Rtn>
class rmiRequestOpaqueReturn0 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)();
  const rmiHandle _h;
  const MemberFuncPtr _m;
  //typename arg_traits<Rtn>::return_storage rtn;
  OpaqueHandle<Rtn> *_rtn;
   
public:
  rmiRequestOpaqueReturn0(const int destThread, const rmiHandle h, const MemberFuncPtr& m, 
			  OpaqueHandle<Rtn> *hd, const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn0) ), _h( h ), _m( m ), _rtn( hd ) {
    if( mustSend ) send_async_rmi( destThread );
    //send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    //rtn = ( c->*_m )();
    _rtn->_value = ( c->*_m )();
    _rtn->_ready = true;
  }
  Rtn result() { return rtn; }
};


template<class Class, class Rtn, class Arg1>
class rmiRequestOpaqueReturn1 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;
  //typename arg_traits<Rtn>::return_storage rtn;
  OpaqueHandle<Rtn> *_rtn;


public:
  rmiRequestOpaqueReturn1(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
			  const typename rmi_arg<Arg1>::type& a1, OpaqueHandle<Rtn> *hd,  
			  const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn1) ), _h( h ), _m( m ), 
    _a1( arg_traits<Arg1>::store(a1), _rtn( hd ) ) {
    if( mustSend ) send_async_rmi( destThread );
    //send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    //rtn = ( c->*_m )( _a1 );
    _rtn->_value = ( c->*_m )( _a1 );
    _rtn->_ready = true;
    arg_traits<Arg1>::destroy( _a1 );
  }

  Rtn result() { return rtn; }
};


template<class Class, class Rtn, class Arg1, class Arg2>
class rmiRequestOpaqueReturn2 : public rmiRequest {
protected:
  typedef Rtn (Class::* MemberFuncPtr)(Arg1, Arg2);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  typename arg_traits<Arg1>::storage _a1;
  typename arg_traits<Arg2>::storage _a2;
  //typename arg_traits<Rtn>::return_storage rtn;
  OpaqueHandle<Rtn> *_rtn;


public:
  rmiRequestOpaqueReturn2(const int destThread, const rmiHandle h, const MemberFuncPtr& m,
			  const typename rmi_arg<Arg1>::type& a1,
			  const typename rmi_arg<Arg2>::type& a2, 
			  OpaqueHandle<Rtn> *hd,  const bool mustSend) :
    rmiRequest( ALIGNED_SIZEOF(rmiRequestOpaqueReturn2) ), _h( h ), _m( m ),
    _a1( arg_traits<Arg1>::store(a1) ), _a2( arg_traits<Arg2>::store(a2)), _rtn( hd ) {
    if( mustSend ) send_async_rmi( destThread );
    //send_sync_rmi( destThread, this );
  }

  void exec(const rmiRegistry& registry) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    //rtn = ( c->*_m )( _a1, _a2 );
    _rtn->_value = ( c->*_m )( _a1, _a2 );
    _rtn->_ready = true;
    arg_traits<Arg1>::destroy( _a1 );
    arg_traits<Arg2>::destroy( _a2 );
  }

  Rtn result() { return rtn; }
};




template<class Class, class Arg>
class rmiRequestReduce2 : public rmiRequestReduce {
private:
  typedef void (Class::* MemberFuncPtr)(Arg*, Arg*);
  const rmiHandle _h;
  const MemberFuncPtr _m;
  Arg* _out;

public:
  rmiRequestReduce2(const rmiHandle h, const MemberFuncPtr& m, 
		    Arg* const in, Arg* const out, const int rootThread)
    : _h( h ), _m( m ), _out( out ) {
    send_reduce_rmi( this, in, rootThread );
  }

  void exec(const rmiRegistry& registry, void* in, void* inout) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( static_cast<Arg*>(in), static_cast<Arg*>(inout) );
  }

  void result(void* result) {
    *_out = *static_cast<Arg*>( result );
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
    send_broadcast_rmi( this, inout, rootThread );
  }

  void exec(const rmiRegistry& registry, void* in) {
    Class* const c = reinterpret_cast<Class*>( registry[_h] );
    ( c->*_m )( _local, static_cast<Arg*>(in), _count );
  }
};




template<class Class, class Rtn>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)()) {
  typedef rmiRequest0<Class, Rtn> rmi; bool b = false;
  new( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, b );
}

template<class Class, class Rtn, class Arg1>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1),
	       const typename rmi_arg<Arg1>::type& a1) {
  typedef rmiRequest1<Class, Rtn, Arg1> rmi; bool b = false;
  new( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, a1, b );
}

template<class Class, class Rtn, class Arg1, class Arg2>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	       const typename rmi_arg<Arg1>::type& a1,
	       const typename rmi_arg<Arg2>::type& a2) {
  typedef rmiRequest2<Class, Rtn, Arg1, Arg2> rmi; bool b = false;
  new( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, a1, a2, b );
}


template<class Class, class Rtn>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)()) {
  typedef rmiRequestReturn0<Class, Rtn> rmi;
  return rmi( destThread, objHandle, memberFuncPtr ).result();
}

template<class Class, class Rtn, class Arg1>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1) {
  typedef rmiRequestReturn1<Class, Rtn, Arg1> rmi;
  return rmi( destThread, objHandle, memberFuncPtr, a1 ).result();
}

template<class Class, class Rtn, class Arg1, class Arg2>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2) {
  typedef rmiRequestReturn2<Class, Rtn, Arg1, Arg2> rmi;
  return rmi( destThread, objHandle, memberFuncPtr, a1, a2 ).result();
}


template<class Class, class Rtn>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(),
	     OpaqueHandle<Rtn> *handle) {
  typedef rmiRequestOpaqueReturn0<Class, Rtn> rmi;  bool b = false;
  new ( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, handle, b );
}

template<class Class, class Rtn, class Arg1>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1,
	     OpaqueHandle<Rtn> *handle) {
  typedef rmiRequestOpaqueReturn1<Class, Rtn, Arg1> rmi; bool b = false;
  new ( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, a1, handle, b);
}

template<class Class, class Rtn, class Arg1, class Arg2>
void sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1, Arg2),
	     const typename rmi_arg<Arg1>::type& a1,
	     const typename rmi_arg<Arg2>::type& a2,
	     OpaqueHandle<Rtn> *handle) {
  typedef rmiRequestOpaqueReturn2<Class, Rtn, Arg1, Arg2> rmi; bool b = false;
  new ( destThread, b ) rmi( destThread, objHandle, memberFuncPtr, a1, a2, handle, b );
}


template<class Class, class Arg>
void reduce_rmi(Arg* in, Arg* out, const rmiHandle objHandle,
		void (Class::* const memberFuncPtr)(Arg*, Arg*),
		const bool commutative, const int rootThread=-1) {
  rmiRequestReduce2<Class, Arg> rmi( objHandle, memberFuncPtr, in, out, rootThread );
}


template<class Class, class Arg>
void broadcast_rmi(Arg* inout, const int count, const rmiHandle objHandle,
		   void (Class::* const memberFuncPtr)(Arg*, Arg*, const int),
		   const int rootThread) {
  rmiRequestBroadcast3<Class, Arg> rmi( objHandle, memberFuncPtr, inout, count, rootThread );
}


} // end namespace
#endif // _THREAD_PRIMITIVES_H
