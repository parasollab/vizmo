/*! 
  \defgroup scheduler
*/

/*!
  \file Task.h
  \author Ping An
  \date Dec. 14, 00
  \ingroup Scheduler
  \brief a general vertex in a DDG (Data Dependence Graph) for Scheduler  

  Stores computation cost and other infos 

*/

#ifndef Task_h
#define Task_h

#ifdef PARALLEL_SCHEDULER
#include <runtime.h>
#endif
#include "SchedulerDefines.h"

namespace scheduler {

//===================================================================
//===================================================================
template<class TASKWEIGHT,class TASKINFO>
class Task {

public:
  //===================================================================
  //  Data
  //===================================================================
  int taskid;
  TASKWEIGHT taskwt;  //!message size
  TASKWEIGHT compcost;
  TASKWEIGHT finishtime;
  TASKWEIGHT starttime;
  int ccid;  //!cluster id in partitioning phase, 
             // corresponding to pid in schedule

  PID mpid;  //!mapping processor id
             // corresponding to ppid in schedule

  int tpsnum; //!topological sort number
  int level;
  int indegree;

public:
  vector<PID> cids;   
  TASKINFO taskinfo;
#ifdef PARALLEL_SCHEDULER
  void define_type(stapl::typer &t)
    {
      t.local(taskid);
      t.local(taskwt);
      t.local(compcost);
      t.local(finishtime);
      t.local(starttime);
      t.local(ccid);
      t.local(mpid);
      t.local(tpsnum);
      t.local(level);
      t.local(cids);
      t.local(taskinfo);
      t.local(indegree);
   }
#endif
 
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
 public:
  Task():taskid(0),taskwt(0),ccid(0),mpid(0),starttime(0),
    finishtime(0),tpsnum(0),level(0)
    { 
      cids.resize(MAX_MACHINE_LEVEL);
      for(int i=0;i<MAX_MACHINE_LEVEL;i++) 
	cids[i] = INVALID_PID;
    }
 
  Task(int _taskid) : taskid(_taskid)  
    {      cids.resize(MAX_MACHINE_LEVEL);
    }  

  Task(int _ccid, PID _mpid):
    ccid(_ccid),
    mpid(_mpid)
    {      cids.resize(MAX_MACHINE_LEVEL);
    } 

  Task(int _vid, TASKWEIGHT _vwt, TASKINFO _tinfo):
    taskid(_vid),
    taskwt( _vwt),
    taskinfo(_tinfo),level(0)
    {      cids.resize(MAX_MACHINE_LEVEL);
    } 

  Task(const Task<TASKWEIGHT, TASKINFO>& _task) {
    taskid = _task.taskid;
    taskwt = _task.taskwt;
    compcost = _task.compcost;
    ccid = _task.ccid;
    mpid = _task.mpid;
    taskinfo = _task.taskinfo;
    starttime = _task.starttime;
    finishtime = _task.finishtime;
    tpsnum = _task.tpsnum;
    level = _task.level;
    indegree = _task.indegree;
    cids.resize(MAX_MACHINE_LEVEL);
    for(int i=0;i<MAX_MACHINE_LEVEL;i++) 
      cids[i] = _task.cids[i];
  }  

  ~Task() {}

  //===================================================================
  //  Other Methods
  //===================================================================
 public:
  //!Getting Data 
  int GetTaskID() const {return taskid;};
  const TASKWEIGHT& GetTaskWeight() const  {return taskwt;}; 
  const TASKWEIGHT& GetCompCost() const {return compcost;}; 
  const TASKWEIGHT& GetStartT() const {return starttime;}; 
  const TASKWEIGHT& GetFinishT() const {return finishtime;}; 
  int GetClusterId() const {return ccid; };
  int GetProcId() const {return mpid; };
  int GetTpsNum() const {return tpsnum;};
  const TASKINFO& GetTaskInfo() const {return taskinfo;};
  TASKINFO GetTempTaskInfo() {return taskinfo;};
  int GetLevel() const {return level;};
  int GetCid(int _plevel) const {
    return cids[_plevel];
  }

  //!Modifying data
  void SetTaskID(const int _vid) {taskid = _vid;};
  void SetTaskWeight(const TASKWEIGHT _wt) {taskwt = _wt;};
  void SetCompCost(const TASKWEIGHT _wt) { compcost=_wt;}; 
  void SetStartT(const TASKWEIGHT _st) {starttime = _st;};
  void SetFinishT(const TASKWEIGHT _ft) {finishtime = _ft;};
  void SetClusterId(const int _cid) { ccid = _cid; };
  void SetProcId(const PID _pid) {mpid = _pid;};
  void SetTpsNum(const int _lev) {tpsnum = _lev;};
  void SetTaskInfo(const TASKINFO _tinfo ) {taskinfo = _tinfo;};
  void SetLevel(const int _lev) {level = _lev;};

  //for hierarchical scheduling
  void SetCids(const PID* _cids) { 
    for(int i=0; i<MAX_MACHINE_LEVEL; i++) cids[i] = _cids[i]; 
  }

  void SetCid(int _plevel, PID _cid) {
    cids[_plevel]=_cid;
  }

  //===================================================================
  // Operators
  //===================================================================

  inline bool operator== (const Task &task) const {
    return ( taskwt == task.taskwt &&
	     taskid == task.taskid );
  }

  inline Task& operator= (const Task &task) {
    taskid = task.taskid;
    taskwt = task.taskwt;
    compcost = task.compcost;
    ccid = task.ccid;
    mpid = task.mpid;
    starttime = task.starttime; 
    finishtime = task.finishtime;
    taskinfo = task.taskinfo;
    tpsnum = task.tpsnum;
    level = task.level;
    indegree = task.indegree;
    for(int i=0;i<MAX_MACHINE_LEVEL;i++) 
      cids[i] = task.cids[i];
    return *this;
  }

};

} //end namespace

typedef scheduler::Task<double,double> GTask;
typedef scheduler::Task<double,int> ITask;

 inline istream& operator>> (istream& s, GTask &task) {
   char ss[10];
   s >> ss;
   double wt=atof(ss);;
   // s >> wt;
   task.SetTaskWeight(wt);
   return s;
 };

 inline  ostream& operator<< (ostream& s, const GTask &task) {
   return s <<"TID:"<<task.GetTaskID()<<" twt:"<<task.GetTaskWeight()
	    <<" stime:"<<task.GetStartT()<<" ftime:"<<task.GetFinishT()
	    <<" clusterid:"<<task.GetClusterId()<<" procid:"<<task.GetProcId()
	    << "tpsnum:"<<task.GetTpsNum()<<endl;
 };

 inline  ostream& operator<< (ostream& s, const ITask &task) {
   return s <<"TID:"<<task.GetTaskID()<<" twt:"<<task.GetTaskWeight()
	    <<" stime:"<<task.GetStartT()<<" ftime:"<<task.GetFinishT()
	    <<" clusterid:"<<task.GetClusterId()<<" procid:"<<task.GetProcId()
	    << "tpsnum:"<<task.GetTpsNum()<<endl;
 };


#endif
