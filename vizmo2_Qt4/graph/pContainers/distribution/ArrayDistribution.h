#ifndef ARRAYDISTRIBUTION_H
#define ARRAYDISTRIBUTION_H

#include <algorithm>
#include <runtime.h>
#include "rmitools.h"

#include "DistributionDefines.h"
#include "BaseDistribution.h"

#include "Defines.h"

/**
 * @ingroup parray
 * @{
 **/
namespace stapl {


  /**
  * Class used to store information about pArray's parts.
  * For each part we store the index of the first element, how many elements,
  * and the \ref Location where the part lives.
  */
  class ArrayPartsDistributionInfo {
    GID gid;
    int size;
    Location loc;

  public:
    ///Default constructor
    ArrayPartsDistributionInfo(){}

    /**@brief 
     *Constructor
     *@param GID first index for the part.
     *@param int the number of elements in the part. 
     *@param Location the thread which will own the part.
     */
    ArrayPartsDistributionInfo(GID g, int sz, Location l){
      gid = g;
      size = sz;
      loc = l;
    }

    /**@brief Start index
     *@return GID the start index for the part
     */
    GID GetGID() const{
      return gid;
    }

    /**@brief Size
     *@return int the size of the part
     */
    int GetSize() const{
      return size;
    }

    /**@brief Part Location
     *@return Location the location of the part
     */
    Location GetLocation() const{
      return loc;
    }
    
    /**@brief Set the start index
     *@param GID start index.
     */
    void SetGID(GID g){
      gid = g;
    }

    /**@brief Set the part size
     *@param int the size of the part.
     */
    void SetSize(int s){
      size = s;
    }

    /**@brief Set the part location
     *@param Location the location of the part.
     */
    void SetLocation(Location l){
      loc = l;
    }   

    /**@brief 
     * defyne type for ARMI packing.
     */
    void define_type(stapl::typer &t)
    {
      t.local(gid);
      t.local(size);
      t.local(loc);
    }
  };
  //@}

  /**
  * Class used to store global information about pArray's all parts(local and remote)
  * For each part we store the index of the first element, how many elements,
  * the \ref Location where the part lives, and its before and next parts' Locations.
  * 
  */
  class GlobalArrayPartsDistributionInfo {
    int id; // the index of each subpContainer's record 
    GID gid;
    int size;
    Location loc;
    Location before;
    Location next;

  public:
    ///Default constructor
    GlobalArrayPartsDistributionInfo(){}

    /**@brief 
     *Constructor
     *@param GID first index for the part.
     *@param int the number of elements in the part. 
     *@param Location the location of this part.
     */
    GlobalArrayPartsDistributionInfo(GID g, int sz, Location l) {
      id = 0;
      gid = g;
      size = sz;
      loc = l;
    }


    /**@brief 
     *Constructor
     *@param GID g first index for the part.
     *@param int sz the number of elements in the part. 
     *@param Location l the location of this part.
     *@param Location b the location of previous part.
     *@param Location n the location of next part.
     */
    GlobalArrayPartsDistributionInfo(int i, GID g, int sz, Location l, Location b, Location n){
      id = i;
      gid = g;
      size = sz;
      loc = l;
      before = b;
      next = n;
    }

    int GetId() const{
      return id;
    }

    /**@brief Start index
     *@return GID the start index for the part
     */
    GID GetGID() const{
      return gid;
    }

    /**@brief Size
     *@return int the size of the part
     */
    int GetSize() const{
      return size;
    }

    /**@brief Part Location
     *@return Location the location of the part
     */
    Location GetLocation() const{
      return loc;
    }

    /**@brief Before Part Location
     *@return Location the location of the previous part 
     */
    Location GetBeforeLocation() const{
      return before;
    }
    
    /**@brief Next Part Location
     *@return Location the location of the next part 
     */
    Location GetNextLocation() const{
      return next;
    }
    void SetId(int i){
      id = i;
    }
    
    /**@brief Set the start index
     *@param GID start index.
     */
    void SetGID(GID g){
      gid = g;
    }

    /**@brief Set the part size
     *@param int the size of the part.
     */
    void SetSize(int s){
      size = s;
    }

    /**@brief Set the part location
     *@param Location the location of the part.
     */
    void SetLocation(Location l){
      loc = l;
    }   

    /**@brief Set the location of the previous part
     *@param Location the previous part's location
     */
    void SetBeforeLocation(Location l){
      before = l;
    }

    /**@brief Set the location of the next part
     *@param Location the next part's location
     */
    void SetNextLocation(Location l){
      next = l;
    }

    /**@brief 
     * defyne type for ARMI packing.
     */
    void define_type(stapl::typer &t)
    {
      t.local(gid);
      t.local(size);
      t.local(loc);
      t.local(before);
      t.local(next);
    }
  };

  struct CompGlobalArrayPartsDistributionInfo {
    bool operator()(const GlobalArrayPartsDistributionInfo& d1, const GlobalArrayPartsDistributionInfo& d2) const {
      return ( d1.GetGID() < d2.GetGID() );
    }
  } ;


  /**
   * @ingroup parray
   * @{
   */

  /**
   * Array Distribution. Specialized distribution that keeps the distribution 
   * information replicated on all the threads. Lookup operations are performed
   * faster all the information beeing available locally. The disadvantage 
   * for this distribution is the fact that when we redistribute the 
   * distribution information has to be updated on all the processors.
   **/
  class ArrayDistribution : public BaseDistribution<pair<int,GID> > {            
  public:
    
    //===========================
    //types
    //===========================

    ///Array Distribution type
    typedef ArrayDistribution Array_Distribution_type;

    ///Distribution info type used to store the replicated distribution.
    typedef vector<ArrayPartsDistributionInfo> DistributionInfo_type;

    typedef vector<ArrayPartsDistributionInfo>::iterator DistributionInfo_iterator;
    typedef valarray<Location> Array_Location_type;
    typedef pair<int, Array_Distribution_type> distribution_pair;
    //===========================
    //data
    //===========================
  protected:
    Array_Location_type    array_location;
    GID                    begin_index;
    int                    chunk_size;
  public:
    DistributionInfo_type  distribution_info;
    int total_size;
    
  public:
    //===========================
    //constructors & destructors
    //===========================
    
    /**@brief Default constructor
     */
    ArrayDistribution(){                                     
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Copy constructor
     */
    ArrayDistribution(const ArrayDistribution& w){
      total_size = w.total_size;
      begin_index = w.begin_index;
      chunk_size = w.chunk_size;

      array_location.resize(w.array_location.size());
      array_location = w.array_location;

      distribution_info = w.distribution_info;  
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**@brief Destructor
     */
    ~ArrayDistribution() {
      stapl::unregister_rmi_object( this->getHandle() );
    }
    
    //===========================
    //public methods
    //===========================
    
    /**@brief Initializing array distribution; make a copy of the distribution info 
     *and set the local information; start index chunk size;
     */
    void InitializeArrayDistribution(int _n, const DistributionInfo_type& _di){
      distribution_info = _di; 
      total_size=_n;
      chunk_size = _di[myid].GetSize();
      begin_index = _di[myid].GetGID();
      //cout<<"DISTRIBUTION:"<<stapl::get_thread_id()<<chunk_size<<":"<<begin_index<<endl;
    }

    void InitializeArrayDistribution() {
      total_size = 0;
      chunk_size = 0;
      begin_index = 0;
    }
    
    /**@brief 
     *Clear the distribution information.
     */
    void ClearDistribution(){
      distribution_info.clear();
      partbrdyinfo.clear();
    }

    /**@brief Method to add a part to the distribution vector.
     *Used by SplitPart method in the pArray.
     *@param GID the start index for the part.
     *@param int the size of the part
     *@param Location the location for the part.
     */
    void AddPartToDistributionVector(GID start, int size, Location loc){
      ArrayPartsDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
    }
    
    /**@brief Method to add a part to the distribution vector. All the information 
     *about the part is  encapsulated inside ArrayPartsDistributionInfo
     @param ArrayPartsDistributionInfo dstribution information about the part that is to be added.
     */
    void AddPartToDistributionVector(ArrayPartsDistributionInfo d){
      distribution_info.push_back(d);
    }

    /**@brief  Method that allows a new part to be added to the pContainer
     * Called from the pArray
     * Broadcasts the change to the rest of the processors
     * @param GID the start index for the part.
     * @param int the size of the part
     * @param Location the location for the part.
     */
    void AddPart(GID start, int size, Location loc){
      ArrayPartsDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
      for (int i=0; i<nprocs; i++){
	if (i != this->myid)
	  async_rmi(i, getHandle(), 
		    &ArrayDistribution::AddPartUpdate, d);
      }
      array_location.resize(array_location.size()+size);
    }

    /*Method to update the distribution vector
     *on the rest of the processors
     */
    void AddPartUpdate(ArrayPartsDistributionInfo d){
      distribution_info.push_back(d);
      return;
    }

    /**@brief Method to split the part into 2 
     *Called from the pContainer SplitPart;
     *@param int id1 part to be splitted
     *@param int id2 the newly created part
     *@param int size that is left to part1
     *Broadcasts the changes to the rest of the processors
    */
    void SplitPart(int id1, int id2, int size){
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == myid && it->GetLocation().partid() == id1){
	  ArrayPartsDistributionInfo d;
	  //info for the new created part
	  d.SetSize(it->GetSize()-size);
	  d.SetGID(it->GetGID() + size);
	  Location loc(myid, id2);
	  d.SetLocation(loc);

	  //update info for the old part; The distribution_info is kept sorted
	  it->SetSize(size);
	  distribution_info.insert(it+1,d); 

	  //since the distribution is mirrored on all procs; 
	  //perform a broadcast here;
	  _StaplQuad<int,int,int,int> q(myid, id1, id2, size);
	  for (int i=0; i<nprocs; i++){
	    if (i != myid)
	      async_rmi(i, getHandle(), 
			&ArrayDistribution::SplitPartUpdate, q);
	  }
	  return;
	}
      }
    }
  
    /*Method to update the distribution vector
       on the rest of the processors
    */
    void SplitPartUpdate(_StaplQuad<int,int,int,int> q){
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == q.first && 
	    it->GetLocation().partid() == q.second){
	  ArrayPartsDistributionInfo d;
	  d.SetSize(it->GetSize()-q.fourth);
	  d.SetGID(it->GetGID()+q.fourth);
	  Location loc(q.first, q.third);
	  d.SetLocation(loc);

	  //update info for the old part; The distribution_info is kept sorted
	  it->SetSize(q.fourth);
	  distribution_info.insert(it+1,d); 
	  return;
	}
      }
    }

    /**@brief Method to find contiguous parts on a specific thread.
     * Returns the parts information.
     *@param int thread id
     *@return pair<int,int> containing two parts that can be merged;
     *if there are not two parts to be merged the pair will contain <0,0>
    */
    pair<int,int> FindContinuousParts(int myid){
      pair<int,int> mypair(0,0);
      DistributionInfo_iterator it = distribution_info.begin();
      while(it<distribution_info.end()-1){
	if (it->GetLocation().locpid()==myid &&
	    (it+1)->GetLocation().locpid()==myid){
	  mypair.first = it->GetLocation().partid();
	  mypair.second = (it+1)->GetLocation().partid();   
	  return mypair;
	}
	it++;
      }
      return mypair;
    }

    /* Method updates the distribution vector after the merge
     * Broadcasts the changes to the rest of the processors
    */
    void MergeUpdate1(int gid, int myid){
      DistributionInfo_iterator it = distribution_info.begin();
      while(it<distribution_info.end()-1){
	if (it->GetGID()==gid){
	  it->SetSize(it->GetSize()+(it+1)->GetSize());
	  distribution_info.erase(it+1);
	  break;
	}
	it++;
      }
      for (int i=0; i<nprocs; i++){
	if (i != myid){
	  async_rmi(i, getHandle(), 
		    &ArrayDistribution::MergeUpdate2,gid);
	}
      }
    }

    /*Method to update the distribution vector
       on the rest of the processors
    */
    void MergeUpdate2(int gid){
      DistributionInfo_iterator it = distribution_info.begin();
      while(it<distribution_info.end()-1){
	if (it->GetGID()==gid){
	  it->SetSize(it->GetSize()+(it+1)->GetSize());
	  distribution_info.erase(it+1);
	  return;
	}
	it++;
      }
    }

    /**@brief Equal operator.
     */
    Array_Distribution_type& operator= (const Array_Distribution_type& w) {
      if(array_location.size() == 0) array_location.resize(w.array_location.size());  //always check for valarray size before assignment -ann
      array_location = w.array_location;
      distribution_info = w.distribution_info;  
      total_size = w.total_size;
      begin_index = w.begin_index;
      chunk_size = w.chunk_size;
      return *this;
    }
    
    //====================================
    //Methods for element location
    //====================================

  protected:
    /*
     * Finding the part to which a certain gid/index belongs; 
     * The parts are stored in a stored order so we can perform a binary search to find 
     * the part that contains the requested gid/index
     */
    PARTID SearchPart(GID _gid) const {
      int __len = distribution_info.size();
      int __half,__middle,__first=0;
      
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < distribution_info[__middle].GetGID())
	  //search inside the left half
	  __len = __half;
	else if(_gid >= distribution_info[__middle].GetGID() && 
	     _gid < distribution_info[__middle].GetGID() + distribution_info[__middle].GetSize())
	  //found
	    return __middle;
	else {
	  //search to the right
	  __first = __middle;
	  ++__first;
	  __len = __len - __half - 1;
	}
      }
      return -1;
    }

  public:
    /**@brief The start index of teh first part on this thread.
     *@return first index sorted on this thread.
     */
    GID get_start_index() { return begin_index; }
    void set_start_index(GID g) { begin_index = g; }


    /**@brief Test if an element specified by its GID is local or not.
     *@param GID of the element
     *@param PARTID& this is an output argument. If the GID it will be initialized with the 
     *the PARTID where the element lives. 
     *@return true if the GID is local and false otherwise.
     */
    bool IsLocal(GID _gid, PARTID& partid) const{
      //binary search SearchPart()
      int temp = SearchPart(_gid);
      if(temp == -1) {
	cout<<"#"<<get_thread_id()<<" Cannot find element in Location Map with given gid "<<_gid<<endl;
	return false;
      }

      if(distribution_info[temp].GetLocation().locpid() == this->myid){
	partid = distribution_info[temp].GetLocation().partid();
	return true;
      }
      else 
	return false;


      /*
	//linear search
      for(int i = 0;i<distribution_info.size();i++){
	if (_gid >= distribution_info[i].GetGID() && 
	    _gid < distribution_info[i].GetGID()+distribution_info[i].GetSize()) {
	  partid = distribution_info[i].GetLocation().partid();
	  if(distribution_info[i].GetLocation().locpid() != this->myid)
	    return false;
	  else 
	    return true;
	}
      }
      cout<<"ERROR While looking for part that contains element"<<_gid<<endl;
      return false;
      */
    }
    

    /**@brief Find where an element specified by its GID lives.
     *@param GID of the element
     *@return Location that owns the GID
     */
    virtual Location Lookup(GID _gid) const {
      //to be replaced by binary search SearchPart()
      for(int i = 0;i<distribution_info.size();i++){
	if (_gid >= distribution_info[i].GetGID() && 
	    _gid < distribution_info[i].GetGID()+distribution_info[i].GetSize()) {
	  return Location(distribution_info[i].GetLocation().locpid(), distribution_info[i].GetLocation().partid());
	}
      }
      cout<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
      return Location::InvalidLocation();
    }
    
  protected:
    /*local lookup - in the current part
     */
    virtual Location _Lookup(GID _gid) const {     
      return array_location[(_gid - begin_index)];
    }
    
    //===========================
    //bookkeeping for element_location_map                  
    //could be called both from local and from remote
    //===========================
    
  public:
    /*UpdateLocationMap
     */
    virtual void UpdateLocationMap(GID _gid, Location _location) {   
      PID map_owner=FindMapOwner(_gid);
      cout<<"UpdateLocationMap: map_owner="<<map_owner<<" myid="<<myid<<endl;
      if(map_owner != myid) {
        stapl::async_rmi(map_owner,
                         getHandle(),
                         &Array_Distribution_type::_UpdateLocationMap,
                         _gid, 
                         _location);
      } else 
      _UpdateLocationMap(_gid, _location);
    }
    
  protected:
    /* local UpdateLocationMap
     */
    virtual void _UpdateLocationMap(GID _gid, Location _location){    
      cout<<"array_location.size()="<<array_location.size()<<" set array_location["<<_gid<<"-"<<begin_index<<"] = "<<_location<<endl;
      array_location[_gid - begin_index] = _location;
    }
    
  public:
    /**@brief 
     *Display the distribution vector
     */
    virtual void DisplayDistributionInfo() const {       
      cout<<"Array Distribution Vector"<<endl;
      for(int i=0; i< distribution_info.size(); i++) {
	cout<<"Processor: "<<distribution_info[i].GetLocation().locpid()<<endl;
	cout<<"Part: "<<distribution_info[i].GetLocation().partid()<<endl;
	cout<<"Size: "<<distribution_info[i].GetSize()<<endl;
	cout<<"Starting at GID: "<<distribution_info[i].GetGID()<<endl;
      } 
    }

    /*Display Element Location Map
     */
    virtual void DisplayElementLocationMap() const {       
      cout<<"Array Location Map on Proc "<<myid<<": \n";
      for(GID it = 0; it < chunk_size; ++it) {
        cout<<"GID : "<<it + begin_index<<" on Proc "<<array_location[it].locpid()<<" with part_id "<<
        array_location[it].partid()<<endl; 
      } 
    }
  
    //====================================
    //Find where the information about the 
    //location of the vertex is stored
    //====================================
  protected:                  
    /*FindMapOwner returns the processor responsible for
       knowing where the specified by GID element is 
    */
    virtual PID FindMapOwner(GID _gid) const {     
      for(int i=0;i<distribution_info.size();i++) {
        if(_gid >= distribution_info[i].GetGID() && _gid < distribution_info[i].GetGID()+distribution_info[i].GetSize()) {
          return i;
        }
      }
#ifdef STAPL_DEBUG
      cout<<"ERROR in find owner"<<endl; 
#endif
      return -1;
    }

    //========================================
    //Synchronize the global distribution info
    //and tell every thread
    //========================================
  public:
    typedef vector<GlobalArrayPartsDistributionInfo> GlobalDistributionInfo_type;
    typedef GlobalDistributionInfo_type::iterator GlobalDistributionInfo_iterator;

  protected:
    GlobalDistributionInfo_type syncdists;

  private:

    // each thread (except #0) will revoke this method on  #0 with its own
    // local information (previous, next, size for each local subpContainer)
    inline void _collect_partsinfo(GlobalDistributionInfo_type _indists) {
     for(GlobalDistributionInfo_iterator it=_indists.begin(); it!=_indists.end(); ++it) {
      int id = it->GetId();
      GID start = it->GetGID();
      int size = it->GetSize();
      Location l = it->GetLocation();
      Location b = it->GetBeforeLocation();
      Location n = it->GetNextLocation();
     }
      syncdists.insert(syncdists.end(), _indists.begin(), _indists.end() );
    }

    inline void _order_partsinfo_bystartindex() {
      CompGlobalArrayPartsDistributionInfo comp;
      std::sort(syncdists.begin(), syncdists.end(), comp);
    }

  
    // this is called on thread#0 only, which owns all the (previous, next,
    // start_index, size) information for each (local or remote) subpContainer,
    // such that in this method, it can order all these subpContainer (order is
    // represented by Id.
    // this method saves both the start_index (GID) and the order (Id) of 
    // each subpContainer in syncdists.
    // 
    inline void _accumulate_startindices() {
      Location current;
      int counter = 0 ;
      int id = 0; 
      for (int i=0; i<syncdists.size(); ++i) {
        if (syncdists[i].GetBeforeLocation().locpid() == INVALID_PID || syncdists[i].GetBeforeLocation().partid() == INVALID_PART ) {
	  counter += syncdists[i].GetSize();
	  current = syncdists[i].GetLocation();
	  syncdists[i].SetId(id);
	  syncdists[i].SetGID(0);
	  id++;
	  break;
	}
      }
      while (id < syncdists.size() ) {
        for (int i=0; i<syncdists.size(); ++i) {
    	  if (syncdists[i].GetBeforeLocation() == current) {
	    syncdists[i].SetGID(counter);
	    counter += syncdists[i].GetSize();
	    current = syncdists[i].GetLocation();
	    syncdists[i].SetId(id);
	    id++;
	    break;
	  }
	}
      }
    }

    // thread#0 invokes this method on each other thread, 
    // such that every other threads gets to know the global information.
    inline void _get_globalinfo(GlobalDistributionInfo_type _globaldists) {
      syncdists.clear();
      for (GlobalDistributionInfo_iterator it=_globaldists.begin(); it!=_globaldists.end(); ++it) 
        syncdists.push_back(*it);
    }

    // Synchronize the global distribution information
    // such that each thread knows about every subpContainer's previous, next, start_index and size
    inline void _sync_distributioninfo(){
      for (int i=0; i<syncdists.size(); i++) {
        Location loci = syncdists[i].GetLocation();
	for (int j=0; j<distribution_info.size(); j++) {
          Location locj = syncdists[j].GetLocation();
	  if (loci == locj) {
	    distribution_info[j].SetGID(syncdists[i].GetGID() );
	    distribution_info[j].SetSize(syncdists[i].GetSize() );
	    break;
	  }
	}
      }
    }

  public:

    const GlobalDistributionInfo_type& GetSyncDistribution() {
      return syncdists;	
    }

    // Method to Synchronize the global distribution information for all
    // (local or remote) subpContainers of pArray, such that each thread
    // knows about the (prev, next, start_index, size) of all of them.
    void SynchronizeDistribution() {
      for (int i=0; i<distribution_info.size(); ++i) {
        if (distribution_info[i].GetLocation().locpid() != myid ) continue;
        int id = 0;
	GID start = distribution_info[i].GetGID();
	int size = distribution_info[i].GetSize();
	Location l = distribution_info[i].GetLocation();
	Location b = GetBdBeginInfo(l.partid());
	Location n = GetBdEndInfo(l.partid());
	GlobalArrayPartsDistributionInfo d(id, start, size, l, b, n);
	syncdists.push_back(d);  // my local info
      }
      stapl::rmi_fence();
      if (myid != 0) { //every thread sends its parts' info. to thread#0
        stapl::async_rmi(0, this->getHandle(), &ArrayDistribution::_collect_partsinfo, syncdists);
      }
      stapl::rmi_fence();

      //thread#0 accumulates the start_indices of all parts 
      if (myid == 0) {
        _accumulate_startindices();
	for (PID dest=1; dest<nprocs; dest++) {
	  stapl::async_rmi(dest, this->getHandle(), &ArrayDistribution::_get_globalinfo, syncdists);
	}
      } 
      stapl::rmi_fence();
      _sync_distributioninfo();
      stapl::rmi_fence();
    }

  };  //end of ArrayDistribution
  
} //end namespace stapl

//@}

#endif
