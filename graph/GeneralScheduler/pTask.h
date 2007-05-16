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

#ifndef pTask_h
#define pTask_h

#include <runtime.h>
#include "SchedulerDefines.h"

namespace scheduler {
//===================================================================
//===================================================================
template<class TASKWEIGHT>
class pTask {
 public:
  //===================================================================
  //  Data
  //===================================================================
  TASKWEIGHT taskwt;  //!message size
  TASKWEIGHT finishtime;
  TASKWEIGHT EMT; //effective message arrival time (from Ps other than enable P)
  TASKWEIGHT LMT; //Last message arrival time

  double blevel;
  double alaplevel;  //!as late as possible level, equals {max_blevel - bleveli}
  //for parallel FCP:
  PID epid;  //!enabling P, from which the latest message could be from.
  PID ipid;  //!ealiest idle processor
  PID spid;  //!scheduled pid, can only be either epid or ipid!!!

   int indegree;  //indegree count to get ready tasks
  bool scheduled; 

  vector<pair<PID,TASKWEIGHT> > predinfos;


  void define_type(stapl::typer &t)
    {
      t.local(taskwt);
      t.local(finishtime);
      t.local(EMT);
      t.local(LMT);
      t.local(blevel);
      t.local(alaplevel);
      t.local(epid);
      t.local(ipid);
      t.local(spid);
      t.local(indegree);
      t.local(scheduled);
      t.local(predinfos);
    }
 
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
 public:
  pTask():taskwt(0),finishtime(0),EMT(0),LMT(0),blevel(0),
    alaplevel(0),epid(-1),ipid(0),spid(0),indegree(0),
    scheduled(false)
    { 
    }
  pTask(TASKWEIGHT tw):taskwt(tw),finishtime(0),EMT(0),LMT(0),blevel(0),
    alaplevel(0),epid(-1),ipid(0),spid(0),indegree(0),
    scheduled(false)
    { 
    }
  pTask(int invalid) {}  //required by BaseGraph VERTEX(-1);

  pTask(const pTask<TASKWEIGHT>& task) {
    taskwt = task.taskwt;
    finishtime = task.finishtime;
    EMT = task.EMT;
    LMT = task.LMT;
    blevel=task.blevel;
    alaplevel=task.alaplevel;
    epid = task.epid;
    ipid = task.ipid;
    spid = task.spid;
    indegree = task.indegree;
    scheduled = task.scheduled;
    predinfos = task.predinfos;
  }  

  ~pTask() {}

  //===================================================================
  //  Other Methods
  //===================================================================
 public:
  const TASKWEIGHT& GetTaskWeight() const  {return taskwt;}; 
  const TASKWEIGHT& GetFinishT() const {return finishtime;}; 

  //!Modifying data
  void SetTaskWeight(const TASKWEIGHT _wt) {taskwt = _wt;};
  void SetFinishT(const TASKWEIGHT _ft) {finishtime = _ft;};


  //===================================================================
  // Operators
  //===================================================================

  inline bool operator== (const pTask &task) const {
    return ( taskwt == task.taskwt );
  }

  inline pTask& operator= (const pTask &task) {
    taskwt = task.taskwt;
    finishtime = task.finishtime;
    EMT = task.EMT;
    LMT = task.LMT;
    blevel=task.blevel;
    alaplevel=task.alaplevel;
    epid = task.epid;
    ipid = task.ipid;
    spid = task.spid;
    indegree = task.indegree;
    scheduled = task.scheduled;
    predinfos = task.predinfos;
    return *this;
  }

};

} //end namespace

typedef scheduler::pTask<double> GpTask;

 inline istream& operator>> (istream& s, GpTask &task) {
   char ss[10];
   s>>ss;
   double wt=atof(ss);
/*    s >> wt; */
   task.SetTaskWeight(wt);
   return s;
 };

 inline  ostream& operator<< (ostream& s, const GpTask &task) {
   return s<<task.GetTaskWeight();
     };
/*  inline  ostream& operator<< (ostream& s, const GpTask &task) { */
/*    return s <<" twt:"<<task.GetTaskWeight() */
/* 	    <<" ftime:"<<task.GetFinishT() */
/* 	    <<" EMT:"<<task.EMT */
/* 	    <<" LMT:"<<task.LMT */
/* 	    <<" alaplevel:"<<task.alaplevel */
/* 	    <<" epid:"<<task.epid */
/* 	    <<" ipid:"<<task.ipid */
/* 	    <<" spid:"<<task.spid<<endl */
/* 	    <<" indegree:"<<task.indegree; */
/*  }; */


#endif
