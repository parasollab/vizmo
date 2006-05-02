/*!
	\file BasePContainer.h
	\date Jan. 9, 03
	\ingroup stapl
	\brief base class to provide interfaces 
	and basic functions for each pContainer to DEVELOPER and advanced user

*/

#ifndef BASEPCONTAINER_H
#define BASEPCONTAINER_H
#define COMPLETE_LOCATION_MAP


#include "rmitools.h"
#include "splBaseElement.h"
#include "AbstractBasePContainer.h"
#include "pContainerBaseIterator.h"
#include "splLocalIterator.h"
#include "stapl_pc_tracer.h"

#ifdef _TIMER_EXPERIMENT
extern stapl::timer tr_BPC_GE_IsLocal,tr_BPC_GE_getelement,tr_BPC_GE_lookup,tr_BPC_GE_retrieve_gid,tr_build,tr_set,tr_merge;
extern double t_BPC_GE_IsLocal,t_BPC_GE_getelement,t_BPC_GE_lookup,t_BPC_GE_retrieve_gid,t_build,t_set,t_merge; 
#endif

namespace stapl {

  /**
   * @addtogroup basepcontainer
   * \ref BasePContainer Class
   * @{
  **/

  /**
   *BasePContainer is the class from which all other pContainers
   *are derived.
   * \nosubgrouping
   **/
  template<class pContainer_Part_type,class Distribution_type,class trace_class, typename Iterator_tag, typename element_tag >
    class BasePContainer : public BasePObject, 
    public AbstractBasePContainer<pContainer_Part_type>
    {
    public:
      

      //=======================================
      //types
      //=======================================
      ///Part type
      typedef pContainer_Part_type pContainerPart_type;

      ///The BasePContainer type
      typedef BasePContainer<pContainerPart_type, Distribution_type, trace_class, Iterator_tag,element_tag>  this_type;

      ///Value type
      typedef typename pContainerPart_type::value_type value_type;

      typedef stapl_iterator_type<this_type,trace_class, element_tag, Iterator_tag> stapl_iterator_def;

      ///pContainer iterator
      typedef typename stapl_iterator_type<this_type, trace_class, element_tag, Iterator_tag>::iterator  iterator;
      ///pContainer const iterator
      typedef const typename stapl_iterator_type<this_type,trace_class, element_tag, Iterator_tag>::iterator  const_iterator;  
      // reverse iterators 
      typedef std::reverse_iterator<iterator>  reverse_iterator;  

      typedef std::reverse_iterator<const_iterator>  const_reverse_iterator;  

      typedef typename pContainerPart_type::Element_Set_type Element_Set_type;

      typedef typename pContainerPart_type::BaseElement_type BaseElement_type;

      ///Sequential container of type value_type
      typedef typename pContainerPart_type::Container_type Container_type;//the seq container on base elements

      typedef typename pContainerPart_type::Sequential_Container_type Sequential_Container_type;


      typedef map<PID, vector<Element_Set_type> > Distribution_Info_type;

      typedef typename vector<pContainerPart_type*>::iterator        parts_internal_iterator;
      typedef typename vector<pContainerPart_type*>::const_iterator  const_parts_internal_iterator;
  
    protected:
      //=======================================
      //data
      //=======================================
      /**@brief  Collection of pContainer Parts 
       **/
      vector<pContainerPart_type*> pcontainer_parts;  

      /**@brief  Associated Distribution object with each pContainer 
       **/
      Distribution_type dist;

      /**@brief pContainer part ID counter
       **/
      size_t part_id_counter;

      
      /**
       * \brief This is a variable that records the trace of the operations
       * invoked on the instatiated pvector if the template argument \b
       * trace_class is \b trace.
       *
       */
      stapl_pc_tracer __tracer;

    public:
      /**@brief  The identifier for the current thread. This is an unsigned int betwen zero and the 
       *number of available threads 
       **/
      PID myid;

      /**@brief  Number of available threads **/
      unsigned int nprocs;

    public:
      /**@name Constructors and Destructor */
      //@{
      //=======================================
      //constructors 
      //=======================================
      /**@brief Default Constructor. It initializes the storage for the pContainer,
       *by adding an initial part.
       **/
      BasePContainer() : 
	myid(get_thread_id()), 
	nprocs(get_num_threads()),
	part_id_counter(0)
	{
	  //add an empty part, so the default 
	  //global begin and end can point to something.
	  AddPart();
	}

      /**@brief 
       * Copy Constructor 
      **/
      BasePContainer(const this_type& _bp) :
	myid(get_thread_id()), 
	nprocs(get_num_threads()),
	dist(_bp.GetDistribution()),
	part_id_counter(0)
	{
	  for(int i=0;i<_bp.pcontainer_parts.size();i++)
	    AddPart(*(_bp.pcontainer_parts[i]));
	}

      /**@brief Constructor taking user input to distribute/readin 
       * data into pcontainer
       **/

      /*
      //to BE fixed
      BasePContainer(const PartitionInputs* user_inputs) :
	part_id_counter(0)
	{
	  myid = stapl::get_thread_id();
	  nprocs = stapl::get_num_threads();
	  if(user_inputs->valid_inputs) {
	    if(user_inputs->partpolicy == GIVEN_DIST) {
	      //TODO: interface methods to read in distribuited data  --ann
	    }  else {
	      if(myid==0) {
		Distribution_Info_type data_map;
		dist.Parser(user_inputs,data_map);
		SetDistributionInfo(data_map);
		Distribute(data_map);
	      }   
	    }
	  } else {
	    cout<<"Error, Invalid user inputs, exit!"<<endl;
	    exit(1);
	  }
	  if(myid == MASTER_THREAD) {
	    global_begin_location = Location(0,0);
	    global_end_location = Location(nprocs-1,0);
	  }
	}
      **/

      /**@brief Destructor. It dealocates the space occupied by the pContainer's parts.    
       **/
      ~BasePContainer() {
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); 
	    ++it) {
	  delete (*it);
	}
	pcontainer_parts.clear();
      }
      //@}

      template<class INTYPE>
      void CopyStructure(const INTYPE& inpcontainer) {
        PARTID partid;
 	int npart = inpcontainer.GetPartsCount();
	for (int i=0; i<npart-1; i++) { //one part has been added in constructor
	  partid = this->AddPart();
	}
	stapl::rmi_fence();
	if(!pcontainer_parts.empty()) {
	  for (partid=0; partid<npart; partid++) {
            Location before = inpcontainer.GetDistribution().GetBdBeginInfo(partid);
            Location next = inpcontainer.GetDistribution().GetBdEndInfo(partid);
	    this->dist.InitBoundaryInfo(partid, before, next);
	  }
	}
	stapl::rmi_fence();
      }

      /**@name Operators */
      //@{
      //=======================================
      //operator
      //=======================================
      /**@brief Assignment operator
       **/
      this_type& operator=(const this_type& _bp) {

	if(!pcontainer_parts.empty()) {
          for(parts_internal_iterator it = pcontainer_parts.begin();
              it !=pcontainer_parts.end(); ++it)
            delete (*it);
	  pcontainer_parts.clear();
	  part_id_counter = 0;
	}

	for(int i=0;i<_bp.pcontainer_parts.size();++i){
	  pContainerPart_type * tmp = new pContainerPart_type(*(_bp.GetPart(i)));
	  pcontainer_parts.push_back(tmp);
	}
	dist = _bp.dist;
	return *this;
      }
      //@}
      


      /**@name Accessors */
      //@{
      //=======================================
      //access elements in pContainer
      //=======================================
      /**@brief Local begin
       * returns a pointer to the beggining of the local data.
       **/
      iterator local_begin() {	 
	return iterator(this);
      }
      
      /**@brief Local end
       * returns a pointer to the end of the local data.
       **/
      iterator local_end()  {
	return iterator(this, pcontainer_parts.end()-1 , (*(pcontainer_parts.end()-1))->end());
      }
      
      /**@brief const local begin
       **/
      const_iterator local_begin() const{
	return const_iterator(this);
      }
      
      /**@brief const local end
       **/
      const_iterator local_end() const {
	return const_iterator(this,
			      (*(pcontainer_parts.begin()+
				 pcontainer_parts.size()-1))->end(),
			      pcontainer_parts.size()-1,true);
      }
 
    /**@brief Local rbegin - returns a reverse iterator to the last element of the local container
     **/
    reverse_iterator local_rbegin() 
    {
      return reverse_iterator(local_end());
    }
        
    /**@brief const local rbegin - returns a constant reverse iterator to the last element of the local container
     **/
    
    const_reverse_iterator local_rbegin() const 
    {
      return const_reverse_iterator(local_end());
    }
	
    /**@brief local rend - returns a constant reverse iterator to the last element of the local container
     **/
    reverse_iterator local_rend() 
      {
	return reverse_iterator(local_begin());
      }
    /**@brief const local rend - returns a constant reverse iterator to the first element of the local container
     **/    
    const_reverse_iterator local_rend() const  
    {
      return const_reverse_iterator(local_begin());
    }
    
    //@}


    /**@name Access and manipulation methods */
    //@{

    //=======================================
    //size statistics
    //=======================================
   
    /**@brief pContainer size
       @return the size of the pContainer
     **/
    virtual size_t size()
      {
	size_t _s=0;
	
	for(int i=0; i<nprocs; ++i)
	  {
	    if(myid == i) _s += local_size();
	    else
	      //_s+=0;
	      _s+=stapl::sync_rmi(i,getHandle(),
				  (size_t (this_type::*)())&this_type::local_size);
	  }
	return _s;
      }
    
    
   
    virtual size_t local_size(){
      size_t _size=0;
      
      for(int i=0; i<pcontainer_parts.size(); ++i)
	{
	  _size+=pcontainer_parts[i]->size();
	}
      return _size;
    }
    

    /**@brief Local size. 
     *@return The size of all the parts that are local to this thread.
     **/
    virtual size_t local_size() const 
      {
	size_t _size=0;
	
	for(int i=0; i<pcontainer_parts.size(); ++i)
	  {
	    _size+=pcontainer_parts[i]->size();
	  }
	
	return _size;
      }
    
    /**@brief Check if the pContainer is empty.
     *@return true if the pContainer is empty and false otherwise.
     **/
    virtual bool empty()  
      {
	
	for(int i=0; i<nprocs; ++i)
	  {
	    if(sync_rmi(i,getHandle(),(bool (this_type::*)())&this_type::local_empty)==false)
	      return false;
	    
	  }
	  return true;
      }

    virtual bool local_empty()
      {
	for(int i=0; i<pcontainer_parts.size(); ++i)
	  {
	    if(pcontainer_parts[i]->empty()==false)
	      return false;
	  }
	return true;
      }
    

    /**@brief Check if all the local part are empty.
     *@return true if all local parts are empty and false otherwise.
     **/
    virtual bool local_empty() const
	{
	  for(int i=0; i<pcontainer_parts.size(); ++i)
	    {
	      if(pcontainer_parts[i]->empty()==false)
		return false;
	    }
	
	  return true;

	}


      /* Compute the next available GID
       * @return the next available GID. 
       * It can be used for methods that add elements to the pContainer.
       * See BaseDistribution::GetNextGid()
       **/
      virtual GID GetNextGid() {
	return dist.GetNextGid();
      }

      /**@brief 
       *Erase all the elements in the PContainer
       **/
      void clear(){
	for(int p=0;p<this->nprocs;p++){
	  stapl::async_rmi(p,
			   this->getHandle(),
			   &this_type::local_clear); 
	}
      }

      /**@brief 
       *Erase all the elements stored inside the parts on the current thread.
       **/
      void local_clear(){
	for(parts_internal_iterator it = pcontainer_parts.begin(); 
	    it !=pcontainer_parts.end(); ++it) {
	  (*it)->clear();
	}
      }

      //=======================================
      //PContainer Elements bookkeeping 
      //=======================================
    protected:
      virtual const value_type _GetElement(const GID _gid, PARTID partid) const
	{
	  if(partid != INVALID_PART) 
	    return GetPart(partid)->GetElement(_gid);
	  else {
	    //throw Invalid_Part();
	    return value_type();
	  }
	
	}

    public:
      /**@brief 
       * Get the value of an element associated with a certain GID.
       * @param _gid the GID for which we request the data.
       * @return value_type the value of the element associated with _gid
       **/
      virtual const value_type GetElement(const GID _gid) const
	{
	  PARTID partid;
#ifdef _TIMER_EXPERIMENT
	  tr_BPC_GE_IsLocal=start_timer();
#endif
	  bool a=IsLocal(_gid,partid);
#ifdef _TIMER_EXPERIMENT
	  t_BPC_GE_IsLocal+=stop_timer(tr_BPC_GE_IsLocal);
#endif
	  if(a)
	    {
#ifdef _TIMER_EXPERIMENT
	      tr_BPC_GE_getelement=start_timer();
#endif
	      const value_type rtr=_GetElement(_gid,partid);
#ifdef _TIMER_EXPERIMENT
	      t_BPC_GE_getelement+=stop_timer(tr_BPC_GE_getelement);
#endif
	      return rtr;
	    }
	  else
	    {
	      value_type rtn;		
#ifdef _TIMER_EXPERIMENT
	      tr_BPC_GE_lookup=start_timer();
#endif
	      Location loc = dist.Lookup(_gid);
#ifdef _TIMER_EXPERIMENT
	      t_BPC_GE_lookup+=stop_timer(tr_BPC_GE_lookup);
	      tr_BPC_GE_retrieve_gid=start_timer();
#endif
              stapl_assert(loc.ValidLocation(),"Invalid id for GetElement");
	      rtn=stapl::sync_rmi(loc.locpid(),getHandle(),
                                  &this_type::_GetElement,_gid,loc.partid());
	      //	      printf("\n Get element %d location (%d,%d) return %d", _gid,loc.part_id,loc.pid,rtn);
#ifdef _TIMER_EXPERIMENT
	      t_BPC_GE_retrieve_gid+=stop_timer(tr_BPC_GE_retrieve_gid);
#endif
	      return rtn;
	    }

	}


    protected:
      virtual void  _SetElement(const pair<GID,PARTID>& gp, const value_type& _t)
	{
	  if(gp.second != INVALID_PART) {
	    GetPart(gp.second)->SetElement(gp.first,_t);
	  }
	  /* 	  else throw Invalid_Part(); */
	}

    public:
      /**@brief 
       * Set the value of an element associated with a certain GID.
       * @param _gid the GID for which we want to set data.
       * @param value_type the value of the element associated with _gid
       **/
      virtual void SetElement(GID _gid, const value_type& _t)    
	{
	  PARTID partid;
	  if(IsLocal(_gid,partid))
	    {
	      _SetElement(pair<GID,PARTID>(_gid,partid),_t);
	    }
	  else
	    {			
	      Location loc = dist.Lookup(_gid);
              stapl_assert(loc.ValidLocation(),"Invalid id for SetElement");
	      stapl::async_rmi(loc.locpid(),getHandle(),
			      &this_type::_SetElement,
			      pair<GID,PARTID>(_gid,loc.partid()),_t);

	    }

	}

    protected:
      virtual int _DeleteElement(GID _gid,PARTID partid)
	{
	  if(partid != INVALID_PART) {
	    GetPart(partid)->DeleteElement(_gid);
	    //should also check if the part is empty, then do 
	    //a clean up, update the global begin and end partid if necessary --ann
	    dist.DeleteFromPartIDMap(_gid);
#ifdef COMPLETE_LOCATION_MAP
	    dist.DeleteFromLocationMap(_gid);
#endif
	    return OK; 
	  }
/* 	  else throw Invalid_Part(); */
	  return ERROR;
	}

    public:


      /**@brief 
       * Delete an element associated with the specified GID
       * @param _gid the GID we want to remove from the pContainer.
       **/

      virtual void DeleteElement(GID _gid)
	{
	  PARTID partid;
	  if(IsLocal(_gid,partid))
	    {
	      _DeleteElement(_gid,partid);
	    }
	  else
	    {
	      Location loc = dist.Lookup(_gid);
	      if (loc.ValidLocation())
	        stapl::async_rmi(loc.locpid(),getHandle(),
			         &this_type::_DeleteElement,_gid,loc.partid());
	    }
	}


      /**@brief Add a new element to the pContainer. A new GID will be generated automaticaly.
       * The element will be added to the last part.
       * @param value_type element to be added. 
       * @return The GID associated with the data   
       **/
      virtual GID AddElement(const value_type& _t) {
	GID _gid = GetNextGid();
	AddElement(_t,_gid);
	return _gid;
      }

      /**@brief Add a new element to the pContainer. The element will be associated with
       * the specified GID. The element will be added to the last part.
       * @param value_type element to be added. 
       * @return The GID associated with the data   
       **/
      virtual void AddElement(const value_type& _t, GID _gid) {
	PARTID lastpartid;
	if(!pcontainer_parts.empty()) {
	  lastpartid = pcontainer_parts.size()-1;
	} else {
	  lastpartid = AddPart();
	} 
	AddElement2Part(_t,lastpartid,_gid);
      }

    public:
      /**@brief Add a new element to the pContainer. A new GID will be generated automaticaly.
       * The element will be added to the specified part.
       * @param value_type element to be added. 
       * @param PARTID the part identifier where the element will be added.
       * @return The GID associated with the data   
       **/
      virtual void AddElement2Part(const value_type& _t, PARTID _partid) {
	GID _gid = GetNextGid();
	AddElement2Part(_t,_partid,_gid);
      }


      /**@brief Add a new element to the pContainer. The element will be associated with
       * the specified GID. The element will be added to the last part.
       * @param value_type element to be added. 
       * @param PARTID the part identifier where the element will be added.
       * @return The GID associated with the data   
       **/
      virtual void AddElement2Part(const value_type _t, PARTID _partid, GID _gid) {
	(*(pcontainer_parts.begin()+_partid))->AddElement(_t,_gid); 
	dist.Add2PartIDMap(_gid,_partid);	
#ifdef COMPLETE_LOCATION_MAP
	Location location(myid,_partid);
	pair<GID,Location> tp(_gid,location);
	dist.Add2LocationMap(tp);
#endif
      }
      
      //===================================================================
      //   Get/Set Vertex field
      //      Methods for setting/getting vertex fields
      //      run a user specified method on the weight or vertex class
      //      this is used to hide the local/remote issue
      //=================================================================
	
  public:
      /**@brief Apply the specified work function Functor to an element
       * identified by it's GID.
       * @param GID to which we want the work function to be applied.
       * @param Functor the work function.
       * the Functor has to be a class or a struct with operator()
       * implemented. 
       * \code
       *  struct assignstate : public unary_function<int, int> {
       *   ELEMENT_TYPE state;
       *  assignstate(ELEMENT_TYPE newstate){
       *   state = newstate;
       *  }
       *  int operator()(ELEMENT_TYPE& x) { 
       *   x = x + state; 
       *  }
       * };
       * \endcode
       **/

      template<class Functor> 
      int UpdateElement(GID _gid,Functor&);

      /**@brief Apply the specified work function Functor to an element
       * identified by it's GID.
       * @param GID to which we want the work function to be applied.
       * @param Functor the work function.
       * the Functor has to be a class or a struct with operator()
       * implemented. 
       * \code
       *  struct assignstate : public unary_function<int, int> {
       *   ELEMENT_TYPE state;
       *  assignstate(ELEMENT_TYPE newstate){
       *   state = newstate;
       *  }
       *  int operator()(ELEMENT_TYPE& x, T& _argument) { 
       *   x = x + state; 
       *  }
       * };
       * \endcode
       **/
      template<class Functor, class T> 
      int UpdateElement(GID _gid,Functor& ,T&);

      /*
	template<class Functor, class T1,class T2> 
	int UpdateElement(VID _gid, Functor& ,T1&,T2&);
	
	template<class Functor, class T1,class T2,class T3> 
	int UpdateElement(VID _gid,Functor&,T1&,T2&,T3&);
      */

    protected:
      template<class Functor> 
      void __UpdateElement(_StaplPair<GID,Functor> args);

      template<class Functor, class T> 
      void __UpdateElement2(_StaplTriple<GID,Functor,T> args);

      /*
	template<class Functor, class T1,class T2> 
	void __UpdateElement(_StaplQuad<GID,Functor,T1,T2> args);
	
	template<class Functor, class T1,class T2,class T3> 
	void __UpdateElement3(_StaplPenta<GID,Functor,T1,T2,T3> args);
      */
      
      //@}

    
      //=======================================
      //PContainerParts get
      //=======================================
      /**@name Parts Manipulation */
      //@{

    public:
      /**@brief Parts iterator pointing to the first part;
       **/
      parts_internal_iterator parts_begin() {
	return pcontainer_parts.begin();
      }

      /**@brief Parts iterator pointing to the last part;
       **/
      parts_internal_iterator parts_end()  {
	return pcontainer_parts.end();
      }
      
      /**@brief Parts const iterator pointing to the first part;
       **/
      const_parts_internal_iterator parts_begin() const{
	return pcontainer_parts.begin();
      }
      
      /**@brief Parts const iterator pointing to the last part;
       **/
      const_parts_internal_iterator parts_end() const {
	return pcontainer_parts.end();
      }

      size_t get_num_parts()
	{
	  return pcontainer_parts.size();
	}

    public:
      
      PARTID GetPartId(GID _gid) const {
	PARTID _id;
	if(IsLocal(_gid, _id)) return _id;
	return INVALID_PART;
      }

      /**@brief 
       *Get the number of parts.
       **/
      int GetPartsCount() const {
	return  pcontainer_parts.size();
      }

      const vector<pContainerPart_type*>& GetPContainer_Parts() const {
	return pcontainer_parts;
      }

      /**@brief 
       *Get the pointer to a part specified by its identifier
       *@param PARTID part identifier 
       *@return pContainerPart_type* pointer to the part
       **/
      const pContainerPart_type* GetPart(PARTID _id) const {
	return *(pcontainer_parts.begin()+_id);
      }

      pContainerPart_type* GetPart(PARTID _id) {
	return *(pcontainer_parts.begin()+_id);
      }


      /**@brief 
       *Add a part to the list of parts on the current thread.
       *@param PARTID part identifier 
       *@param pContainerPart_type the part to be added
       *@return pContainerPart_type* pointer to the part
       **/
      void SetPart(PARTID _id, pContainerPart_type& _other) {
	*(pcontainer_parts[_id]) = _other;
      }

      void SetPart(PARTID _id, pContainerPart_type* _other) {
	pcontainer_parts[_id] = _other;
      }

      //=======================================
      //PContainerParts bookkeeping 
      //=======================================

      /**@brief add a new pContainer part with no elements.
       * @return PARTID of the newly added part.
       **/
      int AddPart() {
	PARTID partid= part_id_counter++;
	pContainerPart_type* _ct = new pContainerPart_type();
	_ct->SetPartId(partid);
	pcontainer_parts.push_back(_ct);
	this->dist.IncreaseBoundaryInfo();
	return partid;
      }

    protected:

      /**@brief add the specified part to the vector of pContainer parts.
       * @param pContainerPart_type& reference to the part to be added.
       * @return PARTID of the newly added part.
       **/
      int AddPart(const pContainerPart_type& _bt) {

	PARTID partid= part_id_counter++;
	pContainerPart_type* _ct = new pContainerPart_type(_bt,partid);
	pcontainer_parts.push_back(_ct);
	this->dist.IncreaseBoundaryInfo();
	AddPart2PartIDMap(*_ct);
#ifdef COMPLETE_LOCATION_MAP
	AddPart2LocationMap(*_ct);
#endif

	return partid;
      }
    
    //don't need this for parray!! need to fix  -ann
      void AddPart2PartIDMap(const pContainerPart_type& _bt) {
	vector<GID> gidvec;
	_bt.GetPartGids(gidvec);
	PARTID _partid = _bt.GetPartId();
	for(vector<GID>::iterator it = gidvec.begin(); it!=gidvec.end(); ++it) {
	  dist.Add2PartIDMap(*it, _partid);
	}
      }

      void AddPart2LocationMap(const pContainerPart_type& _bt) {
	vector<GID> gidvec;
	_bt.GetPartGids(gidvec);
	PARTID _partid = _bt.GetPartId();
	Location location(myid,_partid);
	for(vector<GID>::iterator it = gidvec.begin(); it!=gidvec.end(); ++it) {
	  pair<GID,Location> tp(*it,location);
	  dist.Add2LocationMap(tp);
	}
      }
      //@}

     //=======================================
      //PContainer checks
      //=======================================
    public:


      /**@name Distribution related methods */
      //@{

      /**@brief Get distribution.
       *@return const reference to the distribution associated with the current 
       * pContainer.
       **/
      const Distribution_type& GetDistribution() const { return dist; }


      /**@brief check if the element associated with _gid is local
       * @param GID for which we want the check to be performed.
       * @return true if the element is local and false otherwise.
       **/
      virtual bool IsLocal(GID _gid) const {
	PARTID _id;
	if(IsLocal(_gid, _id)) return true;
	return false;
      }  


      /**@brief check if the element associated with _gid is local and if this 
       * is true initialize the PARTID with the part identifier wher the 
       * element is stored.
       * @param GID for which we want the check to be performed.
       * @return true if the element is local and false otherwise.
       **/
      virtual bool IsLocal(GID _gid, PARTID& _id) const {
	return dist.IsLocal(_gid,_id);
      }  


      /**@brief Find the location where the specified GID lives.
       * @param GID for which we want the check to be performed.
       * @return the location where the element lives. InvalidLocation() if 
       * the element is not found.
      **/
      virtual Location Lookup(GID _gid) { 
	Location loc = dist.Lookup(_gid);
	return loc;
	//if(loc.locpid() != INVALID_PID) return loc;
	//else return CompleteSearch(_gid);
      }


      virtual PID FindRemotePid(GID _gid) {
	return dist.Lookup(_gid).locpid();
      }

     /**@brief Called only for remote nodes after Lookup fails
	 search through each thread
	 cache locally after find, also add to location_map
	 return InvalidLocation() if not find     
      **/
      virtual Location CompleteSearch(GID _gid) {
	Location data_owner = FindDataOwner(_gid);
    
	if(data_owner.ValidLocation()) {

#ifdef STAPL_DEBUG
	  cout<<"Found by search! data_owner of gid "<<_gid <<
	    " is Pid "<<data_owner.locpid()<<" partid "<<data_owner.partid()<<endl;
#endif

#ifdef COMPLETE_LOCATION_MAP
	  pair<GID,Location> _x(_gid,data_owner);
	  dist.Add2LocationMap(_x); 
#endif
	  dist.Add2Cache(_gid,data_owner);
	  return data_owner;
      
	} else throw ElementNotFound(_gid);
      }

    protected:
      virtual Location FindDataOwner(GID _gid) { 
	int i;
	Location data_owner = _GidSearch(_gid);
	if(!data_owner.ValidLocation()) {
	  for(i=0; i<nprocs; i++) {
	    if(i!=myid) {
	      data_owner = sync_rmi(i, getHandle(),
				    &this_type::_GidSearch, _gid);
	      if(data_owner.ValidLocation()) break;
	    }
	  }
	}
	return data_owner;
      }

      virtual Location _GidSearch(GID _gid) {
	PARTID _partid;
	if(IsLocal(_gid,_partid) ) {
	  Location t(myid,_partid);
	  return t;
	}
	else {
	  return Location::InvalidLocation();
	}
      }
 
      //=======================================
      //Distribute and related methods
      //=======================================
    public:
      /**@brief Get Location of previous subpContainer for a subpContainer
       *@param partid PARTID of this subpContainer
       */
      inline const Location& GetPreviousPart(PARTID partid) {
        return this->dist.LookUpBdBeginInfo(partid);
      }

      /**@brief Get Location of next subpContainer for a subpContainer
       *@param partid PARTID of this subpContainer
       */
      inline const Location& GetNextPart(PARTID partid) {
        return this->dist.LookUpBdEndInfo(partid);
      }

      /**@brief Initiate the boundary information for the subpContainer
       *@param partid PARTID of this subpContainer
       *@param prev Location of previous subpContainer
       *@param next Location of next subpContainer
       */
      inline void InitBoundaryInfo(PARTID partid, Location prev, Location next) {
        this->dist.InitBoundaryInfo(partid, prev, next);
      }

    protected:
      /**@brief Build pcontainer parts before rmi transfer.
       * It should build a set of pContainerPart objects based on the distribution info. 
       **/
      virtual void BuildSubContainer( const vector<Element_Set_type>&, 
				      vector<pContainerPart_type>&)=0;
	//       {printf("\n This is the BasePContainer Build");}


      /**@brief  Send a pcontainer part to a processor using rmi 
       * The pContainer part is added into pcontainer_parts
       **/

      virtual int SetSubContainer(const vector<pContainerPart_type>  _ct)=0;
      //      	{
      //      	  printf("\n This is the BasePContainer Set Sub ");
      //      	  cout<<"not implemented yet"<<endl;
      //typename vector<pContainer_Part_type>::const_iterator it;
      /*	To be tested  
		for(it=_ct.begin(); it!=_ct.end(); ++it) {
		AddPart(*it);
		}
      **/
      //      	  return OK;   //to use sync_rmi
	  
      //      	}


      /**@brief Merges pContainer parts that can be merged into larger parts
        **/
      virtual void MergeSubContainer()=0 ;

    public:
      /**@brief  Distribute according to the specified distribution info.
       * It requires BuildSubContainer and MergeSubContainer
       * are implemented for each specific instance of pcontainer.
       *\b GLOBAL
       **/
      virtual void Distribute(const Distribution_Info_type& data_map) 
	{ 

	  typename Distribution_Info_type::const_iterator its;
	  //	  printf("\n Data map size %d",data_map.size());
	  for(its=data_map.begin(); its!=data_map.end(); ++its)  
	    {

	      //	      printf("\n New distribution item for thread %d",get_thread_id());
	      vector<pContainerPart_type> sub_data;
	      //	      printf("\n Before calling BUild sub on thread %d",get_thread_id() );
#ifdef _TIMER_EXPERIMENT
	      tr_build=start_timer();
#endif
	      BuildSubContainer(its->second, sub_data);
#ifdef _TIMER_EXPERIMENT
	      t_build+=stop_timer(tr_build);
#endif
	      //	      printf("\n After calling BUild Sub %d",get_thread_id());
	      //	      printf("\n %d PARTS HAS TO BE SHIPPED to %d from thread %d\n",sub_data.size(),its->first, get_thread_id());
#ifdef _TIMER_EXPERIMENT
	      tr_set=start_timer();
#endif
	      if (sub_data.size()!=0)
		if (its->first!=get_thread_id())
		  stapl::async_rmi(its->first,getHandle(), 
				   &this_type::SetSubContainer,sub_data); 
		else
		  SetSubContainer(sub_data);
#ifdef _TIMER_EXPERIMENT
	      t_set+=stop_timer(tr_set);
#endif
	      //	      printf("\n After calling SetSubContainer Sub %d",get_thread_id());
	    }
	  //	  printf("\n LOOP END\n");

	  rmi_fence(); 

	  //	  printf("\n After fence %d\n\n\n",get_thread_id());
	  //	  pDisplayPContainer();
#ifdef _TIMER_EXPERIMENT
	  tr_merge=start_timer();
#endif
	  MergeSubContainer();
#ifdef _TIMER_EXPERIMENT
	  t_merge+=stop_timer(tr_merge);
#endif
	} 

      //=======================================
      //DistributionInfo interface functions
      //=======================================
    protected:
      /*
       * Collect local distribution information, used in a complete collection
       */
      virtual void _GetDistributionInfo(vector<Element_Set_type>& elmtvec) {
	if(!elmtvec.empty()) elmtvec.clear();
	elmtvec.resize(pcontainer_parts.size());
	for(int i=0; i<pcontainer_parts.size(); ++i) {
	  Element_Set_type t = pcontainer_parts[i]->GetElementSet();
	  elmtvec[i]=t;
	}
      }
      //@}

    public:

      /**@name Input/Output methods */
      //@{
    public:
      /**@brief 
       * Display the data inside all parts.
       **/
      virtual void DisplayPContainer()  {

	const_parts_internal_iterator it;
	printf("pContainer in Thread: %d  with local size = %d  \n",get_thread_id(),local_size());
	printf("\n============PContainer Parts========================\n");
	for(it=pcontainer_parts.begin(); it!=pcontainer_parts.end(); ++it) {
	  (**it).DisplayPart();
	}
	printf("\n===========LOCATION MAP========================\n");
	//dist.DisplayElementLocationMap();
      }

		

      virtual void pDisplayPContainer() 
	{
	  stapl::rmi_fence();
	  for(int i=0; i<stapl::get_num_threads(); ++i)
	    {
	      if(i==stapl::get_thread_id())
		{
		  DisplayPContainer();
		}
	      stapl::rmi_fence();
	    }
	  stapl::rmi_fence();
	}




      //=======================================
      //bundary methods provided to pRange
      //=======================================
    public:

    //print method;
    void _DisplayElements(vector<GID> _indices){
      for(int i=0;i<_indices.size();i++){
	if(IsLocal(_indices[i]))
	  cout<<GetElement(_indices[i])<<" ";
      }
      cout.flush();
      if(myid+1 < nprocs)
	stapl::async_rmi(myid+1,
			 this->getHandle(),
			 &this_type::_DisplayElements,_indices); 
      
    }

    void DisplayElements_1(vector<GID>& _indices){
      if(myid == 0){
	cout<<endl;
      }
      cout.flush();
      stapl::rmi_fence();
      cout.flush();
      if(myid == 0){
	for(int i=0;i<_indices.size();i++){
	  if(IsLocal(_indices[i]))
	    cout<<GetElement(_indices[i])<<" ";
	}
	cout.flush();
	if(myid+1 < nprocs)
	  stapl::async_rmi(myid+1,
			 this->getHandle(),
			 &this_type::_DisplayElements,_indices); 
      }
      stapl::rmi_fence();
      if(myid == 0){
	cout<<endl;
      }
      cout.flush();
      cout.flush();
    }

    /**@brief 
     * Display the elements of the pContainer.
     **/
    void DisplayElements(){
      if(myid == 0){
	int sz = size();
	//cout<<"SIZE"<<sz<<endl;
	for(int i=0;i<sz;i++){
	  cout<<GetElement(i)<<" ";
	}
	//cout.flush();
      }
    }

    /**@brief 
     * Display a subset of the elements of the pContainer.
     * @param vector<GID> the set of GIDs to be displayed.
     **/
    void DisplayElements(vector<GID>& _indices){
      if(myid == 0){
	cout<<endl;
	for(int i=0;i<_indices.size();i++){
	  cout<<GetElement(_indices[i])<<" ";
	}
	cout.flush();
      }
    }
    //@}
    
    //RMI trace calls
    void _trace_rmi_synch(const int& d,trace x)
      {
	__tracer.trace_rmi_synch(d);
      }
    void _trace_rmi_synch(const int&d, no_trace x)
      {
	return;
      }
    void _trace_rmi_asynch(const int& d,trace x)
      {
	__tracer.trace_rmi_asynch(d);
      }
    void _trace_rmi_asynch(const int&d, no_trace x)
      {
	return;
      }
    void _trace_rmi_fence(trace x)
      {
	__tracer.trace_rmi_fence();
      }
    void _trace_rmi_fence( no_trace x)
      {
	return;
      }



    // PContainer trace operations


 void _construct_trace(size_t n, trace x)
 {
   __tracer.create(n);
 }
 void _construct_trace(size_t n, no_trace x)
 { return; }

 void _destroy_trace(size_t n, trace x)
 {
   __tracer.destroy(n);
 }
 void _destroy_trace(size_t n, no_trace x)
 {
   return;
 }

 void _access_trace(size_t n,trace x)
 {
   __tracer.access(n);
 }
 
 void _access_trace(size_t n,no_trace x)
 {
   return;
 }
 void _insert_trace(size_t n , trace x)
 { __tracer.insert(n);}
 void _insert_trace(size_t n, no_trace x)
 {return;}

 void _erase_trace(size_t n, trace x)
 {__tracer.erase(n);}

 void _erase_trace(size_t n, no_trace x)
 {return;}

 void iterator_plus_plus_trace()
 {
   __tracer.iterator_plus_plus(local_size());
 }
 void iterator_minus_minus_trace()
 {
   __tracer.iterator_minus_minus(local_size());
 }
 void iterator_plus_ptrdiff_trace()
 {
   __tracer.iterator_plus_ptrdiff(local_size());
 }
 void iterator_minus_ptrdiff_trace()
 {
   __tracer.iterator_minus_ptrdiff(local_size());
 }
void iterator_minus_this_trace()
 {
   __tracer.iterator_minus_this(local_size());
 }
 void iterator_plus_equal_trace()
 {
   __tracer.iterator_plus_equal(local_size());
 }
 void iterator_minus_equal_trace()
 {
   __tracer.iterator_minus_equal(local_size());
 }
 void iterator_dereference_trace()
 {
   __tracer.iterator_dereference(local_size());
 }
 void iterator_arrow_trace()
 {
   __tracer.iterator_arrow(local_size());
 }










 };//end class definition 
  //@}

/*
 * Auxiliary function for update element; It is invoked remotely when
 * the specified index is not local;
*/
template<class pContainerPart_type,class Distribution_type, class trace_class, typename Iterator_tag, typename element_tag >
template<class Functor> 
void
  BasePContainer<pContainerPart_type,Distribution_type,trace_class,Iterator_tag,element_tag>::
__UpdateElement(_StaplPair<GID,Functor> args){
  pContainerPart_type* _part;
  PARTID _partid;
  //if local
  if(this->IsLocal(args.first,_partid)){
    typename pContainerPart_type::iterator it;
    _part = this->GetPart(_partid);
    if(_part->ContainElement(args.first,&it)){
      args.second(*it);
     }
    else{
      //maybe exception
      cout<<"ERROR:: __UpdateElement: invalid vertex id"<<endl;
    }
  }
}


/**@brief 
 * Update Element method; It is used to apply a certain function f on an element of 
 * the pContainer; The element is specified by its gid; The functor f is expected to 
 * implement the operator() with argument (&data); 
 * See parray_test.cc for an example.
 **/
template<class pContainerPart_type,class Distribution_type, class trace_class, typename Iterator_tag, typename element_tag >
template<class Functor> 
int
BasePContainer<pContainerPart_type,Distribution_type,trace_class,Iterator_tag,element_tag>::
UpdateElement(GID _gid , Functor& f){
  
  typedef _StaplPair<GID,Functor> ARGS;

  pContainerPart_type* _part;
  PARTID _partid;
  //if local
  if(this->IsLocal(_gid,_partid)){
    typename pContainerPart_type::iterator it;
    _part = this->GetPart(_partid);
    if(_part->ContainElement(_gid,&it)){
      return f(*it);
    }
    else{
      //maybe exception
      cout<<"ERROR:: UpdateElement: invalid index"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_gid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_gid, f);
    stapl::async_rmi<this_type,void,ARGS>(_n.locpid(),
					  this->getHandle(),
					  (void (this_type::*)(ARGS))&this_type::__UpdateElement,
    	     args);
    return OK;
  } 
}

/*
 * Auxiliary function for update element; It is invoked remotely when
 * the specified index is not local;
*/
template<class pContainerPart_type,class Distribution_type,class trace_class, typename Iterator_tag, typename element_tag >
template<class Functor,class T> 
void

BasePContainer<pContainerPart_type,Distribution_type,trace_class, Iterator_tag,element_tag>::
__UpdateElement2(_StaplTriple<GID,Functor,T> args){
  pContainerPart_type* _part;
  PARTID _partid;
  //if local
  if(this->IsLocal(args.first,_partid)){
    typename pContainerPart_type::iterator it;
    _part = this->GetPart(_partid);
    if(_part->ContainElement(args.first,&it)){
      args.second(*it, args.third);
     }
    else{
      //maybe exception
      cout<<"ERROR:: __UpdateElement: invalid vertex id"<<endl;
    }
  }
}


/**@brief 
 * Update Element method; It is used to apply a certain function f on an element of 
 * the pContainer; The element is specified by its gid; The functor f is expected to 
 * implement the operator() with arguments (&data, _arg); 
 * See parray_test.cc for an example.
 **/
template<class pContainerPart_type,class Distribution_type,class trace_class, typename Iterator_tag, typename element_tag >
template<class Functor, class T> 
int
BasePContainer<pContainerPart_type,Distribution_type,trace_class, Iterator_tag,element_tag>::
UpdateElement(GID _gid,Functor& f,T& _arg){
  
  typedef _StaplTriple<GID,Functor,T> ARGS;

  pContainerPart_type* _part;
  PARTID _partid;
  //if local
  if(this->IsLocal(_gid,_partid)){
    typename pContainerPart_type::iterator it;
    _part = this->GetPart(_partid);
    if(_part->ContainElement(_gid,&it)){
      return f( *it ,  _arg);     
    }
    else{
      //maybe exception
      cout<<"ERROR:: UpdateElement: invalid index"<<endl;
      return ERROR;
    }
  }
  else{//here  we are if remote
    Location _n=this->Lookup(_gid);
    stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pGetVertexField");
    ARGS args(_gid, f ,_arg);
#ifndef __xlC__
    stapl::async_rmi(_n.locpid(),
		     this->getHandle(),
		     (void (this_type::*)(ARGS))&this_type::__UpdateElement2,
		     args);
#else
    stapl::async_rmi<this_type, void, ARGS>(_n.locpid(),
		     this->getHandle(),
		     (void (this_type::*)(ARGS))&this_type::__UpdateElement2,
		     args);
#endif
    return OK;
  } 
}

} //end namespace stapl

#endif
      /*


      typename pContainerPart_type::iterator  GetPartBegin(PID _pid, PARTID _id) {
	if(myid == _pid) 
	  {
	    return (*(pcontainer_parts.begin()+_id))->begin();
	  }
	else {
	  return sync_rmi(_pid,getHandle(),
			       &this_type::GetPartBegin,_pid,_id);
	}
      }

      typename pContainerPart_type::iterator  GetPartEnd(PID _pid, PARTID _id) {
	if(myid == _pid) 
	  {
	    return (*(pcontainer_parts.begin()+_id))->end();
	  }
	else {
	  return sync_rmi(_pid,getHandle(),
			       &this_type::GetPartEnd,_pid,_id);
	}
      }
      


      virtual void GetBoundaries(vector<pContainer_Linear_Bdry_type>& _bdry) const {
	if(!_bdry.empty()) _bdry.clear();
	_bdry.resize(pcontainer_parts.size());

	const_parts_internal_iterator it;
	int i=0;
	for(it=pcontainer_parts.begin(); it!=pcontainer_parts.end(); ++it,++i) {
	  _bdry[i] = pContainer_Linear_Bdry_type((**it).begin(), --((**it).end()));
	}
#if STAPL_DEBUG
	cout<<"===================================="<<endl;
	cout<<"Linear Boundaries from pContainer in Thread : "<<myid<<endl;
	cout<<"===================================="<<endl;
	vector<pContainer_Linear_Bdry_type>::iterator it1;
	for(it1=_bdry.begin(); it1!=_bdry.end(); ++it1) {
	  cout<<"Thread "<<myid<<":  Begin : "<<(it1->start())->GetGid()
	      <<"  End : "<<(it1->finish())->GetGid()<<endl;
	}	
	cout<<"===================================="<<endl;
#endif
      }
      **/
	

      //=======================================
      //Alin's methods from BasePContainer 
      //=======================================

      /*   void GetPartitionInfo(multimap<size_t,Element_Set_type> ){} */
 
      /**@brief 
       * Even Distribution function for the pContainers.
       * This is a general algorithm which works for all pcontainers.
       * It requires though that  BuildSubContainer' and MergeSubContainers'
       * are implemented for each specific instance of pcontainer.
       * This distribution tries to evenly distribute the pContainer. It calls 
       * a compute_even_distribution function which sets the __distribution data member and
       * then calls the  distribution (  const map<size_t,Element_Set_type>& data_map) with the previously
       * precomputed even distribution.
       * Once these methods are implemented, the distribute function should work 'out of the box'
       */
      /*   void Distribute() */
      /*     { */
      /*       ComputeDistribution(ComputeEvenSplitters());//set the __distribution to evenly divided arrangement */
      /*       Distribute(__distribution);// calls distribute with the previously computed even distribution. */
      /*     } */

      /*   void Distribute(const vector<size_t>& local_splitters) */
      /*     { */
      /*       vector<size_t> global_splitters(nprocs,0); */
      /*       stapl_assert(local_splitters.size()==nprocs," \n The number of distribution splitters is not equal to the number of processors"); **/
      /*       size_t partial_splitter=0; */
      /*       // Creating the global splitters. It is similar to partial sum. */
	/*       for(int i=0;i<nprocs;i++) */
	/* 	{ */
	/* 	  global_splitters[i] =  local_splitters[i] +  partial_splitter; */
	/* 	  partial_splitter  =  global_splitters[i]; */
	/* 	} */
	/*       // Creating the distribution map. */
	/*       ComputeDistribution(global_splitters); */
	/*       // Distribute according to the distribution map. */
      
	/*       Distribute(__distribution); */
      
	/*     } */

	/**@brief 
	 * This function computes the map<size_t,Element_Set_type> for the even distribution and sets
	 * the __distribution data member with the newly computed distribution.
	 * This is a generic method and it works on all pcontainers. It does make use of the 
	 * pure abstract method "local_size()" which is implemented in all pContainers.
	 */

	/*   void ComputeDistribution(const vector<size_t> & splitters); */


	/**@brief 
	 *\b LOCAL
	 * Computes the splitters of the current distribution.
	 * The splitters are the of the last element on each processor from the desired distribution.
	 * the vector of splitters is returned
	 */
	/*   vector<size_t>  ComputeEvenSplitters() */
	/*     { */
	/*       vector<size_t> splitters(nprocs,0); */
	/*       size_t global_size=size(); */
	/*       for(int i=0;i<nprocs;i++) */
	/* 	{ */
	/* 	  if (i!=0)  */
	/* 	    splitters[i]=splitters[i-1]+global_size/nprocs +(global_size%nprocs>i); */
	/* 	  else */
	/* 	    splitters[0]=global_size/this->nprocs +(global_size%nprocs>i); */
	/* 	}  */
	/*       return splitters; */
	/*     } */

