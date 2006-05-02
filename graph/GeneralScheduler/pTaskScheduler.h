/*!
	\file TaskScheduler.h
	\author Ping An	
	\date Dec. 14, 2000
	\brief 
	Input:
	  a dependence graph (with format defined by Graph.h & Task.h);

	Output:
	  Schedule
*/

#ifndef pTaskScheduler_h
#define pTaskScheduler_h

#include "rmitools.h"
#include "pGraph.h"
#include "Comm.h"
#include "pTask.h"
#include "Schedule.h"
#include "SchedulerGraphMethods.h"
#include "TaskSchedAux.h"
#include "../pGraph.h"


#define MASTER_PID 0
#define TASK_WT_RANGE 100     

namespace scheduler {

//===================================================================
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  class pTaskScheduler : public BasePObject
{
 public:
  typedef pTaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> this_type;
  
  typedef pTask<TASKWEIGHT> Ttype;
  typedef Comm<COMMWEIGHT,COMMINFO> Wtype;

  typedef Graph<DG<Ttype,Wtype>, NMG<Ttype,Wtype>, 
    WG<Ttype,Wtype>,Ttype,Wtype > WtDiGraph;

  typedef pGraph<PDG<Ttype,Wtype>, PNMG<Ttype,Wtype>, 
    PWG<Ttype,Wtype>,Ttype,Wtype > pWtDiGraph;

  typedef WtVertexType<Ttype,Wtype> Vert;
  typedef WtEdgeType<Ttype,Wtype> WEdge;

/*   typedef  typename WtDiGraph::VI WI; */

  typedef  typename WtDiGraph::VI VI;
  typedef  typename WtDiGraph::EI EI;

  typedef  typename pWtDiGraph::VI TI;
  typedef  typename pWtDiGraph::CVI CTI;
  typedef  typename pWtDiGraph::EI EGI;
  typedef  typename pWtDiGraph::CEI CEGI;

  typedef  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    SchedulerGraphMethods_type;

  typedef priority_queue<ProcPair, vector< ProcPair >, PP_GT> 
    PP_priority_queue;
  typedef multiset< ProcPair, PP_LT > PP_multiset;

//===================================================================
//  Data
//===================================================================
 public:

  pWtDiGraph dependent_pgraph;
  Schedule newSchedule;
  Schedule complete_schedule;  //only in Master P
  TASKWEIGHT finish; //finish time of the last local task
  int target_np; //target number of processors to schedule for
  PID myid;
  int procscount; //count for thread 0 to know when to exit.
  _StaplTriple<PID,TASKWEIGHT,VID> result;  //the schedule result of the current task 

 public:

  // FCP data structures  (simply parallel ready Q)
  PP_priority_queue task_pq;

  //a copy in each processor, valid and updated only in master procs
  vector<TASKWEIGHT> PRT_list;  // ready time of each proc

  PP_multiset allProc_list;     // list of all procs, s. by -PRT
  vector<bool> color;   //color processors in each loop to check conflict
 public:
//===================================================================
//  Constructors and Destructor
//===================================================================
 pTaskScheduler() : 
   myid(stapl::get_thread_id()), result(-1,0.,-1)
   {    
     procscount=stapl::get_num_threads()-1; //procs other than master
     this->register_this(this);
     rmi_fence(); 
   };

 pTaskScheduler(int _target_np) : 
   myid(stapl::get_thread_id()),
   target_np(_target_np), result(-1,0.,-1)
   {
     procscount=stapl::get_num_threads()-1; //procs other than master
     this->register_this(this);
     rmi_fence(); 
   };

 pTaskScheduler(WtDiGraph& _g, map<PID,vector<VID> >& dmap, int _target_np) : 
   myid(stapl::get_thread_id()),
   target_np(_target_np), result(-1,0.,-1)
   {
     procscount=stapl::get_num_threads()-1; //procs other than master
     this->register_this(this);
     dependent_pgraph.SetPGraphwGraph(_g,dmap);
     rmi_fence(); 
   };

 ~pTaskScheduler() {};
 
 public:

void GraphGen(double ccr, WtDiGraph& g,
	      int m, int _maxcolumn, int _maxedges) {

  vector<VID> tmp(_maxcolumn,0);
  vector<vector<VID> > vids(m,tmp);
  int nv=0; 
  vector<int> n(m,0);
  for(int i=0; i<m; i++) {
    if(_maxcolumn != 1) {
      srand(i);
      while(n[i]==0) n[i]= rand()%_maxcolumn;
/*       cout<<" n["<<i<<"]="<<n[i]<<endl; */
      nv+= n[i];
      for(int j=0; j<n[i]; j++) {
	srand(j);
	double wt = ((double) (rand()%TASK_WT_RANGE))/TASK_WT_RANGE+1;
	pTask<double> pt(wt);
	vids[i][j]=g.AddVertex(pt);
      }
    } else {
      cout<< "Error: _maxcolumn must be > 1" << endl;
      exit(1);
    }
  }

  int ne=0;
  vector<int> e(m,0);
  for(int i=0; i<m-1; i++) {
    if(_maxedges != 1) {
      srand((int) clock());
      while(e[i]==0) e[i]= rand()%_maxedges;
/*       cout<<" e["<<i<<"]="<<e[i]<<endl; */
      ne+= e[i];
      for(int j=0; j<e[i]; j++) {
	srand(j*2);
	double ewt = ccr*(((double) (rand()%TASK_WT_RANGE)/TASK_WT_RANGE)+1);
	int j1 = rand()%n[i];
	int m1 = rand()%(m-1-i);
	int j2 = rand()%n[m1+1+i];	
	VID v1id = vids[i][j1];
	VID v2id = vids[m1+1+i][j2];
/* 	cout<<" v1id "<<v1id<<" v2id "<<v2id<<endl; */
	bool suc = g.AddEdge(v1id,v2id,ewt);
	if(!suc) j--;
      }
    } else {
      cout<< "Error: _maxedges must be > 1" << endl;
      exit(1);
    }
  }
  cout<<"nv "<<nv<<" ne "<<ne<<endl;
  g.WriteGraph("gout");
}

 TASKWEIGHT GetScheduleLength() {
   SetLocalLastFinish();
   TASKWEIGHT slength;
   stapl::reduce_rmi(&finish,&slength, getHandle(),&this_type::max, true);
   return slength;
 }

 void SetLocalLastFinish() {
   TASKWEIGHT LAST=-1.0;
   VID lvid;
   for(TI vi = dependent_pgraph.begin(); 
       vi != dependent_pgraph.end(); ++vi) {
     if(vi->data.finishtime > LAST) {
       lvid = vi->vid;
       LAST = vi->data.finishtime;
     }
   }
/*    cout<<"last vid: "<<lvid<<" local scheduling length: "<<LAST<<endl;  */
   finish = LAST;
 }

 void max(TASKWEIGHT* in, TASKWEIGHT* inout) {
   if(*inout < *in) *inout = *in;
 }

 void SetPidandFT(const _StaplTriple<PID,TASKWEIGHT,VID>& _r) {
   result = _r;
 }

  pair<bool,double> GetColorAndRTandSetColor(PID _pid) {
    if(myid == MASTER_PID) {
      pair<bool,double> pp(color[_pid],PRT_list[_pid]);
      if(!color[_pid]) color[_pid]=true;
      return pp;
    }
    else 
      return sync_rmi(MASTER_PID,getHandle(),&this_type::GetColorAndRTandSetColor,_pid);
  }

 void GetEarliestPidandFT(const _StaplSix<PID,TASKWEIGHT,TASKWEIGHT,
			  TASKWEIGHT,PID,VID> & qd) {
    if(myid == MASTER_PID) {
      int i;
      PID earlypid=target_np;
      TASKWEIGHT tmp=10000000.;
      TASKWEIGHT ft;
      for(i=0; i<target_np; ++i) {
	if(tmp > PRT_list[i]) {
	  tmp=PRT_list[i];
	  earlypid = i;	  
	}
      }
      if(qd.first == -1 || (qd.first != -1 && 
			    PRT_list[earlypid] < PRT_list[qd.first]) ) {
	ft=(qd.second > PRT_list[earlypid] ? qd.second : PRT_list[earlypid])
	  + qd.fourth;
	PRT_list[earlypid]=ft;
	async_rmi(qd.fifth,getHandle(),&this_type::SetPidandFT,
		  _StaplTriple<PID,TASKWEIGHT,VID> (earlypid,ft,qd.sixth) );
      } else {
	ft=( qd.third > PRT_list[qd.first] ? qd.third : PRT_list[qd.first])
	  + qd.fourth;
	PRT_list[qd.first]=ft;
	async_rmi(qd.fifth,getHandle(),&this_type::SetPidandFT,
		  _StaplTriple<PID,TASKWEIGHT,VID> (qd.first,ft,qd.sixth) );
      }
    }
    else async_rmi(MASTER_PID,getHandle(),
		  &this_type::GetEarliestPidandFT,qd);
  }

  pair<PID,TASKWEIGHT> GetEarliestUncoloredandColorIt() {
    if(myid == MASTER_PID) {
      int i;
      PID earlypid=0;
      TASKWEIGHT tmp=10000000.;
      for(i=0; i<target_np; ++i) {
	if(!color[i]) {
	  if(tmp > PRT_list[i]) {
	  tmp=PRT_list[i];
	  earlypid = i;	  
	  }
	}
      }
      color[earlypid]=true;
      if(earlypid!=target_np) 
	return pair<PID,TASKWEIGHT> (earlypid,PRT_list[earlypid]);
      else {
	cout<<"All procs used when trying to get the earliest available PID."<<endl;
	return pair<PID,TASKWEIGHT> (-1,0.0);
      }
    }
    else return sync_rmi(MASTER_PID,getHandle(),
		  &this_type::GetEarliestUncoloredandColorIt);
  }

  void UpdateProcRTandUnSetColor(TASKWEIGHT _ft,PID _pid) {
    if(myid == MASTER_PID) {
      PRT_list[_pid]= _ft;
      color[_pid] = 0;
    }
    else 
      async_rmi(MASTER_PID,getHandle(),&this_type::UpdateProcRTandUnSetColor,_ft,_pid);
  }

  void UnSetColor(PID _pid) {
    if(myid == MASTER_PID) {
      color[_pid]= false;
    }
   else async_rmi(MASTER_PID,getHandle(),&this_type::UnSetColor,_pid);
  }

  void UpdateInfoInSucc(pair<PID,COMMWEIGHT> _pp, VID t2id) {
    TI vi;
    if(dependent_pgraph.IsLocal(t2id)) {
      dependent_pgraph.IsVertex(t2id, &vi);
      vi->data.indegree--;
      vi->data.predinfos.push_back(_pp);
    } else {
      Location loc = dependent_pgraph.Lookup(t2id);
      async_rmi(loc.locpid(),getHandle(),
		&this_type::UpdateInfoInSucc,_pp,t2id);
    }
  }

  ProcPair PriorityListremove(PID pid)
  {
    if(myid == MASTER_PID) {
      typename PP_multiset::iterator pp_loc = allProc_list.begin();
      while (pp_loc != allProc_list.end()) {
	if (pp_loc->id == pid) break;
	pp_loc++;
      }
      if (pp_loc != allProc_list.end()) {
	ProcPair pp=*pp_loc;
	allProc_list.erase(pp_loc);
	return pp;
      } else return ProcPair(0.0,-1);  //already removed.
    } else 
      return sync_rmi(MASTER_PID,getHandle(),
		      &this_type::PriorityListremove,pid);
  }
 
  ProcPair PopFirstProcIDandRT() {
    if(myid == MASTER_PID) {
      ProcPair pp=*(allProc_list.begin());
      allProc_list.erase(allProc_list.begin());
      return pp;
    } else
      return sync_rmi(MASTER_PID,getHandle(),&this_type::PopFirstProcIDandRT);
  }

  void PriorityListInsert(const ProcPair& pp) {
    if(myid == MASTER_PID)  allProc_list.insert(pp);
    else async_rmi(MASTER_PID,getHandle(),&this_type::PriorityListInsert,pp);
 }

  void CalculateEPandEMT(int tid)
  {
    COMMWEIGHT maxLMT = 0.0;  //last message arrived
    PID LMTpid=-1;  //processor from which the last message arrivals
    TI vi;
    dependent_pgraph.IsVertex(tid,&vi);
    typename vector<pair<PID,COMMWEIGHT> >::iterator pi;
    for (pi = vi->data.predinfos.begin(); 
	 pi!=vi->data.predinfos.end(); pi++) {
      COMMWEIGHT _ar = pi->second;
      if (maxLMT < _ar) {
	maxLMT = _ar;
	LMTpid = pi->first;
      }
    }
    //get EMT considering the last proc as the enabling P w/o comm cost
    COMMWEIGHT maxEMT = 0.0; //effective message arrival time
    for (pi = vi->data.predinfos.begin(); 
	 pi!=vi->data.predinfos.end(); pi++) {
      if(pi->first != LMTpid) {
	COMMWEIGHT _ar = pi->second;
	if (maxEMT < _ar) {
	  maxEMT = _ar;
	}
      }
    }
    vi->data.epid = LMTpid;
    vi->data.EMT = maxEMT;
    vi->data.LMT=maxLMT;
  }

  void MergeSchedule(Schedule& s) {
    for(GII gi = s.TaskSet_Map.begin(); 
	gi!=s.TaskSet_Map.end(); ++gi) 
      MergeTaskSet(gi->first, gi->second.taskset);
    stapl::rmi_fence();
  } 

  void MergeTaskSet(PID _pid, vector<VID>& ts) {
    if(myid == MASTER_PID) {
      complete_schedule.AddTasks(_pid,ts);
    } else 
      async_rmi(MASTER_PID,getHandle(),&this_type::MergeTaskSet,_pid,ts);
  } 

  void DecrementProcsCount() {
    if(myid == MASTER_PID) procscount--;
    else async_rmi(MASTER_PID,getHandle(),&this_type::DecrementProcsCount);
  }

 void AddReadyTasks() {
/*   for (TI vi = dependent_pgraph.begin();  */
/*        vi != dependent_pgraph.end(); ++vi) { */
  for (VI vi = (*dependent_pgraph.GetPart(0)).begin(); 
       vi != (*dependent_pgraph.GetPart(0)).end(); ++vi) {
    if(vi->data.indegree == 0 && vi->data.scheduled == false) {
       CalculateEPandEMT(vi->vid);
      task_pq.push(ProcPair (vi->data.alaplevel, vi->vid));
      vi->data.scheduled=true;
    }
  }
 }

    void FCP_pSetup()
      {  

#ifdef CHECKSCHEDULE
	for (PID p = 0; p < get_num_threads()*target_np; p++) {
	  newSchedule.AddTaskSet();
	}
#endif
/* 	int cnt=0; double twt=0.; */
	/*   for (TI vi = dependent_pgraph.begin();  */
	/*        vi != dependent_pgraph.end(); ++vi) { */
	for (VI vi = (*dependent_pgraph.GetPart(0)).begin(); 
	     vi != (*dependent_pgraph.GetPart(0)).end(); ++vi) {
/* 	  cnt++; */
/* 	  twt += vi->data.GetTaskWeight(); */
	  if(vi->data.indegree == 0 && vi->data.scheduled == false) {
	    task_pq.push(ProcPair (vi->data.alaplevel, vi->vid));
	    vi->data.epid=-1;
	    vi->data.EMT=0.;
	    vi->data.scheduled=true;
	  }
	}
/* 	cout<<"cnt: "<<cnt<<" taskwt: "<<twt<<endl; */
      }
//======================================================================
//
// Fast Critical Path
//
//======================================================================

#ifdef CHECKSCHEDULE
 Schedule pFastCriticalPathL()
#else
 void pFastCriticalPathL()
#endif
 {

#ifdef CHECKSCHEDULE
   int epc=0, eic=0;
#endif
     int outer=0;
     int inner=0;
     int addrt=0;

   FCP_pSetup();

   //target_np is the size of local target np
    PRT_list.resize(target_np,0.0);


   VID sch_tid;
/*    PID sch_pid; */
   int rem_tasks = dependent_pgraph.local_size();
   while (rem_tasks > 0) {
     while(task_pq.empty()) { 
       stapl::rmi_poll();
       AddReadyTasks(); 
/*        addrt++; */
     }
     sch_tid = task_pq.top().id;
     task_pq.pop();

     TI vi;
     dependent_pgraph.IsVertex(sch_tid,&vi);

    TASKWEIGHT tmp=1000000.;
     PID spid=0;
     for(int i=0; i<target_np; i++) {
       if(tmp>PRT_list[i]) {
	 tmp = PRT_list[i];
	 spid = i;
       }
     }
     TASKWEIGHT tt;

     if(spid == vi->data.epid) {
       tt= tmp>vi->data.EMT?tmp:vi->data.EMT;
     } else {
       tt = tmp>vi->data.LMT?tmp:vi->data.LMT;
     }
     vi->data.finishtime = vi->data.GetTaskWeight()+ tt;
     PRT_list[spid]=vi->data.finishtime;

#ifdef CHECKSCHEDULE
     newSchedule.AddTask(spid+ myid*target_np, sch_tid);
     if(spid == vi->data.epid) epc++;
     else eic++;
     cout<<"myid "<<myid<<" tid "<<sch_tid<<" spid "<<spid+ myid*target_np
	 <<" finish time "<<vi->data.finishtime<<endl; 
#endif

     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       TASKWEIGHT arrival = vi->data.finishtime + 
	                    ei->weight.GetMessageSize();
       UpdateInfoInSucc( pair<PID,COMMWEIGHT> (spid+ myid*target_np, arrival), 
			 ei->vertex2id); 
     }

 /*     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) { */
/*        VID t2id = ei->vertex2id; */
/*        if(dependent_pgraph.IsLocal(t2id)) { */
/* 	 TI vj; */
/* 	 dependent_pgraph.IsVertex(t2id,&vj); */
/* /* 	 rmi_poll(); */ 
/* 	 outer++; */
/* 	 if (vj->data.indegree == 0 && !vj->data.scheduled) { */
/* 	   CalculateEPandEMT(vj->vid); */
/* 	   task_pq.push( ProcPair(vj->data.alaplevel, vj->vid) ); */
/* 	   vj->data.scheduled = true;  */
/* 	   inner++; */
/* 	 } */
/*        } */
/*      } */
     rem_tasks--;
   }  //while(rem_tasks > 0)

/*    DecrementProcsCount(); */

#ifdef CHECKSCHEDULE
   cout<<"myid "<<myid<<" total: "<<outer
       <<" with "<<inner<<" pushed. AddReadyT called "<<addrt<<" times."<<endl;
   cout<<"epc : "<<epc<<" eic:"<<eic<<endl;
   newSchedule.SetName("FCP");
   return newSchedule;
#endif
 }


#ifdef CHECKSCHEDULE
 Schedule pFastCriticalPathF()
#else
 void pFastCriticalPathF()
#endif
 {

#ifdef CHECKSCHEDULE
   int epc=0, eic=0;
#endif
     int outer=0;
     int inner=0;

   FCP_pSetup();

   if(myid == MASTER_PID) {
     PRT_list.resize(target_np,0.0);
     color.resize(target_np,0);
     while(procscount) rmi_poll();
   }

   VID sch_tid;
/*    PID sch_pid; */
   int rem_tasks = dependent_pgraph.local_size();
   while (rem_tasks > 0) {
     while(task_pq.empty()) { 
       stapl::rmi_poll();
       AddReadyTasks(); 
     }
     sch_tid = task_pq.top().id;
     task_pq.pop();

     TI vi;
     dependent_pgraph.IsVertex(sch_tid,&vi);

     GetEarliestPidandFT( _StaplSix<PID,TASKWEIGHT,TASKWEIGHT,
			  TASKWEIGHT,PID,VID> (vi->data.epid,vi->data.LMT,
					       vi->data.EMT,
					       vi->data.GetTaskWeight(),
					       myid,sch_tid) );
     while(result.first == -1 || result.third != sch_tid) rmi_poll();
     PID spid;
     vi->data.finishtime = result.second;
     spid = result.first;
     result.first = -1;

#ifdef CHECKSCHEDULE
     newSchedule.AddTask(spid, sch_tid);
     if(spid == vi->data.epid) epc++;
     else eic++;
     cout<<"myid "<<myid<<" tid "<<sch_tid<<" spid "<<spid
	 <<" finish time of tid "<<result.second<<endl; 
#endif

     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       TASKWEIGHT arrival = vi->data.finishtime + 
	                    ei->weight.GetMessageSize();
       UpdateInfoInSucc( pair<PID,COMMWEIGHT> (spid, arrival), 
			 ei->vertex2id); 
     }

     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       VID t2id = ei->vertex2id;
       if(dependent_pgraph.IsLocal(t2id)) {
	 TI vj;
	 dependent_pgraph.IsVertex(t2id,&vj);
/* 	 rmi_poll(); */
	 outer++;
	 if (vj->data.indegree == 0 && !vj->data.scheduled) {
	   CalculateEPandEMT(vj->vid);
	   task_pq.push( ProcPair(vj->data.alaplevel, vj->vid) );
	   vj->data.scheduled = true;   
	   inner++;
	 }
       }
     }
     rem_tasks--;
   }  //while(rem_tasks > 0)

   DecrementProcsCount();

#ifdef CHECKSCHEDULE
   cout<<"myid "<<myid<<" total: "<<outer
       <<" with "<<inner<<" pushed."<<endl;
   cout<<"epc : "<<epc<<" eic:"<<eic<<endl;
   newSchedule.SetName("FCP");
   return newSchedule;
#endif
 }




#ifdef CHECKSCHEDULE
 Schedule pFastCriticalPathS()
#else
 void pFastCriticalPathS()
#endif
 {

#ifdef CHECKSCHEDULE
   int epc=0, eic=0;
#endif

   FCP_pSetup();

   if(myid == MASTER_PID) {
     PRT_list.resize(target_np,0.0);
     while(procscount) rmi_poll();
   }

   VID sch_tid;
   PID sch_pid;
   int rem_tasks = dependent_pgraph.local_size();
   while (rem_tasks > 0) {
     while(task_pq.empty()) { 
       stapl::rmi_poll();
       AddReadyTasks(); 
     }
     sch_tid = task_pq.top().id;
/*      cout<<"pop vid:"<<sch_tid<<" alap: "<<alap<<endl; */
     task_pq.pop();

     TI vi;
     dependent_pgraph.IsVertex(sch_tid,&vi);
     bool success = false;

//!earliest Proc
     pair<PID,TASKWEIGHT> pp;
     do {
       pp= GetEarliestUncoloredandColorIt();
     } while (pp.first == -1);

     PID ei_pid=pp.first;

#ifdef CHECKSCHEDULE
     vi->data.ipid = ei_pid; //debug purpose
#endif

/*   cout<<"myid "<<myid<<" tid "<<sch_tid<<" ipid "<<ei_pid
	    <<" ready time of ipid "<<pp.second<<endl; */

     TASKWEIGHT eistart = vi->data.EMT > pp.second? vi->data.EMT : pp.second ;

     TASKWEIGHT epstart;
     //if enabling P can get it to start earlier, set it to ep_pid
     //check conflict in master processor in the same time
     PID ep_pid = vi->data.epid;
     if( vi->data.epid != -1) {
       pair<bool,double> crte = GetColorAndRTandSetColor(vi->data.epid);
/*     cout<<"myid "<<myid<<" vid "<<sch_tid<<" epid "<<vi->data.epid */
/* 	   <<" color and RT "<<crte.second<<" "<<crte.first<<endl; */
       epstart = vi->data.EMT > crte.second? vi->data.EMT : crte.second ;
       if( epstart < eistart && !crte.first) {
	 vi->data.finishtime = epstart + vi->data.GetTaskWeight();
	 vi->data.spid = vi->data.epid;
	 UnSetColor(ei_pid);
	 success = true;
#ifdef CHECKSCHEDULE
	 epc++;
#endif
/*     cout<<"myid "<<myid<<" ep sch_pid:"<<vi->data.spid<<" vid:"<<sch_tid<<endl; */
       } 
     }

     if(!success) {
       TASKWEIGHT estart;
       if(ei_pid != vi->data.epid) 
	 estart = vi->data.LMT > eistart? vi->data.LMT : eistart;
       else estart = vi->data.EMT > eistart? vi->data.EMT : eistart;
       vi->data.finishtime = estart + vi->data.GetTaskWeight();
       vi->data.spid = ei_pid;
       if(vi->data.epid != -1) UnSetColor(vi->data.epid);
       success = true; 

#ifdef CHECKSCHEDULE
       eic++;
#endif
/*     cout<<"myid "<<myid<<"ei sch_pid:"<<vi->data.spid<<" vid:"<<sch_tid<<endl; */
     }

     // update the processor ready time, set color to false, 
     // and schedule task
     UpdateProcRTandUnSetColor(vi->data.finishtime,vi->data.spid);

#ifdef CHECKSCHEDULE
     newSchedule.AddTask(vi->data.spid, sch_tid);
/*   cout<<"myid "<<myid<<" sched vid " */
/* 	 <<sch_tid<<" on pid "<<vi->data.spid<<endl; */
#endif

     //give arrival time, reduce the indegree, 
     //and compute the successor's EMT and EP calculation.
     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       TASKWEIGHT arrival = vi->data.finishtime + ei->weight.GetMessageSize();
       UpdateInfoInSucc( pair<PID,COMMWEIGHT> (vi->data.spid, arrival), 
			 ei->vertex2id); 
     }

     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       VID t2id = ei->vertex2id;
       if(dependent_pgraph.IsLocal(t2id)) {
	 TI vj;
	 dependent_pgraph.IsVertex(t2id,&vj);
	 rmi_poll();
	 if (vj->data.indegree == 0 && !vj->data.scheduled) {
	   CalculateEPandEMT(vj->vid);
	   task_pq.push( ProcPair(vj->data.alaplevel, vj->vid) );
  /* cout<<"myid "<<myid<<" push inloop vid "<<vj->vid<<endl; */
	   vj->data.scheduled = true;    
	 }
       }
     }
     rem_tasks--;
   }  //while(rem_tasks > 0)

   DecrementProcsCount();

#ifdef CHECKSCHEDULE
   cout<<"epc : "<<epc<<" eic:"<<eic<<endl;
   newSchedule.SetName("FCP");
   return newSchedule;
#endif
 }

#ifdef CHECKSCHEDULE
 Schedule pFastCriticalPathP()
#else
 void pFastCriticalPathP()
#endif
 {

#ifdef CHECKSCHEDULE
   int epc=0, eic=0;
#endif

   FCP_pSetup();

   if(myid == MASTER_PID) {
     for (PID p = 0; p < target_np; p++) {
       allProc_list.insert( ProcPair(0.0, p) );
     } 
     while(procscount) rmi_poll();
   }

   VID sch_tid;
   PID sch_pid;
   int rem_tasks = dependent_pgraph.local_size();
   while (rem_tasks > 0) {
     while(task_pq.empty()) { 
       stapl::rmi_poll();
       AddReadyTasks(); 
     }
     sch_tid = task_pq.top().id;
/*      cout<<"pop vid:"<<sch_tid<<" alap: "<<alap<<endl; */
     task_pq.pop();

     TI vi;
     dependent_pgraph.IsVertex(sch_tid,&vi);
     bool success = false;

//!earliest Proc
     ProcPair pp = PopFirstProcIDandRT();
     PID ei_pid=pp.id;

#ifdef CHECKSCHEDULE
     vi->data.ipid = ei_pid; //debug purpose
#endif

/*   cout<<"myid "<<myid<<" tid "<<sch_tid<<" ipid "<<ei_pid
	    <<" ready time of ipid "<<pp.second<<endl; */

     TASKWEIGHT eistart = vi->data.EMT > pp.priority? vi->data.EMT : pp.priority;

     TASKWEIGHT epstart;
     //if enabling P can get it to start earlier, set it to vi->data.epid
     //check conflict in master processor in the same time
     ProcPair crte;
     crte.id = -1;
     if( vi->data.epid != -1) {
       crte=PriorityListremove(vi->data.epid);
       epstart = vi->data.EMT > crte.priority? vi->data.EMT : crte.priority;
       if( epstart < eistart && crte.id != -1) {
	 //put back ei_pid that wasn't used
	 PriorityListInsert(pp);
	 vi->data.finishtime = epstart + vi->data.GetTaskWeight();
	 vi->data.spid = vi->data.epid;
	 success = true;
#ifdef CHECKSCHEDULE
	 epc++;
#endif
       } 
     }

     if(!success) {
       if(crte.id != -1) PriorityListInsert(crte);
       TASKWEIGHT estart;
       if(ei_pid != vi->data.epid) 
	 estart = vi->data.LMT > eistart? vi->data.LMT : eistart;
       else estart = vi->data.EMT > eistart? vi->data.EMT : eistart;
       vi->data.finishtime = estart + vi->data.GetTaskWeight();
       vi->data.spid = ei_pid;
       success = true; 
#ifdef CHECKSCHEDULE
       eic++;
#endif
     }

     // update the processor ready time, set color to false, 
     // and schedule task
     PriorityListInsert(ProcPair(vi->data.finishtime, vi->data.spid));

#ifdef CHECKSCHEDULE
     newSchedule.AddTask(vi->data.spid, sch_tid);
/*   cout<<"myid "<<myid<<" sched vid " */
/* 	 <<sch_tid<<" on pid "<<vi->data.spid<<endl; */
#endif

     //give arrival time, reduce the indegree, 
     //and compute the successor's EMT and EP calculation.
     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       TASKWEIGHT arrival = vi->data.finishtime + ei->weight.GetMessageSize();
       UpdateInfoInSucc( pair<PID,COMMWEIGHT> (vi->data.spid, arrival), 
			 ei->vertex2id); 
     }

     for(EGI ei=vi->edgelist.begin(); ei!= vi->edgelist.end(); ++ei) {
       VID t2id = ei->vertex2id;
       if(dependent_pgraph.IsLocal(t2id)) {
	 TI vj;
	 dependent_pgraph.IsVertex(t2id,&vj);
	 rmi_poll();
	 if (vj->data.indegree == 0 && !vj->data.scheduled) {
	   CalculateEPandEMT(vj->vid);
	   task_pq.push( ProcPair(vj->data.alaplevel, vj->vid) );
  /* cout<<"myid "<<myid<<" push inloop vid "<<vj->vid<<endl; */
	   vj->data.scheduled = true;    
	 }
       }
     }
     rem_tasks--;
   }  //while(rem_tasks > 0)

   DecrementProcsCount();

#ifdef CHECKSCHEDULE
   cout<<"epc : "<<epc<<" eic:"<<eic<<endl;
   newSchedule.SetName("FCP");
#endif

#ifdef CHECKSCHEDULE
  return newSchedule;
#endif
 }


 void AssignBlevelAndALAPlevel(WtDiGraph& _gg)
 {
   // Get topological sort
  vector<VID> tsort;
  TopologicalSort<WtDiGraph>(_gg,tsort);

  // Initialize for computing b-levels
  int nvertices = _gg.GetVertexCount();
  vector<TASKWEIGHT>  blevel_list(nvertices,0.0);

  // Assign b-levels (including edge weights)
  // VID_vector succs;
  TASKWEIGHT maxval, taskwt;
  COMMWEIGHT edge_wt;
  VID parentid, childid;
  vector<VID>::reverse_iterator i, rend;
  CIVID j, send;
  for (i = tsort.rbegin(), rend = tsort.rend(); i != rend; i++) {
    parentid = *i;
    vector<VID> succs;
    _gg.GetSuccessors(parentid,succs);
    maxval = 0;
    for (j = succs.begin(), send = succs.end(); j != send; j++) {
      childid = *j;
      edge_wt = _gg.GetEdgeWeight(parentid, childid).GetMessageSize();
      if (edge_wt + blevel_list[childid] > maxval)
        maxval = edge_wt + blevel_list[childid];
    }
    taskwt = _gg.GetData(parentid).GetTaskWeight();
    blevel_list[parentid] = taskwt + maxval;
  }
/*   for(int i=0; i<blevel_list.size(); ++i) */
/*     cout<<"bl["<<i<<"]="<<blevel_list[i]<<endl; */
  
  TASKWEIGHT maxbl=-1.0;
  typename vector <TASKWEIGHT>::iterator _p;
  for(_p = blevel_list.begin(); _p!=blevel_list.end(); _p++) {
    if( *_p > maxbl ) maxbl = *_p;
  }
  for(_p = blevel_list.begin(); _p!=blevel_list.end(); _p++) 
    *_p = maxbl - *_p;
/*     cout<<"maxbl "<<maxbl<<endl; */

  for(typename WtDiGraph::VI vi=_gg.begin(); vi!=_gg.end(); ++vi) {
    vi->data.alaplevel = blevel_list[vi->vid];
/*     cout<<"alap["<<vi->vid<<"]="<<vi->data.alaplevel<<endl; */
  }
}

};

}  //end namespace

#endif
