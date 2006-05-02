#ifndef _POINTER_H
#define _POINTER_H

/**
 * @ingroup parallelPrimitives
 * pointer_to_integral uses templates to find an integral type that can safely
 * store a pointer type.  The C++ standard states that
 *
 * sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
 *
 * and that reinterpret_cast can safely cast pointers to integrals and back
 * again.  However, it does not specify which integrals, and hence the
 * appropriate integral type varies across platforms and makes using the wrong
 * integral is dangerous (e.g., casting a pointer that only fits in a long to
 * an int will truncate some of the pointer information).  This template is
 * not valid with function pointers and member function pointers.
 *
 * Example:
 * \code
 * typedef pointer_to_integral<int*>::integral integral;
 * int* a;
 * integral b = reinterpret_cast<integral>( a );
 * int* c = reinterpret_cast<int*>( b );
 * \endcode
 **/

namespace stapl {
  
  template<class Ptr, bool equalsShort> struct ptrShort { typedef char integral; };
  template<class Ptr> struct ptrShort<Ptr, true>        { typedef short integral; };
  
  template<class Ptr, bool equalsInt> struct ptrInt {
    typedef typename ptrShort<Ptr, sizeof(Ptr)==sizeof(short)>::integral integral;
  };
  template<class Ptr> struct ptrInt<Ptr, true>          { typedef int integral; };
  
  template<class Ptr, bool equalsLong> struct ptrLong {
    typedef typename ptrInt<Ptr, sizeof(Ptr)==sizeof(int)>::integral integral;
  };
  template<class Ptr> struct ptrLong<Ptr, true>         { typedef long integral; };
  
  template<class Ptr> struct pointer_to_integral { 
    typedef typename ptrLong<Ptr, sizeof(Ptr)==sizeof(long)>::integral integral;
  };
}

// Ensures the given input condition is true.  If expr is true, nothing
// happens, otherwise the error message is printed and the program aborts.
#define stapl_assert(expr, msg) { if( !(expr) ) { \
  fprintf( stderr, "STAPL ASSERTION: %s (file: %s, line: %d)\n", \
	           msg, __FILE__, __LINE__ ); abort(); } }

#endif // _POINTER_H
