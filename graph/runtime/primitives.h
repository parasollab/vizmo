#include <typeinfo>

#ifndef _PRIMITIVES_H
#define _PRIMITIVES_H

#include "pointer.h"

/**
 * @addtogroup parallelPrimitives
 *
 * The parallel communication primitives are designed to abstract the
 * creation, registration, communication and synchronization of parallelism in
 * a STAPL program, allowing for performance and portability on a large number
 * of systems.
 *
 * The unit of parallel execution is called a thread.  Contrary to the concept
 * of shared-memory threads, STAPL threads may or may not exist in the same
 * address space.  As such, it is undefined behavior to try to share writeable
 * global variables, including static class members, between threads.
 *
 * Upon program startup, all threads begin SPMD execution in parallel.  There
 * are no purely sequential regions!  The starting point for execution is
 *
 * \code void stapl_main(int argc, char* argv[]) \endcode
 *
 * which replaces the sequential standard
 *
 * \code int main(int argc, char* argv[]) \endcode
 *
 * The primitives provide shared-object parallelism.  Threads communicate with
 * each other using remote method invocation (RMI).  It is expected that
 * shared-objects used in communication are actually distributed across all
 * threads, and hence have a corresponding local object on each thread.
 * Shared-objects are identified by a handle, and their local objects are
 * identified by a thread id and a handle.  As such, all objects that are
 * communication targets must be registered with the primitives to obtain a
 * handle (e.g., register_rmi_object).  This handle allows for proper address
 * translation between threads.  Since each thread owns a local portion of a
 * larger shared/distributed object, it is not necessary for a thread to use
 * RMI to access its local object, even for mutual exclusion.  However, it is
 * still valid to use RMI on the local objects.  It is up to the distributed
 * object implementation to keep track of which portions are local and which
 * are remote (e.g., ghost nodes, a list of local indices, etc.).
 *
 * During communication transport, all RMI arguments are copied using either
 * their C++ copy constructor, or the stapl::typer.  The specific method
 * varies with the implementation.  The typer obtains information about a
 * class to determine the optimal means of packing/unpacking and copying any
 * given type (see stapl::typer for details).  As such, each user-defined
 * class passed as an argument must implement a single method that clearly
 * defines its data members to the typer:
 *
 * \code void define_type(stapl::typer& t) \endcode
 *
 * Some communication calls are collective, meaning all threads must call the
 * function before it can complete.  Collective calls typically need to
 * perform complicated communication patterns among all threads.  The rest of
 * the communication calls are point-to-point, and hence need to be called by
 * only one thread.
 *
 * Point-to-point calls can be used to explicitly synchronize specific threads
 * through synchronization calls such as rmi_wait().  Collective calls imply
 * synchronization, since they do not return until all threads have completed
 * the necessary operations.
 *
 * Point-to-point calls may be aggregated for improved performance, by
 * decreasing the amount of network congestion due to many small messages.
 * See set_aggregation() for more details.
 *
 * To ensure portability, only this interface should be used to express
 * parallelism and synchronization within a STAPL program.  The actual
 * implementation varies (OpenMP, Pthreads, MPI, etc).  Even if it is known
 * that the primitives have been implemented a certain way (e.g., OpenMP) for
 * a certain system, using calls outside this specification (e.g., \c omp \c
 * atomic) is completely non-portable and highly discouraged.
 *
 * A number of higher level STAPL components are written in terms of the
 * primitives, such as the \ref pContainers.  Although the parallel
 * communication primitives are always available for use, it is generally
 * better to use these higher level STAPL components to accomplish a given
 * task.  Typical users will probably never need to utilize the primitives,
 * except for simple requests like querying the number of threads in use.
 * However, STAPL developers and advanced users writing user-defined STAPL
 * components will likely use the primitives a great deal.
 *
 * <b>SEMANTICS OF RMI:</b>\n
 * RMI makes a number of guarantees.  First, RMI requests always maintain
 * order, i.e., a newer request may not overtake and execute before an older
 * request.  However, there is no guarantee of fairness between threads.  For
 * example, although threads 1 and 2 may simultaneously issue requests to
 * thread 3, thread 3 may receive all of thread 1's requests before receiving
 * any of thread 2's requests.
 *
 * Second, remotely invoked methods execute atomically, i.e., they will not be
 * interrupted by other incoming requests or local operations.  The only
 * exception is if the remotely invoked method explicitly uses RMI operations
 * (e.g., async_rmi, sync_rmi, rmi_wait, rmi_poll).  In this case, all
 * operations before the usage is atomic, as well as all operations after,
 * until either the end of the method or the next RMI operation.
 *
 * RMI also has a few semantic differences from traditional C++ method
 * invocation.  First, the arguments to RMI are call-by-value, regardless of
 * type (e.g., pointers and references), i.e., the calling thread will not see
 * any modifications made to the arguments.  Likewise, the receiving thread
 * will not see any modifications made to a return value of a sync_rmi.  If
 * the return value is a pointer or a reference, it will be reallocated
 * privately by the calling thread (it is the user's responsibility to
 * subsequently delete this object!).  This semantic difference ensures there
 * are no global/shared aliases between threads.  Second, although remotely
 * invoked methods may use and modify the supplied arguments freely, they
 * should should not store pointers or references to the arguments after the
 * invocation completes.  This allows implementations to reuse argument
 * buffers, instead of continuously allocating space.  Also, since arguments
 * may exist within RMI maintained buffers, remotely invoked methods should
 * not try to delete/free the object, or perform a C-style realloc.
 *
 * <b>OPTIMAL USAGE:</b>\n
 * As in traditional C++ method invocation, the style of passing arguments can
 * have a significant impact on performance.  Most arguments are passed
 * quicker as a reference or pointer, since no intermediate copies are
 * necessary.  Although RMI requires a copy from the calling to the receiving
 * thread, to preserve copy-by-value semantics, all other copies will be
 * eliminated.  In addition, specifying the argument to be a const may allow
 * the implementation to optimize copying even more.  As such, it is almost
 * always quickest to pass a type 'T' as a 'const T&' if no modifications to T
 * will be necessary.
 *
 * The sync_rmi operation is optimized for returning small objects (i.e.,
 * large arrays are better transported via async_rmi).  As such, it is usually
 * quickest to return a type 'T' as a 'T'.  This also alleviates the user from
 * worrying about deallocating the return value.
 *
 * \include
 * <runtime.h>
 * @{ 
 **/

#include "typer.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef _STAPL_THREAD
#include <mpi.h>
#endif

/**
 * The starting point for SPMD user code execution, replacing the sequential
 * standard:
 *
 * \code int main(int argc, char* argv[]) \endcode
 **/
extern void stapl_main(int argc, char *argv[]);


// Ensures the given input condition is true.  If expr is true, nothing
// happens, otherwise the error message is printed and the program aborts.
#define stapl_assert(expr, msg) { if( !(expr) ) { \
  fprintf( stderr, "STAPL ASSERTION: %s (file: %s, line: %d)\n", \
           msg, __FILE__, __LINE__ ); abort(); } }


#ifdef STAPL_DEBUG
#define dassert(expr, msg) { if( !(expr) ) { \
  fprintf( stderr, "dassert: %s (file: %s, line: %d)\n", \
           msg, __FILE__, __LINE__ ); abort(); } }

#define wassert(expr, msg) { if( !(expr) ) { \
  fprintf( stderr, "wassert: %s (file: %s, line: %d)\n", \
           msg, __FILE__, __LINE__ ); } }

#else
#define dassert(expr, msg)
#define wassert(expr, msg)
#endif

namespace stapl {

// Handles the standard error checking associated with malloc.
inline void* safe_malloc(size_t size) {
  void* tmp = malloc( size );
  stapl_assert( tmp != 0, "Memory Allocation Failed" );
  return tmp;
}


/**
 * The rmi_arg template specifies the type for passing an argument to RMI
 * functions.  It allows a single codebase to handle all combinations of
 * const/non-const and basic/pointer/reference types.
 **/
template<class Arg> struct rmi_arg             { typedef Arg type; };
template<class Arg> struct rmi_arg<const Arg>  { typedef Arg type; };
template<class Arg> struct rmi_arg<Arg&>       { typedef Arg type; };
template<class Arg> struct rmi_arg<const Arg&> { typedef Arg type; };
template<class Arg> struct rmi_arg<Arg*>       { typedef Arg* type; };
template<class Arg> struct rmi_arg<const Arg*> { typedef Arg* type; };


} // end namespace


#include "rmi_registry.h"

#if defined( _STAPL_MPI )
#  define mpi_primitives stapl
#  include "mpi_primitives.h"
#elif defined( _STAPL_THREAD )
#  define thread_primitives stapl
#  include "thread_primitives.h"
#elif defined( _STAPL_MIXED )
#  include "mixed_primitives.h"
#elif defined( _STAPL_COMM )
#  include "comm_primitives.h"
#endif


#if defined( STAPL_DOCUMENTATION_ONLY )
/**
 * The typer class is used for automatically typing and packing/unpacking
 * arguments to RMI functions.  Typing determines whether packing is necessary
 * for a given type.  Packing serializes the type for communication, and
 * unpacking restores it for use in destination user-code.
 *
 * The typer operates on a user-defined class via its define_type function,
 * which must clearly define its members to ensure proper communication.  The
 * body of define_type has three sections:
 *
 * \code
 * void define_type(stapl::typer& t) {
 *   // 1) prelimary calculations
 *   // 2) data member definitions
 *   // 3) offset member definitions
 * }
 * \endcode
 *
 * Section 1 is for performing preliminary calculations and storing temporary
 * values that may be necessary in sections 2 or 3 (e.g., offsets, sizes,
 * etc).  This is necessary because section 2 may modify the contents of
 * members during packing/unpacking, making their use undefined.  If the class
 * being defined inherits from one or more base classes, those base classes'
 * define_types should be explicitly called at the end of this section.
 *
 * Section 2 is for defining data members: variables that store data, either
 * automatically or via dynamic memory allocation.  Section 2 starts with the
 * first call to local or dynamic.  Upon RMI transfer, each data member will
 * be properly packed and communicated to the destination thread.  If a data
 * member is a user-defined object, define_type will be applied recursively to
 * ensure proper transfer of the aggregate type.
 *
 * Section 3 is for defining offset members: pointers used as points of
 * reference within the data members previously defined in section 2 (e.g.,
 * STL vectors store a dynamic array of data, along with an offset pointer
 * referencing one past the array's end).  Section 3 starts with the first
 * call to localOffset or dynamicOffset.  Only the relative address within the
 * type is packaged upon RMI transfer, not the object pointed to, since that
 * object was already defined in section 2.  Attempting to define an offset
 * member to an object not defined in section 2 is undefined behavior.
 *
 * In general, class members should be defined in the order they are declared
 * in the class, although this is not necessary for correctness.  The two
 * instances when this may not be possible are multiple inheritance and offset
 * members, since all offset members must be in section 3, regardless of their
 * declaration within the class body.
 *
 * \warning
 * Arrays and STL sequence iterators are not directly supported by the
 * primitives as RMI arguments.  Arrays are passed as pointers, and don't
 * contain information about their size.  Iterators typically come in pairs,
 * one for the beginning and one for the ending of a sequence, although as
 * arguments the primitives don't have the information to realize they
 * represent a sequence.  Both cases are overcome by using wrapper classes
 * that explictly implement a define_type method that does provide the
 * necessary information.  For example, an array can be wrapped in a class
 * that stores its pointer and size, and the iterators can both be stored in a
 * class that define_types them as a sequence.  Both of these examples are
 * similar to the array example below.
 *
 * \warning
 * If two data members both contain pointers to the same object, that object
 * will be packed twice during RMI transfer, and the resulting object passed
 * to the remotely invoked method will NOT share the common object.  In some
 * cases, such shared pointers are necessary for the structure of the
 * aggregate object, and using offset pointers may still allow for a valid
 * define_type.  For example, each node in a doubly-linked list has two
 * pointers, allowing for forward and backward traversals through the list.
 * Defining both pointers to be dynamic is not valid and could yield
 * disastrous results!  However, defining the next pointer to be dynamic, to
 * recursively define the list, and the previous pointer to be a
 * dynamicOffset, to correctly link the list, is valid.
 *
 * \warning
 * Some implementations use define_type to pack a class into contiguous memory
 * before transfer (e.g., MPI).  This packing may use memcpy, or other
 * low-level mechanisms unaware of C++ classes.  The C++ Standard states that
 * only plain old data (e.g., built-in types and classes without constructors)
 * may be safely memcpy'ed.  However, the purpose of define_type is to give
 * the typer enough information to still safely use memcpy.  The one exception
 * is for classes that use virtual inheritance (as opposed to
 * public/private/protected, multiple and polymorphic inheritance), which may
 * be implemented by a compiler by using relative pointers within the class.
 * Attempting to memcpy such a structure will invalidate the relative offsets,
 * since they will still point to the pre-memcpy locations.  Although it could
 * be possible to implement a safe_memcpy, which uses define_type to look for
 * gaps between members of the class and update those gaps as if they were
 * relative pointers, this operation would be expensive.  As such, the current
 * interface does not allow virtual inheritance in order to provide higher
 * performance.
 *
 * \ex
 * A user-defined struct for holding data, demonstrating local members.
 * \code
 * struct simple {
 *   int a;
 *   double b;
 *   void define_type(stapl::typer& t) {
 *     t.local( a );
 *     t.local( b );
 *   }
 * };
 * \endcode
 *
 * \ex
 * A user-defined array class, demonstrating dynamic and offset members.
 * \code
 * class array {
 *   int* begin;
 *   int* end;
 * public:
 *   array(const int size) {
 *     begin = new int[size];
 *     end = begin + size;
 *   }
 *   void define_type(stapl::typer& t) {
 *     const int size = end - begin;        // section 1
 *     t.dynamic( begin, size );            // section 2
 *     t.dynamicOffset( end, begin, size ); // section 3
 *   }
 * };
 * \endcode
 *
 * \ex
 * A user-defined inherited class, demonstrating how to pack a base class.
 * \code
 * struct simpleDerived : public simple {
 *   char c[10];
 *   void define_type(stapl::typer& t) {
 *     simple::define_type( t );
 *     t.local( c, 10 );
 *   }
 * };
 * \endcode
 **/
class typer {
public:

  /**
   * Define a single local member.
   *
   * \param t reference to the member
   **/
  template<class T> void local(T& t);
  /**
   * Define an array of local members.  It is undefined behavior to set size <= 0.
   *
   * \param t reference to the array
   * \param size size of the array
   **/
  template<class T> void local(T* t, const int size);
  /**
   * Define a single, or an array of, dynamically allocated member(s).  For
   * safety, if \e t equals 0 (i.e., null), nothing will be packed.  Likewise,
   * if size equals 0, nothing will be packed.  It is undefined behavior to
   * set size < 0.
   *
   * \param t reference to the dynamically allocated member
   * \param size size of t if t is an array, 1 otherwise
   **/
  template<class T> void dynamic(T*& t, const int size=1);

  /**
   * Define an offset member that aliases a local member.
   *
   * \param t reference to the offset member
   * \param ref reference to the aliased local member
   **/
  template<class T> void localOffset(T*& t, T& ref);
  /**
   * Define an offset member that aliases an element of a local array.  It is
   * unsafe to try to compute the offset using t (e.g., offset = t - ref),
   * regardless of whether the calculation is in section 1 or 3.  For a
   * discussion of why, see the warning about virtual inheritance above.
   *
   * \param t reference to the offset member
   * \param ref reference to the aliased local array
   * \param offset offset within the array
   **/
  template<class T> void localOffset(T*& t, T* const ref, const int offset=0);
  /**
   * Define an offset member that aliases a dynamically allocated member.
   *
   * \param t reference to the offset member
   * \param ref reference to the aliased dynamic member
   * \param offset offset within the array, or 0 if \e ref is a single element
   **/
  template<class T> void dynamicOffset(T*& t, T* const ref, const int offset=0);
};
#endif

namespace stapl {




//****************************************************************************
// Parallelization Primitives
// - functions to obtain information about threads
//****************************************************************************

/**
 * Returns the calling thread's id in the range 0..p-1, where p is the number
 * of threads currently executing.
 *
 * \complexity 
 * \f$O(1)\f$
 *
 * \return the calling thread's id
 **/
int get_thread_id();


/**
 * Returns the number of threads currently executing.
 *
 * \complexity 
 * \f$O(1)\f$
 *
 * \return the number of threads in use
 **/
int get_num_threads();




//****************************************************************************
// Registration Primitives
// - functions to register objects for communication with the RMI registry
//****************************************************************************

/**
 * Register the given object for RMI communication.  It is the user's
 * responsibility to keep track of the assigned handle and unregister it as
 * necessary.  Trying to communicate with an unregisted object is undefined
 * behavior.
 *
 * This call assumes an SPMD model.  Specifically, when a global object,
 * comprised of one sub-object per thread, tries to register itself, the model
 * expects each thread to perform its sub-object registrations in the same
 * order, hence allowing each corresponding sub-object to have the same
 * handle.  This assumption alleviates concerns with scoping and forcing
 * unique names for global objects, and allows for faster address translation.
 *
 * \warning
 * Although this call is conceptually collective, it does not synchronize,
 * meaning some threads may finish before others.  It is the user's
 * responsibility to utilize synchronization before issuing RMI's using the
 * given handle.  This design occasionally allows for reduced communication
 * overhead (e.g., register multiple objects, perform a single rmi_fence(),
 * then start using the handles safely).
 *
 * \complexity 
 * Amortized \f$O(1)\f$.  If many objects are registered, the registry may
 * need to increase in size.  As such, the registry doubles in size as needed,
 * allowing for amortized constant time registration.
 *
 * \param objectPointer the object to register
 * \return the object's assigned rmiHandle
 **/
//rmiHandle register_rmi_os(void* const objectPointer, std::type_info t);
#ifndef _ARMI_REGISTRY_CHECK_TYPE
 rmiHandle register_rmi_object(void* const objectPointer);


#else
 rmiHandle register_rmi_object(void *const objectPointer, int id);

 rmiHandle register_rmi_object(void* const objectPointer,
			       const std::type_info &objectType);

#endif

/**
 * Update the address of a registered object.  Useful if an object moves
 * within a single thread (e.g., because of a resize or realloc).
 *
 * \complexity 
 * \f$O(1)\f$
 *
 * \param handle the object's rmiHandle
 * \param objectPointer the object's new address
 **/
#ifndef _ARMI_REGISTRY_CHECK_TYPE
 void update_rmi_object(rmiHandle handle, void* const objectPointer);
#else
 void update_rmi_object(rmiHandle const &handle, void* const ojbectPointer, 
			std::type_info const &objectType);
#endif




/**
 * Remove an object from the registry, and return space for new objects.
 * Using the handle of an unregistered object is undefined behavior.
 *
 * \warning
 * Although this call is conceptually collective, it does not synchronize,
 * meaning some threads may finish before others.
 *
 * \complexity 
 * \f$O(1)\f$
 *
 * \param handle the object's rmiHandle
 **/
 void unregister_rmi_object(rmiHandle const &handle);

/**
 * The expected base class for execute_parallel_task().  Execution starts at
 * the \e execute method.  \e rmiHandle is the proper handle to use for
 * inter-parallel_task RMI communication.
 **/
#ifndef P_TASK
#define P_TASK
struct parallel_task {
  /**
   * Virtual destructor for full support of virtual method execute.
   **/
  virtual ~parallel_task() {};

  /**
   * Proper handle to use for communication between the currently executing
   * parallel_tasks.
   **/
  enum rmiHandleConstant { rmiHandle = 0 };

  /**
   * Method defining the work to actually execute.
   **/
  virtual void execute() = 0;
};
#endif

/**
 * Although a program using the communication primitives always executes in
 * parallel, there are times when all threads will be performing the same task
 * in parallel.  This collective call automatically registers the given task
 * object and begins execution.  As such, one threads's task may issue RMI
 * requests to other threads' tasks using \e rmiHandle, instead of using
 * manual object registration.  The call does not, however, imply
 * synchronization before or after execution (i.e., no implied rmi_fence()).
 *
 * Aggregation and polling settings are automatically increased by this call
 * before execution (see set_aggregation and set_poll_rate for more details).
 * In addition, the aggregation or polling settings may be changed during
 * execution.  In both cases, the original values (i.e., from before
 * execute_parallel_task) will automatically be restored upon completion.
 *
 * \complexity 
 * \f$O(1)\f$
 *
 * \param task parallel_task to register and execute
 **/
void execute_parallel_task(parallel_task* const task);

void rmi_spawn_thread(void (*func)(void));

//****************************************************************************
// Synchronization Primitives
// - functions to synchronize threads
//****************************************************************************

/**
 * Override the current polling rate.  Some implementations of the primitives
 * rely on internal polling for scheduling the processing of incoming RMI
 * requests.  This polling occurs during communication calls, such as
 * async_rmi() or sync_rmi().  Polling too much reduces performance by
 * performing wasteful work, while polling too little increases RMI latency.
 *
 * By default, polling is proportional to the aggregation setting while
 * running execute_parallel_task, and 1 otherwise.  In general, the poll rate
 * should not be adjusted unless algorithm specific knowledge deems it
 * necessary or can improve performance.  For example, if bulk communication
 * is evenly distributed between all threads during a computation, then a good
 * setting may be the aggregation factor times the number of threads.
 *
 * \param rate the requested polling rate, 1 in \e rate calls will poll
 **/
void set_poll_rate(const unsigned int rate);


/**
 * Obtain the current polling rate.
 *
 * \return the current polling rate, and 0 if the implementation does
 *         not rely on polling
 **/
unsigned int get_poll_rate();


/**
 * Poll causes the calling thread to check for and process all available RMI
 * requests.  If none are available it returns immediately.  The main purpose
 * of poll is to improve timeliness of request processing for a thread that
 * does not perform much communication, in support of a thread that does.
 **/
void rmi_poll();


/**
 * Wait causes the calling thread to wait for and process the next RMI request
 * to arrive since the previous call to rmi_wait or rmi_fence().  If the next
 * request has already arrived and been processed, rmi_wait returns
 * immediately.  Other messages, such as internal control messages, may be
 * processed while waiting.  Wait provides for point-to-point synchronization.
 **/
void rmi_wait();


/**
 * Fence is a collective call that acts as a barrier for threads, providing
 * all-to-all synchronization.  Additionally, all outstanding RMI requests on
 * all threads are guaranteed to complete by the release.
 **/
void rmi_fence();



//****************************************************************************
// Communication Primitives
// - functions to perform communication between registered objects
//****************************************************************************

/**
 * Asynchronous Remote Method Invocation.  This call starts an RMI request to
 * a remote thread and returns, possibly before completion.  It ignores any
 * return value.  Requests are guaranteed to have complete by the release of
 * rmi_fence().
 *
 * In many cases, and especially when using aggregation settings greater than
 * 1, starting a request does not imply it has been transferred to or executed
 * by the destination thread.  There are three stages of an async_rmi request:
 * creation, issue, and execution.  Only the creation stage is guaranteed to
 * complete when this call completes, which ensures enough information has
 * been gathered and stored to ensure the request may subsequently execute as
 * expected.  After aggregation settings are met, a group of requests is
 * issued to the destination thread, performing the necessary data transfer.
 * An implementation is only required to allow one request group to be
 * in-flight at a time, potentially causing this stage to block while issueing
 * a second group, until a first group is fully transfered.  Once a request
 * group is received by the destination, it is eventually scheduled and
 * executed to maintain the atomicity requirements.
 *
 * Only a 1 argument definition is given here, although 0..N argument versions
 * are supported and included by the specific implementation.  There is no
 * theoretical upper bound on N.  However, each number requires a significant
 * amount of duplicated code.  As such, implementations may provide only a
 * small number of arguments (e.g., up to 4), with the option for the user to
 * make simple modifications to increase the number if necessary.
 *
 * \param destThread destination thread id
 * \param handle rmiHandle of the desired object
 * \param memberFuncPtr method to invoke on the desired object
 * \param a1 parameter required by \e memberFuncPtr
 *
 * \warning
 * Some compilers have problems with function template argument deduction.  If
 * your compiler issues such an error, it may be related to several issues:
 * - multiple member functions of the class match \e memberFuncPtr's name
 * - \e a1 requires implicit casting before properly matching \e memberFuncPtr's
 *   expected argument
 *
 * A simple solution is to specify \e memberFuncPtr more exactly (i.e.,
 * specify the correct types for Rtn, Class, Arg1):
 * \code async_rmi( ..., ..., (Rtn (Class::*)(Arg1))Class::memberFuncPtr, ...) \endcode
 **/
template<class Class, class Rtn, class Arg1>
void async_rmi(const int destThread, const rmiHandle objHandle,
	       Rtn (Class::* const memberFuncPtr)(Arg1),
	       const typename rmi_arg<Arg1>::type& a1);



/**
 * Flush all remaining aggregated RMI requests on the calling thread.  This is
 * useful upon completion of a bulk communication phase of async_rmi requests
 * to ensure the final few messages, which will be fewer than the current
 * aggregation setting, to each thread are in transit.  Note that rmi_fence
 * implies a flush.
 **/
void rmi_flush();


/**
 * Synchronous Remote Method Invocation.  This call starts an RMI request to a
 * remote thread and waits to receive the return value.  Other incoming
 * requests may be processed while waiting for the return value, ensuring
 * corresponding sync_rmi's between two threads will not cause deadlock.
 *
 * Only a 1 argument definition is given here, although 0..N argument versions
 * are supported and included by the specific implementation.  There is no
 * theoretical upper bound on N.  However, each number requires a significant
 * amount of duplicated code.  As such, implementations may provide only a
 * small number of arguments (e.g., up to 4), with the option for the user to
 * make simple modifications to increase the number if necessary.
 *
 * \param destThread destination thread id
 * \param handle rmiHandle of the desired object
 * \param memberFuncPtr method to invoke on the desired object
 * \param a1... parameter(s) required by \e memberFuncPtr
 *
 * \warning
 * Some compilers have problems with function template argument deduction.  If
 * your compiler issues such an error, it may be related to several issues:
 * - multiple member functions of the class match \e memberFuncPtr's name
 * - \e a1 requires implicit casting before properly matching \e memberFuncPtr's
 *   expected argument
 *
 * A simple solution is to specify \e memberFuncPtr more exactly (i.e.,
 * specify the correct types for Rtn, Class, Arg1):
 * \code sync_rmi( ..., ..., (Rtn (Class::*)(Arg1))Class::memberFuncPtr, ...) \endcode
 **/
class OpaqueBase {
 public:
  virtual void setRtn(void *retBuf) = 0;
};

 
template<class Rtn> 
class OpaqueHandle : public OpaqueBase { 
  
 public:
  bool _ready;
  //typename arg_traits<Rtn>::return_storage _value;
  Rtn _value;

 public:
  OpaqueHandle() {
    _ready = false;
  }

  bool ready(bool poll = true) {
    if (!_ready) {
      if (poll) {
	rmi_flush(); rmi_poll();
      }
    }
    return _ready;
  };

  Rtn value(void) { return _value; }

  void setRtn(void *retBuf) {
#ifdef _STAPL_THREAD
    stapl_assert(0, "Thread Primitives Shouldn't Be Calling OpaqueHandle::setRtn");
#else 
    mpi_primitives::arg_storage<Rtn>* r  = 
      reinterpret_cast<mpi_primitives::arg_storage<Rtn>*>(retBuf); 
     _value = r->unpack_copy( r ); 
     _ready = true; 
#endif
  }
};


template<class Class, class Rtn, class Arg1>
void sync_rmi(const int destThread, const rmiHandle objHandle,
                           Rtn (Class::* const memberFuncPtr)(Arg1),
                           const typename rmi_arg<Arg1>::type& a1,
	                   OpaqueHandle<Rtn> *handle);


template<class Class, class Rtn, class Arg1>
Rtn sync_rmi(const int destThread, const rmiHandle objHandle,
	     Rtn (Class::* const memberFuncPtr)(Arg1),
	     const typename rmi_arg<Arg1>::type& a1);


/**
 * Reduction Remote Method Invocation.  This collective call performs a
 * parallel reduction by using the \e memberFuncPtr to combine \e in from all
 * threads.  It assumes that the size of \e in and \e out are equal, and that
 * sizes do not change during the course of the reduction.
 *
 * \param in input to the reduction
 * \param out output to the reduction
 * \param handle rmiHandle of the desired object
 * \param memberFuncPtr a reduction operation, assumed to be associative
 * \param commutative \e true means \e memberFuncPtr can be performed in any
 *        order (i.e., is commutative), and may enable the implementation to
 *        perform some optimizations not otherwise possible
 * \param rootThread by default, all threads store the results of the
 *        reduction into \e out; manually setting \e rootThread causes only the
 *        specified thread to obtain and store the results, which may reduce
 *        communication overhead and allows \e out to be null (i.e., 0)
 **/
template<class Class, class Arg>
void reduce_rmi(Arg* in, Arg* out, const rmiHandle objHandle,
		void (Class::* const memberFuncPtr)(Arg*, Arg*),
		const bool commutative, const int rootThread);


/**
 * Broadcast Remote Method Invocation.  This collective call performs a
 * broadcast of the \e rootThread's \e inout as an argument to all other
 * thread's \e memberFuncPtr.  It assumes all thread's \e inout's are of equal
 * size.
 *
 * \param inout input of the broadcast
 * \param count number of contiguous elements in \e inout
 * \param handle rmiHandle of the desired object
 * \param memberFuncPtr a broadcast operation called by all threads other than
 *        \e rootThread.  Argument one is the thread's local \e inout.
 *        Argument two is the rootThread's \e inout, and argument three is the
 *        count.  Modifications to the local \e inout will remain after
 *        completion.
 * \param rootThread the thread broadcasting data
 **/
template<class Class, class Arg>
void broadcast_rmi(Arg* inout, const int count, const rmiHandle objHandle,
		   void (Class::* const memberFuncPtr)(Arg*, Arg*, const int),
		   const int rootThread);


void abort_rmi(char* str);
/**
 * Override the current aggregation settings.  async_rmi() requests can be
 * internally buffered and issued in groups to reduce network congestion
 * caused by many small requests.  Requests are aggregated per destination
 * thread, meaning a setting of 5 issues requests in groups of 5 to a specific
 * destination thread.
 *
 * By default, aggregation is the maximum possible (as determined by internal
 * buffers) while running within execute_parallel_task, and 1 otherwise.  If
 * possible, user-defined aggregation will be satisfied, as constrained by the
 * size of internal buffers.  For example, it may not be possible to aggregate
 * several large requests given small internal buffers.  Consult your specific
 * implementation for ways of increasing internal buffers.
 *
 * \param agg the requested aggregation setting
 * \return the actual aggregation setting obtained, which may be less than
 *         requested if internal buffers are not large enough
 **/
unsigned int set_aggregation(const unsigned int agg);


/**
 * Obtain the current aggregation setting.
 *
 * \return the current aggregation setting
 **/
unsigned int get_aggregation();


/**
 * Make a suggestion to the implementation about scheduling nested RMI
 * requests.  A nested RMI occurs when an executing request blocks (e.g.,
 * sync_rmi), and the implementation schedules another RMI to execute instead
 * of doing nothing.  The new request may also block, allowing yet another
 * nested request to be scheduled.  Although nested requests allow useful work
 * to occur, they have the potential to starve the original request, by
 * continuing to do work after the original request's reason for blocking is
 * satisfied.
 *
 * By default, an implementation may schedule an unlimited number of levels of
 * nested requests.  This function gives a suggestion for the upper bound of
 * the number of nested levels allowed.
 *
 * \param nest the requested nesting level, -1 resets to the default
 * \return the actual nesting level obtained
 **/
int set_nesting(const int nest);


/**
 * Obtain the current upper bound on the nesting level.
 *
 * \return the current nesting level
 **/
int get_nesting();



/** @} **/
} // end namespace
#endif // _PRIMITIVES_H

