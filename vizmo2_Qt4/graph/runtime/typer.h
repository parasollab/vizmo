#ifndef _TYPER_H
#define _TYPER_H

#include "pointer.h"

// The size of a word.  The default is 8 bytes, which is typical for larger
// systems, although x86 machines use 4 byte words.  Setting the value too
// high can waste space, whereas setting the value too low can cause bus
// errors.  May be defined at compile time (e.g., -D_BYTE_ALIGNMENT=???).
#ifndef _BYTE_ALIGNMENT
#define _BYTE_ALIGNMENT 8
#endif

// Returns the size of an object aligned to the word size.
//#define ALIGNED_SIZEOF(type) ( sizeof(type) + _BYTE_ALIGNMENT - sizeof(type)%_BYTE_ALIGNMENT )
#define ALIGNED_SIZEOF(type) (sizeof(type)+_BYTE_ALIGNMENT-((sizeof(type)-1)%_BYTE_ALIGNMENT + 1))


namespace stapl {

// packing_req is the amount of packing required for a given type:
// UNKNOWN -> not yet determined
// BASIC   -> packing is unnecessary
// PACK  -> packing is necessary
enum packing_req { UNKNOWN, BASIC, PACKED };


// typer_traits determine the packing_req, additional packed size
// requirements (e.g., objects that contain pointers require extra space to
// serialize the data pointed to), and implement the packing functions.
template<class T> class typer_traits {
  static packing_req _pr;
  static int _size;
public:
  static packing_req type(const T* const t);
  static int packed_size(const T* t, const int numT=1);
  static void pack(T* t, const int numT, char* rmiRequestBase, int* offset);
  static void unpack(T* t, const int numT, char* rmiRequestBase);
};
#ifndef _COMPILING_PRIMITIVES
template<class T> packing_req typer_traits<T>::_pr = UNKNOWN;
template<class T> int typer_traits<T>::_size = -1;
#endif


// typer is used for typing or packing/unpacking arguments used in RMI
// transfer.  Typing determines the packing_req of an argument, as required by
// typer_traits.  Packing/unpacking performs the operations necessary to
// serialize data for transfer via MPI, TCP/IP, etc.
//
// Each argument class must implement 'void define_type(stapl::typer& t)' to
// ensure proper transfer of all its data members.  This method cannot be
// const, because the typer may be unpacking data, thus modifying the contents
// of the members.  This is also the reason local and dynamic do not accept
// const arguments.
//
// Methods are not virtual for speed, since nothing currently needs to inherit
// from typer.  This could be changed, allowing for specialized typers, or to
// leverage the existing typer interface to facilitate other types of
// operations (e.g., garbage collection).
class typer {
  enum pass { TYPING, SIZE, PACKING, UNPACKING };
  const pass _pass; // the pass being executed
  packing_req* _pr; // the packing_req of the current argument
  int* _offset;     // the current offset into the rmiRequest being examined (PACKING/UNPACKING)
                    // <or> the packed size of the current type (TYPING)
  char* _base;      // the 'this' pointer for the rmiRequest being examined (PACKING/UNPACKING)
public:
  typer(packing_req* const pr);                 // for typing
  typer(int* const curSize);                    // for sizing
  typer(char* rmiRequestBase, int* initOffset); // for packing
  typer(char* rmiRequestBase);                  // for unpacking

  template<class T> void local(T& t);
  template<class T> void local(T* t, const int num);
  template<class T> void dynamic(T*& t, const int num=1);
  template<class T> void localOffset(T*& t, T& ref);
  template<class T> void localOffset(T*& t, T* const ref, const int offset=0);
  template<class T> void dynamicOffset(T*& t, T* const ref, const int offset=0);
};
#ifdef _COMPILING_PRIMITIVES
typer::typer(packing_req* const t) : _pass( TYPING ), _pr( t ), _offset( 0 ) {}
typer::typer(int* const curSize) : _pass( SIZE ), _pr( 0 ), _offset( curSize ) {}
typer::typer(char* rmiRequestBase, int* initOffset) :
  _pass( PACKING ), _pr( 0 ), _offset( initOffset ), _base( rmiRequestBase ) {}
typer::typer(char* rmiRequestBase) :
  _pass( UNPACKING ), _pr( 0 ), _offset( 0 ), _base( rmiRequestBase ) {}
#endif


template<class T> void typer::local(T& t) {
  if( typer_traits<T>::type(&t) == PACKED ) {  
    switch( _pass ) {
    case TYPING:
      *_pr = PACKED;
      break;
    case SIZE:
      *_offset += typer_traits<T>::packed_size( &t );
      break;
    case PACKING:
      typer_traits<T>::pack( &t, 1, _base, _offset );
      break;
    case UNPACKING:
      typer_traits<T>::unpack( &t, 1, _base );
      break;
    }
  }
}

template<class T> void typer::local(T* t, const int num) {
  if( typer_traits<T>::type(t) == PACKED ) {
    switch( _pass ) {
    case TYPING:
      *_pr = PACKED;
      break;
    case SIZE:
      *_offset += typer_traits<T>::packed_size( t, num );
      break;
    case PACKING:
      typer_traits<T>::pack( t, num, _base, _offset );
      break;
    case UNPACKING:
      typer_traits<T>::unpack( t, num, _base );
      break;
    }
  }
}

template<class T> void typer::dynamic(T*& t, const int num) {
  typedef typename pointer_to_integral<T*>::integral my_int;
  switch( _pass ) {
  case TYPING:
    *_pr = PACKED;
    break;
  case SIZE:
    if( t != 0 && num != 0 ) {
//      *_offset += sizeof( T ) * num + ( sizeof(T) * num ) % _BYTE_ALIGNMENT;
      int pad = _BYTE_ALIGNMENT-((sizeof(T)*num-1)%_BYTE_ALIGNMENT + 1);
      *_offset += sizeof( T ) * num + pad;
      if( typer_traits<T>::type(t) == PACKED )
	*_offset += typer_traits<T>::packed_size( t, num );
    }
    break;
  case PACKING:
    if( t == 0 || num == 0 )
      t = 0;
    else {
      memcpy( _base + *_offset, t, sizeof(T) * num );
      const int tmpOffset = *_offset;
//      *_offset += sizeof( T ) * num + ( sizeof(T) * num ) % _BYTE_ALIGNMENT;
      int pad = _BYTE_ALIGNMENT-((sizeof(T)*num-1)%_BYTE_ALIGNMENT + 1);
      *_offset += sizeof( T ) * num + pad;
      if( typer_traits<T>::type(t) == PACKED )
	typer_traits<T>::pack( reinterpret_cast<T*>(_base+tmpOffset), num, _base, _offset );
      t = reinterpret_cast<T*>( tmpOffset );
    }
    break;
  case UNPACKING:
    if( t != 0 && num != 0 ) {
      t = reinterpret_cast<T*>( _base + reinterpret_cast<my_int>(t) );
      if( typer_traits<T>::type(t) == PACKED )
	typer_traits<T>::unpack( t, num, _base );
    }
    break;
  }
}

template<class T> void typer::localOffset(T*& t, T& ref) {
  typedef typename pointer_to_integral<T*>::integral my_int;
  switch( _pass ) {
  case TYPING:
    break;
  case SIZE:
    break;
  case PACKING:
    t = reinterpret_cast<T*>( reinterpret_cast<char*>(&ref) - _base );
    break;
  case UNPACKING:
    if( t != 0 )
      t = reinterpret_cast<T*>( _base + reinterpret_cast<my_int>(t) );
    break;
  }
}

template<class T> void typer::localOffset(T*& t, T* const ref, const int offset) {
  typedef typename pointer_to_integral<T*>::integral my_int;
  switch( _pass ) {
  case TYPING:
    break;
  case SIZE:
    break;
  case PACKING:
    t = reinterpret_cast<T*>( reinterpret_cast<char*>(ref + offset) - _base );
    break;
  case UNPACKING:
    if( t != 0 )
      t = reinterpret_cast<T*>( _base + reinterpret_cast<my_int>(t) );
    break;
  }
}

template<class T> void typer::dynamicOffset(T*& t, T* const ref, const int offset) {
  typedef typename pointer_to_integral<T*>::integral my_int;

  switch( _pass ) {
  case TYPING:
    break;
  case SIZE:
    break;
  case PACKING:
    t = ref + offset;
    break;
  case UNPACKING:
    if( t != 0 )
      t = reinterpret_cast<T*>( _base + reinterpret_cast<my_int>(t) );
    break;
  }
}


template<class T> packing_req typer_traits<T>::type(const T* const t) {
  if( _pr == UNKNOWN ) {
    _pr = BASIC;
    typer ct( &_pr );
    const_cast<T*>( t )->define_type( ct );
  }
  return _pr;
}

template<class T> int typer_traits<T>::packed_size(const T* t, const int numT) {
  if( _size == -1 || typer_traits<T>::type(t) == PACKED ) {
    int size = 0;
    typer ct( &size );
    for( int i=0; i<numT; ++i, ++t )
      const_cast<T*>( t )->define_type( ct );
    _size = size;
  }
  return _size;
}

template<class T> void typer_traits<T>::pack(T* t, const int numT, char* rmiRequestBase, int* offset) {
  typer ct( rmiRequestBase, offset );
  for( int i=0; i<numT; ++i, ++t )
    t->define_type( ct );
}

template<class T> void typer_traits<T>::unpack(T* t, const int numT, char* rmiRequestBase) {
  typer ct( rmiRequestBase );
  for( int i=0; i<numT; ++i, ++t )
    t->define_type( ct );
}


// The following specialize typer_traits for the C++ fundamental types, which
// are all 'BASIC', meaning they don't need to be packed, and hence don't
// require additional space for packing.
template<> struct typer_traits<bool> {
  static packing_req type(const bool* const) { return BASIC; }
  static int packed_size(const bool*, const int numT=1) { return 0; }
  static void pack(bool*, const int, char*, int*) {}
  static void unpack(bool*, const int, char*) {}
};
template<> struct typer_traits<char> {
  static packing_req type(const char* const) { return BASIC; }
  static int packed_size(const char*, const int numT=1) { return 0; }
  static void pack(char*, const int, char*, int*) {}
  static void unpack(char*, const int, char*) {}
};
template<> struct typer_traits<signed char> {
  static packing_req type(const signed char* const) { return BASIC; }
  static int packed_size(const signed char*, const int numT=1) { return 0; }
  static void pack(signed char*, const int, char*, int*) {}
  static void unpack(signed char*, const int, char*) {}
};
template<> struct typer_traits<unsigned char> {
  static packing_req type(const unsigned char* const) { return BASIC; }
  static int packed_size(const unsigned char*, const int numT=1) { return 0; }
  static void pack(unsigned char*, const int, char*, int*) {}
  static void unpack(unsigned char*, const int, char*) {}
};
template<> struct typer_traits<signed short int> {
  static packing_req type(const signed short int* const) { return BASIC; }
  static int packed_size(const signed short int*, const int numT=1) { return 0; }
  static void pack(signed short int*, const int, char*, int*) {}
  static void unpack(signed short int*, const int, char*) {}
};
template<> struct typer_traits<unsigned short int> {
  static packing_req type(const unsigned short int* const) { return BASIC; }
  static int packed_size(const unsigned short int*, const int numT=1) { return 0; }
  static void pack(unsigned short int*, const int, char*, int*) {}
  static void unpack(unsigned short int*, const int, char*) {}
};
template<> struct typer_traits<signed int> {
  static packing_req type(const signed int* const) { return BASIC; }
  static int packed_size(const signed int*, const int numT=1) { return 0; }
  static void pack(signed int*, const int, char*, int*) {}
  static void unpack(signed int*, const int, char*) {}
};
template<> struct typer_traits<unsigned int> {
  static packing_req type(const unsigned int* const) { return BASIC; }
  static int packed_size(const unsigned int*, const int numT=1) { return 0; }
  static void pack(unsigned int*, const int, char*, int*) {}
  static void unpack(unsigned int*, const int, char*) {}
};
template<> struct typer_traits<signed long int> {
  static packing_req type(const signed long int* const) { return BASIC; }
  static int packed_size(const signed long int*, const int numT=1) { return 0; }
  static void pack(signed long int*, const int, char*, int*) {}
  static void unpack(signed long int*, const int, char*) {}
};
template<> struct typer_traits<unsigned long int> {
  static packing_req type(const unsigned long int* const) { return BASIC; }
  static int packed_size(const unsigned long int*, const int numT=1) { return 0; }
  static void pack(unsigned long int*, const int, char*, int*) {}
  static void unpack(unsigned long int*, const int, char*) {}
};
template<> struct typer_traits<signed long long int> {
  static packing_req type(const signed long long int* const) { return BASIC; }
  static int packed_size(const signed long long int*, const int numT=1) { return 0; }
  static void pack(signed long long int*, const int, char*, int*) {}
  static void unpack(signed long long int*, const int, char*) {}
};
template<> struct typer_traits<unsigned long long int> {
  static packing_req type(const unsigned long long int* const) { return BASIC; }
  static int packed_size(const unsigned long long int*, const int numT=1) { return 0; }
  static void pack(unsigned long long int*, const int, char*, int*) {}
  static void unpack(unsigned long long int*, const int, char*) {}
};
template<> struct typer_traits<float> {
  static packing_req type(const float* const) { return BASIC; }
  static int packed_size(const float*, const int numT=1) { return 0; }
  static void pack(float*, const int, char*, int*) {}
  static void unpack(float*, const int, char*) {}
};
template<> struct typer_traits<double> {
  static packing_req type(const double* const) { return BASIC; }
  static int packed_size(const double*, const int numT=1) { return 0; }
  static void pack(double*, const int, char*, int*) {}
  static void unpack(double*, const int, char*) {}
};
template<> struct typer_traits<long double> {
  static packing_req type(const long double* const) { return BASIC; }
  static int packed_size(const long double*, const int numT=1) { return 0; }
  static void pack(long double*, const int, char*, int*) {}
  static void unpack(long double*, const int, char*) {}
};

#ifndef __xlC__
// Allow member function pointers to be transferred as well (for future
// reference: this is valid on homogeneous systems only!)
template<class Rtn, class Class> struct typer_traits<Rtn (Class::*)()> {
  typedef Rtn (Class::*memberFuncPtr)();
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};
template<class Rtn, class Class, class Arg1> struct typer_traits<Rtn (Class::*)(Arg1)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};
template<class Rtn, class Class, class Arg1, class Arg2>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};
template<class Rtn, class Class, class Arg1, class Arg2, class Arg3>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};
template<class Rtn, class Class, class Arg1, class Arg2, class Arg3, class Arg4>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3, Arg4)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3,Arg4);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

// Allow member const function pointers to be tranfered
//added by gabrielt to solve a compiler error on SGI;
template<class Rtn, class Class> struct typer_traits<Rtn (Class::*)() const> {
  typedef Rtn (Class::*memberFuncPtr)() const;
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1> struct typer_traits<Rtn (Class::*)(Arg1) const> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1) const;
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1, class Arg2>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2) const> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2) const;
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1, class Arg2, class Arg3>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3) const> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3) const;
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1, class Arg2, class Arg3, class Arg4>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3, Arg4) const> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3,Arg4) const;
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}
};

#else
//xlc only
// Allow member function pointers to be transferred as well (for future
// reference: this is valid on homogeneous systems only!)
template<class Rtn, class Class> struct typer_traits<Rtn (Class::*)()> {
  typedef Rtn (Class::*memberFuncPtr)();
  typedef Rtn (Class::*constmemberFuncPtr)() const;

  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static packing_req type(constmemberFuncPtr const*) { return BASIC; }

  static int packed_size(memberFuncPtr const*, const int numT=1) { return 0; }
  static int packed_size(constmemberFuncPtr const*, const int numT=1) { return 0; }

  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void pack(constmemberFuncPtr const*, const int, char*, int*) {}

  static void unpack(memberFuncPtr const*, const int, char*) {}
  static void unpack(constmemberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1> struct typer_traits<Rtn (Class::*)(Arg1)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}

  typedef Rtn (Class::*constmemberFuncPtr)(Arg1) const;
  static packing_req type(constmemberFuncPtr const*) { return BASIC; }
  static int packed_size(constmemberFuncPtr*, const int numT=1) { return 0; }
  static void pack(constmemberFuncPtr const*, const int, char*, int*) {}
  static void unpack(constmemberFuncPtr const*, const int, char*) {}
};
template<class Rtn, class Class, class Arg1, class Arg2>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}

  typedef Rtn (Class::*constmemberFuncPtr)(Arg1,Arg2) const;
  static packing_req type(constmemberFuncPtr const*) { return BASIC; }
  static int packed_size(constmemberFuncPtr*, const int numT=1) { return 0; }
  static void pack(constmemberFuncPtr const*, const int, char*, int*) {}
  static void unpack(constmemberFuncPtr const*, const int, char*) {}
};

template<class Rtn, class Class, class Arg1, class Arg2, class Arg3>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}

  typedef Rtn (Class::*constmemberFuncPtr)(Arg1,Arg2,Arg3) const;
  static packing_req type(constmemberFuncPtr const*) { return BASIC; }
  static int packed_size(constmemberFuncPtr*, const int numT=1) { return 0; }
  static void pack(constmemberFuncPtr const*, const int, char*, int*) {}
  static void unpack(constmemberFuncPtr const*, const int, char*) {}

};
template<class Rtn, class Class, class Arg1, class Arg2, class Arg3, class Arg4>
struct typer_traits<Rtn (Class::*)(Arg1, Arg2, Arg3, Arg4)> {
  typedef Rtn (Class::*memberFuncPtr)(Arg1,Arg2,Arg3,Arg4);
  static packing_req type(memberFuncPtr const*) { return BASIC; }
  static int packed_size(memberFuncPtr*, const int numT=1) { return 0; }
  static void pack(memberFuncPtr const*, const int, char*, int*) {}
  static void unpack(memberFuncPtr const*, const int, char*) {}

  typedef Rtn (Class::*constmemberFuncPtr)(Arg1,Arg2,Arg3,Arg4) const;
  static packing_req type(constmemberFuncPtr const*) { return BASIC; }
  static int packed_size(constmemberFuncPtr*, const int numT=1) { return 0; }
  static void pack(constmemberFuncPtr const*, const int, char*, int*) {}
  static void unpack(constmemberFuncPtr const*, const int, char*) {}
};
#endif

} // end namespace
#endif // _TYPER_H
