#ifndef PARRAY_H
#define PARRAY_H
#ifndef _STAPL_THREAD
#include <mpi.h>
#endif
#include <vector>
#include <algorithm>
#include "BasePContainer.h"
#include "ArrayPart.h"
#include "ArrayDistribution.h"
#include <pRange.h>

/**
 * @ingroup pContainers
 * @defgroup parray Parallel Array
 * @{
 **/
namespace stapl {

#ifdef DET_PARRAY
  template<class T>
  struct closer : public binary_function<T, T, bool> {
    bool operator()(T x, T y) {
      if (x < 0)
        x = -x;
      if (y < 0)
        y = -y;
      if (x < y) 
        return true;
      return false;
    }
  };
#endif

  template<class R,class T>
  class ArrayExtractor {
    public:
    ArrayExtractor(){}
    
    R& GetUserData(const T& _current) const{
      return *_current;
    }
    
    GID GetGID(const T& _current) const{
      cout<<"GetGID is not implemented in pArray"<<endl;
    }

  };


  /**
    * Class definition
   */
  template<class T>
  class pArray : public BasePContainer<ArrayPart<T>, ArrayDistribution, no_trace, random_access_iterator_tag, stapl_element_tag >
    {
    public:
      //=======================================
      //types
      //=======================================

      ///The type of the elements stored inside pArray
      typedef T value_type;

      typedef typename ArrayPart<value_type>::Element_Set_type Array_Range_type;
      typedef  ArrayPart<value_type> pContainerPart_type;

      ///BasePContainer is the base class for pArray
      typedef BasePContainer<ArrayPart<value_type>, ArrayDistribution, no_trace, random_access_iterator_tag, stapl_element_tag  > BasePContainer_type;

      typedef typename BasePContainer_type::parts_internal_iterator parts_internal_iterator;

      typedef typename BasePContainer_type::iterator iterator;  
      typedef typename BasePContainer_type::reverse_iterator reverse_iterator;  

      ///pArray type
      typedef pArray<value_type> pArray_type;
      ///pArray distribution type
      typedef typename ArrayDistribution::DistributionInfo_type ArrayDistributionInfo_type;

      /**@brief 
       * pArray distribution info type; The pArray distribution is a 
       * vector of elements of this type
       */
      typedef ArrayDistributionInfo_type::iterator DistributionInfo_iterator;
      
      ///Part type
      typedef ArrayPart<T> Part_type;

      ///Iterator trough elements of the part
      typedef typename ArrayPart<T>::iterator part_iterator;

      ///Boundary information type
      typedef pair<Location,Location> Partbdryinfo_type;


      //Linear boundary for the pcontainer 
      typedef  stapl::linear_boundary<iterator> linearBoundaryType;
      typedef  stapl::linear_boundary<reverse_iterator> reverse_linearBoundaryType;

      //pRange corresponding to the pcontainer
      typedef stapl::pRange<linearBoundaryType, pArray_type> PRange;
      typedef stapl::pRange<reverse_linearBoundaryType, pArray_type> reverse_PRange;

      /**@name Constructors and Destructor */
      //@{
      //=======================================
      //constructors 
      //=======================================
 
      /**@brief  Default Constructor
       */
      pArray() {
        this->register_this(this);
        stapl::rmi_fence();
      }

      /**@brief  Constructor for a pArray of a specified size.
       *@param int the size of the pArray
       *The pArray is allocated in balanced way across all available threads.
       */
      pArray(int total_size) {
        int size = total_size;
	vector<ArrayPartsDistributionInfo> vec;
	ArrayPartsDistributionInfo temp;

        for (int i=0; i<this->nprocs; i++){
          if (i == (this->nprocs-1)){
	    temp.SetSize(size - (size/this->nprocs)*(this->nprocs - 1));
            temp.SetGID(i*(size/this->nprocs));
	    Location l(i,0);
	    temp.SetLocation(l);
	    vec.push_back(temp);
          }
          else{
	    temp.SetSize(size/this->nprocs);
            temp.SetGID(i*(size/this->nprocs));
	    Location l(i,0);
	    temp.SetLocation(l);
	    vec.push_back(temp);
          }
        }
        if (this->myid == (this->nprocs-1)){
          this->GetPart(0)->SetPart(size - (size/this->nprocs)*(this->nprocs - 
                      1), this->myid*(size/this->nprocs));   
        }
        else{
          this->GetPart(0)->SetPart(size/(this->nprocs), this->myid*(size/this->nprocs));           }

        this->register_this(this);
        rmi_fence(); 

        this->dist.InitializeArrayDistribution(size,vec);
        stapl::rmi_fence();
        AddPart2LocationMap(*(this->GetPart(0)));
        stapl::rmi_fence();
      }

      /**@brief  Constructor for a pArray of a specified size and distribution.
       *@param int the size of the pArray
       *@param ArrayPartsDistributionInfo_type user specified distribution.
       *The pArray is allocated according to the distribution specified by the user.
       */
      pArray(int total_size, const ArrayDistributionInfo_type& _distinfo) {
        this->dist.InitializeArrayDistribution(total_size,_distinfo);
	stapl::rmi_fence();
       
	this->GetPart(0)->SetPart(_distinfo[this->myid].GetSize(),_distinfo[this->myid].GetGID());
	AddPart2LocationMap(*(this->GetPart(0)));
        this->register_this(this);
        rmi_fence(); 
      }
  
      /*@brief
       * Constructor that allows for data initialization also; The functor will be 
       * called on every element of the pArray after the allocation. 
       */
      /*
      template <class Function>
      pArray(int total_size, Function init){
	int size = total_size;
	vector<ArrayPartsDistributionInfo> vec;
	ArrayPartsDistributionInfo temp;
	
        for (int i=0; i<this->nprocs; i++){
          if (i == (this->nprocs-1)){
	    temp.SetSize(size - (size/this->nprocs)*(this->nprocs - 1));
            temp.SetGID(i*(size/this->nprocs));
	    Location l(i,0);
	    temp.SetLocation(l);
	    vec.push_back(temp);
          }
          else{
	    temp.SetSize(size/this->nprocs);
            temp.SetGID(i*(size/this->nprocs));
	    Location l(i,0);
	    temp.SetLocation(l);
	    vec.push_back(temp);
          }
        }
        if (this->myid == (this->nprocs-1)){
          this->GetPart(0)->SetPart(size - (size/this->nprocs)*(this->nprocs - 
                      1), this->myid*(size/this->nprocs));   
        }
        else{
          this->GetPart(0)->SetPart(size/(this->nprocs), this->myid*(size/this->nprocs));   
        }
        this->register_this(this);
        rmi_fence(); 
        this->dist.InitializeArrayDistribution(size,vec);
        stapl::rmi_fence();
        AddPart2LocationMap(*(this->GetPart(0)));
        stapl::rmi_fence();	

	//initialize the elements of the parray according to the functor
	this->initialize(init);
      }
      */

      /**@brief  Copy constructor; use the base copy constructor
       *@param pArray source
       */
      pArray(const pArray& _other) : BasePContainer_type(_other) {
        this->register_this(this);
        rmi_fence(); 
      }

      /**@brief  
       *Copy constructor from an input PRange
       */
      pArray(PRange& _pr, bool copy_data = true) {
        /* variables in BasePContainer */
        this->myid = get_thread_id();
	this->nprocs = get_num_threads();
	this->part_id_counter = 0;

        typedef typename PRange::subrangeType SubrangeType;
        typename PRange::iteratorType pr_it, pr_end;
        vector<SubrangeType> vr = _pr.get_subranges();
        PARTID partid;
        int npart = vr.size();

	/* the first part has been added by BasePContainer's 
	 * default constructor, but the parray's "dist" didn't
	 * have its distribution_info vector initialized
	 * so put a record of ArrayPartsDistributionInfo_type for
	 * part#0 for each thread
	 * notice that size hasn't been set
	 */
        this->dist.InitializeArrayDistribution(); 
        rmi_fence(); 
	 
	/* resize part0
	 * add the other parts with correct size
	 * AddPart has handled the problem of AddPart to distribution_info
	 */
        if (copy_data == true) {
	  int size = std::distance(vr[0].get_boundary().start(), vr[0].get_boundary().finish() );
	  (*(pcontainer_parts[0])).SetPart(size, 0);
	  this->dist.AddPart(0, size, Location(myid, 0));
	  for (int i=1; i<npart; i++) {
	    size = std::distance(vr[i].get_boundary().start(), vr[i].get_boundary().finish() );
	    //cout<<"#"<<get_thread_id()<<" vr["<<i<<"].size="<<size<<endl;
	    this->AddPart(size);
	  } 
	} else {
	  (*(pcontainer_parts[0])).SetPart(1, 0);
	  this->dist.AddPart(0, 1, Location(myid, 0));
	  for (int i=1; i<npart; i++) {
	    this->AddPart(1);
	  }
	}
        stapl::rmi_fence();

	/* set boundaries for all parts
	 */
	if (! pcontainer_parts.empty() ) {
          for (partid = 0; partid < npart; partid++) {
	    Location before = _pr.GetPreviouspRange(partid);
	    Location next = _pr.GetNextpRange(partid);
	    //cout<<"pid="<<myid<<" partid="<<partid<<" before=("<<before.locpid()<<","<<before.partid()<<") next=("<<next.locpid()<<","<<next.partid()<<")"<<endl;
	    this->dist.InitBoundaryInfo(partid, before, next);
	  }
	}
        stapl::rmi_fence();

	/* thread#0 collects all parts' info in its syncdists
	 * accumulate the start_indices
	 * then send the vector back to everybody
	 */
        typedef typename ArrayDistribution::GlobalDistributionInfo_type GlobalDistributionInfo_type;
	GlobalDistributionInfo_type syncdists;
	this->dist.SynchronizeDistribution();
	stapl::rmi_fence();
	syncdists = this->dist.GetSyncDistribution();
        rmi_fence();
	for (int i=0; i < syncdists.size(); i++) {
	  if (syncdists[i].GetLocation().locpid() != myid) 
	    continue;
	    //cout<<"#"<<myid<<" i="<<i<<" syncdists[i].GetLocation()="<<syncdists[i].GetLocation()<<endl;
	  PARTID partid = syncdists[i].GetLocation().partid();
	  if (copy_data == true) {
	    int index=syncdists[i].GetGID();
	    int size=syncdists[i].GetSize();
	    //cout<<"#"<<myid<<" i="<<i<<" index="<<index<<" size="<<size<<endl;
	    (*(pcontainer_parts[partid])).SetPart(size, index);
	    if (i == 0) this->dist.set_start_index(index);
	    pr_it = vr[partid].get_boundary().start(); 
	    pr_end = vr[partid].get_boundary().finish();
	    for (int j=0; pr_it != pr_end; ++pr_it, ++index, ++j) {
	      Location myloc(myid, partid);
	      //cout<<"#"<<get_thread_id()<<" update ("<<index<<", ("<<myloc.locpid()<<","<<myloc.partid()<<")"<<endl;
	      //this->dist.UpdateLocationMap(index, myloc);
	      //cout<<"#"<<get_thread_id()<<" setelet ("<<index<<" as "<<*pr_it<<endl;
	      (*(pcontainer_parts[partid])).part_data[j] = *pr_it;
	    }
	  } else {
	    int index=syncdists[i].GetId();
	    if (i == 0) this->dist.set_start_index(index);
	    (*(pcontainer_parts[partid])).SetPart(1, index);
	    Location myloc(myid, partid);
	    //cout<<"#"<<get_thread_id()<<" to update ("<<index<<", ("<<myloc.locpid()<<","<<myloc.partid()<<")"<<endl;
	  }
	}
	stapl::rmi_fence();

        this->register_this(this);
        stapl::rmi_fence();
      }
  
      /**@brief  
       *Copy constructor from an input reverse_PRange
       */
      pArray(reverse_PRange& _pr, bool copy_data = true) {
        /* variables in BasePContainer */
        this->myid = get_thread_id();
	this->nprocs = get_num_threads();
	this->part_id_counter = 0;

        typedef typename reverse_PRange::subrangeType SubrangeType;
        typename reverse_PRange::iteratorType pr_it, pr_end;
        vector<SubrangeType> vr = _pr.get_subranges();
        PARTID partid;
        int npart = vr.size();

	/* the first part has been added by BasePContainer's 
	 * default constructor, but the parray's "dist" didn't
	 * have its distribution_info vector initialized
	 * so put a record of ArrayPartsDistributionInfo_type for
	 * part#0 for each thread
	 * notice that size hasn't been set
	 */
        this->dist.InitializeArrayDistribution(); 
        rmi_fence(); 
	 
	/* resize part0
	 * add the other parts with correct size
	 * AddPart has handled the problem of AddPart to distribution_info
	 */
        if (copy_data == true) {
	  int size = std::distance(vr[0].get_boundary().start(), vr[0].get_boundary().finish() );
	  (*(pcontainer_parts[0])).SetPart(size, 0);
	  this->dist.AddPart(0, size, Location(myid, 0));
	  for (int i=1; i<npart; i++) {
	    size = std::distance(vr[i].get_boundary().start(), vr[i].get_boundary().finish() );
	    //cout<<"#"<<get_thread_id()<<" vr["<<i<<"].size="<<size<<endl;
	    this->AddPart(size);
	  } 
	} else {
	  (*(pcontainer_parts[0])).SetPart(1, 0);
	  this->dist.AddPart(0, 1, Location(myid, 0));
	  for (int i=1; i<npart; i++) {
	    this->AddPart(1);
	  }
	}
        stapl::rmi_fence();

	/* set boundaries for all parts
	 */
	if (! pcontainer_parts.empty() ) {
          for (partid = 0; partid < npart; partid++) {
	    Location before = _pr.GetPreviouspRange(partid);
	    Location next = _pr.GetNextpRange(partid);
	    //cout<<"pid="<<myid<<" partid="<<partid<<" before=("<<before.locpid()<<","<<before.partid()<<") next=("<<next.locpid()<<","<<next.partid()<<")"<<endl;
	    this->dist.InitBoundaryInfo(partid, before, next);
	  }
	}
        stapl::rmi_fence();

	/* thread#0 collects all parts' info in its syncdists
	 * accumulate the start_indices
	 * then send the vector back to everybody
	 */
        typedef typename ArrayDistribution::GlobalDistributionInfo_type GlobalDistributionInfo_type;
	GlobalDistributionInfo_type syncdists;
	this->dist.SynchronizeDistribution();
	stapl::rmi_fence();
	syncdists = this->dist.GetSyncDistribution();
        rmi_fence();
	for (int i=0; i < syncdists.size(); i++) {
	  if (syncdists[i].GetLocation().locpid() != myid) 
	    continue;
	  //cout<<"#"<<myid<<" i="<<i<<" syncdists[i].GetLocation()="<<syncdists[i].GetLocation()<<endl;
	  PARTID partid = syncdists[i].GetLocation().partid();
	  if (copy_data == true) {
	    int index=syncdists[i].GetGID();
	    int size=syncdists[i].GetSize();
	    //cout<<"#"<<myid<<" i="<<i<<" index="<<index<<" size="<<size<<endl;
	    (*(pcontainer_parts[partid])).SetPart(size, index);
	    if (i == 0) this->dist.set_start_index(index);
	    pr_it = vr[partid].get_boundary().start(); 
	    pr_end = vr[partid].get_boundary().finish();
	    for (int j=0; pr_it != pr_end; ++pr_it, ++index, ++j) {
	      Location myloc(myid, partid);
	      //cout<<"#"<<get_thread_id()<<" update ("<<index<<", ("<<myloc.locpid()<<","<<myloc.partid()<<")"<<endl;
	      //this->dist.UpdateLocationMap(index, myloc);
	      //cout<<"#"<<get_thread_id()<<" setelet ("<<index<<" as "<<*pr_it<<endl;
	      (*(pcontainer_parts[partid])).part_data[j] = *pr_it;
	    }
	  } else {
	    int index=syncdists[i].GetId();
	    if (i == 0) this->dist.set_start_index(index);
	    (*(pcontainer_parts[partid])).SetPart(1, index);
	    Location myloc(myid, partid);
	    //cout<<"#"<<get_thread_id()<<" to update ("<<index<<", ("<<myloc.locpid()<<","<<myloc.partid()<<")"<<endl;
	  }
	}
	stapl::rmi_fence();

        this->register_this(this);
        stapl::rmi_fence();
      }

      /** Destructor
       */     
      ~pArray() {
        stapl::unregister_rmi_object( this->getHandle());
      }

      void define_type(typer &t)  {
        cout<<"ERROR::Method not yet implemented"<<endl;
      }
      //@}

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
	stapl::rmi_fence();
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
	stapl::rmi_fence();
      }

      /**@brief 
       * Initialize the data of the pArray; The functor will be called for every index of
       * the parray and the result will be stored in the element corresponding
       * to the index e.g., {parray[index] = functor(index)}.
       * @param Functor the user defined function that will be used to initialize the 
       * elements of the pArray.
       */
      template<class Functor>
      void initialize(Functor gen_fun){
	Part_type* part;
	part_iterator pi;
	int current_index;
	//for all parts
	for (int i=0; i < this->GetPartsCount();i++){
	  current_index = this->GetPart(i)->start_index;
	  part = this->GetPart(i);
	  for (pi = part->begin(); pi != part->end(); ++pi){
            *pi = gen_fun(current_index);
	    current_index++;
          }
	}
      }

      /**@brief  Assignment operator with a vector as argument
       *@param const vector<T> vector containing the elements that will 
       *be copied inside the current pArray.
       */
      pArray<T>& operator=(const vector<T>& v) {
	// ??? take a look at this because is not quite clean;
	// we should use BasePContainer copy constructor;
        T localsize = 0;
        for (int i=0; i<this->GetPartsCount();i++){
          for (int j=0; j<(this->GetPart(i))->size(); j++){
            localsize += (*(this->GetPart(i)))[j];
          }
        }

        GID start = this->dist.get_start_index();
        int j = 0;
        if (v.size() >= (localsize + start) )
          j = start;
        for (typename pArray<T>::iterator i = this->local_begin();
                                          i != this->local_end();
                                        ++i, ++j)
          *i = v[j];
      }


      void resize(int local_size){
	vector<int> gd(this->nprocs);
	vector<ArrayPartsDistributionInfo> vec;
	ArrayPartsDistributionInfo temp;
	int prefix=0;
	
	stapl::rmi_fence();
	for (int i=0; i<this->GetPartsCount();i++){
	  (this->GetPart(i))->clear();
	  delete (this->GetPart(i));
	}
	this->pcontainer_parts.clear();
	this->dist.ClearDistribution();
#ifndef _STAPL_THREAD
	// ???? temporary until we clarify if the primitives will provide this or not
	MPI_Allgather(&local_size,1,MPI_INT,&gd[0],1,MPI_INT,MPI_COMM_WORLD);
#endif

	//for(int i=0;i<this->nprocs;i++)
	//cout<<"RESIZE"<<gd[i]<<endl;
	for(int i=0;i<this->nprocs;i++){
	 temp.SetSize(gd[i]);
	 temp.SetGID(prefix);
	 Location l(i,0);
	 temp.SetLocation(l);
	 vec.push_back(temp);
	 prefix += gd[i];
	}
	this->dist.InitializeArrayDistribution(prefix,vec);
	Part_type* part = new Part_type(local_size, vec[myid].GetGID(), 0);
	this->pcontainer_parts.push_back(part);
	stapl::rmi_fence();
      }

      /**@brief  Copy the elements of the pArray to a vector
       * param vector<T> vector containing a copy of the elements of the pArray.
       */
      void initVector(typename std::vector<T>& v) {
        T localsize = 0;
        for (int i=0; i<this->GetPartsCount();i++){
          for (int j=0; j<(this->GetPart(i))->size(); j++){
            localsize += (*(this->GetPart(i)))[j];
          }
        }

        GID start = this->dist.get_start_index();
        int j = 0;
        if (v.size() >= (localsize + start) )
          j = start;
        for (typename pArray<T>::iterator i = this->local_begin();
                                          i != this->local_end();
                                        ++i, ++j)
          v[j] = *i;
      }

      int start_index(){
	return this->GetPart(0)->start_index;
      }      
       size_t max_size() {return size_t(-1);}
  
       /**@name Distribution related methods */
       //@{
       virtual bool IsLocal(GID _gid, PARTID& pid){
	 cout<<"IsLocal with PARTID not visible to the user"<<endl;
	 return true;
       }
       
       /**@brief Check if the element associated with _gid is local.
	* @param GID for which we want the check to be performed.
	* @return true if the element is local and false otherwise.
	**/
       bool IsLocal(GID _gid){
	 PARTID p;
	 return this->dist.IsLocal(_gid,p);
       }

       
       bool IsLocal(GID _gid) const{
	 PARTID p;
	 return this->dist.IsLocal(_gid,p);
       }

       /**@brief Find the location where the specified GID lives. 
       * @param GID for which we want the check to be performed.
       * @return Location the location where the element lives. InvalidLocation() if 
       * the element is not found.
      **/
       Location Lookup(GID _gid) const{
	 return this->dist.Lookup(_gid);
       }

       //@}

       //=======================================
       // Global element bookkeeping methods
       //=======================================
       
       /**@name Access and manipulation methods */
       //@{
 
       /**@brief Index operator should only be called in the right-hand-side of expressions.
       * if need to modify a remote element, use SetElement(GID,Data);
       */
       const value_type operator[](const GID _id) const {
	 return this->GetElement(_id);
       }
       //@}

      /*add part to the location map, element by element */
      virtual void AddPart2LocationMap(const ArrayPart<T>& _bt) {
	/*
        PARTID _partid = _bt.GetPartId();
        Location location(this->myid,_partid);
        for(int it=0; it<_bt.size(); ++it) {
          this->dist.UpdateLocationMap(_bt.start_index+it,location);
        }
	*/
      }
    
      /*@brief Used in BasePContainer, but not needed in pArray
       */
      virtual void AddPart2PartIDMap(const ArrayPart<T>& _bt) {}
      
      
  //=======================================
  //PContainerParts bookkeeping 
  //=======================================
    public:
      
      /**@name Parts Manipulation */
      //@{
      
      /**@brief Method to add an empty part to the pContainer
       *Calls the methods in the Base
       */
      virtual int AddPart() {
	return BasePContainer_type::AddPart();
      }
      
      /**@brief Method to add a premade part to the pContainer
       *Reimplemented from the base.
       */
      virtual int AddPart(const Part_type& _ct,const Partbdryinfo_type& _bd) {
	PARTID id = BasePContainer_type::AddPart(_ct);
	this->dist.SetPartBdry(id,_bd);
	return id;
      }
      
      /**@brief Method to add a part with a specified size
       *@param the size of the part to be added.
       */
      int AddPart(int size){
	PARTID id = this->GetPartsCount();
	GID start = this->size();
	Part_type* part = new Part_type(size, start, id);
	this->pcontainer_parts.push_back(part);
	Location loc(this->myid, id);
	Location before(this->myid,id-1);
	Location after(-1,INVALID_PART);  
	this->dist.IncreaseBoundaryInfo();
	InitBoundaryInfo(id,before,after);
	this->dist.AddPart(start, size, loc);
	return id;
      }

      int AddPart(int size, GID start) {
        PARTID id = this->GetPartsCount();
	Part_type* part = new Part_type(size, start, id);
	this->pcontainer_parts.push_back(part);
	Location loc(this->myid, id);
	Location before(this->myid,id-1);
	Location after(-1,INVALID_PART);  
	this->dist.IncreaseBoundaryInfo();
	InitBoundaryInfo(id,before,after);
	this->dist.AddPart(start, size, loc);
	return id;
      }
      
      /**@brief Merges contiguous pContainer parts
       */
      void MergeSubContainer() {
	cout<<"in MergeSubContainer "<<stapl::get_thread_id()<<endl;
	pair<int,int> mypair = this->dist.FindContinuousParts(this->myid);
	if (mypair.first != 0 || mypair.second != 0){
	  for (int i=0; i<this->GetPartsCount(); i++){
	    if (pcontainer_parts[i]->GetPartId()==mypair.first){
	      for (int j=0; j<this->GetPartsCount(); j++){
		if (pcontainer_parts[j]->GetPartId()==mypair.second){
		  Part_type* part1 = this->GetPart(i);
		  Part_type* part2 = this->GetPart(j);
		  Part_type* part = new Part_type(part1->size() + part2->size(),
						  part1->start_index, mypair.first);
		  part_iterator it=part->begin();
		  for (part_iterator iit=part1->begin(); iit<part1->end(); iit++){
		    *it = *iit;
		    it++;
		  }
		  for (part_iterator jit=part2->begin(); jit<part2->end(); jit++){
		    *it = *jit;
		    it++;
		  }
		  
		  parts_internal_iterator iter = this->pcontainer_parts.begin()+i;
		  delete part1;
		  *iter = part;
		  delete part2;
		  pcontainer_parts.erase(this->pcontainer_parts.begin()+j);
		  
		  this->dist.MergeUpdate1(part->start_index, myid);
		  
		  Location loc(this->myid, i);
		  Location before1(-1,INVALID_PART);
		  Location before2(this->myid,i-1);
		  Location after1(this->myid,i+1);
		  Location after2(-1,INVALID_PART);  
		  this->dist.IncreaseBoundaryInfo();
		  if (i==0){
		    if (i<this->GetPartsCount()-1){
		      InitBoundaryInfo(i,before1,after1);
		    }
		    else{
		      InitBoundaryInfo(i,before1,after2);
		    }
		  }
		  else{
		    if (i<this->GetPartsCount()-1){
		      InitBoundaryInfo(i,before2,after1);
		    }
		    else{
		      InitBoundaryInfo(i,before2,after2);
		    }
		  }
		  return;
		}
	      }
	    }
	  }
	}
	
      }
      
      /**@brief 
       *Init boundary info for multiple parts.
       *@param PARTID the part for which we are setting the Boundaru info.
       *@param Location _before: the location of the predecessor part 
       *@param Location _after: the location of the succesor part 
       */
      virtual void InitBoundaryInfo(PARTID _partid, Location _before, Location _after) {
	this->dist.InitBoundaryInfo(_partid, _before, _after);
      }
      
      /*Setup bdry info after redistribution
       */
      void SetPartBdry(PARTID _id, const Partbdryinfo_type& _bd) {
	this->SetPartBdry(_id,_bd);
      }
      
      //pair<iterator,iterator> GetPartBoundary(PARTID _partid){
      // pair<iterator,iterator> p;
      //  p.first = iterator(this,this->pcontainer_parts.begin()+_partid,(*(this->pcontainer_parts.begin()+_partid))->begin());
      //  p.second = iterator(this,this->pcontainer_parts.begin()+_partid,(*(this->pcontainer_parts.begin()+_partid))->end());
      //  return p;
      //}

  
      /**@brief Method to split a part into 2 parts.
       * requires the size of the first part to be specified
       *@param PARTID the part to be split.
       *@param int size1 the size of the first part(the size of the second one is deducted).
       */ 
      void SplitPart(PARTID _id, int size1){
	assert(_id < this->GetPartsCount());
	Part_type* part = this->GetPart(_id);
	part_iterator j=part->begin();
	
	Part_type* part1 = new Part_type(size1, part->start_index, _id);
	Part_type* part2 = new Part_type(part->size() - size1, part->start_index + size1, this->GetPartsCount());
	
	Location loc1(this->myid, _id);
	Location before1a(-1,INVALID_PART);
	Location before1b(this->myid,_id-1);
	Location after1(this->myid,_id+1);
	this->dist.IncreaseBoundaryInfo();
	if (_id == 0)
	  InitBoundaryInfo(_id,before1a,after1);
	else
	  InitBoundaryInfo(_id,before1b,after1);
	
	Location loc2(this->myid, this->GetPartsCount());
	Location before2(this->myid,this->GetPartsCount()-1);
	Location after2(-1,INVALID_PART);  
	this->dist.IncreaseBoundaryInfo();
	InitBoundaryInfo(_id,before2,after2);
	
	for (part_iterator i=part1->begin(); i<part1->end(); i++){
	  *i = *j;
	  j++;
	}
	
	for (part_iterator i=part2->begin(); i<part2->end(); i++){
	  *i = *j;
	  j++;
	}
	
	parts_internal_iterator it = this->pcontainer_parts.begin()+_id;
	delete part;
	*it = part1;
	this->pcontainer_parts.push_back(part2);
	this->dist.SplitPart(_id, pcontainer_parts.size()-1, size1);
      }
      //end of parts bookkeeping
      //@}


      /**@name Numeric methods */
      //@{

      /**@brief Basic sum function
       */
      void sum(T* in, T* inout) { *inout += *in; }
      
      /**@brief Accumulate function of an array 
       */
      T accumulate(){ 
	T result = _accumulate();
	T result1 = 0;
#ifdef DET_PARRAY
	reduce_rmi(&result,&result1, this->getHandle(),&pArray_type::sum, false);
#else
	reduce_rmi(&result,&result1, this->getHandle(),&pArray_type::sum, true);
#endif
	return result1;
      } 
      
      T _accumulate(){
	T localresult = 0;
	for (int i=0; i<this->GetPartsCount();i++){
	  for (int j=0; j<(this->GetPart(i))->size(); j++){
	    localresult += (*(this->GetPart(i)))[j];
	  }
	}
	return localresult;
      }
      
      /**@brief "Dot product" of  2 arrays of the same size
       *@param pArray<T>& reference to the second pArray.
       *@return T scalar corresponding to the dot product for the two pArrays.
       */
      T dotproduct(pArray<T>& x){ 
	pArray<T> z;
	stapl::rmi_fence();
	z = (*this) * x;
	
#ifdef DET_PARRAY
        std::sort(z.local_begin(), z.local_end(), closer<T>());
#endif
	T result = z.accumulate();
	return result;
      } 
      
      /**@brief Euclidean norm of a vector.
       *@return long double the euclidean norm of the pArray.
       */
      long double euclideannorm(){
	pArray<T> z;
	stapl::rmi_fence();
	
	z = (*this) * (*this);
	
#ifdef DET_PARRAY
        std::sort(z.local_begin(), z.local_end(), closer<T>());
#endif
	T temp = z.accumulate();
	long double result =  ::sqrt((long double)temp);
	return result;
      }
      //@}
      
      /**@name Input/Output methods */
      //@{
      //                      I/O related methods

      /**@brief
       *Display distribution info.
       */
      void DisplayDistributionInfo(){
	this->dist.DisplayDistributionInfo();
      }
      
      /**@brief Display boundary info.
       */
      void DisplayBoundaryInfo(){
	this->dist.DisplayBoundaryInfo();
      }
      //@}

      int SetSubContainer(const vector<pContainerPart_type>  _ct){
	printf("\n This is -SetSubContainer- method from pArray ...... NOT YET IMPLEMENTED");
	return OK;
      }
      
      void BuildSubContainer( const vector<Array_Range_type>&,
			      vector<pContainerPart_type>&){
	printf("\n This is -BuildSubContainer- method from pArray ...... NOT YET IMPLEMENTED");
      }
            
    };//end of pArray
  //@}

  /**@brief 
   * @ingroup parray
   * @defgroup numeric_array Numeric algorithms for pArray
   * @{
   **/
  
  /**@brief  Adding a scalar and a pArray.
   */
  template <class T>
  inline pArray<T>  operator+(const T& scalar, const pArray<T>& x){  
    pArray<T> z(x);
    stapl::rmi_fence();
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
        (*z.GetPart(i))[j] = scalar + (*(z.GetPart(i)))[j] ;
      }
    }
    return z;
  } 
  
  /**@brief  Subtracting a scalar and a pArray.
   */
  template <class T>
  inline pArray<T>  operator-(const pArray<T>& x, const T& scalar){  
    pArray<T> z(x);
    stapl::rmi_fence();
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
        (*z.GetPart(i))[j] = (*(z.GetPart(i)))[j] - scalar;
      }
    }
    return z;
  } 
  
  /**@brief  Subtracting a vector and a pArray.
   */
  template <class T>
  inline pArray<T>  operator-(const pArray<T>& x, const std::vector<T>& vec) {  
    pArray<T> z(x);
    stapl::rmi_fence();
    int index = 0;
    for (int i=0; i<x.GetPartsCount(); i++){
      //index = (x.GetPart(i))->start_index;
      for (int j=0; j<(x.GetPart(i))->size(); ++j, ++index){ 
        (*z.GetPart(i))[j] = (*(z.GetPart(i)))[j] - vec[index];
      }
    }
    return z;
  } 
  
  /**@brief  Subtracting a vector and a pArray.
   */
  template <class T>
  inline pArray<T>  operator-(const std::vector<T>& vec, const pArray<T>& x) {  
    pArray<T> z(x);
    stapl::rmi_fence();
    int index = 0;
    for (int i=0; i<x.GetPartsCount(); i++){
      //index = (x.GetPart(i))->start_index;
      for (int j=0; j<(x.GetPart(i))->size(); ++j, ++index){ 
        (*z.GetPart(i))[j] = vec[index] - (*(z.GetPart(i)))[j];
      }
    }
    return z;
  } 
  
  /**@brief  Adding a scalar and a pArray.
   */
  template <class T>
  inline pArray<T>  operator+(const pArray<T>& x, const T scalar){  
    pArray<T> z;
    stapl::rmi_fence();
    z = x;
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
	(*z.GetPart(i))[j] = scalar + (*(z.GetPart(i)))[j] ;
      }
    }
    return z;
  }
  
  /**@brief Adding 2 arrays of the same size.
   */
  template <class T>
  inline pArray<T> operator+(const pArray<T>& x, const pArray<T>& y){
    pArray<T> z;
    stapl::rmi_fence();
    z = x;
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){
	(*z.GetPart(i))[j] += y[(*x.GetPart(i)).start_index+j];
      }
    }
    return z;
  }
  
  /**@brief Subtracting 2 arrays of the same size.
   */
  template <class T>
  inline pArray<T> operator-(const pArray<T>& x, const pArray<T>& y){
    pArray<T> z;
    stapl::rmi_fence();
    z = x;
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){
	(*z.GetPart(i))[j] -= y[(*x.GetPart(i)).start_index+j];
      }
    } 
    return z;
  }
  

 
  /**@brief  Multiplying a scalar and a pArray.
   */
  template <class T>
  inline pArray<T>  operator*(const T scalar, pArray<T>& x){  
    pArray<T> z(x) ;
    stapl::rmi_fence();
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
	(*z.GetPart(i))[j] = scalar * (*(z.GetPart(i)))[j] ;
      }
    }
    return z;
  } 
  
  template <class T>
  inline pArray<T>  operator*(const pArray<T>& x,const T scalar){  
    pArray<T> z(x);
    stapl::rmi_fence();
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
	(*z.GetPart(i))[j] = scalar * (*(z.GetPart(i)))[j] ;
      }
    }
    return z;
  } 
  
  /**@brief  Multiplying two arrays of the same size.
   */
  template <class T>
  inline pArray<T>  operator*(const pArray<T>& x, const pArray<T>& y){  
    pArray<T> z(x);
    stapl::rmi_fence();
    for (int i=0; i<x.GetPartsCount(); i++){
      for (int j=0; j<(x.GetPart(i))->size(); j++){ 
        (*z.GetPart(i))[j] *= y[(*z.GetPart(i)).start_index+j];
      }
    }
    return z;
  } 

  /*@brief  Multiplying an array with a STL vector whose size is the global size of the array.
   */
//    template <class T>
//        inline T  operator*(pArray<T>& x, std::vector<T>& y) {
//        }

  /**@brief  Multiplying an array with a STL vector whose size is the global size of the array.
   */
//    template <class T>
//        inline T  operator*(std::vector<T>& x, pArray<T>& y) {
//        }

  /**@brief  Multiplying an array by a 2D STL vector.  
    *  The vector size is equal to the total size of the array.
    */
  template <class T>
  inline pArray<T>  operator*(std::vector<std::vector<T> >& x, pArray<T>& y) {
    pArray<T> z(y);
    stapl::rmi_fence();
    for (int i = 0; i < y.GetPartsCount(); ++i) {
      int index = (*y.GetPart(i)).start_index;
      for (int j = 0; j < (y.GetPart(i))->size(); ++j, ++index) { 
        z.SetElement(index, 0);
        for (int k = 0; k < x.size(); ++k) {
          z.SetElement(index, z[index] + x[k][index] * y[index]);
        }
      }
    }
    return z;
  }

  /**@brief  Multiplying an array by a 2D STL vector.  
   *  The vector size is equal to the total size of the array.
   */
  template <class T>
  inline pArray<T>  operator*(pArray<T>& x, std::vector<std::vector<T> >& y) {
    pArray<T> z;
    stapl::rmi_fence();
    z = x;
    for (int i = 0; i < x.GetPartsCount(); ++i) {
      int index = (*x.GetPart(i)).start_index;
      for (int j = 0; j < (x.GetPart(i))->size(); ++j, ++index) { 
        z.SetElement(index, 0);
        for (int k = 0; k < x.size(); ++k) {
          z.SetElement(index, z[index] + y[k][index] * x[index]);
        }
      }
    }
    return z;
  }

} //end namespace stapl
//@}
#endif
