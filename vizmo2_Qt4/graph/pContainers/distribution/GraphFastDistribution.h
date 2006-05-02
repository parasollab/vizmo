#ifndef ARRAYDISTRIBUTION_H
#define ARRAYDISTRIBUTION_H

#include <runtime.h>
#include "rmitools.h"

#include "DistributionDefines.h"
#include "BaseDistribution.h"

#include <hash_map>
#include <valarray>
#include <assert.h>
namespace stapl {
  
  class GraphFastDistributionInfo {
    GID gid;
    int size;
    Location loc;

  public:
    GraphFastDistributionInfo(){}

    GraphFastDistributionInfo(GID g, int s, Location l){
      gid = g;
      size = s;
      loc = l;
    }
    GID GetGID() const{
      return gid;
    }
    int GetSize() const{
      return size;
    }
    Location GetLocation() const{
      return loc;
    }
    void SetGID(GID g){
      gid = g;
    }
    void SetSize(int s){
      size = s;
    }
    void SetLocation(Location l){
      loc = l;
    }   
    void define_type(stapl::typer &t)
    {
      t.local(gid);
      t.local(size);
      t.local(loc);
    }
  };

  template <class T1>
  class GraphFastDistribution : public BaseDistribution<pair<int,GID> > {            
  public:
    
    //===========================
    //types
    //===========================
    
    typedef GraphFastDistribution GraphFast_Distribution_type;
    typedef vector<GraphFastDistributionInfo> DistributionInfo_type;
    typedef vector<GraphFastDistributionInfo>::iterator DistributionInfo_iterator;
    typedef valarray<Location> GraphFast_Location_type;
    typedef pair<int, GraphFast_Distribution_type> distribution_pair;
    //===========================
    //data
    //===========================
  protected:
    GraphFast_Location_type    array_location;
    GID                    begin_index;
    DistributionInfo_type  distribution_info;
    int                    chunk_size;
  public:
    int total_size;
    
  public:
    //===========================
    //constructors & destructors
    //===========================
    
    /**default constructor
     */
    GraphFastDistribution(){                                     
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**copy constructor
     */
    GraphFastDistribution(const GraphFastDistribution& w){
      total_size = w.total_size;
      begin_index = w.begin_index;
      chunk_size = w.chunk_size;
      array_location.resize(chunk_size);
      array_location = w.array_location;
      distribution_info = w.distribution_info;  
      this->register_this(this);
      rmi_fence(); 
    }
    
    /**destructor
     */
    ~GraphFastDistribution() {
      stapl::unregister_rmi_object( this->getHandle() );
    }
    
    //===========================
    //public methods
    //===========================
    
    /**Initializing array distribution; make a copy of the distribution info 
     *and set the local information; start index chunk size;
     */
    void InitializeGraphFastDistribution(int _n, const DistributionInfo_type& _di){
      distribution_info = _di; 
      total_size=_n;
      chunk_size = _di[myid].GetSize();
      begin_index = _di[myid].GetGID();
      cout<<"DISTRIBUTION:"<<stapl::get_thread_id()<<chunk_size<<":"<<begin_index<<endl;
    }
    
    /**Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(GID start, int size, Location loc){
      GraphFastDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
    }
    
    /**Method to add a part to the distribution vector
     */
    void AddPartToDistributionVector(GraphFastDistributionInfo d){
      distribution_info.push_back(d);
    }

    /** Method that allows a new part to be added to the pContainer
     * Called from the pGraphFast
     * Broadcasts the change to the rest of the processors
     */
    void AddPart(GID start, int size, Location loc){
      GraphFastDistributionInfo d;
      d.SetSize(size);
      d.SetGID(start);
      d.SetLocation(loc);
      distribution_info.push_back(d);
      for (int i=0; i<nprocs; i++){
	if (i != this->myid)
	  async_rmi(i, getHandle(), 
		    &GraphFastDistribution::AddPartUpdate, d);
      }
    }

    /**Method to update the distribution vector
     *on the rest of the processors
    */
    void AddPartUpdate(GraphFastDistributionInfo d){
      distribution_info.push_back(d);
      return;
    }

    /**Method to split the part into 2 
     *Called from the pContainer SplitPart; The arguments are :
     *id1 part to be splitted
     *id2 the newly created part
     *size that is left to part1
     *Broadcasts the changes to the rest of the processors
    */
    void SplitPart(int id1, int id2, int size){
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == myid && it->GetLocation().partid() == id1){
	  GraphFastDistributionInfo d;
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
			&GraphFastDistribution::SplitPartUpdate, q);
	  }
	  return;
	}
      }
    }
  
    /**Method to update the distribution vector
       on the rest of the processors
    */
    void SplitPartUpdate(_StaplQuad<int,int,int,int> q){
      for (DistributionInfo_iterator it = distribution_info.begin(); 
	   it != distribution_info.end(); it++){
	if (it->GetLocation().locpid() == q.first && 
	    it->GetLocation().partid() == q.second){
	  GraphFastDistributionInfo d;
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

    /**Method finds continuous parts on a processor
     * Returns the parts information to the pContainer
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

    /** Method updates the distribution vector after the merge
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
		    &GraphFastDistribution::MergeUpdate2,gid);
	}
      }
    }

    /**Method to update the distribution vector
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

    /**
     *Method to display the distribution vector
     */
    virtual void DisplayDistributionInfo() const {       
      cout<<"GraphFast Distribution Vector"<<endl;
      for(int i=0; i< distribution_info.size(); i++) {
	cout<<"Processor: "<<distribution_info[i].GetLocation().locpid()<<endl;
	cout<<"Part: "<<distribution_info[i].GetLocation().partid()<<endl;
	cout<<"Size: "<<distribution_info[i].GetSize()<<endl;
	cout<<"Starting at GID: "<<distribution_info[i].GetGID()<<endl;
      } 
    }
    
    /**copy constructor
     */
    GraphFast_Distribution_type& operator= (const GraphFast_Distribution_type& w) {
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
    /**
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
    /** return first index sorted on this thread.
     */
    GID get_start_index() { return begin_index; }


    /**an element is local if it is in the specified part
     */
    bool IsLocal(GID _gid, PARTID& partid) const{
      //binary search SearchPart()
      int temp = SearchPart(_gid);
      if(temp == -1) {
	cout<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
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
    
    /**finding an element by the GID: first look in local cache,
       else find the map owner, add to the local cache 
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
    
    void Add2LocationMap(_StaplPair<GID,Location>& _x) {
    }

  protected:
    /**local lookup - in the current part
     */
    virtual Location _Lookup(GID _gid) const {     
      return array_location[(_gid - begin_index)];
    }
    
    //===========================
    //bookkeeping for element_location_map                  
    //could be called both from local and from remote
    //===========================
    
  public:
    /**UpdateLocationMap
     */
    virtual void UpdateLocationMap(GID _gid, Location _location) {   
      PID map_owner=FindMapOwner(_gid);
      if(map_owner != myid) {
        stapl::async_rmi(map_owner,
                         getHandle(),
                         &GraphFast_Distribution_type::_UpdateLocationMap,
                         _gid, 
                         _location);
      } else 
      _UpdateLocationMap(_gid, _location);
    }
    
  protected:
    /** local UpdateLocationMap
     */
    virtual void _UpdateLocationMap(GID _gid, Location _location){    
      array_location[_gid - begin_index] = _location;
    }
    
  public:
    /**DisplayElementLocationMap
     */
    virtual void DisplayElementLocationMap() const {       
      cout<<"GraphFast Location Map on Proc "<<myid<<": \n";
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
    /**FindMapOwner returns the processor responsible for
       knowing where the specified by GID element is 
    */
    virtual PID FindMapOwner(GID _gid) const {     
      for(int i=0;i<distribution_info.size();i++) {
        if(_gid >= distribution_info[i].GetGID() && _gid < distribution_info[i].GetGID()+distribution_info[i].GetSize()) {
          //cout<<"GID OWNER:"<<_gid<<","<<i<<endl;
          return i;
        }
      }
#ifdef DEBUG
      cout<<"ERROR in find owner"<<endl; 
#endif
      return -1;
    }

  };  //end of GraphFastDistribution
  
} //end namespace stapl

#endif
