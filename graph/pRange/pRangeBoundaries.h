#ifndef P_RANGE_BOUNDARIES_H
#define P_RANGE_BOUNDARIES_H

#include "../pGraph.h"
#include "../pContainers/pgraph/SubpGraphIterator.h"
namespace stapl {

/**
 * @addtogroup prange_boundary
 *
 * @{
 */

/**
 * Base boundary is a virtual class that defines the interface required of
 * a pRange boundary.
 *
 * In addition to the methods defined in the class, the operator== must 
 * also be defined on the boundary class.
 */
template<class Iterator>
class base_boundary {
public:

  /**
   * \brief the boundary must provide the iteratorType typedef that returns
   * the type of iterator that will allow iteration from the beginning to 
   * the end of the data elements contained in the boundary.
   */
  typedef Iterator iteratorType;

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   */
  virtual void define_type(stapl::typer& t) = 0;

  /**
   * \brief Check if the argument points to the start of the boundary
   * @param i Iterator pointing to an element
   * @return bool indicating if the iterator is pointing to
   * the first element contained in the boundary.
   */
  virtual bool at_start(const iteratorType& i) = 0;

  /**
   * Check if the argument points to the end of the boundary
   * @param i Iterator pointing to an element
   * @return bool indicating if the iterator is pointing to
   * the last element contained within the boundary.
   */
  virtual bool at_finish(const iteratorType& i) = 0;

  /** 
   * Get the start iterator
   * @return iterator pointing to the first element contained within the 
   * boundary
   */
  virtual iteratorType start() const = 0;

  /**
   * Get the finish iterator
   * @return iterator pointing to the last element contained within the 
   * boundary
   */
  virtual iteratorType finish() const = 0;

  /** 
   * Set the start iterator
   * @param i iterator pointing to the new first element contained within 
   * the boundary
   *
   */
  virtual void start(const iteratorType& i) = 0;

  /** 
   * Set the finish iterator
   * \param i iterator pointing to the new last element contained within 
   * the boundary
   */
  virtual void finish(const iteratorType& i) = 0;

  // In addition to the member functions above the == operator should also 
  // be defined.
};


/**
 * linear_boundary is a simple boundary made of a pair of iterators that 
 * point to the first and last elements in the range of data elements 
 * contained in the boundary.  The boundary differs from the STL begin() 
 * and end() iterators by providing the inclusive boundary [first, last] 
 * while the STL containers provide a partially open ended boundary 
 * [begin, end).  This difference requires attention when loops inside work
 * functions that operate on the elements defined in a boundary.
 */
template<class Iterator>
class linear_boundary : public base_boundary<Iterator> {
public:
  /**
   * \brief the type of iterator passed in to define the boundary.
   */
  typedef Iterator iteratorType;

private:
  /**
   * \brief iterator pointing to the first data element contained by the 
   * boundary.
   */ 
  iteratorType begin;

  /**
   * \brief iterator pointing to the last data element contained by the 
   * boundary.
   */
  iteratorType end;

  /**
   * \brief the number of data elements contained within the boundary
   */
  int num_elements;

#ifdef _STAPL_LOAD_SCHEDULER_
 //This file id required because the Transferable_pRange stores subranges 
 // as a vector of boundaried. When the pRange is fully implemented this 
 // could be taken out as the Subrange ( which ) is a pRange by itself has 

  /**
   * \brief a unique global id.
   * \warning This is redundant.  The pRange global id is already unique 
   * and should be used.  This member will be removed in the future.
   */
  int id;
#endif

public:

#ifdef _STAPL_LOAD_SCHEDULER_
 /**
  * \brief Set the boundary id.
  *
  * \param _id_ the new global id for the boundary
  */
 void SetID(int _id_)  {  id = _id_;  }

 /**
  * \brief Get the boundary id.
  */
 int GetID(){  return id;  }

#endif

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   * 
   * define_type should never be used since it is used only when an object 
   * is packed by the primitives before being sent to another thread.  
   * Since the linear_boundary continas iterators that are only valid on 
   * this thread and there is no mechanism to currently update the 
   * iterators we cause an assertion to stop the program.
   */
  void define_type(stapl::typer& t) {
    stapl_assert(0,"pRange linear boundary define_type used.\n");
  }

  //Constructors, Destructors, and Assignment operators

  /** 
   * Default constructor 
   */
  linear_boundary() : begin(), end() { 
    stapl_assert(1,"Linear boundary default constructor used.\n");
  }

  /**
   * Constructor setting the begin and end of the range
   * \param b Iterator pointing to the first element in the pRange
   * \param e Iterator pointing to the last element in the pRange
   */
  linear_boundary(const iteratorType& b, 
                  const iteratorType& e) : begin(b), end(e) { 
    num_elements = std::distance(b, e) + 1;
  }

  /** 
    * Default destructor 
    */
  ~linear_boundary() { }

  /**
   * Assignment operator
   * \param lb linear_boundary used to set the values of the boundary
   * \return linear_boundary while boundary information matching lb
   */
  linear_boundary& operator=(const linear_boundary& lb) {
    if (this == &lb) return *this; //handle self assignment;

    begin = lb.begin;
    end   = lb.end;
    num_elements =  lb.num_elements;
#ifdef _STAPL_LOAD_SCHEDULER_ 
    id = lb.id; 
#endif 
    return *this;
  }

  //Member Functions to set and check data members

  /**
   * Check if the iterator points to the start of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the first element of the pRange
   */
  bool at_start(const iteratorType& i) {
    return (begin == i);
  }

  /**
   * Check if the iterator points to the end of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the last element of the pRange
   */
  bool at_finish(const iteratorType& i) {
    return (end == i);
  }

  /** 
   * Get an iterator to the first element contained within the boundary
   * \return iterator pointing to the first element of the boundary
   */
  iteratorType start() const {
    return begin;
  }

  /** 
   * Get an iterator to the last element contained within the boundary
   * \return iterator pointing to the last element of the boundary
   */
  iteratorType finish() const {
    return end;
  }

  /**
   * Get the number of elements bounded by the boundary.
   */
  int size()  { return num_elements; }
  
  /** 
   * Set the begin iterator
   * \param i iterator pointing to the new begin of the boundary
   */
  void start(const iteratorType& i) {
    begin = i;
    num_elements = std::distance(begin, end) + 1;
  }

  /** 
   * Set the end iterator
   * \param i iterator pointing to the new end of the boundary
   */
  void finish(const iteratorType& i) {
    end = i;
    num_elements = std::distance(begin, end) + 1;
  }

  //Operators

  /**
   * \brief Equality operator
   */
  inline bool operator == (const linear_boundary<Iterator>& boundary) const {
    if ((begin == boundary.start()) && (end == boundary.finish()))
      return true;
    else
      return false;
  }
};


/**
 * counted_boundary is a simple boundary made up of an iterator and an 
 * integer count.  The iterator points to the first element contained 
 * within the boundary.  The last element in the boundary is the nth 
 * element from the first element in the boundary, where n is the integer 
 * given to the boundary counstructor.  The boundary provides an inclusive 
 * boundary [first, first + count].
 */
template<class Iterator, class Counter = int>
class counted_boundary : public base_boundary<Iterator> {
public:
  /**
   * \brief the type of iterator passed in to define the boundary.
   */
  typedef Iterator iteratorType;

  /**
   * \brief the type used to represent the integer count that is used to 
   * define the end of the boundary.
   */
  typedef Counter counterType;

private:
  /**
   * \brief iterator pointing to the first data element contained by the 
   * boundary.
   */
  iteratorType begin;

  /**
   * \brief Counter defining the number of elements between the first and 
   * last elements in the boundary.
   */
  counterType num_elements;

  /**
   * \brief Compute the number of elements between two iterators.
   *
   * \param b iterator pointing to the first element in the range
   * \param e iterator pointing to the last element in the range
   * \return counter with the number of elements in [b, e]
   */
  counterType distance(Iterator b, Iterator e, input_iterator_tag) const {
    counterType count = 0;
    while (b++ != e)
      ++count;
    ++count;
    return count;
  }

  /**
   * \brief Compute the number of elements between two iterators.
   *
   * \param b iterator pointing to the first element in the range
   * \param e iterator pointing to the last element in the range
   * \return counter with the number of elements in [b, e]
   */
  counterType distance(Iterator b, Iterator e, random_access_iterator_tag) const {
    return ((e - b) + 1);
  }

  /**
   * \brief Compute the iterator pointing to the last element in the boundary
   *
   * \return iterator pointing to the last element in the boundary
   */
  iteratorType compute_finish(input_iterator_tag) const {
    iteratorType end = begin;
    for (int i = 1; i <  num_elements; ++i)
      ++end;
    return end;
  }

  /**
   * \brief Compute the iterator pointing to the last element in the boundary
   *
   * \return iterator pointing to the last element in the boundary
   */
  iteratorType compute_finish(random_access_iterator_tag) const {
    return begin + (num_elements - 1);
  }

public:

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * define_type should never be used since it is used only when an object 
   * is packed by the primitives before being sent to another thread.  
   * Since the linear_boundary continas iterators that are only valid on 
   * this thread and there is no mechanism to currently update the 
   * iterators we cause an assertion to stop the program.
   */
  void define_type(stapl::typer& t) {
    stapl_assert(0,"pRange counted boundary define_type used.\n");
  }

  //Constructors, Destructors, and Assignment operators

  /** 
   * \brief Default constructor 
   */
  counted_boundary() : begin(), num_elements(0) { 
    stapl_assert(0,"Counted boundary default constructor used.\n");
  }

  /**
   * Constructor setting the start of the range and the number of elements
   * \param b Iterator pointing to the first element in the pRange
   * \param n Counter whose value is the number of elements in the pRange
   */
  counted_boundary(const iteratorType& b,
                   const counterType& n) : begin(b), num_elements(n) { }

  /** 
   * Default destructor 
   */
  ~counted_boundary() { }

  /**
   * Assignment operator
   * \param cb counted_boundary used to set the values of the boundary
   * \return counted_boundary while boundary information matching cb
   */
  counted_boundary& operator=(const counted_boundary& cb) {
    if (this == &cb) 
      return *this; //handle self assignment;

    begin = cb.begin;
    num_elements = cb.num_elements;

    return *this;
  }

  //Member Functions to set and check data members

  /**
   * Check if the argument points to the start of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the first element of the pRange
   */
  bool at_start(const iteratorType& i) {
    return (begin == i);
  }

  /**
   * Check if the argument points to the end of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the last element of the pRange
   */
  bool at_finish(const iteratorType& i) {
    counterType dist = distance(begin,i, 
                            iterator_traits<Iterator>::iterator_category());
    return (dist == num_elements);
  }

  /** 
   * Get an iterator to the first element contained within the boundary
   * \return iterator pointing to the first element of the boundary
   */
  iteratorType start() const {
    return begin;
  }

  /** 
   * Get an iterator to the last element contained within the boundary
   * \return iterator pointing to the last element of the boundary
   */
  iteratorType finish() const {
    return compute_finish(iterator_traits<Iterator>::iterator_category());
  }

  /** 
   * Set the begin iterator
   * \param i iterator pointing to the new begin of the boundary
   */
  void start(const iteratorType& i) {
    begin = i;
  }

  /** 
   * Set the end of the boundary using an iterator to the last element 
   * contained within the boundary
   * \param i iterator pointing to the last element of the boundary
   */
  void finish(const iteratorType& i) {
    num_elements = distance(begin,i,
                            iterator_traits<Iterator>::iterator_category());
  }

  /**
   * Get the number of elements in the boundary
   * \return counter whose value is the number of elements.
   */
  counterType size() { return num_elements; }

  /** 
   * Set the number of elements
   * \param n counter whose value is the number of elements in the pRange
   */
  void size(const counterType& n) { num_elements = n; }

  //Operators

  /**
   * \brief Equality operator
   */
  inline 
  bool operator== (const counted_boundary<Iterator, Counter>& boundary) const {
    if ((begin == boundary.start()) && 
        (num_elements == boundary.num_elements()))
      return true;
    else
      return false;
  }
};


/**
 * graph_boundary is a more complex boundary type that is used to define 
 * subgraphs on a general graph type.  The boundary is a collection of 
 * graph edges.  An edge is in the collection if it is from a vertex in the
 * boundary to a vertex that is not contained within the boundary.
 */
template<class PGRAPH>
class graph_boundary {
//        : public base_boundary<SubpGraphIterator<SubpGraph<PGRAPH> > > {
public:

  /**
   * \brief Type of subgraph used to represent the sugraph defined by the 
   * boundary.
   */
  typedef SubpGraph<PGRAPH> subgraphType;

  /**
   * \brief Type of iterator used to traverse the subgraph defined by the
   * boundary.
   */
  typedef SubpGraphIterator<subgraphType> iteratorType;

private:

  /**
   * \brief subgraph defined by the boundary
   */
  subgraphType subgraph;

  /**
   * \brief number of elements contained in the boundary.
   */
  int num_elements;

protected:

  /**
   * \brief compute the number of elements in the boundary.
   */
  inline int find_size() {
    int count = 0;
    for (iteratorType i = subgraph.begin(); i != subgraph.end(); ++i)
      ++count;
    return count;
  }

public:

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \param t typer object used by ARMI to automatically pack the class
   */
  void define_type(stapl::typer& t) {
      t.local(subgraph);
  }

  /**
   * \brief Default constructor
   */
  graph_boundary() { }

  /**
   * Constructor that fully defines the boundary.
   * \param boundary hash_multimap that contains a set of edges, which are 
   * pairs of vertex ids.
   * \param pgr graph container on which the boundary is defined
   * \param start_vid the first vertex in the boundary to be processed in a
   * traversal of the elements within the boundary.
   */
  graph_boundary(const hash_multimap<VID, VID, hash<VID> >& boundary, 
                 PGRAPH* pgr, 
                 const int start_vid = 0) 
    : subgraph(pgr, boundary, start_vid) 
  { num_elements = this->find_size(); }

  /**
   * Copy constructor
   * \param other graph_boundary used to initialize the current boundary
   */
  graph_boundary(const graph_boundary<PGRAPH>& other) {
    subgraph = other.subgraph;
    num_elements = other.num_elements;
  }

  /**
   * Default destructor
   */
  ~graph_boundary() { }

  /**
   * Assignment operator
   * \param gb graph_boundary used to initialize the boundary.
   */
  graph_boundary& operator=(const graph_boundary& gb) {
    if (this == &gb) 
      return *this;
    subgraph  = gb.subgraph;
    num_elements = gb.num_elements;
    return *this;
  }

  /**
   * Check if the argument points to the start of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the first element of the pRange
   */
  inline bool at_start(const iteratorType& i) { 
    return (subgraph.begin() == i); 
  }

  /**
   * Check if the argument points to the end of the boundary
   * \param i Iterator pointing to an element in the pRange
   * \return bool indicating if the iterator is pointing to
   * the last element of the pRange
   */
  inline bool at_finish(const iteratorType& i) { 
    return (subgraph.end() == i); 
  }

  /** 
   * Get an iterator to the first element contained within the boundary
   * \return iterator pointing to the first element of the boundary
   */
  inline iteratorType start() { 
    return subgraph.begin(); 
  }

  /** 
   * Get an iterator to the last element contained within the boundary
   * \return iterator pointing to the last element of the boundary
   */
  inline iteratorType finish() { 
    return subgraph.end(); 
  }

  /**
   * \brief the start modifier method is not valid on the graph boundary.
   */
  inline void start(const iteratorType& i) { 
    stapl_assert(0,"start modifier method not implemented for graph boundary.");
  }

  /**
   * \brief the finish modifier method is not valid on the graph boundary.
   */
  inline void finish(const iteratorType& i) { 
    stapl_assert(0,"finish modifier method not implemented for graph boundary.");
  }

  /**
   * Get the number of elements bounded by the boundary.
   */
  int size()  { return num_elements; }

  /**
   * Equality operator
   */
  bool operator == (const graph_boundary<PGRAPH>& boundary) const {
    if (subgraph == boundary.subgraph)
      return true;
    else
      return false;
  }
};
/**
 * @}
 */

} //end namespace stapl
#endif
