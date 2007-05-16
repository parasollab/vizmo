#ifndef PLIST_H
#define PLIST_H

#include <runtime.h>
#include "ListPart.h"
#include "LinearOrderPContainer.h"
#include "ListDistribution.h"
#include <pRange.h>
namespace stapl {


/**
 * @ingroup pContainers
 * @defgroup plist Parallel List
 * @{
 **/


/**
 * pList implements the parallel counterpart of the sequential list.
 **/
template<class T>
class pList : public LinearOrderPContainer<ListPart<T>, ListDistribution<T>,no_trace,bidirectional_iterator_tag, stapl_base_element_tag > {
 public:
  //=======================================
  //types
  //=======================================

  ///The type of the elements stored inside pList
  typedef T value_type;

  typedef typename ListPart<value_type>::Element_Set_type List_Range_type;
  
  typedef vector<pair<List_Range_type,PID> > Part_Distribution_Info_type;
  typedef vector<Part_Distribution_Info_type> List_Distribution_Info_type;

  ///  BasePContainer from which the pList derives trough  LinearOrderPContainer.
  typedef BasePContainer<ListPart<value_type>,
    ListDistribution<value_type> , no_trace,bidirectional_iterator_tag, stapl_base_element_tag > BasePContainer_type;
  

  ///LinearOrderPContainer is the base class for pList.
  typedef LinearOrderPContainer<ListPart<value_type>,
    ListDistribution<value_type> ,no_trace, bidirectional_iterator_tag,stapl_base_element_tag > LinearOrderPContainer_type;
  
  ///pList type
  typedef pList<value_type> pList_type;

  ///pList iterator
  typedef typename LinearOrderPContainer_type::iterator iterator;  
  typedef typename LinearOrderPContainer_type::reverse_iterator reverse_iterator;  
  ///pList const iterator
  typedef typename LinearOrderPContainer_type::const_iterator const_iterator;
  typedef typename LinearOrderPContainer_type::const_reverse_iterator const_reverse_iterator;  

  /// Linera boundary for pList
  typedef stapl::linear_boundary<iterator> linearBoundaryType;
  typedef stapl::linear_boundary<reverse_iterator> reverse_linearBoundaryType;

  /// pRange corresponding to the pList. - commented, use "PRange" in LinearOrderPContainer instead.
  typedef stapl::linearpRange<linearBoundaryType, pList_type> PRange;  
  typedef stapl::linearpRange<reverse_linearBoundaryType, pList_type> reverse_PRange;


  ///Part type
  
  typedef typename LinearOrderPContainer_type::pContainerPart_type pContainerPart_type;
  typedef typename LinearOrderPContainer_type::Element_Set_type Element_Set_type;

  /**@name Constructors and Destructor */
  //@{
  //=======================================
  //constructors 
  //=======================================
  /**@brief 
   *Default constructor. It creates an empty pList.
   */
  pList() {
    this->register_this(this);
    rmi_fence(); 
  }

  /**@brief 
   *Constructor. Allocates a pList of size n equally distributed across available threads.
   */
  pList(int n) : LinearOrderPContainer_type(n) {
    this->register_this(this);
    rmi_fence(); 
  }
  /**@brief 
   *Constructor. Allocates a pList of size n equally distributed across available threads.
   *All elements of the pList will be initialized with the specified value_type.
   *@param int the size of the pList.
   *@param value_type the value that will be assigned to all elements.
   */
  pList(int n, const value_type& _data) : LinearOrderPContainer_type(n,_data) {
    this->register_this(this);
    rmi_fence(); 
  }
  /**@brief 
   *Copy constructor
   */
  pList(const pList& _list) : LinearOrderPContainer_type(_list) { 
    this->register_this(this);
    rmi_fence(); 
  }
  /*   pList(const PartitionInputs* user_inputs) :  */
  /*     LinearOrderPContainer_type(user_inputs) {} */


  /**@brief
   *Copy constructor from an input pRange
   * @param _pr PRange from which the copy is
   * @param copy_data boolean to control whether to copy data addition
   *  to layout
   */
  pList(PRange& _pr, bool copy_data = true) {
    this->myid = get_thread_id();
    this->nprocs = get_num_threads();
    this->part_id_counter = 0;
    typedef typename PRange::subrangeType SubrangeType;
    typename PRange::iteratorType pr_it, pr_end;
    vector<SubrangeType> vr = _pr.get_subranges();
    int npart = vr.size();
    for (int i=0; i<npart-1; ++i) { //one part has been added in constructor
      this->AddPart();
    }
    for (int i=0; i<npart; ++i) { //one part has been added in constructor
      pContainerPart_type* lp = this->GetPart(i);
      if (copy_data == true) {
        //lp->resize(vr[i].container_size() );
	pr_end = vr[i].get_boundary().finish();
	for (pr_it = vr[i].get_boundary().start(); pr_it != pr_end; ++pr_it) 
	  this->AddElement2Part(*pr_it, i);
      } else {
        //lp->resize(1);
	//only and 1 element, and we just put the first element into it
	// b/c we don't know what to put otherwise
	pr_it = vr[i].get_boundary().start();
	this->AddElement2Part(*pr_it, i);
      }
    }
    stapl::rmi_fence();
    if (! pcontainer_parts.empty() ) {
      for (int i = 0; i < npart; ++i) {
        Location before = _pr.GetPreviouspRange(i);
        Location next = _pr.GetNextpRange(i);
	this->dist.InitBoundaryInfo(i, before, next);
      }
    }
    stapl::rmi_fence();

    this->register_this(this);
    rmi_fence();
  }

  /**@brief
   *Copy constructor from an input pRange
   * @param _pr reverse_PRange from which the copy is
   * @param copy_data boolean to control whether to copy data addition
   *  to layout
   */
  pList(reverse_PRange& _pr, bool copy_data = true) {
    this->myid = get_thread_id();
    this->nprocs = get_num_threads();
    this->part_id_counter = 0;
    typedef typename reverse_PRange::subrangeType SubrangeType;
    typename reverse_PRange::iteratorType pr_it, pr_end;
    vector<SubrangeType> vr = _pr.get_subranges();
    int npart = vr.size();
    for (int i=0; i<npart-1; ++i) { //one part has been added in constructor
      this->AddPart();
    }
    for (int i=0; i<npart; ++i) { //one part has been added in constructor
      pContainerPart_type* lp = this->GetPart(i);
      if (copy_data == true) {
        //lp->resize(vr[i].container_size() );
	pr_end = vr[i].get_boundary().finish();
	for (pr_it = vr[i].get_boundary().start(); pr_it != pr_end; ++pr_it) 
	  this->AddElement2Part(*pr_it, i);
      } else {
        //lp->resize(1);
	//only and 1 element, and we just put the first element into it
	// b/c we don't know what to put otherwise
	pr_it = vr[i].get_boundary().start();
	this->AddElement2Part(*pr_it, i);
      }
    }
    stapl::rmi_fence();
    if (! pcontainer_parts.empty() ) {
      for (int i = 0; i < npart; ++i) {
        Location before = _pr.GetPreviouspRange(i);
        Location next = _pr.GetNextpRange(i);
	this->dist.InitBoundaryInfo(i, before, next);
      }
    }
    stapl::rmi_fence();

    this->register_this(this);
    rmi_fence();
  }

  /**@brief Destructor.
   */
  ~pList() {
    rmiHandle handle = this->getHandle();
    if(handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  //@}
  
  size_t max_size() {return size_t(-1);}
  
  
 public:
  /**@name Accessors */
  //@{
  /**@brief Begin iterator.
   *@return iterator a pointer to the begining of the local data.
   **/
  iterator begin() {	 
    return this->local_begin();
  }

  /**@brief End iterator.
   *@return iterator a pointer to the end of the local data.
   **/  
  iterator end()  {
   return this->local_end();
  }
  
  /**@brief Begin const iterator.
   *@return const iterator pointing to the begining of the local data.
   **/
  const_iterator begin() const{
    return this->local_begin();
  }

  /**@brief End const iterator.
   *@return const iterator pointing to the end of the local data.
   **/    
  const_iterator end() const {
    return this->local_end();
  }
  
  /**@brief add an element to the front of the plist
   *@param value_type the value to be pushed.
   */
  virtual void push_front(const value_type& _elm) {
    const iterator& it = this->local_begin(); 
    this->insert(it,_elm); 
  }
  
  /**@brief Erase an element from the front of the global plist.
   */
  virtual void pop_front() {
    const iterator& it = this->begin(); 
    this->erase(it); 
  }

  /**@brief First element of the pList
   *@return a reference to the front element of the pList.
   */
  //T& front() {
  //const iterator& it = this->begin(); 
  //return *it;
  //}

  int generate_gid(PARTID _partid, int _threadid){
    //!!!!the maxparts should be extended to maxint; check potential problems
    int maxparts=10000;
    return _threadid*(maxparts/nprocs) + _partid;
  }

   /*@brief
    *Build a pRange associated with the pcontainer
    *@param pr PRange output parameter
    */
 void get_prange(PRange& pr){
   for(int i=0;i<this->get_num_parts();i++){
     pair<iterator,iterator> pi=this->GetPartBoundary(i);
     typename PRange::subrangeType _leaf_prange(generate_gid(i,myid),this,linearBoundaryType(pi.first,pi.second),10000,&pr);
     pr.add_subrange(_leaf_prange);
   }
  }

   /*@brief
    *Build a pRange associated with the pcontainer
    *@param pr reverse_PRange output parameter
    */
 void get_prange(reverse_PRange& pr){
   for(int i=0;i<this->get_num_parts();i++){
     pair<iterator,iterator> pi=this->GetPartBoundary(i);
     reverse_iterator first = reverse_iterator(pi.second);
     reverse_iterator second = reverse_iterator(pi.first);
     typename reverse_PRange::subrangeType _leaf_prange(generate_gid(i,myid),this,reverse_linearBoundaryType(first,second),10000,&pr);
     pr.add_subrange(_leaf_prange);
   }
  }


  //Distribute and related methods
  //=======================================
  void DisplayDistributionInfo(const List_Distribution_Info_type& _di) {
    typename List_Distribution_Info_type::const_iterator it;
    int i=0;
    for(it =_di.begin(); it!=_di.end(); ++it,++i) {
      cout<<"Distribution Info for part: "<<i<<endl;
      typename Part_Distribution_Info_type::const_iterator itt;
      for(itt = it->begin(); itt!= it->end(); ++itt) {
	cout<<"range("<<itt->first.first<<","<<itt->first.second
	    <<") to thread "<<itt->second<<endl;
      }
    }
  }
      

  /**@name Distribution Related*/
  //@{
  
  /**@brief Merges pContainer parts that can be merged into larger parts
   */
  virtual void MergeSubContainer() {}
  
  virtual int SetSubContainer(const vector<pContainerPart_type>  _ct){
    printf("\n This is -SetSubContainer- method from pList ...... NOT YET IMPLEMENTED");
    return OK;
  }
  
  virtual void BuildSubContainer( const vector<Element_Set_type>&,
			  vector<pContainerPart_type>&){
    printf("\n This is -BuildSubContainer- method from pList ...... NOT YET IMPLEMENTED");
  }
  //@}
};
} //end namespace stapl
//@}
#endif
