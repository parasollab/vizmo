#ifndef LINEARORDERPCONTAINER_H
#define LINEARORDERPCONTAINER_H

#include "BasePContainer.h"
#include <pRange.h>

  /**
   * @addtogroup linearorderpcontainer
   * @{
  **/
namespace stapl {
  /**
   *LinearOrderPContainer class encompasses the common traits of
   *pContainers that have an order defined among the its
   *elements. pvector, plist, pmap, pmulti-map, pset and pmultiset
   *inherit from this class.
   * 
   * iterator_tag represents the type of iterator
   * LinearOrderPContainer is instatiates with. This should be either
   * random_access_iterator or bedirectioonal_iterator
   *
   * element_tag is either the value type of the element if the
   * pcontainer DOES NOT store the gid together in a structure with
   * the element, or splBaseElement is the element if the gid is
   * stored together with the element in a structure.
   *
   */

/**
 * Class definition
 **/
template<class Part_type, class Distribution_type,class trace_class, class Iterator_tag, typename  element_tag>
class LinearOrderPContainer 
      : public BasePContainer<Part_type,Distribution_type,trace_class,Iterator_tag, element_tag> 
{
 public:
  //=======================================
  //types
  //=======================================
  /**@brief 
     Basic Value type 
   */
  typedef  typename Part_type::value_type value_type;

  /**@brief 
     Range type 
   */
  typedef typename Part_type::Element_Set_type Linear_Range_type;

  /**@brief 
   * pContainer type
   */
  typedef Part_type pContainer_Part_type;

  /**@brief 
     Sequential Container type that part uses.
   */
  typedef typename Part_type::Sequential_Container_type Sequential_Container_type;
  /**@brief 
     Sequential Container type that part uses.
   */
  typedef typename Part_type::Container_type Container_type;

  //for one part in one thread
  typedef vector<pair<Linear_Range_type,PID> > Part_Distribution_Info_type;

  //ASSUMPTION: Ranges are consequtive from left to right
  typedef vector<Part_Distribution_Info_type> Linear_Distribution_Info_type;

  /**@brief 
     BasePContainer type
   */
  typedef BasePContainer<Part_type, Distribution_type, trace_class, Iterator_tag,element_tag> BasePContainer_type;
  /**@brief 
     This type
   */
  typedef LinearOrderPContainer<Part_type,Distribution_type,trace_class, Iterator_tag,element_tag> LinearOrderPContainer_type;


  /**@brief 
     Iterator on elements
   */
  //typedef typename BasePContainer_type::iterator iterator;  
  typedef typename BasePContainer_type::iterator iterator;  
  //added by tao
  typedef typename BasePContainer_type::reverse_iterator reverse_iterator;  
  /**@brief 
     Const iterator on elements
   */
  typedef typename BasePContainer_type::const_iterator const_iterator; 
  //added by tao
  typedef typename BasePContainer_type::const_reverse_iterator const_reverse_iterator; 

  //Linear boundary for the pcontainer 
  typedef  stapl::linear_boundary<iterator> linearBoundaryType;
  typedef  stapl::linear_boundary<reverse_iterator> reverse_linearBoundaryType;

  //pRange corresponding to the pcontainer
  typedef stapl::pRange<linearBoundaryType, LinearOrderPContainer_type> PRange;

  typedef stapl::pRange<reverse_linearBoundaryType, LinearOrderPContainer_type> reverse_PRange;

  /**@brief 
     Type for part order ( part predecessor,part successor)
   */
  typedef pair<Location,Location> Partbdryinfo_type;

  /**@brief 
     Iterator for parts in BasePContainer
  */
  typedef typename BasePContainer_type::parts_internal_iterator parts_internal_iterator;

  //=======================================
  //constructors 
  //=======================================
  /**@name Constructors and Destructor */
  //@{
  /**@brief 
     default constructor : instantiates a and empty part and initializes the part order
   */
  LinearOrderPContainer() {
    // A part is added in the BasePContainer constructor
    this->AddPart2PartIDMap(*(this->pcontainer_parts[0]));
#ifdef COMPLETE_LOCATION_MAP
    this->AddPart2LocationMap(*(this->pcontainer_parts[0]));
#endif
    this->dist.InitBoundaryInfo();
  }

  /**@brief 
   * Constructor: each thread creates a part of size n/p with default
   * value for its elements, initializes the part order and assigns
   * the distribution data members
   */

  LinearOrderPContainer(size_t n) {
    vector<GID> gidvec;
    int element_number=n/this->nprocs + (( n%this->nprocs) > this->myid);
    this->dist.GetNextGids(element_number,gidvec);
    pContainer_Part_type* _ct= new pContainer_Part_type(element_number,gidvec,0);
    //BasePContainer added an empty part already. Just copy into it
    this->pcontainer_parts[0] = _ct;      
    this->AddPart2PartIDMap(*_ct);
#ifdef COMPLETE_LOCATION_MAP
    this->AddPart2LocationMap(*_ct);
#endif
    this->dist.InitBoundaryInfo();
  }

   /**@brief 
   * Constructor: each thread creates a part of size n/p with the
   * value passed as parameter for its elements, initializes the part
   * order and assigns the distribution data members
   */
  LinearOrderPContainer(size_t  n, const value_type& _data) {
    vector<GID> gidvec;
    int element_number=n/this->nprocs+ + (( n%this->nprocs) > this->myid);
    this->dist.GetNextGids(element_number,gidvec);
    pContainer_Part_type* _ct= new pContainer_Part_type(element_number,
							gidvec,_data,0);
    //BasePContainer added an empty part already. Just copy into it
    this->pcontainer_parts[0] = _ct;      
    this->AddPart2PartIDMap(*_ct);
#ifdef COMPLETE_LOCATION_MAP
    this->AddPart2LocationMap(*_ct);
#endif
    this->dist.InitBoundaryInfo();  
  }

   /**@brief 
   * Copy constructor
   */

  LinearOrderPContainer(const LinearOrderPContainer_type& _lt) 
    : BasePContainer_type(_lt) {
  }

  /**@brief 
     Destructor
   */
  ~LinearOrderPContainer() {}
  //@}
  //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  // User Common Interface : All Containers derived from this class should have these methods
  //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

 public:
  virtual size_t max_size()=0;
  /**@name Accessors */
  //@{

  // The fron, local_front , back and local_back return type has been changed : in STL they all
  // return "reference" , but in STAPL they return iterator. Therefore they look just like the 
  // end , begin, local_begin and local_end. They were kept though for the sake of consistency.
  // Alin .
  /**@brief First element of the pList
   *@return an iterator to the front element of the pList.
   */
  iterator front() 
    {return this->local_begin();}
  /**@brief 
     Returns a constant iterator to the first local element
   */
  const_iterator front() const 
    {return this->local_begin();}
  /**@brief 
     Returns an iterator to the position right after the last local
     element
   */
  iterator back()
    {return this->local_end();}
  /**@brief 
     Returns a constant iterator to the position right after the last local
     element
   */
  const_iterator back ()const
    {return this->local_end();}



  //=======================================
  // Global element bookkeeping methods
  //=======================================
 public:

  /**@brief Adds elements from a sequential container at the end of the last
   *local part. It records all the distribution information for each
   *element that is inserted. 
   */
      
  virtual void AddElements(const Sequential_Container_type& _data) {
    for(typename Sequential_Container_type::const_iterator it = _data.begin(); 
	it!=_data.end(); ++it) 
      {
	this->AddElement(*it);
      }
  }
     
  /**@brief  
   * Inserts an element before the position pointed by it. It updates the distribution.
   */
  iterator insert( iterator _it, const value_type& _elm) {
    GID _gid = this->GetNextGid();
    typename Part_type::iterator sit = (*(_it.get_part()))->insert(_it.get_iterator(), _elm, _gid);
    this->dist.Add2PartIDMap(_gid,(*(_it.get_part()))->GetPartId());	
#ifdef COMPLETE_LOCATION_MAP
    Location location(this->myid,(*(_it.get_part()))->GetPartId());
    pair<GID,Location> tp(_gid,location);
    this->dist.Add2LocationMap(tp);
#endif
    return iterator(this,_it.get_part(),sit);
  }

 public:
  /**@brief 
   * Erases an element that is pointed to by the iterator it.
   */
  virtual iterator erase(iterator it) {
    printf("\n This is a use of Gid extracted from the iterator class. This is not general enough for all the linea pContainers ( some linear pContainers do no have the GID stored with the element (eg. pvector)");
    GID _gid=0;
    //    GID _gid = (it.get_iterator())->GetGid();
    this->dist.DeleteFromPartIDMap(_gid);
#ifdef COMPLETE_LOCATION_MAP
    this->dist.DeleteFromLocationMap(_gid);
    
#endif
    return iterator(it.get_pcontainer(),it.get_part(),(*(it.get_part()))->erase(it.get_iterator()));
  }
  
 protected:
  /* @brief Erase an element in the global plist, done locally, used in erase.
   *\b LOCAL
   */
  virtual iterator _erase(iterator _it) {  
    cout<<" _erase not implemented "<<endl;
    return _it;
  }

 public:
  /**@brief 
   * Adds an element at the end of the local storage
   */
  virtual void push_back(const value_type& _elm) {
    this->AddElement(_elm);
  }

  /**@brief 
   *Erases an element from the end of the local storage
   */
  virtual void pop_back() {   
    erase(--(this->local_end()));
  }

  //=======================================
  //PContainerParts bookkeeping 
  //=======================================
/*  protected: */
 public:

  /**@brief Adds a pContainer part
   *\b LOCAL
   */
  virtual int AddPart() {
    return BasePContainer_type::AddPart();
  }
  /**@brief 
   * Adds a part ot the local thread and sets its predecessor and
   * successor in the part order.
   */

  virtual int AddPart(const pContainer_Part_type& _ct,const Partbdryinfo_type& _bd) {
    PARTID id = BasePContainer_type::AddPart(_ct);
    this->dist.SetPartBdry(id,_bd);
    return id;
  }

  /**@brief 
   * Sets the predecessor and
   * successor for a part in the part order.
   */
  virtual void InitBoundaryInfo(PARTID _partid, Location _before, Location _after) {
    this->dist.InitBoundaryInfo(_partid, _before, _after);
  }

  /**@brief 
   *Sets part order 
   */
  void SetPartBdry(PARTID _id, const Partbdryinfo_type& _bd) {
    this->SetPartBdry(_id,_bd);
  }
  /**@brief 
   * Returns a pair of iterators that are set to the beginnig and end
   * of the part
   */
  pair<iterator,iterator> GetPartBoundary(PARTID _partid){
    pair<iterator,iterator> p;
    p.first = iterator(this,this->pcontainer_parts.begin()+_partid,(*(this->pcontainer_parts.begin()+_partid))->begin());
    if(_partid+1 < this->pcontainer_parts.size())
      p.second = iterator(this,this->pcontainer_parts.begin()+_partid+1,(*(this->pcontainer_parts.begin()+_partid+1))->begin());
    else
      p.second = iterator(this,this->pcontainer_parts.begin()+_partid,(*(this->pcontainer_parts.begin()+_partid))->end());
    return p;
  }

   int generate_gid(PARTID _partid, int _threadid) {
    //!!!!the maxparts should be extended to maxint; check potential problems
     int maxparts=10000;
      return _threadid*maxparts + _partid;
   }

   /*@brief
    *Build a pRange associated with the pcontainer
    *@param PRange output parameter
    */
 void get_prange(PRange& pr){
//   this->DisplayBoundaryInfo();
   for(int i=0;i<this->get_num_parts();i++){
     pair<iterator,iterator> pi=this->GetPartBoundary(i);
     typename PRange::subrangeType _leaf_prange(generate_gid(i,myid),this,linearBoundaryType(pi.first,pi.second),10000,&pr);
     pr.add_subrange(_leaf_prange);
   }
   stapl::rmi_fence();
  }

 void get_prange(reverse_PRange& pr){
//   this->DisplayBoundaryInfo();
   for(int i=0;i<this->get_num_parts();i++){
     pair<iterator,iterator> pi=this->GetPartBoundary(i);
     reverse_iterator first = reverse_iterator(pi.second);
     reverse_iterator second = reverse_iterator(pi.first);
     typename reverse_PRange::subrangeType _leaf_prange(generate_gid(i,myid),this,reverse_linearBoundaryType(first,second),10000,&pr);
     pr.add_subrange(_leaf_prange);
   }
   stapl::rmi_fence();
  }

  /**@brief 
   * Prints part order
   */
  void DisplayBoundaryInfo(){
    this->dist.DisplayBoundaryInfo();
  }
  //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  // End : User Common Interface 
  //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&



  //=======================================
  //Distribute and related methods
  //=======================================
 /**@brief Build pcontainer parts before rmi transfer.
  * It should build a pContainerPart object based on the distribution info. 
  */
  virtual void BuildSubContainer( const pair<int,int>& distinfo, 
			  vector<pContainer_Part_type>& subparts) { }

  /**@brief  Send a collection of pcontainer parts and associated bdryinfo
   * to a processor using rmi 
   * The pContainer part is added into pcontainer_parts
   * The bdryinfo is stored in Distribuion object
   *\b LOCAL
   */
  virtual int SetSubContainer(const vector<pair<pContainer_Part_type,
			                        Partbdryinfo_type> >&  _ct)
    {
      return OK;   //to use sync_rmi
    }

 /**@brief Merges pContainer parts that can be merged into larger parts
  */
   virtual void MergeSubContainer() {}
  
};

} //end namespace stapl
//@}
#endif
