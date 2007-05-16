#ifndef DISTRIBUTION_DEFINES_H
#define DISTRIBUTION_DEFINES_H
#include <runtime.h>
#include <Defines.h>

namespace stapl {   

#define _StaplPair pair

/**
 * @addtogroup basepcontainer
 * @{
 **/

#ifndef MASTER_THREAD
#define MASTER_THREAD 0
#endif

  ///Thread identifier
#ifndef PID
typedef short PID;
#endif

  ///Part identifier
#ifndef PARTID
typedef int PARTID;
#endif

/// Global identifier
#ifndef GID  
typedef int GID;
#endif

#ifndef INVALID_PID
#define INVALID_PID -99
#endif

#ifndef INVALID_PART
#define INVALID_PART -99
#endif

#ifndef INVALID_GID
#define INVALID_GID -99
#endif

#ifndef REMOTE_LAST_PART
#define REMOTE_LAST_PART -100
#endif

#ifndef MAX_NUM_PARTS_PER_THREAD
#define MAX_NUM_PARTS_PER_THREAD 5
#endif


/**
 * Location Class
 */

class Location {
 public:
  PID pid;
  PARTID part_id;

  void define_type(stapl::typer &t)  {
    t.local(pid);
    t.local(part_id);
  }

  Location() {}

  Location(const Location& _loc) {
    pid = _loc.pid;
    part_id = _loc.part_id;
  }

  Location(PID _pid, PARTID _id) {
    pid = _pid;
    part_id = _id;
  }

  Location& operator=(const Location& _loc) {
    pid = _loc.pid;
    part_id = _loc.part_id;
    return *this;
  }

  bool operator!=(const Location& _loc) const {
    return (pid != _loc.pid || 
	    part_id != _loc.part_id );
  }

  bool operator==(const Location& _loc) const {
    return (pid == _loc.pid && 
	    part_id == _loc.part_id );
  }

  PID locpid() const { return pid; }
  PARTID partid() const { return part_id; }

  void SetPid(PID _id) { pid = _id; }
  void SetPartId(PARTID _id) { part_id = _id; }

  pair<PID,PARTID> GetLocation() const {
    return pair<PID,PARTID> (pid,part_id);
  }

  bool ValidLocation() {
    if(pid != INVALID_PID) return true;
    else return false;
  }
  
  static Location InvalidLocation() {
    return Location(INVALID_PID,INVALID_PART);
  }

};

template<class iterator>
class IterLocation {

 protected:

 Location loc;
 iterator local_iterator;

public:

 void define_type(stapl::typer &t) 
  {
    stapl_assert(1,"IterLocation define_type used.\n");
    t.local(loc);
    t.local(local_iterator);
  }

  IterLocation() {}

  IterLocation(const IterLocation& _loc) {
    loc = _loc.loc;
    local_iterator = _loc.local_iterator;
  }

  IterLocation(PID _pid, PARTID  _id, const iterator& _d) {
    loc.pid = _pid;
    loc.part_id = _id;
    local_iterator = _d;
  }

  IterLocation(const Location& _loc, const iterator& _d) {
    loc=_loc;
    local_iterator = _d;
  }

  IterLocation& operator=(const IterLocation& _loc) {
    loc = _loc.loc;
    local_iterator = _loc.local_iterator;
    return *this;
  }

  bool operator!=(const IterLocation& _loc) const {
    return ( loc != _loc.loc ||
	     local_iterator != _loc.local_iterator );
  }

  bool operator==(const IterLocation& _loc) const {
    return ( loc == _loc.loc && 
	     local_iterator == _loc.local_iterator );
  }
  
  const Location& location() const { return loc; }

  iterator local_iter() const { return local_iterator; }

  void Set_Local_Iter(const iterator& _it) {
    local_iterator = _it;
  }

  void SetLocation(const Location& _loc) {
	loc = _loc;
  }

  void Set(const IterLocation<iterator>& _it) {
        local_iterator = _it.local_iterator;
	loc.pid = _it.loc.pid;
	loc.part_id = _it.loc.part_id;
  }

  void Set(PID _pid, PARTID  _id, iterator _d) {
    loc.SetPid(_pid);
    loc.SetPartId(_id);
    local_iterator = _d;
  }

};

inline ostream& operator << (ostream& s,const Location& loc) {
  return s<<"(pid "<<loc.locpid()<<", partid "<<loc.partid()<<") ";
}

#ifndef MAX_MACHINE_LEVEL
#define MAX_MACHINE_LEVEL 1
#endif

#define TASKPOOL vector<VID>

enum PartitionPolicy {ALG,PERCENTAGE,UNIFORM,GIVEN_PART,GIVEN_MAP,
		      GIVEN_DIST,NIL_POLICY};

} /* //end namespace stapl */
//@}
#endif
