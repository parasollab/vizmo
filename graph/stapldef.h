#ifndef _STAPLDEF_H
#define _STAPLDEF_H



/**
 * In general, pranges are distributed into DIST_FACTOR * p segments,
 * where p = the number of threads.
 **/
#define DIST_FACTOR 1


/**
 * Utilize HOARD as the underlying memory allocator
 * With the upgrade to stlport, this feature may no longer work...
 **/
#define HOARD_MALLOC_HP


/**
 * Vector elements are stored as pointers to elements instead of just elements.
 * With the upgrade to stlport, this feature may no longer work...
 **/
//#define _pointer_members


/**
 * this number decides (in \ref p_for_all) the level of parallelism for
 * commit phase.
 *
 * \code
 * if # < CONTAINER_NUMBER_THRESHOLD_FOR_PARALLEL_COMMIT
 *    for each pcontainer do
 *          parallel commit
 * if # > CONTAINER_NUMBER_THRESHOLD_FOR_PARALLEL_COMMIT
 *    PARALLEL for each pcontainer pardo
 *          sequential commit
 * \endcode 
 **/
#define CONTAINER_NUMBER_THRESHOLD_FOR_PARALLEL_COMMIT 30


/** 
 * This flag decides when the signature of a pcontainer is added to the global
 * instances vector.  if ON then at the time of a parallel method which
 * requires privatization if OFF then at the time of instantianting the
 * pcontainer.
 **/
#define CONTAINER_SIGNATURE_ON_THE_FLY


/**
 * This flag dictates whether or not the signatures are stored per processor
 * (as each processor touches a p$ if ON then each processor has a vector of
 * its own touched pcontainers, and when its time to commit, these p vectors
 * of signatures are "unique"d in a single vector on which the commit phase
 * operates if OFF then the signatures are added in an atomic operation in a
 * global vector of signatures 
 *
 * \warning
 * If this flag is ON then CONTAINER_SIGNATURE_ON_THE_FLY needs to be OFF
 **/
//#define ADD_SIGNATURE_PARALLEL


/**
 * Maximum number of processors in the system
 **/
#define MAX_PROCESSORS 64


#endif
