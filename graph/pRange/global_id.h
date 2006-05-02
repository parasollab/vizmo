#ifndef GLOBAL_ID_H
#define GLOBAL_ID_H

/**
 * \file global_id.h
 * \date 01/15/2003
 * \ingroup stapl
 * \brief Contains the class implementing the mechanism used by STAPL 
 * objects to obatain a unique global id.
*/

#include <runtime.h>

using namespace std;

namespace stapl {

/**
 * @addtogroup prange_gid
 *
 * @{
 */

/**
 * The global_id class is used by the pRange to generate unique global ids 
 * for its subranges.  When the user passes a global id in with a boundary 
 * that id is used as the global id of the subrange and the global_id class
 * is used to generate a global id of the top level pRange.  Subranges 
 * added to the pRange will also use the class.
 *
 * \nosubgrouping
 */
class global_id {
private:
  /**
   * \brief Number of consecutive global ids to use.
   */
  int offset;

  /**
   * \brief The global id past the end of the block of global ids used.
   *
   * The global id of last_id is used as the first id on the next thread.
   * When the next_id to be returned is equal to last_id the thread will 
   * move to the next available block of global ids to ensure the global id
   * returned is unique across all threads.
   */
  int last_id;

  /**
   * \brief The next global id to return when requested.
   */
  int next_id;

public:
  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * @param t typer object used by ARMI for automated packing.
   * @return void
   */
  void define_type(stapl::typer &t) {
    t.local(offset);
    t.local(last_id);
    t.local(next_id);
  }

  /**
   * \brief Constructor
   *
   * @param o Optional parameter sets the number of global ids that are 
   * returned before the thread jumps to the next sequence of global ids to
   * use.
   */
  global_id(const int& o = 10000) : offset(o) { 
    int thread_id = get_thread_id();
    int numthreads = get_num_threads();
    next_id = thread_id * offset;
    last_id = next_id + offset - 1;
  }

  /**
   * \brief Return the number of consecutive global ids returned.
   */
  inline int get_offset() const { return offset; }

  /**
   * \brief Return the next unused global id
   *
   * If the end of the block of global ids has been reached compute the 
   * global id at the start of the next block.
   */
  int get() { 
    if (next_id < last_id) {
      return next_id++;
    } else {
      int thread_id = get_thread_id();
      int numthreads = get_num_threads();
      next_id += offset * numthreads;
      last_id = next_id + offset - 1;
      return next_id++;
    }
  }
};

/**
 * @}
 */

} //end namespace stapl
#endif
