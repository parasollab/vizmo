#include "Schedule.h"

namespace scheduler {

//===================================================================  
//  Methods for class TaskSet
//===================================================================
  
//===================================================================
//  Constructors and Destructor
//===================================================================
//      TaskSet();
//      TaskSet(PID);
//      TaskSet(VID, PID);
//      TaskSet(vector<VID>&, PID);
//      ~TaskSet();
//===================================================================

TaskSet::TaskSet()  :  ppid(-1),pid(-1) {
};

TaskSet::TaskSet(PID _pid) {
  pid = _pid; ppid=_pid;
};

TaskSet::TaskSet(VID _vid, PID _pid) : pid(_pid), ppid(_pid) {
  taskset.push_back(_vid);
};

TaskSet::TaskSet(vector<VID>& _vidlist, PID _pid) : pid(_pid), ppid(_pid) {
  taskset = _vidlist;
};

TaskSet::~TaskSet() {
};
  

//===================================================================
//  Other Methods for class TaskSet
//===================================================================
//Adding and Deleting Tasks
//   void AddTask(VID);
//   void AddTask(vector<VID>&);
//   void DeleteTask(VID);
//   void DeleteTask(vector<VID>);
//   void EraseTaskSet();
//===================================================================

void TaskSet::AddTask(VID _vid) {
  taskset.push_back(_vid);
};
  
void TaskSet::AddTask(vector<VID>& _vidl) {
  int i;
  for(i=0; i< _vidl.size(); i++) {
    taskset.push_back(_vidl[i]);
  }      
};

void TaskSet::DeleteTask(VID _vid) {
  IVID vi;
  for(vi = taskset.begin(); vi < taskset.end(); vi++ ) {
    if(_vid == *vi) {
      taskset.erase(vi);
      break;
    }  
  }
};

void TaskSet::DeleteTask(vector<VID> _vidl) {
  int i;
  IVID vi;
  for(i=0; i< _vidl.size(); i++) {
    for(vi = taskset.begin(); vi < taskset.end(); vi++ ) {
      if(_vidl[i] == *vi) {
        taskset.erase(vi);
        break;
      }
    }
  }
};

void TaskSet::EraseTaskSet() {
  taskset.erase(taskset.begin(),taskset.end());
  pid = 0;
};

//===================================================================
//Checking a Task in a TaskSet or not
//   bool IsTask(VID,IVID&);
//===================================================================
        
bool TaskSet::IsTask(VID _vid, IVID& vi) {
  bool found=false;
  if(taskset.empty()) return false;
  IVID pvi = taskset.begin();
  while (pvi != taskset.end() && !found) {
    if ( *pvi == _vid ) {
      found = true;
      vi = pvi;
      return found;
    } else {
      pvi++;
    }
  }
  return found;
};

int TaskSet::GetSize()
{
  return taskset.size();
}

bool TaskSet::Empty()
{
  return taskset.empty();
}

//===================================================================
//Output taskset information
//   void DisplayTaskSet();
//   void WriteTaskSet(ostream&);
//===================================================================

void 
TaskSet::
DisplayTaskSet() 
{
  cout << "pid =" << "   " << pid << " vertices={";
  for (CIVID pvi = taskset.begin(); pvi < taskset.end(); pvi++) {
    cout << " " << *pvi << " ";
    if (pvi != taskset.end() - 1) cout << " ";
  }
  cout << "} \n";
}
   
void 
TaskSet::
WriteTaskSet(ostream& _myostream) 
{
  _myostream << taskset.size() <<" ";
  for (CIVID pvi = taskset.begin(); pvi < taskset.end(); pvi++) {
    _myostream << *pvi << " ";
  }
  _myostream << endl;
}


//================================================================$
//  Methods for class Schedule
//================================================================$

//================================================================$
//  Constructors and Destructor
//================================================================$
//      Schedule();
//      Schedule(int);         // Reserve space for tasksetvector
//      ~Schedule();
//================================================================$

// Default constructor
Schedule::
Schedule() : numTaskSets(0),lastTaskSetID(-1),numTasks(0)
{}

// Constructor that allocates the request amount of tasksets
Schedule::
Schedule(int _size) 
{
  // !!!!!!!!!!!!!!!!  TaskSet_Map.reserve(_size);
/* for (int i = 0; i < _size; i++) */
/*     AddTaskSet(); */
}

Schedule::
Schedule(int _size, PID _pids) 
{
  // !!!!!!!!!!!  TaskSet_Map.resize(_size);
  lastTaskSetID = _pids-1;  //!!!check

}
Schedule::
Schedule(map<PID,vector<VID> >& dmap) : numTaskSets(0), 
					lastTaskSetID(-1),numTasks(0) {
  map<PID,vector<VID> >::iterator it=dmap.begin();
  for (;it!=dmap.end();++it) {
    AddTaskSet(it->second);
  }
}
 
// Default destructor
Schedule::
~Schedule() 
{
  TaskSet_Map.clear();
  TaskSetPID_Map.clear();
  numTaskSets = 0;
  lastTaskSetID = -1;
  numTasks = 0;
}
  
void Schedule::
SetSchedule(map<PID,vector<VID> >& dmap) {
  map<PID,vector<VID> >::iterator it=dmap.begin();
  for (;it!=dmap.end();++it) {
    this->AddTaskSet(it->second);
  }
}              
//===================================================================
// Adding and Deleting TaskSets
//      PID AddTaskSet();
//      PID AddTaskSet(VID);
//      PID AddTaskSet(vector<VID>&);
//      PID AddTask(PID, VID);
//      PID AddTask(PID, vector<VID>&);
//      void DeleteTask(VID);
//      void DeleteTask(vector<VID>);
//      void DeleteTaskSet(PID);
//      void EraseTaskSetVector();
//===================================================================
void 
Schedule::
AddSchedule(Schedule& s) {
  for(int i=0; i<s.TaskSet_Map.size(); ++i) {
    AddTaskSet((s.TaskSet_Map)[i].taskset);
  }
}

// Add a new task set using the next available PID
PID 
Schedule::
AddTaskSet() 
{
  // Insert new taskset
  PID _pid = ++lastTaskSetID;
  TaskSet_Map[_pid] = TaskSet(_pid);

  numTaskSets++;
  return _pid;
}

// Add a new task set with a given PID
PID 
Schedule::
AddTaskSetwithPID(PID _pid) 
{
  // Is there a taskset with requested pid?
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc == TaskSet_Map.end());

  // Insert new taskset
  TaskSet_Map[_pid] = TaskSet(_pid);

  lastTaskSetID = max(lastTaskSetID, _pid);
  numTaskSets++;
  return _pid;
}

// Add a new task set with 1 task
PID 
Schedule::
AddTaskSet(VID _vid) 
{
  // See if task is already assigned to an existing taskset
  V_P_MAPITER ploc = TaskSetPID_Map.find(_vid);
  assert(ploc == TaskSetPID_Map.end());

  // Insert new taskset
  PID _pid = ++lastTaskSetID;
  TaskSet_Map[_pid] = TaskSet(_vid, _pid);

  // Assign taskset PID to task
  TaskSetPID_Map[_vid] = _pid;

  numTaskSets++;
  numTasks++;
  return _pid;
}

// Add a new task set with a vector of tasks
PID 
Schedule::
AddTaskSet(vector<VID> & _vidlist) 
{
  // Check if all tasks are unassigned
  V_P_MAPITER ploc;
  vector<VID>::iterator vl_iter, vl_end;
  for (vl_iter = _vidlist.begin(), vl_end = _vidlist.end(); 
       vl_iter != vl_end; vl_iter++) {
    ploc = TaskSetPID_Map.find(*vl_iter);
    assert(ploc == TaskSetPID_Map.end());
  }

  // Insert new taskset
  PID _pid = ++lastTaskSetID;
  TaskSet_Map[_pid] = TaskSet(_vidlist, _pid);

  // Assign taskset PID to tasks
  for (vl_iter = _vidlist.begin(), vl_end = _vidlist.end(); 
       vl_iter != vl_end; vl_iter++) {
    TaskSetPID_Map[*vl_iter] = _pid;
  }

  numTaskSets++;
  numTasks += _vidlist.size();
  return _pid;
}

// Add a task to an existing taskset
PID
Schedule::
AddTask(PID _pid, VID _vid) 
{
  // Get taskset with requested pid
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());

  // See if task is already assigned to an existing taskset
  V_P_MAPITER ploc = TaskSetPID_Map.find(_vid);
  assert(ploc == TaskSetPID_Map.end());

  // Add task to taskset
  // TaskSet_Map[_pid].AddTask(_vid);
  ts_loc->second.AddTask(_vid);

  // Assign taskset PID to task
  TaskSetPID_Map[_vid] = _pid;

  numTasks++;
  return _pid;
}

// Add a vector of tasks to an existing taskset        
PID
Schedule::
AddTask(PID _pid, vector<VID> & _vidlist) 
{
  // Check if all tasks are unassigned
  V_P_MAPITER ploc;
  vector<VID>::iterator vl_iter, vl_end;
  for (vl_iter = _vidlist.begin(), vl_end = _vidlist.end(); 
       vl_iter != vl_end; vl_iter++) {
    ploc = TaskSetPID_Map.find(*vl_iter);
    assert(ploc == TaskSetPID_Map.end());
  }

  // Get taskset with requested pid
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());

  // Add tasks to taskset
  // TaskSet_Map[_pid].AddTask(_vidlist);
  ts_loc->second.AddTask(_vidlist);

  // Assign taskset PID to each task
  for (vl_iter = _vidlist.begin(), vl_end = _vidlist.end(); 
       vl_iter != vl_end; vl_iter++) {
    TaskSetPID_Map[*vl_iter] = _pid;
  }

  numTasks += _vidlist.size();
  return _pid;
}

// Delete task
void
Schedule::
DeleteTask(VID _vid) 
{
  // Get taskset pid that has _vid
  V_P_MAPITER ploc = TaskSetPID_Map.find(_vid);
  assert(ploc != TaskSetPID_Map.end());
  // PID _pid = TaskSetPID_Map[_vid];
  PID _pid = ploc->second;

  // Get taskset from pid
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());

  // Delete task
  // TaskSet_Map[_pid].DeleteTask(_vid);
  ts_loc->second.DeleteTask(_vid);
  // TaskSetPID_Map.erase(_vid);
  TaskSetPID_Map.erase(ploc);
  // if (TaskSet_Map[_pid].Empty())
  if (ts_loc->second.Empty())
    TaskSet_Map.erase(ts_loc);
  numTasks--;
}

// Delete a vector of tasks
void
Schedule::
DeleteTask(vector<VID> & _vidlist) 
{
  P_TS_MAPITER ts_loc;
  V_P_MAPITER ploc;
  vector<VID>::iterator vl_iter, vl_end;
  PID _pid;

  for (vl_iter = _vidlist.begin(), vl_end = _vidlist.end(); 
       vl_iter != vl_end; vl_iter++) {
    // get taskset containing vid
    ploc = TaskSetPID_Map.find(*vl_iter);
    assert(ploc != TaskSetPID_Map.end());
    // _pid = TaskSetPID_Map[*vl_iter];
    _pid = ploc->first;

    ts_loc = TaskSet_Map.find(_pid);
    assert(ts_loc != TaskSet_Map.end());
    
    // Delete task
    // TaskSet_Map[_pid].DeleteTask(*vl_iter);
    ts_loc->second.DeleteTask(*vl_iter);
    TaskSetPID_Map.erase(*vl_iter);
    // if (TaskSet_Map[_pid].Empty())
    if (ts_loc->second.Empty())
      TaskSet_Map.erase(ts_loc);
  }
  numTasks -= _vidlist.size();
}

// Delete a taskset by pid
void
Schedule::
DeleteTaskSet(PID _pid) 
{
  // Get taskset from pid
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());
  
  // iterate over taskset and remove from tasksetPID map
  // TaskSet_Map[_pid].EraseTaskSet();
  ts_loc->second.EraseTaskSet();
  TaskSet_Map.erase(ts_loc);
}

// Clear schedule  
void    
Schedule::
Clear() 
{
  TaskSet_Map.clear();
  TaskSetPID_Map.clear();
  numTaskSets = 0;
  lastTaskSetID = -1;
  numTasks = 0;
}

//===================================================================
// Checking  TaskSet
//      bool IsTaskSet(PID, TaskSet**);
//	bool IsSameProcessor(VID&,VID&);
//===================================================================

// Check if there is a taskset with pid
bool
Schedule::
IsTaskSet(PID _pid) 
{
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  if (ts_loc != TaskSet_Map.end()) {
    return true;
  }
  return false;
}

// Get the taskset pid from vid
// return INVALID_PID if task is not assigned in schedule
PID
Schedule::
GetTaskSetPID(VID _vid)
{
  // Get taskset pid owning pid
  V_P_MAPITER ploc = TaskSetPID_Map.find(_vid);
  // assert(ploc != TaskSetPID_Map.end());
  if (ploc == TaskSetPID_Map.end()) {
    return INVALID_PID;
  } else {
    // return TaskSetPID_Map[_vid];
    return ploc->second;
  }
}

// Get taskset with pid
TaskSet
Schedule::
GetTaskSetbyPID(PID _pid)
{
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());
  return ts_loc->second;
}

// Get taskset with vid
TaskSet
Schedule::
GetTaskSetbyVID(VID _vid)
{
  V_P_MAPITER ploc = TaskSetPID_Map.find(_vid);
  assert(ploc != TaskSetPID_Map.end());
  PID _pid = ploc->second;
  P_TS_MAPITER ts_loc = TaskSet_Map.find(_pid);
  assert(ts_loc != TaskSet_Map.end());
  return ts_loc->second;
}

// Get number of task sets
int
Schedule::
GetNumTaskSets()
{
  return numTaskSets;
}

// Get number of task sets
int
Schedule::
GetNumTasks()
{
  return numTasks;
}

// See if two vids are on the same processor
bool
Schedule::
IsSameProcessor(VID & _v1id, VID & _v2id) 
{
  V_P_MAPITER ploc1 = TaskSetPID_Map.find(_v1id);
  V_P_MAPITER ploc2 = TaskSetPID_Map.find(_v2id);
  return (ploc1 == ploc2);
}

// Verify schedule
bool
Schedule::
CheckSchedule(const vector<VID> &vidlist, vector<VID> &vidnot, 
              vector<VID> &vidmultiple)
{
  int nvertices = vidlist.size();
  vector<int> mark;
  mark.reserve(nvertices);
  mark.assign(nvertices, 0);

  // Count # of occurrences of each task
  P_TS_MAPITER ts_iter = TaskSet_Map.begin();
  P_TS_MAPITER ts_end = TaskSet_Map.end();
  CIVID t_iter, t_end;
  for (; ts_iter != ts_end; ts_iter++) {
    t_iter = ts_iter->second.taskset.begin();
    t_end = ts_iter->second.taskset.end();
    for (; t_iter != t_end; t_iter++) {
      mark[*t_iter]++;
    }
  }

  t_iter = vidlist.begin();
  t_end = vidlist.end();
  VID vid;
  for (; t_iter != t_end; t_iter++) {
    vid = *t_iter;
    if (mark[vid] == 0) {
      vidnot.push_back(vid);
    } else if (mark[vid] > 1) {
      vidmultiple.push_back(vid);
    }
  }

  //count tasksets
  int emptycnt=0;
  P_TS_MAPITER ts_iter1 = TaskSet_Map.begin();
  P_TS_MAPITER ts_end1 = TaskSet_Map.end();
  for (; ts_iter1 != ts_end1; ts_iter1++) {
    if(ts_iter1->second.taskset.empty()) {
      TaskSet_Map.erase(ts_iter1);
      emptycnt++;
    }
  }

  if (vidnot.size() == 0 && vidmultiple.size() == 0) {
    cout << "Schedule is correct." << endl;
    if(emptycnt != 0) {
    cout<< "Errased "<<emptycnt<<" empty tasksets."<<endl;
    }

    return true;
  }
  if (vidnot.size() != 0 ) {
    cout << "ERROR, there are tasks not scheduled." << endl;
  } 
  if (vidmultiple.size() != 0) {
    cout << "ERROR, there are tasks scheduled more than once." << endl;
  }
  return false;
}

double Schedule::
GetLoadBalancingFactor() {
    double mean =0., var=0., std=0.;
    double sum=0.;
    map<PID, TaskSet>& stlevel = TaskSet_Map;
    for(int i=0; i<stlevel.size(); i++) {
      sum += stlevel[i].taskset.size();
    }
    mean = sum/(double)(stlevel.size());
    for(int i=0; i<stlevel.size(); i++) {
      var += (stlevel[i].taskset.size()-mean)*
	(stlevel[i].taskset.size()-mean);
    }
    var /= (double)(stlevel.size()-1);
    std = sqrt(var);
    return std/mean;
}

// Set schedule name by string type

void
Schedule::
SetName(string _s)
{
  AlgName = _s;
}

string
Schedule::
GetName()
{
  return AlgName;
}


//===================================================================
//Output taskset information
//	void ReadSchedule(char*);
//	void ReadSchedule(istream&);
//      void DisplaySchedule();   
//	void WriteSchedule(char*) const;
//      void WriteSchedule(ostream&) const;
//===================================================================

// Read in schedule from a file by filename
void
Schedule::
ReadSchedule(const char* _fname) 
{
  ifstream  myifstream(_fname);
  if (!myifstream) {
    cout << "\nIn ReadSchedule: can't open infile: " << _fname ;
    return;
  }
  ReadSchedule(myifstream);
  myifstream.close();
}

// Read in schedule from a file by input stream
void
Schedule::
ReadSchedule(istream& myifstream) 
{
  vector<VID> _t;
  VID tmp;
  int npid, ntask;

  myifstream >> npid;
  for (int i = 0; i < npid; i++) {
    myifstream >> ntask;
    for (int j = 0; j < ntask; j++) {
      myifstream >> tmp;
      _t.push_back(tmp);
    }
    AddTaskSet(_t);
    _t.clear();
  } 
}

// Write schedule to stdout
void
Schedule::
DisplaySchedule() 
{
  cout<< "##### Schedule by " << AlgName << " ####"<< endl;

  cout<< "Number of Processors used  " << TaskSet_Map.size() <<endl;

  P_TS_MAPITER ts_iter = TaskSet_Map.begin();
  P_TS_MAPITER ts_end = TaskSet_Map.end();
  for (; ts_iter != ts_end; ts_iter++) {
    ts_iter->second.DisplayTaskSet();
  }
}
 
// Write schedule to a file
void
Schedule::
WriteSchedule(const char* _fname) 
{
  ofstream  myofstream(_fname);
  if (!myofstream) {
    cout << "\nInWriteSchedule: can't open outfile: " << _fname;
  }
  WriteSchedule(myofstream);
  myofstream.close();
}
        
// Write schedule to a file
void
Schedule::
WriteSchedule(ostream& _myostream) 
{
  _myostream << TaskSet_Map.size() << endl;;

  P_TS_MAPITER ts_iter = TaskSet_Map.begin();
  P_TS_MAPITER ts_end = TaskSet_Map.end();
  for (; ts_iter != ts_end; ts_iter++) {
    ts_iter->second.WriteTaskSet(_myostream);
  }
}

void
Schedule::
ScheduleTransform(map<VID,VID>& vidmap) {
  map<VID,VID> revmap;
  map<VID,VID>::iterator mi = vidmap.begin();
  for(int i=0; i<vidmap.size(); ++i,++mi) 
    revmap[mi->second]=mi->first;

  for(int i=0; i<TaskSet_Map.size(); ++i) {
     for(int j=0; j<(TaskSet_Map[i].taskset).size(); ++j) {
       VID nvid = (TaskSet_Map[i].taskset)[j];
       (TaskSet_Map[i].taskset)[j] = revmap[nvid];
     }
  }
}

}  //end namespace


