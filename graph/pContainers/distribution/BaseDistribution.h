#ifndef BASEDISTRIBUTION_H
#define BASEDISTRIBUTION_H

#include <runtime.h>
#include "rmitools.h"

//gabrielt #include "Scheduler.h"

#include "DistributionDefines.h"

//gabrielt #include "UserInputStruct.h"


#include "Defines.h"

/**
 * @addtogroup basepcontainer
 * @{
 **/

#ifdef _TIMER_EXPERIMENT
extern stapl::timer tr_BD_cache,tr_BD_retrieve_gid,tr_BD_local_lookup;
extern double t_BD_cache,t_BD_retrieve_gid,t_BD_local_lookup;
#endif

namespace stapl {

  struct base_distribution {};
  struct vector_distribution {};

  /**
   *Base Distribution class. The template argument denotes a type
   *that should represent a range of elements. The most general type
   *would be an enumeration of elements ( vector of global
   *identifiers(GID)). For more specific pcontainers, it could a pair of
   *indicies ( for pvector), a pair of iterators ( for plist), etc.
   * \nosubgrouping 
   */

template<class ELEMENT_SET>
class BaseDistribution : public BasePObject {
 public:

  //=======================================
  //types
  //=======================================

  /**@brief  
      Element_set type.
      Elementset can be vector<GID> for general containers, 
      vector<Key> for map and set,
      ranges for linearly ordered containers, like list, vector, arrays
   */
  typedef ELEMENT_SET Element_Set_type;

  //vector<Element_Set_type> corresponds to the collection of parts
  //partid is the index of the vector

  /**@brief  
      This type represents a gloabl map of ranges of elements that
      represent the pcontainer.
   */ 
  typedef map<PID, vector<Element_Set_type> > Distribution_Info_type;

  /**@brief  
      This type 
  */
  typedef BaseDistribution<Element_Set_type> Base_Distribution_type;

  /**@brief 
     This type represents a map between a GID and a Location that
     knows where that GID is located.
   */

  typedef hash_map<GID, Location, hash<GID> > Location_Map_type;

  /**@brief 
     This type represents a map between a GID and a part id where that
     GID is located in
   */
  typedef hash_map<GID, PARTID, hash<GID> > PartId_Map_type;
  
  /**@brief 
     Iterator for PartId_Map_type
   */
  typedef Location_Map_type::iterator ILIT;
  /**@brief 
     Constant Iterator for PartId_Map_type
   */
  typedef Location_Map_type::const_iterator CILIT;

  /**@brief 
     This type is used to keep the part order in a pcontainer. It
     stores the predecessor and successor for each part a Location
   */
  typedef pair<Location,Location> Partbdryinfo_type;

  /**@brief 
     This is the distribution tag that the BaseDistribution has been
     instantiated with. It has no use in this class, but the classes
     that inherit BaseDistribution use it.
   */
  typedef base_distribution distribution_tag;
  
  //===========================
  //data
  //===========================
 protected:

  /**@brief 
     Distributed complete gid map for random lookup using gid. This
     data member stores for each GID that is assigned to this thread,
     the location of that GID. The GID assignment to threads is done
     using % (modulo) number of processors. Each thread keeps track of the GIDs whose
     modulo p (number of processors) matches their own thread id.

     e.g. thread [0] :<0,location0>,<p,locationp>,<2p,location2p>..etc
          thread [1] :<1,location1>,<p+1,location p+1>,<2p+1,location 2p+1>..etc
	  ....
	  thread [p] :<p-1,location p-1>,<2p-1,location2p-1>,<3p-1,location3p-1...etc

     Location consists of a pair of processor id and part id. <proc id,part id>
   */
  Location_Map_type element_location_map;  

  /**@brief 
   *local cached gid map for fast lookup. Everytime an element is
   *lookedup by the base distribution object, its location is stored
   *in this cache. Using the cache saves one armi call for finding
   *where that GID resides.
   *
   *e.g.
   * <GID=3 - Thread_id=0, part_id=0> -the gid is location on thread 0 and it's in part whose local id is 0.
   */
  Location_Map_type element_location_cache;  

   /**@brief 
    *element to part id map (local elements only). All the local parts
    *register their elements' gids in to this map.
    *
    * e.g.the entry <0, <1,1> > and <0,<2,0> > means that for the
    * local part id 0 (first argument), its predecessor ( first
    * location in the pair is the predecessor) is on thread 1 (second
    * argument) and it the part whose local id is 1. The successor
    * (second location in the pair) of local part 0 is on thread 2 and
    * it is the part whose local id is 0.
   */
  PartId_Map_type element_partid_map;

  /**@brief Distribution Version, so pRange can know if its own one is valid
     or not.
   */
  unsigned int dist_version;

  /**@brief  
      Thread id
   */
  PID myid;

  /**@brief  
      Number of processors 
  */
  int nprocs;

 /**@brief 
    Clusters' ids for hierarchical parititioning of pContainers
  */
  PID cids[MAX_MACHINE_LEVEL];

  /**@brief  Last local gid assigned. This counter constantly increases until it rolls its type down. */
  GID local_max_gid;

  /**@brief  
   * First Location in the pair is previous part
   *  Second is next part, following global order, not necessarily local.
   */
  vector<Partbdryinfo_type > partbrdyinfo;


  /*flag used in global iterator ++operator
   * to find out the next part wrt. the global order
   * if not true, a dummy order will be used.
   * otherwise partbrdyinfo will be checked
   */
  bool validbdinfo;

  /*
     counter that can be used for different measurements
   */
  int counter;
  /*
     Flag that indicates wether the counting shoudl precede or not.
   */
  bool flg_cnt;

public:
  /**@name Constructors and Destructor */
  //@{
  //===========================
  //constructors & destructors
  //===========================
  /**@brief 
     Default Constructor.
   */
  BaseDistribution() 
    {
      nprocs = get_num_threads();
      validbdinfo = 0;
      myid = get_thread_id();
      local_max_gid=myid-nprocs;
      counter = 0;
      flg_cnt = false;
    }
  /**@brief 
     Copy Constructor
   */
  
  BaseDistribution(const BaseDistribution& w) :
    myid(get_thread_id()),
    nprocs(get_num_threads()),
    element_location_map(w.element_location_map),
    element_location_cache(w.element_location_cache),
    element_partid_map(w.element_partid_map),
    dist_version(w.dist_version),
    local_max_gid(w.local_max_gid),
    partbrdyinfo(w.partbrdyinfo),
    validbdinfo(w.validbdinfo),counter(w.counter)
    {
      for (int i = 0; i < MAX_MACHINE_LEVEL; ++i)
	cids[i] = w.cids[i];
    }

  /**@brief 
     Destructor
   */
  ~BaseDistribution() {}
  //@}

  /**@name Utilities */
  //@{
  //===========================
  //public methods
  //===========================
  /**@brief 
     Operator = Assignment
  */
  Base_Distribution_type& operator= (const Base_Distribution_type& w) {
    element_location_map  = w.element_location_map;
    element_partid_map = w.element_partid_map;
    element_location_cache  = w.element_location_cache;
    dist_version=w.dist_version;
    local_max_gid=w.local_max_gid;
    for (int i = 0; i < MAX_MACHINE_LEVEL; ++i)
      cids[i] = w.cids[i];
    partbrdyinfo=w.partbrdyinfo;
    validbdinfo = w.validbdinfo; 
    counter = w.counter;
    return *this;
  }


  /**@brief  Get the distribution version */
  virtual int GetDistributionVersion() const {
    return dist_version;
  }
  /**@brief  Set the distribution version */
  virtual void SetDistriubtionVersion(int _v) {
    dist_version = _v;
  }
  /**@brief  Get the pointer to clusters' ids data structure */
  virtual const PID* GetCids() const { return &cids[0]; }

  /**@brief  Set the clusters' ids for the machine */
  virtual void SetCids(const PID* pids) {
    for(int i=0; i<MAX_MACHINE_LEVEL; i++) cids[i]=pids[i];
  }

  /**@brief  Set the flag counting flag to true. Counting remote accesses can precede now */
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
  //====================================
  //Methods for parsing user-inputs about partitioning/mapping/distribution
  //====================================
 public:
/*  
virtual void Parser(PartitionInputs* user_inputs, Distribution_Info_type& d_map) {
    if(user_inputs->partpolicy == NIL_POLICY) {
      // cout<<"User Inputs before deciding on a policy:"<<*user_inputs<<endl;
      PartitionInputs default_inputs;
      if(!((Nil_Policy*) user_inputs->inputs)->hasDDG) {
	default_inputs.partpolicy = UNIFORM;
      } else {
	default_inputs.partpolicy = ALG;
	Alg* default_alg = new Alg;
	default_alg->ddgptr  = ((Nil_Policy*) user_inputs->inputs)->ddgptr;
	switch(((Nil_Policy*) user_inputs->inputs)->performance_degree) {
	case 0:
	  default_alg->cat = DSC;
	  default_alg->aat = LLB;
	  break;
	case 1:
	  default_alg->cat = FLB;
	  break;
	default:
	  if(myid == SRC) cout<<"Invalid performance degree. Exit. "<<endl;
	  exit(1);
	}
	default_inputs.inputs = default_alg;
      }
      user_inputs = &default_inputs;
    }

//     cout<<"User Inputs used: "<<*user_inputs<<endl; 
    scheduler::SchedulerType<double,double,int,int> sched(&user_inputs->machineinfo);

    switch(user_inputs->partpolicy) {
    case ALG:
      //       sched.Scheduling(*user_inputs->inputs, d_map);  --ann 
      break;

    case PERCENTAGE:
      //       DefaultPartitioner:: 
      // 	PartByPercent((user_inputs->inputs)->percentiles, 
      // 		      d_map);
      break;
      
    case UNIFORM:
      //DefaultPartitioner::PartInUniform(d_map); 
      break;

    case GIVEN_PART:
      //sched.Mappinging(*user_inputs->inputs, d_map); 
      break;
      
    case GIVEN_MAP:
      //type mismatch, Given_Map only for map<PID,vector<GID> > --ann fix
      //d_map = ((Given_Map*) (user_inputs->inputs))->data_map; 
      break;
      
    default:
      if(myid == SRC) cout<<"Invalid User Inputs. Exit. "<<endl;
      exit(1);
    }
  }

*/


  //@}
  /**@name Boundary Methods */
  //@{
public:
  //=======================================
  //setup pcontainer parts boundaries, to specify 
  //the connection for global iterator to use
  //=======================================
  /**@brief  Initializes the part order (part bdry info) when there is only
      one part in each sub pcontainer. It assumes that the order is
      dictated by the thread id order ( thread 0 part id 0 is frist,
      followed by thread 1 part id o, thread 2 part id 0,...thread p
      part id 0). 

      It is used in plist and pvector constructors with n elements
      initially e.g. plist myplist(n,data);
   */

  virtual void InitBoundaryInfo() {
    Location be,end;
    PARTID start_partid = 0;
 
    if(get_thread_id() == 0) {
      be=Location(get_thread_id(), INVALID_PART);
    } else {
      be=Location(get_thread_id()-1, start_partid);
    }
 
    if(get_thread_id() == get_num_threads()-1)  {
      end=Location(get_thread_id(), INVALID_PART);
    } else {
      end=Location(get_thread_id()+1, start_partid);
    }
                                                                                
    partbrdyinfo[start_partid]= Partbdryinfo_type(be,end);
    validbdinfo = 1;
   }

  /**@brief 
     Adds an empty part in the par order.
   */
  void IncreaseBoundaryInfo(){
    Location be(-1,INVALID_PART),end(-1,INVALID_PART);
    partbrdyinfo.push_back(Partbdryinfo_type(be,end));
  }

  /**@brief 
   * Sets the predecessor and successor in the part order for a
   * specific part whose id is passed as argument
   */
  virtual void InitBoundaryInfo(PARTID _partid, Location _before, Location _after) {
    stapl_assert( _partid < partbrdyinfo.size(), "ERROR:: Invalid part id in BaseDistribution::InitBoundaryInfo" );
    partbrdyinfo[_partid]= Partbdryinfo_type(_before,_after);
    validbdinfo = 1;
   }

  /**@brief 
   * Assigns an entry in the part order based on the part id
   */
  void SetPartBdry(PARTID _id, const Partbdryinfo_type& _bd) {
    stapl_assert( _id < partbrdyinfo.size(), "ERROR:: Invalid part id in BaseDistribution::InitBoundaryInfo" );
    partbrdyinfo[_id]=_bd;
  }

  /**@brief 
   * Sets flag to valid after bdry setup so query won't go to
   * dummy_lookupbdinfo
  */
  void SetValidBdInfo() { validbdinfo = 1; }
public:

  /**@brief  
   * Returns the part successor, from the part order, of the part
   * whose id is passed as argument. It returns the location of that part
   */
  virtual Location GetBdEndInfo(PARTID _partid) const {
    if(validbdinfo) 
      return LookUpBdEndInfo(_partid);
    else
      return Location(myid, INVALID_PART);
  }

  //protected:
  /*
   * Returns the part successor, from the part order, of the part
   * whose id is passed as argument. It returns the location of that part in a const variable.

   * Called only to access remote nodes
   * used in the definition of global iterators
   * check boundary info
   */
  const Location& LookUpBdEndInfo(PARTID _partid) const {
    vector<Partbdryinfo_type>::const_iterator 
      it = partbrdyinfo.begin()+_partid;
    if(_partid < partbrdyinfo.size()) {
      return it->second;
    } else {
      cout<<"Error! Cannot find part in list bdry info."<<endl;
      exit(-1);
      return *(new Location());//to avoid compiler warnings
    }
  }

  /*
   *Dummy lookup function to find the next part. If next part is not
   *available, it returns the first part from the next processor.
  */
  virtual  Location Dummy_LookUpBdEndInfo(PARTID _partid, 
					  int _num_parts) const {
    if(_partid+1 < _num_parts) {
      return Location(myid,_partid+1);
    } else {
      if(myid+1 < nprocs) 
	return Location(myid+1, 0);
      else 
	return Location(myid, INVALID_PART);
    }
  }

  /*
   *Dummy lookup function to find the next part. If next part is not
   *available, it returns the first part from the next processor.
  */
  virtual  Location Local_LookUpBdEndInfo(PARTID _partid, 
					  int _num_parts) const {
    if(_partid+1 < _num_parts) {
      return Location(myid,_partid+1);
    } 
    return Location(myid, INVALID_PART);
  }

public:

  /**@brief 
   * It returns the location of the predecessor part for the part
   * whose id is passed as argument.
   */
  virtual Location GetBdBeginInfo(PARTID _partid) const {
    if(validbdinfo) 
      return LookUpBdBeginInfo(_partid);
    else
      return Location(myid, INVALID_PART);
  }

  //protected:
  /*
   * It returns the location of the successor part for the part
   * whose id is passed as argument.
   */

  const Location& LookUpBdBeginInfo(PARTID _partid) const {
    vector<Partbdryinfo_type>::const_iterator 
      it = partbrdyinfo.begin()+_partid;
    if( _partid < partbrdyinfo.size()) {
      return it->first;
    } else {
      cout<<"Error! Cannot find part in list bdry info."<<endl;
      exit(-1);
      return *(new Location());//to avoid compiler warnings
    }
  }

  /**@brief 
   *dummy lookup function to find the previous part for global
   *iterator, if previous part available, go to previous, else go to
   *last part in previous processor 
  */
  virtual  Location Dummy_LookUpBdBeginInfo(PARTID _partid) const {
    if(_partid > 0) {
      return Location(myid,_partid-1);
    } else {
      if(myid-1 > 0) 
	return Location(myid-1, REMOTE_LAST_PART);
      else 
	return Location(myid, INVALID_PART);
    }
  }
  /*
   *dummy lookup function to find the previous part for global
   *iterator, if previous part available, go to previous, else go to
   *last part in previous processor 
  */

  virtual  Location Local_LookUpBdBeginInfo(PARTID _partid) const {
    if(_partid > 0) {
      return Location(myid,_partid-1);
    }  
    return Location(myid, INVALID_PART);
  }

public:
  /**@brief 
   * Returns the successor and the predecessor, in the part order, of
   * the part whose id is passed as argument.
   */
  const Partbdryinfo_type& GetPartBdry(PARTID _id) const {
    return partbrdyinfo[_id];
  }
  /**@brief 
   * Returns the local part order as a vector of pairs of locations
   */
  const vector<Partbdryinfo_type>& PartBrdyInfo() const { 
    return partbrdyinfo; 
  }
  /**@brief 
   * Prints the part order
   */
  void DisplayBoundaryInfo() const {
    vector<Partbdryinfo_type>::const_iterator it;
    PARTID id=0;
    for(it=partbrdyinfo.begin(); it!=partbrdyinfo.end(); ++it,++id) {
      cout<<"##### Part "<<id<<endl;
      cout<<"begin: "<<(*it).first<<" end: "<<(*it).second<<endl;
    }
  }
  //@}

  /**@name Element location */
  //@{
  //====================================
  //Methods for element location
  //====================================
 public:

  /**@brief 
   * It looks up a gid and returns its location. It checks the local
   * cache first. If it's not in the cache, it finds the thread
   * responsible for storing the location of that gid and sends a rmi
   * inquiry message to that thread. In case of an error, it returns
   * invalid location.
   */
  virtual Location Lookup(GID _gid) const {
    //check in local cache 
#ifdef _TIMER_EXPERIMENT
    tr_BD_cache=start_timer();
#endif
    Location data_owner=Cache_Lookup(_gid);//(INVALID_PID,INVALID_PART)
#ifdef _TIMER_EXPERIMENT
    t_BD_cache+=stop_timer(tr_BD_cache);
#endif
    if(data_owner.ValidLocation()) { 
#ifdef STAPL_DEBUG1
	cout<<"Found in cache directly! data_owner of gid "<<_gid <<
	  " is "<<data_owner<<endl;
#endif
      return data_owner;

    } else {

      PID map_owner=FindMapOwner(_gid);
#ifdef STAPL_DEBUG
      cout<<"map owner of gid "<<_gid<<" is "<<map_owner<<endl;
#endif
 
      if(map_owner != myid) {
	if (flg_cnt) {
	  BaseDistribution *const_hack = const_cast<BaseDistribution*> (this);
	  const_hack->counter++;
	  //cout<<"PPROBLEM:"<<stapl::get_thread_id()<<":"<<_gid<<endl;
	}
#ifdef _TIMER_EXPERIMENT
	tr_BD_retrieve_gid=start_timer();
#endif
	data_owner = stapl::sync_rmi(map_owner,
				     getHandle(),
				     &Base_Distribution_type::_Lookup,_gid);
#ifdef _TIMER_EXPERIMENT
	t_BD_retrieve_gid+=stop_timer(tr_BD_retrieve_gid);
#endif
      } else {

#ifdef _TIMER_EXPERIMENT
	tr_BD_local_lookup=start_timer();
#endif
	data_owner = _Lookup(_gid);    
#ifdef _TIMER_EXPERIMENT
	t_BD_local_lookup+=stop_timer(tr_BD_local_lookup);
#endif
      }

      //assert(data_owner.locpid()<nprocs);
      if (data_owner.locpid() >= nprocs) {
	cout <<" Howdy2 " << data_owner.locpid() << " gid = " << _gid << endl << flush;	
      }
      
      if(data_owner.ValidLocation()) {
#ifdef STAPL_DEBUG
	cout<<"Found in remote map directly! data_owner of gid "<<_gid <<
	  " is "<<data_owner<<endl;
#endif
        //ain't this pretty?
#ifdef _TIMER_EXPERIMENT
	tr_BD_cache=start_timer();
#endif
	BaseDistribution *const_hack = const_cast<BaseDistribution*> (this);
	const_hack->Add2Cache(_gid,data_owner);
	//	Add2Cache(_gid,data_owner);
#ifdef _TIMER_EXPERIMENT
	t_BD_cache+=stop_timer(tr_BD_cache);
#endif
	return data_owner;

      } else {
	//cout<<"Cannot find element in Location Map with given gid "<<_gid<<endl;
	return Location::InvalidLocation();
      }
    }
  }

 protected:
  /**@brief 
   * Checks to see if the gid's location is present in the local
   * element location cache. If it is, it returns the location. If
   * it's not in the cache, it returns invalid location.
   */
  virtual Location Cache_Lookup(GID _gid) const {
    CILIT mi=element_location_cache.find(_gid);
    if( mi != element_location_cache.end() ) {
      return mi->second;
    } else return Location::InvalidLocation();
  }

  /**@brief 
   * Checks if the gid is present in the local element location
   * map. If it is , it returns the location. If it's not in the local
   * element location map, it returns invalid location.
   */
  virtual Location _Lookup(GID _gid) const {

    CILIT mi=element_location_map.find(_gid);

#ifdef STAPL_DEBUG
    cout <<"Asked info about"<<_gid<<"returned:"<<mi->second.locpid()<<endl;
#endif
    if( mi != element_location_map.end() ) {
      return mi->second;
    } else return Location::InvalidLocation();
  }

  //===========================
  //bookkeeping for element_location_cache
  //===========================
 public:
  /**@brief 
   * Adds a predetermined location of a specific gid to the cache
   * location
   */
  virtual void Add2Cache(GID const _gid , Location const _pid) {    
    pair<GID,Location> mypair(_gid,_pid);
    element_location_cache.insert(mypair);
  }

  /**@brief 
   * Clears the cache. Called after a distribute method is invoked (
   * the distribution mehtod might change the location of the
   * pcontainer elements) making the element location cache obsolete.
   */
  virtual void FlushCache() {
    element_location_cache.clear();
  }

 protected:
  /**@brief 
   * Updates the location of a gid in the element location cache. 
   */
  virtual void UpdateCache(GID const _gid ,Location const _pid) {
    element_location_cache[_gid]=_pid;
  }

 public:
  /**@brief 
   * Deletes an element location from the cache. If the cache gets
   * full, elements from it are erased.
   */
  virtual void DeleteFromCache(GID const _gid) {
    element_location_cache.erase(_gid);
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

 public:
  /**@brief 
   * Inserts the location of an element whose gid is passed as
   * argument to the element location map. It finds the thread that is
   * responsible for keeping track of the address of that gid and
   * sends a rmi to that thread to set the address for that specific
   * gid.
   */

  virtual void Add2LocationMap(_StaplPair<GID,Location>& _x) {
    PID map_owner=FindMapOwner(_x.first);
    if(map_owner != myid) {
      stapl::async_rmi(map_owner,
		      getHandle(),
		      &Base_Distribution_type::_Add2LocationMap,_x);
    } else 
      _Add2LocationMap(_x);    
  }

  /**@brief 
   * Adds a gid that belongs to the part whose id is passed as
   * argument to the element part map
   */
  virtual void Add2PartIDMap(GID _gid,PARTID _pid)   
  {
        element_partid_map[_gid]=_pid;
  }

  /**@brief 
   * Returns the local part id that has the element whose gid is
   * passed as argument. If the element is not found, it returns
   * INVALID_PART.
   */
  virtual PARTID LookupPartID(GID _gid) 
  {
        PartId_Map_type::iterator it=element_partid_map.find(_gid);
      
        if(it!=element_partid_map.end())
                return it->second;
        else
                return INVALID_PART;
 
  }
  /**@brief 
   *  Delete a gid from the element part map
   */
  virtual void DeleteFromPartIDMap(GID _gid)
  {
	element_partid_map.erase(_gid);
  }

  /**@brief 
   * It determines whether the gid is local to the thread or not. If
   * so, it sets the partid argument to the part that has that gid and
   * returns true. If the gid is not local, it sets the partid
   * argument to INVALID_PART and returns false.
   */
  virtual bool IsLocal(GID _gid,PARTID &partid) const
  {
    PartId_Map_type::const_iterator it=element_partid_map.find(_gid);
    
    if(it!=element_partid_map.end())
    {
      partid=it->second;
      return true;
    }
    else
    {
      partid=INVALID_PART;
      return false;
    }
  }

 protected:
  /*
   * Helper method : for other threads to call it when they update the
   * element location map.  
   *
   *It inserts the location of gid into the local element location
   *map.
   */
  virtual void _Add2LocationMap(_StaplPair<GID,Location>& _x){    
    pair<GID,Location> mypair(_x.first,_x.second);
    element_location_map.insert(mypair);
  }

 public:
  /**@brief 
   * It updates the element gid location in the element location
   * map.If the gid is not local, it sends an rmi to the onwer thread
   * to do so.
   */
  virtual void UpdateLocationMap(_StaplPair<GID,Location>& _x) {
    PID map_owner=FindMapOwner(_x.first);
    if(map_owner != myid) {
      stapl::async_rmi(map_owner,
		      getHandle(),
		      &Base_Distribution_type::_UpdateLocationMap,_x);
    } else 
      _UpdateLocationMap(_x);    
  }

 protected:
 /*
   * Helper method : for other threads to call it when they update the
   * element location map It updates the element gid location in the
   * local element location map.
   */
  virtual void _UpdateLocationMap(_StaplPair<GID,Location>& _x){    
    element_location_map[_x.first] = _x.second;
  }

 public:
  /**@brief 
   * It erases an gid from the element location map. If the gid is not
   * local, it sends an rmi to the onwer thread to do so.
   */
  virtual void DeleteFromLocationMap(GID _gid){    
    PID map_owner=FindMapOwner(_gid);
    if(map_owner != myid) {
      stapl::async_rmi(map_owner,
		      getHandle(),
		      &Base_Distribution_type::_DeleteFromLocationMap,_gid);
    } else 
      _DeleteFromLocationMap(_gid);
  }

 protected:
  /*
   * Helper method : for other threads to call it when they update the element location map 
   * It erases the element gid from the local element location map.
   */
  virtual void _DeleteFromLocationMap(GID _gid) {    
    element_location_map.erase(_gid);
  }

 protected:
  /**@brief 
   * This method returns the processor id that is responsible for
   * having the location of the element whose gid is passed as
   * argument. 

   * Note : there is a convention that each processor tracks the
   * location of the gids whose modulo (%) p (number of processors) is
   * equal to their thread id. If this convention changes, this is the
   * only method that should reflect that. The rest of the code is
   * modularized and uses this method to find the processor that keeps
   * track of this gid.
   */
  virtual PID FindMapOwner(GID _gid) const {
    return (_gid%nprocs);
  }
  
 public:
  /*
   * The method returns a new GID ( based on the gid convention, the
   * next available gid is local_max_gid + P).  
   *
   * If the GID assignment changes, this is the only method that needs
   * to be changed.
  */
    virtual GID GetNextGid()
      {
	local_max_gid += nprocs;
	return local_max_gid;
      }

  /**@brief 
     The function returns the next n available GIDs using the GetNextGid method..
  */
    virtual void GetNextGids(int n, vector<GID>& gids )
      {
	if(!gids.empty()) gids.clear();
	gids.resize(n);
	for(int i=0; i<n; ++i)
	  gids[i] = GetNextGid();
	    
      }
  //@}

 public:

  /**@name I/O methods */
  //@{
  /**@brief 
   * Prints all the BaseDistribution data members
   *
   */
  virtual void DisplayElementLocationMap() const {
    cout<<"Element Location Map on Proc "<<myid<<": \n";
    CILIT it=element_location_map.begin();
    for(;it!=element_location_map.end(); ++it) {
     cout<<"GID : "<<it->first<<" on Proc "<<it->second.locpid()<<" with part_id "<<
       it->second.partid()<<endl; 
    } 

    cout<<"Element Location Cache on Proc "<<myid<<": \n";
    for(it=element_location_cache.begin(); 
	it!=element_location_cache.end(); ++it) {
     cout<<"GID : "<<it->first<<" on Proc "<<it->second.locpid()<<" with part_id "<<
       it->second.partid()<<endl; 
    } 


   cout<<"PartID map  on Proc "<<myid<<": \n";
   PartId_Map_type::const_iterator iter=element_partid_map.begin();
   for( ;       iter!=element_partid_map.end(); ++iter)
   {
        cout<<"GID = "<<iter->first<<" PAIRTID = "<<iter->second<<endl;
   }
   int i=0;
   cout<<" Part Boundary Info "<< myid<<": \n";
   for(vector<Partbdryinfo_type >::const_iterator part_it=partbrdyinfo.begin();part_it!=partbrdyinfo.end();++part_it)
     {
       cout<<"Part "<<i<<" has the predecessor part "<<(*part_it).first.partid()<<" which is located on thread "<<(*part_it).first.locpid()<<endl;
       cout<<"Part "<<i<<" has the successor part "<<(*part_it).second.partid()<<" which is located on thread "<<(*part_it).second.locpid()<<endl;
       i++;
     }
    
  }
  //@}
};

/*
 *  It returns the distribution tag class 
 */
 template<class Distribution_type> 
   typename Distribution_type::distribution_tag  distribution_category(Distribution_type &x) {return typename Distribution_type::distribution_tag();}


} //end namespace stapl
//@}
#endif
