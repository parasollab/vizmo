#ifndef P_RANGE_H
#define P_RANGE_H

/**
 * \file pRange.h
 * \author Timmie Smith
 * \date 07/02/02
 * \ingroup stapl
 * \brief This file contains the pRange class.
 *
 * pRange boundary classes can be found in pRangeBoundaries.h.  A parallel DDG 
 * class can be found in pDDG.h if a custom DDG class isn't needed.
 */

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>

#include <runtime.h>

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>

#include "pDDG.h"
#include "pRangeBoundaries.h"
#include "global_id.h"


namespace stapl {

/**
 * @addtogroup prange
 * 
 * @{
 */

/**
 * The pRange class is the only class used to implement the STAPL pRange.
 * The class is templated with the boundary, ddg, and container types.  
 *
 * The Boundary type is a class that is used to implement the scoped data 
 * space a pRange is defined on.  The boundary class must implement the 
 * interface defined in base_boundary.  Several boundary classes are 
 * provided in pRangeBoundaries.h.
 *
 * The DDG type is a class that is used to store the dependence 
 * relationships between subranges of the pRange, if they exist.  The DDG 
 * type must implement the interface defined in DDG class defined in 
 * pDDG.h.  If the user needs only simple DDG functionality then a parallel
 * DDG class, pDDG, is provided in pDDG.h.
 *
 * The Container type is the pContainer class that stores the data on which
 * the pRange is defined.  The container type is needed because the pRange 
 * stores a pointer to the container.  This pointer is used by the pRange 
 * to invoke pContainer methods.  This allows the pRange to trigger events 
 * such as the redistribution of the pContainer.
 *
 * \nosubgrouping 
 */
template <class Boundary, class Container,
          class SubrangeDDG = pDDG<DDGTask, DDGWeight>, 
          class ElementDDG = DDG<DDGTask, DDGWeight>,
          class Subrange = void>
class pRange {
public:

  //Type definitions for the class

  /**
   * \brief Data element iterator type
   *
   * Iterators of this type are used to iterate between elements in the 
   * pContainer.  The type could be obtained from Container::iterator.  It 
   * is obtained from the Boundary class to allow for greater flexibility.
   */
  typedef typename Boundary::iteratorType  iteratorType;

  /**
   * \brief Class used to represent the boundary of a subrange.
   *
   * The boundary class is used to allow iteration through a subrange using 
   * an iterator of iteratorType.  The boundary object provides information
   * needed to determine which element in the subrange should be processed 
   * first, and when all of the elements in the subrange have been 
   * processed.  The boundary class is a general form of the pair of 
   * iterators returned by the STL container methods begin() and end().
   */
  typedef Boundary boundaryType;

  /**
   * \brief The type of pContainer used to store the data on which the 
   * pRange is defined.
   */
  typedef Container ContainerType;

  /**
   * \brief Type definition of the current class to allow for easy self 
   * reference.
   */
  typedef pRange<Boundary, Container, SubrangeDDG, ElementDDG, Subrange>  pRangeType;

  /**
   * \brief Type definition of the current class to allow for easy self 
   * reference.
   */
  typedef pRange<Boundary, Container, SubrangeDDG, ElementDDG, Subrange>  thisType;

  /**
   * \brief Type definition of the subrange type.
   */
  typedef typename boost::mpl::if_c<boost::is_same<Subrange, void>::value, pRangeType, Subrange>::type subrangeType;

  /**
   * \brief Type of dependence graph that will be used to store the 
   * dependence relationships between the subranges of the pRange.
   */
  typedef SubrangeDDG  DDGType;

  /**
   * \brief Type of dependence graph that will be used to store the 
   * dependence relationships between elements contained in the subrange.
   */
  typedef ElementDDG  ElemDDGType;

protected:

  /**
   * \brief Collection of subranges.
   *
   * A pRange can be partitioned into a collection of disjoint subranges.  
   * This data member stores the collection of subranges after a pRange has
   * been partitiond.  The data member is private since each pRange class
   * defines this member itself to easily allow the subranges to be 
   * instances of the correct type.
   */
  vector<subrangeType>  sub_p_ranges;


  /**
   * \brief Handle used by RMI communication calls.
   *
   * This object is instantiated in the constructors by calls to 
   * register_rmi_object().
   */
  rmiHandle  handle;

  /**
   * \brief Pointer to the parent pRange.
   *
   * When the pRange is constructed a pointer to the parent pRange may be
   * provided. If a pointer to a parent pRange is provided then the 
   * current subrange isn't registered within ARMI.  The current subrange 
   * uses the rmiHandle from its parent to communicate with siblings in 
   * another thread.
   */
  void*  parent;

  /**
   * \brief The version number of the pContainer on which the pRange is 
   * defined.
   *
   * The container version is obtained from the pContainer when the pRange 
   * is constructed or (re)distributed.  If the pRange's container version 
   * is less than the pContainer's current version then the pRange is 
   * invalid and must be repartitioned before it can be used.
   */
  int  container_version;

  /**
   * \brief The global id number of the pRange.
   *
   * The global id of a pRange need only be unique within a pRange.  The 
   * global id is used to communicate between subranges in a pRange.  It is
   * also used by the user to request specific subranges of the pRange.  No
   * communication is permitted between pRanges that are not subranges of a
   * common pRange, therefore the global id does not need to be unique 
   * among all pRanges in a user's application.
   */
  int  gid;

  /**
   * \brief Global id generator for the pRange.
   *
   * The global id of a subrange must only be unique within the pRange.  
   * The gid_producer is an instance of the global_id class, which provides
   * a simple mechanism for assigning unique id numbers to subranges.  It 
   * ensures that the global ids used on one thread of computation are not 
   * used on any other thread of computation.
   */
  global_id  gid_producer;

  /**
   * \brief The id number of the thread where the pRange is currently 
   * allocated.
   *
   * The thread id of a pRange can change when a work stealing scheduler is
   * used and a thread of computation moves the pRange from one thread to 
   * another.  The member may be used in the future as a means of 
   * indicating which the thread to which the subrange should be sent.
   */
  int  thread_id;

  /**
   * \brief Pointer to the container that owns the data accessed by the pRange.
   * \warning This is NULL if the owner is not an STAPL (STL) container.
   *
   * The pointer to a container is used to call pContainer methods.  This 
   * is used to initiate a redistribution of the pContainer, and to obtain 
   * the pContainer version number, which is used to test the validity of 
   * the pRange.
   */
  Container*  container;

  /**
   * \brief Boundary of the data represented by the pRange.
   *
   * The boundary completely describes all of the elements contained within
   * the pRange.  The boundary is able to provide the first element in the 
   * subrange to process, and is able to indicate when all elements in the 
   * subrange have been processed. If the pRange has been partitioned the 
   * subrange boundaries are used instead of this boundary.
   */
  boundaryType  boundary;

  /**
   * \brief DDG between subranges.
   *
   * The DDG between subranges is used in calls to p_for_all().  An edge 
   * from subrange A to subrange B indicates to p_for_all() that subrange A
   * must be processed before subrange B may be processed.  If the 
   * algorithm the pRange is used to execute does not have any data 
   * dependencies then the DDG should contain a vertex for each subrange 
   * and no edges.
   */
  DDGType*  subrange_ddg;

  /**
   * \brief DDG between elements in the subrange.
   *
   * The DDG between elements in the subrange is used by work functions 
   * passed into p_for_all() and p_for_each() to ensure the elements in the
   * subrange are processed in the correct order.  An edge from element A to 
   * element B indicates to the work function that element A must be 
   * processed before element B may be processed.
   *
   * The element DDG and the subrange DDG do not exist together within the
   * same pRange.  If a subrange DDG is present then the pRange contains a
   * set of subranges instead of a boundary, and therefore there are no 
   * elements for the element DDG to be defined on.  If the pRange is not 
   * partitioned and instead of a set of subranges it contains a boundary 
   * then the element DDG can be defined to specify the partial ordering in
   * which the elements must be processed.
   */
  ElemDDGType*  element_ddg;

public:
  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \warning The pointers to the parent pRange and the pContainer
   * the pRange is defined on are currently not set correctly.  These 
   * issues must be addressed before a pRange can be sent between 
   * threads.  
   *
   * @param t typer object used by ARMI for automated packing.
   * @return void
   */
  void define_type(stapl::typer& t) {
    t.local(sub_p_ranges);
    t.local(handle);
    t.dynamic(parent, 0);
    t.local(container_version);
    t.local(gid);
    t.local(gid_producer);
    t.local(thread_id);
    t.dynamic(container, 0);
    t.local(boundary);
    t.dynamic(subrange_ddg);
    t.dynamic(element_ddg);
  }

  /**
   * \brief Default constructor
   *
   * The pRange constructed is not valid, and should only be used after 
   * subranges defined on the container provided are added via the 
   * add_subrange method.
   *
   * A pointer to the container the pRange is defined on is not provided, 
   * therefore the container's methods will be inaccessible and the pRange 
   * will be unable to trigger events such as pContainer redistribution.
   *
   * This constructor is needed because several classes that contain 
   * pRanges have constructors that are not given all the information 
   * necessary to construct their pRange member.  This violates the 
   * principle that a constructor must provide an object that is valid, 
   * but it is programmatically useful.
   *
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   */
  pRange(const int g_offset = 10000) 
    : handle(-1), parent(NULL), gid_producer(g_offset),
      container(NULL), container_version(-1),
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    handle = register_rmi_object(this);
    thread_id = stapl::get_thread_id();
    gid = gid_producer.get();
    subrange_ddg = new DDGType();
  }

  /**
   * \brief Constructor for simple pRange on a non-STAPL container.
   *
   * This creates a pRange with one subrange per thread.
   *
   * A pointer to the container the pRange is defined on is not provided, 
   * therefore the container's methods will be inaccessible and the pRange
   * will be unable to trigger events such as pContainer redistribution.
   *
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  pRange(const boundaryType& p_range_boundary, 
         const int g_offset = 10000, 
         void* const p = NULL)
    : handle(-1), parent(p), gid_producer(g_offset), 
      container(NULL), container_version(0),
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    thread_id = stapl::get_thread_id();
    //global_id = global_id_offset * thread_id;
    gid = gid_producer.get();
    if (NULL == parent) {
      handle = register_rmi_object(this);
      int next_gid = gid_producer.get();
      subrangeType tmp(next_gid, p_range_boundary, g_offset, this);
      sub_p_ranges.push_back(tmp);
      subrange_ddg = new DDGType();
      subrange_ddg->add_task(next_gid);
    } else {
      boundary = p_range_boundary;
    }
  }

  /**
   * \brief Constructor for simple pRange on a STAPL container.
   *
   * This creates a pRange with one subrange per thread.
   *
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  pRange(Container* const c, 
         const boundaryType& p_range_boundary, 
         const int g_offset = 10000, 
         void* const p = NULL)
    : handle(-1), parent(p), gid_producer(g_offset),
      container(c), container_version(0),
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    //container_version = c->getDistributionVersion();
    thread_id = stapl::get_thread_id();
    gid = gid_producer.get();
    sub_p_ranges.clear();
    if (NULL == parent) {
      handle = register_rmi_object(this);
      int next_gid = gid_producer.get();
      subrangeType tmp(next_gid, c, p_range_boundary, g_offset, this);
      sub_p_ranges.push_back(tmp);
      subrange_ddg = new DDGType();
      subrange_ddg->add_task(next_gid);
    } else {
      boundary = p_range_boundary;
    }
  }

  /**
   * \brief Constructor for an incomplete pRange on a STAPL container.
   *
   * The pRange constructed contains no boundary or subranges, and 
   * therefore abstracts no data.  Subranges defined on the container 
   * provided will be added via the add_subrange method after the pRange is
   * constructed.  The pRange is assumed to be a root pRange. Its parent is
   * set to NULL and it is registered with ARMI.
   *
   * This violates the principle that a constructor must provide an object
   * that is valid, but it is programmatically useful.
   *
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   */
  pRange(Container* const c, 
         const int g_offset = 10000)
    : handle(-1), parent(NULL), gid_producer(g_offset),
      container(c), container_version(0),
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    thread_id = stapl::get_thread_id();
    gid = gid_producer.get();
    handle = register_rmi_object(this);
    subrange_ddg = new DDGType();
  }

  /**
   * \brief Constructor for simple pRange on a non-STAPL container.
   *
   * This creates a pRange with one subrange per thread and assigns its 
   * global id to the value passed in to the method.
   *
   * @param g The global id that should be assigned to the 
   * subrange that contains the boundary passed in.  The root pRange will 
   * have an automatically generated global id.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  pRange(const int g,
         const boundaryType& p_range_boundary, 
         const int g_offset = 10000,
         void* const p = NULL)
    : handle(-1), parent(p), gid(g), gid_producer(g_offset),
      container(NULL), container_version(0), 
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    if (NULL == parent) {
      handle = register_rmi_object(this);
      int next_gid = gid_producer.get();
      gid = next_gid;
      subrangeType tmp(g, p_range_boundary, g_offset, this);
      sub_p_ranges.push_back(tmp);
      subrange_ddg = new DDGType();
      subrange_ddg->add_task(g);
    } else {
      boundary = p_range_boundary;
    }
    thread_id = stapl::get_thread_id();
  }

  /**
   * \brief Constructor for simple pRange on a STAPL container.
   *
   * This creates a pRange with one subrange per thread and assigns its 
   * global id to the value passed in to the method.
   *
   * @param g The global id that should be assigned to the 
   * subrange that contains the boundary passed in.  The root pRange will 
   * have an automatically generated global id.
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  pRange(const int g,
          Container* const c, 
          const boundaryType& p_range_boundary, 
          const int g_offset = 10000, 
          void* const p = NULL)
    : handle(-1), parent(p), gid(g), gid_producer(g_offset),
      container(c), 
      sub_p_ranges(), subrange_ddg(NULL), element_ddg(NULL)
  { 
    if (NULL == parent) {
      handle = register_rmi_object(this);
      int next_gid = gid_producer.get();
      gid = next_gid;
      subrangeType tmp(g, c, p_range_boundary, g_offset, this);
      sub_p_ranges.push_back(tmp);
      subrange_ddg = new DDGType();
      subrange_ddg->add_task(g);
    } else {
      boundary = p_range_boundary;
    }
    thread_id = stapl::get_thread_id();
    //container_version = c->getDistributionVersion();
  }

  /**
   * \brief Copy constructor that uses the copy constructor of all data 
   * members except the rmiHandle and the DDG pointer.  
   *
   * The default constructors of those members is called, and then the 
   * assignment operator is used to initialize them.  This is done to 
   * avoid having multiple pRanges with the same rmiHandle or sharing a 
   * DDG.  DDGs cannot be shared between pRanges since the Executor used in
   * p_for_all is capable of processing multiple pRanges concurrently.  A 
   * shared DDG in this case would lead to unexpected behavior as some 
   * subranges in both pRanges would not be processed by the Executor.
   */
  pRange(const pRangeType& pr) 
    : handle(-1), parent(pr.parent), 
      gid(pr.gid), gid_producer(pr.gid_producer), thread_id(pr.thread_id),
      container(pr.container), container_version(container_version),
      boundary(pr.boundary), 
      sub_p_ranges(pr.sub_p_ranges), subrange_ddg(NULL), element_ddg(NULL)
  {
    if (NULL == parent) {
      if (NULL != pr.subrange_ddg)
        subrange_ddg = new DDGType(*pr.subrange_ddg);
      handle = register_rmi_object(this);
    }
    if (NULL != pr.element_ddg)
      element_ddg = new ElemDDGType(*pr.element_ddg);
  }

  /**
   * \brief Assignment operator that uses the assignement operator of all 
   * data members except the rmiHandle and the DDG pointer.  
   *
   * The DDG pointer is assigned to NULL and the rmiHandle to -1, and then 
   * the assignment operator is used again to initialize them.  This is 
   * done to avoid having multiple pRanges with the same rmiHandle or 
   * sharing a DDG.  DDGs cannot be shared between pRanges since the 
   * Executor used in p_for_all is capable of processing multiple pRanges 
   * concurrently.  A shared DDG in this case would lead to unexpected 
   * behavior as some subranges in both pRanges would not be processed by 
   * the Executor.
   */
  pRangeType& operator=(const pRangeType& other) {
    //guard against self assignment
    if (this != &other) {
      parent = other.parent;
      subrange_ddg = NULL;
      element_ddg = NULL;
      if (NULL == parent) {
        if (-1 == handle) 
          handle =  register_rmi_object(this);
        if (NULL != other.subrange_ddg)
          subrange_ddg = new DDGType(*other.subrange_ddg);
      }
//        element_ddg = other.element_ddg;
      if (NULL != other.element_ddg)
        element_ddg = new ElemDDGType(*other.element_ddg);
      container_version = other.container_version;
      gid = other.gid;
      gid_producer = other.gid_producer;
      thread_id = other.thread_id;
      container = other.container;
      boundary = other.boundary;
      sub_p_ranges = other.sub_p_ranges;
    }
    return *this;
  }

  /**
   * \brief Destructor unregisters the pRange if it has been registered.
   */
  ~pRange() { 
    if ((parent == NULL) && (-1 <= handle)) {
      unregister_rmi_object(handle); 
    }
    if (NULL != subrange_ddg) delete subrange_ddg;
    if (NULL != element_ddg) delete element_ddg;
  }

  /**
   * \brief Determine if the pRange is invalid.
   */
  inline bool is_invalid() const { 
    if (container_version < 0)
      return true;
//    if (container_version == 0)
      return false;
//    if (container_version > 0)
//      return (container->getDistributionVersion() != container_version); 
  }

  /**
   * \brief Get the pContainer version number from the pRange.
   */
  inline int get_container_version() const { return container_version; }

  /**
   * \brief Set the pContainer version number from the pRange.
   */
  inline void update_container_version() { 
//    container_version = container->getDistributionVersion(); 
  }

  /**
   * \brief Signal the pContainer to resize the local part to the size 
   * given.
   *
   * @param s The target size of the local part of the pContainer.
   * @note Leaf pRanges have to be rebuilt after the resize is complete.
   */
  inline void resize(const int s) {
    this->container->resize(s);

    //FIX ME!!!!

    //We need to consider the different boundary types.  Right now I assume
    //the boundary type is a linear boundary.  I also assume there is only 
    //one leaf pRange per thread.
    if (sub_p_ranges.size() > 0) {
      typename vector<pRangeType>::iterator sr = sub_p_ranges.begin();
      boundaryType& b = sr->get_boundary();
      b.SetData(container->local_begin(), 
                container->local_end(),
                s);
    } else {
      pRangeType* sr = this;
      boundaryType& b = sr->get_boundary();
      b.SetData(container->local_begin(), 
                container->local_end(),
                s);
    }
  }

  /**
   * \brief Get the global id of the pRange.
   */
  inline int get_global_id() const { return gid; }

  /**
   * \brief Get the global id generator used by the pRange.
   */
  inline global_id& get_gid_producer() { return gid_producer; }

  /**
   * \brief Get the thread id the pRange is assigned.
   */
  inline int get_thread_id() const { return thread_id; }

  /**
   * \brief Set the thread id the pRange.
   *
   * @param t The thread id to the pRange will be given.
   */
  inline void set_thread_id(const int tid) { thread_id = tid; }

  /**
   * \brief Get the RMI handle used by the pRange.
   *
   * The handle used by subranges is the same handle used by the root 
   * pRange.
   */
  inline rmiHandle get_handle() { return handle; }

  /**
   * \brief Get the pRange boundary.
   * 
   * The method is only valid if the pRange is a leaf pRange.  If the 
   * pRange contains subranges, then no boundary is available.
   *
   * \warning The method does not currently check if the pRange contains a 
   * valid boundary before returning.
   */
  inline boundaryType& get_boundary() { return boundary; }

  /**
   * \brief Set the pRange boundary.
   */
  inline void set_boundary(const boundaryType& b) { boundary = b; }

  /**
   * \brief Get the DDG defined on pRange subranges.
   */
  inline DDGType& get_ddg() { return *subrange_ddg; }

  /**
   * \brief Set the DDG defined on pRange subranges.
   *
   * @param s A DDG as described in the data members section above.
   */
  inline void set_ddg(DDGType* s) { subrange_ddg = s; }

  /**
   * \brief Get the DDG defined on elements within a subrange.
   */
  inline ElemDDGType& get_element_ddg() { return *element_ddg; }

  /**
   * \brief Set the DDG defined on elements within a subrange.
   *
   * @param s A DDG as described in the data members section above.
   */
  inline void set_element_ddg(ElemDDGType* s) { element_ddg = s; }


// This set of methods operates on the subranges of the pRange.  They
// must be implemented in each pRange class.
// BEGIN methods operating on subranges.

  //***********************************************************************
  /* NEW FUNCTIONS TO BE IMPLEMENTED - FOR LOAD SCHEDULER */

  /** 
   * \brief Calculate the size of data enclosed by this pRange 
   * and its subranges.
   */
  int local_size() { 
    if (sub_p_ranges.size() == 0)
      return boundary.size();
    else {
      int size(0);
      typename vector<subrangeType>::iterator i = sub_p_ranges.begin();
//FIX ME      for (; i != sub_p_ranges.end(); ++i)
//FIX ME        size += i->local_size();
      return size;
    }
  } 
   
  void send_subrange(const int gid, const int pid) {
    /*
     * SEND THE SUBRANGE GIVEN BY "ID" TO PROCESSOR GIVEN BY "_pid"
     */
  }

  void send_subranges(const vector<int>& gids, const int pid) {
    /*
     * SEND THE SUBRANGES GIVEN BY the vector "tosend" TO PROCESSOR GIVEN 
     * BY "_pid"
     */
  }
 
  /* END OF NEW FUNCTIONS TO BE IMPLEMENTED - FOR LOAD SCHEDULER */
  //***********************************************************************

  /**
   * \brief Get the collection of subranges.
   *
   * \warning The method does not currently check if the pRange contains a 
   * collection of subranges before returning.
   */
  inline vector<subrangeType>& get_subranges() { return sub_p_ranges; }

  /**
   * \brief Get the subrange whose global id is equal to the specified value.
   *
   * @param id The global id of the subrange to return.
   * @note pVector lookups with find_if will be expensive if there is a 
   * large number of subranges.  Something to consider is storing the 
   * pRanges in a map with their global id as the key to speed up queries.
   */
  inline subrangeType& get_subrange(const int id) {
    for ( typename vector<subrangeType>::iterator i = sub_p_ranges.begin(); 
                                                 i != sub_p_ranges.end(); 
                                               ++i) {
      if ((*i).get_global_id() == id)
        return *i;
    }
    cout << "prange leaf not found: " << id << " on processor " 
         << stapl::get_thread_id() << endl;
    return *(sub_p_ranges.end());
  }

  /**
   * \brief Add the subrange to the set of subranges.
   *
   * @param subrange The subrange to be added to the pRange.
   */
  inline void add_subrange(subrangeType& subrange) {
    sub_p_ranges.push_back(subrange);
    subrange_ddg->add_task(subrange.get_global_id());
  }

  /**
   * \brief Get the number of subranges.
   */
  int get_num_subranges() const {
    return sub_p_ranges.size();
  }

  /**
   * \brief partition the pRange into subranges.
   *
   * Create num_ranges subranges on each thread from the current subrange.
   * If num_ranges == 1 then nothing is * done.
   *
   * @param num_subranges The number of subranges to create on each thread.
   * \warning This function is not implemented.
   */
  void partition(const int num_subranges = 1) {
    int id = stapl::get_thread_id();
    int line = 0;

    if (1 == num_subranges)
      return;
    map<PID,vector<VID> > dist_info;

    if (id == 0) {
      container->GetDistributionInfo(dist_info);
      container->DisplayDistributionInfo(dist_info);
                       }  
      for(map<PID,vector<VID> >::const_iterator i = dist_info.begin(); 
                                                i != dist_info.end();
                                              ++i) {
      cout << "Processor " << i->first << ":" << endl;
      for(vector<VID>::const_iterator j = i->second.begin();
                                      j != i->second.end();
                                      ++j) {
        cout << "  " << *j << endl;
      }
    }
  }

  /**
   * \brief partition the pRange into subranges.
   *
   * Create subranges from the current subrange whose sizes are given in 
   * the vector passed to the method.
   *
   * @param subrange_sizes Vector of subrange sizes.  The size of the 
   * vector is the number of subranges that will be created.
   * \warning This function is not implemented.
   */
  void partition_like(const vector<int>& subrange_sizes) { }

// END methods operating on subranges.


  bool operator== (const pRangeType &prange) const {
    if ((container_version == prange.get_container_version()) &&
        (gid == prange.get_global_id()) &&
        (thread_id == prange.get_thread_id()) &&
        (boundary == prange.get_boundary()))
      return true;
    else
      return false;
  }


// Every pRange will be defined on a pContainer that will provide 
// implementations of the BasePContainer class.  The pRange will
// pRovide methods that wrap around the BasePContainer methods in 
// order to allow the user to access them.  Only methods that might be
// used in the implementation of a pAlgorithm are provided.  pContainers
// derived from the BasePContainer that add methods to the interface will
// require the definition of a new pRange type that contains the necessary
// wrappers for the particular class of pContainer.
// 
// The name of the pContainer method is not changed except where it conflicts
// with the name of a pRange method.  size() and local_size() are two examples.
// The pContainer size() and local_size() methods are provided via the pRange
// methods container_size() and container_local_size().

  /**
   * \brief Get total(global) size of the pContainer on which the pRange is
   * defined.
   */
  size_t container_size() {
    return container->size();
  }

  /**
   * \brief Get local size of the pContainer on which the pRange is 
   * defined.
   *
   * This differs from the local_size method because it returns the size of
   * all pContainer parts on the thread, while local_size returns only the 
   * number of elements contained within the pRange boundary or subrange 
   * boundaries.
   */
  size_t container_local_size() {
    return container->local_size();
  }

  /**
   * \brief Get local size of the pContainer on which the pRange is 
   * defined.
   *
   * This differs from the local_size method because it returns the size of
   * all pContainer parts on the thread, while local_size returns only the 
   * number of elements contained within the pRange boundary or subrange 
   * boundaries.
   *
   * The method differs from the other local_size method because it is
   * declared as a const method.
   */
  size_t container_local_size() const {
    return container->local_size();
  }

  /**
   * \brief Return whether the pContainer is empty.
   */
  bool empty() { 
    return container->empty(); 
  }

  /**
   * \brief Return whether the local portion of the pContainer is empty.
   */
  bool local_empty() { 
    return container->local_empty(); 
  }

  /**
   * \brief Return whether the local portion of the pContainer is empty.
   *
   * The method differs from the other local_empty method because it is
   * declared as a const method.
   */
  bool local_empty() const { 
    return container->local_empty(); 
  }

  /**
   * \brief Delete all elements from the pContainer.
   */
  void clear() {
    return container->clear();
    //FIX ME: The pRange is invalid at this point and needs to be updated.
  }

  /**
   * \brief Delete all elements on this thread from the pContainer.
   */
  void local_clear() {
    return container->local_clear();
    //FIX ME: The pRange is invalid at this point and needs to be updated.
  }

  /**
   * \brief Return an iterator to the first element on the thread.
   */
  typename ContainerType::iterator local_begin() { 
    return container->local_begin(); 
  }

  /**
   * \brief Return an iterator to one past the last element on the thread.
   */
  typename ContainerType::iterator local_end() { 
    return container->local_end(); 
  }

  /**
   * \brief Return a const_iterator to the first element on the thread.
   */
  typename ContainerType::const_iterator local_begin() const { 
    return container->local_begin(); 
  }

  /**
   * \brief Return a const_iterator to one past the last element on the thread.
   */
  typename ContainerType::const_iterator local_end() const { 
    return container->local_end(); 
  }

  /**
   * \brief Return a reverse_iterator to the last element on the thread.
   */
  typename ContainerType::reverse_iterator local_rbegin() { 
    return container->local_rbegin(); 
  }

  /**
   * \brief Return a reverse_iterator to one before the first element 
   * on the thread.
   */
  typename ContainerType::reverse_iterator local_rend() { 
    return container->local_rend(); 
  }

  /**
   * \brief Return a const_reverse_iterator to the last element on the thread.
   */
  typename ContainerType::const_reverse_iterator local_rbegin() const { 
    return container->local_rbegin(); 
  }

  /**
   * \brief Return a const_reverse_iterator to one before the first element 
   * on the thread.
   */
  typename ContainerType::const_reverse_iterator local_rend() const { 
    return container->local_rend(); 
  }

  /**
   * \brief Get a specified element from the pContainer.
   *
   * @param gid Global id of the element to retrieve
   */
  typename Container::value_type GetElement(GID gid) {
    return container->GetElement(gid); 
  }

  /**
   * \brief Set a specified element from the pContainer.
   *
   * @param gid Global id of the element to modify
   * @param value New value of the modified element
   */
  void SetElement(GID gid, typename Container::value_type value) {
   return container->SetElement(gid, value); 
  }

  /**
   * \brief Update an element using a work function.
   *
   * @param gid The GID of the element to update.
   * @param func The work function to apply to the elment to update it.
   */
  template<class Functor>
  int UpdateElement(GID gid, Functor& func) {
    return container->UpdateElement(gid, func);
  }

  /**
   * \brief Update an element using a work function that accepts an argument.
   *
   * @param gid The GID of the element to update.
   * @param func The work function to apply to the elment to update it.
   * @param arg The argument passed to the operator() of the work function.
   */
  template<class Functor, class T>
  int UpdateElement(GID gid, Functor& func, T& arg) {
    return container->UpdateElement(gid, func, arg);
  }

  /**
   * \brief Delete the specified element from the pCotainer.
   *
   * @param gid Global id of the element to delete.
   */
  void DeleteElement(GID gid) {
    return container->DeleteElement(gid);
    //FIX ME: The pRange may be invalid and need to be updated.
  }

  /**
   * \brief Add an element to the pContainer.
   *
   * @param value Value of the new element.
   */
  GID AddElement(const typename ContainerType::value_type& val) {
    return container->AddElement(val);
    //FIX ME: The pRange may be invalid and need to be updated.
  }

  /**
   * \brief Add an element with the GID provided to the pContainer.
   *
   * @param val Value of the new element.
   * @param gid GID of the new element.
   */
  void AddElement(const typename ContainerType::value_type& val, GID gid) {
    return container->AddElement(val, gid);
    //FIX ME: The pRange may be invalid and need to be updated.
  }

  /**
   * \brief Add a collection of elements to the pContainer.
   *
   * @param elems Elements to be added to the pContainer.
   */
  void AddElements(const typename ContainerType::Container_type& elems) {
    return container->AddElements(elems);
    //FIX ME: The pRange may be invalid and need to be updated.
  }

  /**
   * \brief Determine if an element is stored on the local thread.
   *
   * @param gid GID of the element to check.
   */
  bool IsLocal(GID gid) {
    return container->IsLocal(gid);
  }

  /**
   * \brief Determine if an element is stored on the local thread.
   *
   * @param gid GID of the element to check.
   */
  bool IsLocal(GID gid) const {
    return container->IsLocal(gid);
  }

  /**
   * \brief Determine if an element is stored on the local thread.
   *
   * @param gid GID of the element to check.
   * @param partid PartID of the element if it is stored locally.
   */
  bool IsLocal(GID gid, PARTID& partid) const {
    return container->IsLocal(gid, partid);
  }

  /**
   * \brief Find the location of an element in the pContainer.
   *
   * @param gid GID of the element to find.
   */
  Location Lookup(GID gid) {
    return container->Lookup(gid);
  }

  /**
   * \brief Find the location of an element in the pContainer.
   *
   * @param gid GID of the element to find.
   */
  Location Lookup(GID gid) const {
    return container->Lookup(gid);
  }

  /**
   * \brief Determine the thread id of the thread storing an element.
   *
   * @param gid GID of the element to locate.
   */
  PID FindRemotePid(GID gid) {
    return container->FindRemotePid(gid);
  }

  /**
   * \brief Perform a search to locate an element in the pContainer.
   *
   * @param gid GID of the element to locate.
   */
  Location CompleteSearch(GID gid) {
    return container->CompleteSearch(gid);
  }

  /**
   * \brief Get the previous pRange's Location for a subpContianer's pRange.
   * @param partid PARTID of the underlying subpContainer.
   */
  inline const Location& GetPreviouspRange(PARTID partid) const {
    return (*container).GetPreviousPart(partid);
  }

  /**
   * \brief Get the next pRange's Location for a subpContianer's pRange.
   * @param partid PARTID of the underlying subpContainer.
   */
  inline const Location& GetNextpRange(PARTID partid) const {
    return (*container).GetNextPart(partid);
  }


};

/**
 * \brief pRange output operator.
 *
 * The method prints the global id and other meta data about the pRange and
 * the global id of each of its subranges.
 */
template <class Boundary, class Container, class Subrange, class SubrangeDDG, class ElementDDG>
ostream& operator << (ostream &s, pRange<Boundary, Container, SubrangeDDG, ElementDDG>& pr) {
  typedef pRange<Boundary, Container, SubrangeDDG, ElementDDG> pRangeType;

  s << "\n" 
    << "pRange " << pr.get_global_id() << "\n"
    << "   thread id     : " << pr.get_thread_id() << "\n"
    << "   invalid       : " << pr.get_invalid() << "\n"
    << "   num subranges : " << pr.get_num_subranges() << "\n";
  if (pr.get_num_subranges() > 0) {
    s << "   subrange ids  : ";
    vector<pRange<Boundary, SubrangeDDG, ElementDDG, Container> >& subranges = pr.get_subranges();
    typename vector<pRangeType>::iterator subrange;
    for (subrange = subranges.begin(); 
         subrange != subranges.end();
       ++subrange) {
      s << subrange->get_global_id() << " ";
    }
    s << "\n";
  }
  return s;
}

/**
 * The LinearpRange class provides access to the pContainer methods
 * found in the LnearPContainer base container class.  This allows
 * STAPL users to access pContainer methods when necessary in their code
 * without providing complete and unrestricted access to the pCotnainer.
 * The pRange is able to monitor which methods are called and keep itself
 * in a valid state when methods that modify the pContinaer are called.
 * The class is templated with the boundary, ddg, and container types,
 * and is derived from the pRange class.  
 *
 * \nosubgrouping 
 */
template <class Boundary, class Container,
          class SubrangeDDG = pDDG<DDGTask, DDGWeight>,
          class ElementDDG = DDG<DDGTask, DDGWeight> >
class linearpRange 
  : public pRange<Boundary, Container, SubrangeDDG, ElementDDG, 
                  linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> > {
public:
  /**
   * \brief Class used to represent the boundary of a subrange.
   *
   * The boundary class is used to allow iteration through a subrange using 
   * an iterator of iteratorType.  The boundary object provides information
   * needed to determine which element in the subrange should be processed 
   * first, and when all of the elements in the subrange have been 
   * processed.  The boundary class is a general form of the pair of 
   * iterators returned by the STL container methods begin() and end().
   */
  typedef Boundary boundaryType;

  /**
   * \brief The type of pContainer used to store the data on which the 
   * pRange is defined.
   */
  typedef Container ContainerType;

  /**
   * \brief Type definition of the current class to allow for easy self 
   * reference.
   */
  typedef linearpRange<Boundary, Container, SubrangeDDG, ElementDDG>  pRangeType;

  /**
   * \brief Type definition of the current class to allow for easy self 
   * reference.
   */
  typedef linearpRange<Boundary, Container, SubrangeDDG, ElementDDG>  thisType;

  /**
   * \brief Type definition of the subrange type.
   */
  typedef linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> subrangeType;

private:
  typedef pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> > basepRangeType;


public:
  /**
   * \brief Default constructor
   *
   * The pRange constructed is not valid, and should only be used after 
   * subranges defined on the container provided are added via the 
   * add_subrange method.
   *
   * A pointer to the container the pRange is defined on is not provided, 
   * therefore the container's methods will be inaccessible and the pRange 
   * will be unable to trigger events such as pContainer redistribution.
   *
   * This constructor is needed because several classes that contain 
   * pRanges have constructors that are not given all the information 
   * necessary to construct their pRange member.  This violates the 
   * principle that a constructor must provide an object that is valid, 
   * but it is programmatically useful.
   *
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   */
  linearpRange(const int g_offset = 10000) 
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(g_offset)
  { }

  /**
   * \brief Constructor for simple pRange on a non-STAPL container.
   *
   * This creates a pRange with one subrange per thread.
   *
   * A pointer to the container the pRange is defined on is not provided, 
   * therefore the container's methods will be inaccessible and the pRange
   * will be unable to trigger events such as pContainer redistribution.
   *
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  linearpRange(const boundaryType& p_range_boundary, 
               const int g_offset = 10000, 
               void* const p = NULL)
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(p_range_boundary, g_offset, p)
  { }

  /**
   * \brief Constructor for simple pRange on a STAPL container.
   *
   * This creates a pRange with one subrange per thread.
   *
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  linearpRange(Container* const c, 
               const boundaryType& p_range_boundary, 
               const int g_offset = 10000, 
               void* const p = NULL)
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(c, p_range_boundary, g_offset, p)
  { }

  /**
   * \brief Constructor for an incomplete pRange on a STAPL container.
   *
   * The pRange constructed contains no boundary or subranges, and 
   * therefore abstracts no data.  Subranges defined on the container 
   * provided will be added via the add_subrange method after the pRange is
   * constructed.  The pRange is assumed to be a root pRange. Its parent is
   * set to NULL and it is registered with ARMI.
   *
   * This violates the principle that a constructor must provide an object
   * that is valid, but it is programmatically useful.
   *
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   */
  linearpRange(Container* const c, 
               const int g_offset = 10000)
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(c, g_offset)
  { }

  /**
   * \brief Constructor for simple pRange on a non-STAPL container.
   *
   * This creates a pRange with one subrange per thread and assigns its 
   * global id to the value passed in to the method.
   *
   * @param g The global id that should be assigned to the 
   * subrange that contains the boundary passed in.  The root pRange will 
   * have an automatically generated global id.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  linearpRange(const int g,
               const boundaryType& p_range_boundary, 
               const int g_offset = 10000,
               void* const p = NULL)
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(g, p_range_boundary, g_offset, p)
  { }

  /**
   * \brief Constructor for simple pRange on a STAPL container.
   *
   * This creates a pRange with one subrange per thread and assigns its 
   * global id to the value passed in to the method.
   *
   * @param g The global id that should be assigned to the 
   * subrange that contains the boundary passed in.  The root pRange will 
   * have an automatically generated global id.
   * @param c pointer to the pContainer on which the pRange is
   * defined.
   * @param p_range_boundary instantiation of a boundary 
   * class that bounds the data abstracted by the single subrange
   * @param g_offset optional argument that initializes the 
   * global id generator to claim blocks if ids of size g_offset. 
   * @param p pointer to the parent pRange of this subrange.
   * @note If not specified the prange assumes it is the root pRange and
   * registers with ARMI.
   */
  linearpRange(const int g,
               Container* const c, 
               const boundaryType& p_range_boundary, 
               const int g_offset = 10000, 
               void* const p = NULL)
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(g, c, p_range_boundary, g_offset, p)
  { }

  /**
   * \brief Copy constructor that uses the copy constructor of all data 
   * members except the rmiHandle and the DDG pointer.  
   *
   * The default constructors of those members is called, and then the 
   * assignment operator is used to initialize them.  This is done to 
   * avoid having multiple pRanges with the same rmiHandle or sharing a 
   * DDG.  DDGs cannot be shared between pRanges since the Executor used in
   * p_for_all is capable of processing multiple pRanges concurrently.  A 
   * shared DDG in this case would lead to unexpected behavior as some 
   * subranges in both pRanges would not be processed by the Executor.
   */
  linearpRange(const pRangeType& pr) 
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(pr)
  { }

  linearpRange(const basepRangeType& pr) 
    : pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >(pr)
  { }

  /**
   * \brief Assignment operator that uses the assignement operator of all 
   * data members except the rmiHandle and the DDG pointer.  
   *
   * The DDG pointer is assigned to NULL and the rmiHandle to -1, and then 
   * the assignment operator is used again to initialize them.  This is 
   * done to avoid having multiple pRanges with the same rmiHandle or 
   * sharing a DDG.  DDGs cannot be shared between pRanges since the 
   * Executor used in p_for_all is capable of processing multiple pRanges 
   * concurrently.  A shared DDG in this case would lead to unexpected 
   * behavior as some subranges in both pRanges would not be processed by 
   * the Executor.
   */
  linearpRange<Boundary, Container, SubrangeDDG, ElementDDG>& operator=(const pRangeType& other) {
    pRange<Boundary, Container, SubrangeDDG, ElementDDG, linearpRange<Boundary, Container, SubrangeDDG, ElementDDG> >::operator=(other);
    return *this;
  }

  /**
   * \brief Return the maximum size of the pContainer.
   */
  size_t max_size() {
    return container->max_size();
  }

  /**
   * \brief Return an iterator to the first element in the pContainer.
   */
  typename ContainerType::iterator front() {
    return container->front();
  }

  /**
   * \brief Return a const_iterator to the first element in the pContainer.
   */
  typename ContainerType::const_iterator front() const {
    return container->front();
  }

  /**
   * \brief Return an iterator to one past the last local element in the pContainer.
   */
  typename ContainerType::iterator back() {
    return container->back();
  }

  /**
   * \brief Return a const_iterator to one past the last local element in the pContainer.
   */
  typename ContainerType::const_iterator back() const {
    return container->back();
  }

  /**
   * \brief Insert an element into the pContainer after the specified position.
   *
   * @param it Iterator pointing to the position where the element will be inserted.
   * @param elem Element to be inserted into the pContainer.
   */
  typename ContainerType::iterator 
  insert(typename ContainerType::iterator it,
         const typename ContainerType::value_type& elem) {
    return container->insert(it, elem);
    //FIX ME: The pRange is invalid at this point and needs to be updated.
  }

  /**
   * \brief Delete an element from the pContainer.
   *
   * @param it Iterator pointing to the element to be removed.
   */
  typename ContainerType::iterator 
  erase(typename ContainerType::iterator it) {
    return container->erase(it);
    //FIX ME: The pRange is invalid at this point and needs to be updated.
  }


  /**
   * \brief Add an element to the end of the local portion of the pContainer.
   *
   * @param elem Element to add to the pContainer.
   */
  void push_back(const typename ContainerType::value_type& elem) {
    return container->push_back(elem);
    //FIX ME: The pRange is invalid at this point and needs to be updated.
  }

  /**
   * \brief Remove an element from the end of the local portion of the pContainer.
   */
  void pop_back() {
    return container->pop_back();
  }

  /**
   * \brief Initialize the boundary information of the pContainer.
   *
   * @param partid The part id of the current part.
   * @param prev The part id of the previous part (i.e. this part's neighbor).
   * @param next The part id of the next part (i.e. this part's neighbor).
   */
  void InitBoundaryInfo(PARTID partid,
                        Location prev,
                        Location next) {
    return container->InitBoundaryInfo(partid, prev, next);
  }

};

/**
 * @}
 */

}
#endif
