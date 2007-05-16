#ifndef BASEDISTRIBUTIONRANGES_H
#define BASEDISTRIBUTIONRANGES_H

#include <runtime.h>
#include "rmitools.h"
#include "DistributionDefines.h"
#include <vector>
#include "common/Defines.h"
#include <assert.h>

namespace stapl {
  
  class RangeInfo {
    GID          startGid;
    unsigned int size;
    Location     loc;

  public:
    RangeInfo(){
      size = 0;
      loc = Location::InvalidLocation();
    }

    RangeInfo(GID _g, int _s, Location _l){
      startGid  = _g;
      size      = _s;
      loc       = _l;
    }
    GID GetStartGID() const {
      return startGid;
    }
    int GetSize() const {
      return size;
    }
    Location GetLocation() const{
      return loc;
    }
    void SetStartGID(GID _g){
      startGid = _g;
    }
    void SetSize(int _s){
      size = _s;
    }
    void SetLocation(Location _l){
      loc = _l;
    }   
    void define_type(stapl::typer &t)
    {
      t.local(startGid);
      t.local(size);
      t.local(loc);
    }
  };

  template <class T1>
    class BaseDistributionRanges : public BasePObject {
  public:
    
    //===========================
    //types
    //===========================
    
    typedef BaseDistributionRanges      Distribution_type;
    typedef vector<RangeInfo>           DistributionInfo_type;
    typedef vector<RangeInfo>::iterator DistributionInfo_iterator;
    typedef vector<_StaplTriple<GID,unsigned int,PARTID> >  PartIDMap_type;

    //===========================
    //data
    //===========================
  protected:

    DistributionInfo_type element_location_map;
    DistributionInfo_type element_location_cache;
    PartIDMap_type        element_partid_map;
    unsigned int          chunk;
    GID                   local_max_gid;
    unsigned int          local_max_chunk;
    /*
      Flag that indicates wether the counting shoudl precede or not.
    */
    bool flg_cnt;
    /*
      counter that can be used for different measurements
    */
    int counter;

  /**@brief  
      Thread id
   */
  PID myid;

  /**@brief  
      Number of processors 
  */
  int nprocs;

  public:
    int total_size;
    
  public:
    //===========================
    //constructors & destructors
    //===========================
    
    /**default constructor
     */
    BaseDistributionRanges(unsigned int _chunk = 10000){
      counter = 0;
      chunk = _chunk;
      nprocs = get_num_threads();
      myid = get_thread_id();      
      local_max_gid = myid * chunk;
      local_max_chunk = myid * chunk;
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**copy constructor
     */
    BaseDistributionRanges(const BaseDistributionRanges& w){
      total_size = w.total_size;
      nprocs = w.nprocs;
      myid = w.myid;//????
      chunk = w.chunk;
      element_location_map   = w.element_location_map;
      element_location_cache = w.element_location_cache;
      element_partid_map     = w.element_partid_map;
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**destructor
     */
    ~BaseDistributionRanges() {
      stapl::unregister_rmi_object( this->getHandle() );
    }
    
    //===========================
    //public methods
    //===========================

    void StartCounting(){
      flg_cnt = true;
    }
    /**@brief  Set the counting flag to false. Counting stops now */
    void StopCounting(){
      flg_cnt = false;
    }
    /**@brief  Print counter */
    void PrintCounter(){
      cout<<stapl::get_thread_id()<<"-Distribution destroyed: remote calls "<<counter<<endl;
    }

    virtual void FlushCache() {
      element_location_cache.clear();
    }

 /**@brief 
   * Clears the part order. Called after a distribute method is invoked (
   * the distribution mehtod might change the part order and ids ) making the part order obsolete.
   */
  virtual void FlushPartsMap() {
    element_partid_map.clear();
  }

  /**@brief 
   * Clears the element location map. Called after a distribute method is invoked (
   * the distribution mehtod might change the location of the
   * pcontainer elements) making the element location map obsolete.
   */
  virtual void FlushLocationMap() {
    element_location_map.clear();
  }
    
    /**Initializing array distribution; make a copy of the distribution info 
     *and set the local information; start index chunk size;
     */
    void InitializeBaseDistributionRanges(int _n, const DistributionInfo_type& _di){

      cout<<myid<<":"<<"BDR:: check this method"<<endl;

      //distribution_info = _di; 
      total_size = _n;
      chunk = _chunk;
    }
    
    /**Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(GID start, int size, Location loc){
      RangeInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      element_location_map.push_back(d);
    }
    
    /**Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(RangeInfo d){
      //distribution_info.push_back(d);
    }

    /** Method that allows a new part to be added to the pContainer
     * Called from the pGraphFast
     * Broadcasts the change to the rest of the processors
     */
    void AddPart(GID start, int size, Location loc){
      cout<<myid<<":"<<"BDR:: method not reimplemented"<<endl;
      /*
      RangeInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
      for (int i=0; i<nprocs; i++){
	if (i != this->myid)
	  async_rmi(i, getHandle(), 
		    &BaseDistributionRanges::AddPartUpdate, d);
      }
      */
    }

    /**Method to update the distribution vector
     *on the rest of the processors
    */
    void AddPartUpdate(RangeInfo& d){
      cout<<myid<<":"<<"BDR:: method not reimplemented"<<endl;
      //distribution_info.push_back(d);
      //return;
    }

    /**Method to split the part into 2 
     *Called from the pContainer SplitPart; The arguments are :
     *id1 part to be splitted
     *id2 the newly created part
     *size that is left to part1
     *Broadcasts the changes to the rest of the processors
    */
    void SplitPart(int id1, int id2, int size){
      /*
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == myid && it->GetLocation().partid() == id1){
	  BaseDistributionRangesInfo d;
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
			&BaseDistributionRanges::SplitPartUpdate, q);
	  }
	  return;
	}
      }
      */
    }
  
    /**Method to update the distribution vector
       on the rest of the processors
    */
    void SplitPartUpdate(_StaplQuad<int,int,int,int> q){
      /*
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == q.first && 
	    it->GetLocation().partid() == q.second){
	  BaseDistributionRangesInfo d;
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
      */
    }

    /**Method finds continuous parts on a processor
     * Returns the parts information to the pContainer
    */
    pair<int,int> FindContinuousParts(int myid){
      pair<int,int> mypair(0,0);
      /*
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
      */
      return mypair;
    }

    /** Method updates the distribution vector after the merge
     * Broadcasts the changes to the rest of the processors
    */
    void MergeUpdate1(int gid, int myid){
      /*
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
		    &BaseDistributionRanges::MergeUpdate2,gid);
	}
      }
      */
    }

    /**Method to update the distribution vector
       on the rest of the processors
    */
    void MergeUpdate2(int gid){
      /*
      DistributionInfo_iterator it = distribution_info.begin();
      while(it<distribution_info.end()-1){
	if (it->GetGID()==gid){
	  it->SetSize(it->GetSize()+(it+1)->GetSize());
	  distribution_info.erase(it+1);
	  return;
	}
	it++;
      }
      */
    }

    /**
     *Method to display the distribution vector
     */
    virtual void DisplayDistributionInfo() const {       
      cout<<myid<<":"<<"GraphFast Distribution Vector"<<endl;
      //for(int i=0; i< distribution_info.size(); i++) {
      //cout<<myid<<":"<<"Processor: "<<distribution_info[i].GetLocation().locpid()<<endl;
      //cout<<myid<<":"<<"Part: "<<distribution_info[i].GetLocation().partid()<<endl;
      //cout<<myid<<":"<<"Size: "<<distribution_info[i].GetSize()<<endl;
      //cout<<myid<<":"<<"Starting at GID: "<<distribution_info[i].GetGID()<<endl;
      //} 
    }
    
    /**copy constructor
     */
    Distribution_type& operator= (const Distribution_type& w) {
      total_size = w.total_size;
      chunk = w.chunk;
      element_location_map   = w.element_location_map;
      element_location_cache = w.element_location_cache;
      element_partid_map     = w.element_partid_map;     
      return *this;
    }
    
    //====================================
    //Methods for element location
    //====================================

  protected:
    /**
     * Finding the part to which a certain gid/index belongs; 
     * The parts are stored in a stored order so we can perform a binary search to find 
     * the part that contains the requested gid/index
     */
    PARTID SearchPart(GID _gid) const {
      //the search is done inside PartIDMap
      int __len = element_partid_map.size();
      int __half,__middle,__first=0;
      
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < element_partid_map[__middle].first)
	  //search inside the left half
	  __len = __half;
	else if(_gid >= element_partid_map[__middle].first && 
	     _gid < element_partid_map[__middle].first + element_partid_map[__middle].second)
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

    PARTID SearchWhereToInsertPart(GID _gid) const {
      //the search is done inside PartIDMap
      int __len = element_partid_map.size();
      int __half,__middle,__first=0;
      
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < element_partid_map[__middle].first)
	  //search inside the left half
	  __len = __half;
	else {
	  //search to the right
	  __first = __middle;
	  ++__first;
	  __len = __len - __half - 1;
	}
      }
      return __first;
    }
  
    inline Location LocationMapLookup(GID _gid) const {
      int temp = SearchTable(element_location_map,_gid);
      if(temp == -1) return Location::InvalidLocation();
      else return element_location_map[temp].GetLocation();
    }

    inline Location CacheLookup(GID _gid) const {
      int temp = SearchTable(element_location_cache,_gid);
      if(temp == -1) return Location::InvalidLocation();
      else return element_location_cache[temp].GetLocation();
    }

    inline int SearchTable(const DistributionInfo_type& table, GID _gid) const {
      int __len = table.size();
      int __half,__middle,__first=0;
      
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < table[__middle].GetStartGID())
	  //search inside the left half
	  __len = __half;
	else if(_gid >= table[__middle].GetStartGID() && 
	     _gid < table[__middle].GetStartGID() + table[__middle].GetSize())
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

    inline int SearchWhereToInsert(const DistributionInfo_type& table, GID _gid) const {
      int __len = table.size();
      int __half,__middle,__first=0;
      
      while (__len > 0) {
	__half = __len >> 1;
	__middle = __first;
	__middle += __half;
	if (_gid < table[__middle].GetStartGID())
	  //search inside the left half
	  __len = __half;
	else {
	  //search to the right
	  __first = __middle;
	  ++__first;
	  __len = __len - __half - 1;
	}
      }
      return __first;
    }



  public:

    virtual void Add2Cache(RangeInfo& rinfo) {
      if(element_location_cache.size() == 0){
	element_location_cache.push_back(rinfo);
	return;
      }

      int temp = SearchWhereToInsert(element_location_cache,rinfo.GetStartGID());
      //if(temp == element_location_cache.size()) --temp;
      if(temp < element_location_cache.size()){
	RangeInfo& current = element_location_cache[temp];
	//append at the begining
	if((current.GetStartGID() == rinfo.GetStartGID() + rinfo.GetSize()) &&
	   ( current.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  current.SetStartGID(rinfo.GetStartGID());
	  current.SetSize(current.GetSize() + rinfo.GetSize());
	  return;
	}
	//append at the end;
	if((current.GetStartGID() + current.GetSize() == rinfo.GetStartGID()) &&
	   ( current.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  current.SetSize(current.GetSize() + rinfo.GetSize());
	  return;
	}
      }
      if(temp > 0){
	RangeInfo& prev = element_location_cache[temp - 1];
	//append at the begining
	if((prev.GetStartGID() == rinfo.GetStartGID() + rinfo.GetSize()) &&
	   ( prev.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  prev.SetStartGID(rinfo.GetStartGID());
	  prev.SetSize(prev.GetSize() + rinfo.GetSize());
	  return;
	}
	//append at the end;
	if((prev.GetStartGID() + prev.GetSize() == rinfo.GetStartGID()) &&
	   ( prev.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  prev.SetSize(prev.GetSize() + rinfo.GetSize());
	  return;
	}
      }
      //eventually for the next
      element_location_cache.insert(element_location_cache.begin()+(temp),rinfo);
      //if merge not possible insert      
    }

    virtual void Add2Cache(GID const _gid , Location const _pid) {
      RangeInfo r(_gid,1,_pid);
      Add2Cache(r);
    }


    virtual void _Add2LocationMap(RangeInfo& rinfo) {
      if(element_location_map.size() == 0){
	element_location_map.push_back(rinfo);
	return;
      }

      int temp = SearchWhereToInsert(element_location_map,rinfo.GetStartGID());
      //if(temp == element_location_map.size()) --temp;
      if(temp < element_location_map.size()){
	RangeInfo& current = element_location_map[temp];

	//append at the begining
	if((current.GetStartGID() == rinfo.GetStartGID() + rinfo.GetSize()) &&
	   ( current.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  current.SetStartGID(rinfo.GetStartGID());
	  current.SetSize(current.GetSize() + rinfo.GetSize());
	  return;
	}
	//append at the end;
	if((current.GetStartGID() + current.GetSize() == rinfo.GetStartGID()) &&
	   ( current.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  current.SetSize(current.GetSize() + rinfo.GetSize());
	  return;
	}
      }
      if(temp > 0){
	RangeInfo& prev = element_location_map[temp - 1];
	//append at the begining
	if((prev.GetStartGID() == rinfo.GetStartGID() + rinfo.GetSize()) &&
	   ( prev.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  prev.SetStartGID(rinfo.GetStartGID());
	  prev.SetSize(prev.GetSize() + rinfo.GetSize());
	  return;
	}
	//append at the end;
	if((prev.GetStartGID() + prev.GetSize() == rinfo.GetStartGID()) &&
	   ( prev.GetLocation().locpid() == rinfo.GetLocation().locpid()) ){
	  prev.SetSize(prev.GetSize() + rinfo.GetSize());
	  return;
	}
      }
      //eventually for the next
      
      //if merge not possible insert
      element_location_map.insert(element_location_map.begin()+(temp),rinfo);
    }

    void Add2LocationMap(_StaplPair<GID,Location>& _x) {
      PID map_owner=FindMapOwner(_x.first);
      RangeInfo r(_x.first,1,_x.second);
      if(map_owner != myid) {
	stapl::async_rmi(map_owner,
			 getHandle(),
			 &Distribution_type::_Add2LocationMap,r);
      } else 
	_Add2LocationMap(r);
    }

    virtual void Add2PartIDMap(_StaplTriple<GID,unsigned int,PARTID>& rinfo) {
      if(element_partid_map.size() == 0){
	element_partid_map.push_back(rinfo);
	return;
      }

      int temp = SearchWhereToInsertPart(rinfo.first);
      //if(temp == element_partid_map.size()) --temp;
      if(temp < element_partid_map.size()){
	_StaplTriple<GID,unsigned int,PARTID>& current = element_partid_map[temp];
	//append at the begining
	if((current.first == rinfo.first + rinfo.second) &&
	   ( current.third == rinfo.third) ){
	  current.first  = rinfo.first;
	  current.second += rinfo.second;
	  return;
	}
	//append at the end;
	if((current.first + current.second == rinfo.first) &&
	   ( current.third == rinfo.third) ){
	  current.second += rinfo.second;
	  return;
	}
      }
      if(temp > 0){
	_StaplTriple<GID,unsigned int,PARTID>& prev = element_partid_map[temp - 1];
	//append at the begining
	if((prev.first == rinfo.first + rinfo.second) &&
	   ( prev.third == rinfo.third) ){
	  prev.first = rinfo.first;
	  prev.second += rinfo.second;
	  return;
	}
	//append at the end;
	if((prev.first + prev.second == rinfo.first) &&
	   ( prev.third == rinfo.third) ){
	  prev.second += rinfo.second;
	  return;
	}
      }
      //eventually for the next
      
      //if merge not possible insert
      element_partid_map.insert(element_partid_map.begin()+(temp),rinfo);
    }
    virtual void Add2PartIDMap(GID _gid,PARTID _pid) {
      _StaplTriple<GID,unsigned int,PARTID> temp(_gid,1,_pid);
      Add2PartIDMap(temp);
    }


    /**an element is local if it is in the specified part
     */
    bool IsLocal(GID _gid, PARTID& partid) const{
      //binary search SearchPart()
      int temp = SearchPart(_gid);
      if(temp == -1) {
	//cout<<myid<<":"<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
	return false;
      }
      partid = element_partid_map[temp].third;
      return true;
    }
    
    /**finding an element by the GID: first look in local cache,
       else find the map owner, add to the local cache 
    */
    virtual Location Lookup(GID _gid) const {
      RangeInfo rinfo;
      Location  data_owner;
      PID       map_owner;
      //check in local cache 
#ifdef _TIMER_EXPERIMENT
      tr_BD_cache=start_timer();
#endif
      
      data_owner = CacheLookup(_gid);
      
#ifdef _TIMER_EXPERIMENT
      t_BD_cache+=stop_timer(tr_BD_cache);
#endif
      
      if(data_owner.ValidLocation()) { 
#ifdef STAPL_DEBUG
	cout<<myid<<":"<<"Found in cache directly! data_owner of gid "<<_gid <<
	  " is "<<data_owner<<endl;
#endif
	return data_owner;
	
      } else {
	map_owner=FindMapOwner(_gid);
#ifdef STAPL_DEBUG
	cout<<myid<<":"<<"map owner of gid "<<_gid<<" is "<<map_owner<<endl;
#endif
	if(map_owner != myid) {
	  if (flg_cnt) {
	    Distribution_type *const_dist = const_cast<Distribution_type*> (this);
	    const_dist->counter++;
	  }
#ifdef _TIMER_EXPERIMENT
	  tr_BD_retrieve_gid=start_timer();
#endif
	  rinfo = stapl::sync_rmi(map_owner,
				  getHandle(),
				  &Distribution_type::_Lookup,_gid);
	  if(rinfo.GetLocation().ValidLocation()) {
#ifdef STAPL_DEBUG
	    cout<<myid<<":"<<"Found in remote map directly! data_owner of gid "<<_gid <<
	      " is "<<rinfo.GetLocation()<<endl;
#endif
#ifdef _TIMER_EXPERIMENT
	    tr_BD_cache=start_timer();
#endif
	    Distribution_type *const_dist = const_cast<Distribution_type*> (this);
	    const_dist->Add2Cache(rinfo);
#ifdef _TIMER_EXPERIMENT
	    t_BD_cache+=stop_timer(tr_BD_cache);
#endif
	    return rinfo.GetLocation();
	  } else {
	    cout<<myid<<":"<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
	    return Location::InvalidLocation();
	  }
#ifdef _TIMER_EXPERIMENT
	  t_BD_retrieve_gid+=stop_timer(tr_BD_retrieve_gid);
#endif
	} else {
	  
#ifdef _TIMER_EXPERIMENT
	  tr_BD_local_lookup=start_timer();
#endif

	  return LocationMapLookup(_gid);    

#ifdef _TIMER_EXPERIMENT
	  t_BD_local_lookup+=stop_timer(tr_BD_local_lookup);
#endif
	}
      }
    }
    
  protected:
    /**local lookup - in the current part
     */
    RangeInfo _Lookup(GID _gid) const {
      int temp = SearchTable(element_location_map,_gid);
      if(temp == -1) {
	cout<<myid<<":"<<"ERROR inside _Lookup while searching for"<<_gid<<endl;
	return RangeInfo();
      }
      else return element_location_map[temp];
    }
    
    //===========================
    //bookkeeping for element_location_map                  
    //could be called both from local and from remote
    //===========================
    
  public:
    /**UpdateLocationMap
     */
    virtual void UpdateLocationMap(GID _gid, Location _location) {
      /*
      PID map_owner=FindMapOwner(_gid);
      if(map_owner != myid) {
        stapl::async_rmi(map_owner,
                         getHandle(),
                         &GraphFast_Distribution_type::_UpdateLocationMap,
                         _gid, 
                         _location);
      } else 
      _UpdateLocationMap(_gid, _location);
      */
    }

  void IncreaseBoundaryInfo(){
    //Location be(-1,INVALID_PART),end(-1,INVALID_PART);
    //partbrdyinfo.push_back(Partbdryinfo_type(be,end));
  }

  virtual void DeleteFromPartIDMap(GID _gid)
  {
    //element_partid_map.erase(_gid);
  }

  virtual void DeleteFromLocationMap(GID _gid){    
  }

  virtual void DeleteFromCache(GID const _gid) {
    //element_location_cache.erase(_gid);
  }

  protected:
    /** local UpdateLocationMap
     */
    virtual void _UpdateLocationMap(GID _gid, Location _location){    
      //array_location[_gid - begin_index] = _location;
    }
    
  public:
    /**DisplayElementLocationMap
     */
    virtual void Display() const {       
      int i;
      cout<<myid<<":"<<"GraphDistribution  "<<myid<<": \n";
      cout<<myid<<":"<<"=================================="<<endl;
      cout<<myid<<":"<<"PartIdMap:"<<endl;
      for(i=0;i<element_partid_map.size();i++){
	cout<<myid<<":"<<i<<" start gid="<<element_partid_map[i].first<<" size="<<element_partid_map[i].second<<" partid="<<element_partid_map[i].third<<endl;
      }
      cout<<myid<<":"<<"Element location map"<<endl;
      for(i=0;i<element_location_map.size();i++){
	cout<<myid<<":"<<i<<" start gid="<<element_location_map[i].GetStartGID()<<" size="<<element_location_map[i].GetSize()<<" location="<<element_location_map[i].GetLocation().locpid()<<endl;
      }
      cout<<myid<<":"<<"Element location cache"<<endl;
      for(i=0;i<element_location_cache.size();i++){
	cout<<myid<<":"<<i<<" start gid="<<element_location_cache[i].GetStartGID()<<" size="<<element_location_cache[i].GetSize()<<" location="<<element_location_cache[i].GetLocation().locpid()<<endl;
      }
      cout<<myid<<":"<<"=================================="<<endl;
    }

    //====================================
    //Find where the information about the 
    //location of the vertex is stored
    //====================================

    GID GetNextGid(){
      int temp = local_max_gid;
      if(local_max_gid + 1 == local_max_chunk + chunk ){
	local_max_chunk += nprocs*chunk;
	local_max_gid = local_max_chunk;
      }
      else local_max_gid++;
      return temp;
    }

    void LocationMapRead(istream& _myistream){
    }

    void LocationMapWrite(ostream& _myistream){
    }

    //protected:                  
    /**FindMapOwner returns the processor responsible for
       knowing where the specified by GID element is 
    */
    PID FindMapOwner(GID _gid) const {     
      return ((_gid % (nprocs*chunk))/chunk);
    }

  };  //end of BaseDistributionRanges
  
} //end namespace stapl

#endif
