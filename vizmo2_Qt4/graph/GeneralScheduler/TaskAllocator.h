/*!
	\file TaskAllocator.h
	\author Ping An	
	\date Feb. 12, 02
	\brief Allocator, mapping clustered tasks onto machines hierarchically

	class TaskAllocator 

	0	Genetic Allocation algorithm

	should get a schedule from TaskScheduler, and output mapped graph to 
	Scheduler

	Input:
	  a schedule (with format defined by schedule.h);
	  MachineInfo
	  a graph

	Output:
	  mapped/annoted graph

	Note:
*/

#ifndef TaskAllocator_h
#define TaskAllocator_h

#include <runtime.h>
#include "rmitools.h"
#include "Graph.h"
#include "Comm.h"
#include "Task.h"
#include "Schedule.h"
#include "MachineInfo.h"
#include "TaskSchedAux.h"

namespace scheduler {

//===================================================================
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
class TaskAllocator : public BasePObject
{ 
 friend class SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>;

  typedef Task<TASKWEIGHT,TASKINFO> Ttype;
  typedef Comm<COMMWEIGHT,COMMINFO> Wtype;

  typedef Graph<DG<Ttype,Wtype>, NMG<Ttype,Wtype>, 
    WG<Ttype,Wtype>,Ttype,Wtype > WtDiGraph;

  typedef WtVertexType<Ttype,Wtype> Vert;
  typedef WtEdgeType<Ttype,Wtype> WEdge;

  typedef  typename vector< Vert >::iterator TI;
  typedef  typename vector< Vert >::const_iterator CTI;
  typedef  typename vector< WEdge >::iterator EGI;
  typedef  typename vector< WEdge >::const_iterator CEGI;

  typedef TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    this_type;

   struct IdleProc {
    TASKWEIGHT itime;
    double pcost;
    PID pid;
    IdleProc(TASKWEIGHT _t, double _p, PID _id) 
      : itime(_t), pcost(_p), pid(_id) {}
  };

//===================================================================
//  Data
//===================================================================
protected:
  int target_np;
  int systemlevel;
  PID systemid;
  MachineInfo* machineinfo;

  WtDiGraph annoted_graph;

  pair<PID,TASKWEIGHT> mymakespan;  //record local makespan in parallel RAND
  TASKWEIGHT makespan;  //result of mapped schedule

  //clustering result, passed in from TaskScheduler
  Schedule schedule;

  //mapping result
  Schedule mapped_schedule;

  //mapping vec, store vector of CIDs in each target procs
  vector<vector<CID> > mappingvec;

  //used in neighborhood search, randomized algorithm
  vector<CID> neighbors;

 private:
  // common data structures for LLB
  vector<TASKWEIGHT> blevel_list;        // blevel of each vertex
  vector<int> indegree_list;    // indegree of each vertex
  vector<TASKWEIGHT> FT_list;            // finish time of each vertex
  vector<PID> PROC_list;         // assigned proc of each vertex
//===================================================================
//  Constructors and Destructor
//===================================================================
public:
  TaskAllocator();
  TaskAllocator(const Schedule&,MachineInfo*,const WtDiGraph&);
  ~TaskAllocator();

//===================================================================
//  Other Methods
//===================================================================

  //====================================
  // Mapping Algorithms
  //====================================
  double Randmized_Mapping(int niter);

  double GA_Mapping(int ntiter, int niter);

  Schedule MapbyLLB();

  //====================================
  //Computing makespan
  //====================================
  TASKWEIGHT GetMakespan();  //makespan based on clustering results
  TASKWEIGHT GetMappedMakespan(); //makespan based on mapping results

  //====================================
  //Get methods
  //====================================
  const WtDiGraph& GetAnnotedGraph() const { return annoted_graph; };
  Schedule GetMappedSchedule() const { return mapped_schedule; };

private:
  //====================================
  //helper functions in Randomized_Mapping
  //====================================
  void Setup_Mapping(vector<pair<CID,TASKWEIGHT> >& clusterweights);
  void Setup_Mapping_Blevel(vector<pair<CID,TASKWEIGHT> >& clusterweights);
  void SetSchedulePpid();

  void InitialRandomMapping(const vector<pair<CID,TASKWEIGHT> >& 
			    clusterweights);
  void GetNeighborSearchCids(const vector<pair<CID,TASKWEIGHT> >& 
			     clusterweights);
  double SwappingPids(int niter);

  void ExchangeBestMappingVec(TASKWEIGHT& mksp);
  void DistributeBestSched();

  void SetMappingVec(const vector<CID>& in);
  
  double DoOneSwap(vector<pair<pair<PID,int>, pair<PID,int> > >& 
		   acceptedswaps,pair<pair<PID,int>, pair<PID,int> >& tpr);

  pair<pair<PID,int>, pair<PID,int> > 
    SelectRandomSwap(vector<pair<pair<PID,int>, pair<PID,int> > >& 
		     acceptedswaps);

  pair<pair<PID,int>, pair<PID,int> > 
    SelectRandomSwap_old(vector<pair<pair<PID,int>, pair<PID,int> > >& 
		     acceptedswaps);

  void Swapping(const pair<pair<PID,int>, pair<PID,int> >&);

  void UpdateMappingVector(PID rpid, CID rcid1, CID rcid2);

  void UpdateGraph(const pair<pair<PID,int>, pair<PID,int> >& tpr);

  void AnnoteGraphwithPid(Schedule&);
  void AnnoteGraphwithPid();
  void AnnoteGraphwithProcId(Schedule&);

  bool MyFind(const vector<pair<pair<PID,int>, pair<PID,int> > >&,
	      const pair<pair<PID,int>, pair<PID,int> >& tpr);

  //used in reduce_rmi to get max makespan in parallel mapping
  void minmksp(pair<PID,TASKWEIGHT>* in, pair<PID,TASKWEIGHT>* inout) {
   if((*inout).second > (*in).second) *inout = *in;
  }

  void min1(TASKWEIGHT* in, TASKWEIGHT* inout) {
   if((*inout) > (*in)) *inout = *in;
  }

  void max1(int* in, int* inout) {
   if((*inout) < (*in)) *inout = *in;
  }
	
  void DisplayMappingVec();

  void AnnoteGraphwithCid();

  Schedule MakeNewSchedule(); //based on mappingvec;

  //====================================
  //functions used to compute makespan
  //====================================
  void BlevelOrdering(Schedule& s2);
  void EvaluateCosts();

  //makespan after considering system and network speed. 
  //Used only after EvaluateCosts() is called.
  TASKWEIGHT GetMakespan(Schedule&); 

  TASKWEIGHT FindLatestOutQPred(const VID&);

  //====================================
  //used in LLB algorithm
  //====================================
  void AssignBlevelsUNC(Schedule &);

};

//===================================================================
//  Constructors and Destructor
//===================================================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
TaskAllocator() {
     this->register_this(this);
     rmi_fence(); 
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
TaskAllocator(const Schedule& _s, MachineInfo* _m, const WtDiGraph& _g) {
     this->register_this(this);
     rmi_fence(); 
  //consider only the first level of the system
  //currently a flat system
  //system level and system id can be adjusted if an hierarchical system is 
  //provided.  -ann
  machineinfo = _m;
  systemlevel = 0;
  systemid = 0;
  target_np = machineinfo->GetNodeSize(systemlevel,systemid);

  //preprocessing of annoted_graph, set predecessors and cluster id
  annoted_graph = _g;
  annoted_graph.SetPredecessors();

  schedule = _s;
  AssignBlevelsUNC(schedule);
  int nvertices = annoted_graph.GetVertexCount();
  for (int vid = 0; vid < nvertices; vid++) {
    vector<VID> tmpv;
    annoted_graph.GetPredecessors(vid,tmpv);
    indegree_list.push_back( tmpv.size() );
  }
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
~TaskAllocator() {
};

template <class T>
struct __TDS : public binary_function<T, T, bool> {
  bool operator()(T x, T y) { return x.second > y.second; }
};


template <class T>
struct _LESS : public binary_function<T, T, bool> {
  bool operator() (const T& p1, const T& p2) {
      return (p1.itime < p2.itime);
  }
};
template <class T>
struct _LARG : public binary_function<T, T, bool> {
  bool operator() (const T& p1, const T& p2) {
      return (p1.itime > p2.itime);
  }
};
//===================================================================
//  public Methods
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
Setup_Mapping(vector<pair<CID,TASKWEIGHT> >& clusterweights){
  //based on annoted_graph with cluster ids annoted
  //sort cluster weights as summation of task weights
  AnnoteGraphwithCid();  //based on schedule

  clusterweights.resize(schedule.size());
  for(int i=0; i<schedule.size(); ++i) {
    clusterweights[i].first = i;
  }
  TI v1= annoted_graph.v.begin();
  for(; v1!= annoted_graph.v.end(); ++v1) {
    CID tcid = v1->data.GetClusterId();
    clusterweights[tcid].second += v1->data.GetTaskWeight();
  }
  sort(clusterweights.begin(), clusterweights.end(),__TDS<pair<CID,TASKWEIGHT> >());

#ifdef DEBUG_MAPPING
  for(int i=0; i<clusterweights.size(); ++i) {
    cout<<"Cluster ID: "<<clusterweights[i].first
	<<" Weight Sum: "<<clusterweights[i].second<<endl;
  }
#endif
}
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
Setup_Mapping_Blevel(vector<pair<CID,TASKWEIGHT> >& clusterweights){
  clusterweights.resize(schedule.size());
  for(int i=0; i<schedule.size(); ++i) {
    clusterweights[i].first = i;
  }
  for(GII si = schedule.tasksetvector().begin(); 
       si != schedule.tasksetvector().end(); si++) {
    VID vid = *(si->second.taskset.begin());
    clusterweights[si->first].second = blevel_list[vid];
  }
  sort(clusterweights.begin(), clusterweights.end(),__TDS<pair<CID,TASKWEIGHT> >());

#ifdef DEBUG_MAPPING
  for(int i=0; i<clusterweights.size(); ++i) {
    cout<<"Cluster ID: "<<clusterweights[i].first
	<<" Blevel: "<<clusterweights[i].second<<endl;
  }
#endif
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
InitialRandomMapping(const vector<pair<CID,TASKWEIGHT> >& clusterweights) {
  //initial random mapping
  //first p clusters assigned to each proc
  //rest random
  assert(target_np<schedule.size());
  mappingvec.resize(target_np);
  vector<TASKWEIGHT> pftime(target_np,0.);
  typename vector<pair<CID,TASKWEIGHT> >::const_iterator cii;
  int j=0;
  unsigned int seed = time(NULL)*(get_thread_id()+1);
  srand(seed);
  for(cii =clusterweights.begin(); cii!=clusterweights.end();
      cii++,j++) {
    if(j<target_np) {
      mappingvec[j].push_back(cii->first);
      schedule.tasksetvector()[cii->first].SetPpid(j);
      pftime[j%target_np]=cii->second;
    }
    else {
      PID earlypid = min_element(pftime.begin(), pftime.end())-pftime.begin();
      mappingvec[earlypid].push_back(cii->first);
      schedule.tasksetvector()[cii->first].SetPpid(earlypid);
      pftime[earlypid] +=cii->second;
    } 
  }
#ifdef DEBUG_MAPPING
  DisplayMappingVec();
#endif
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
SetSchedulePpid() {
  for(int i=0; i<mappingvec.size(); ++i) {
    for(int j=0; j<mappingvec[i].size(); ++j) {
      schedule.tasksetvector()[mappingvec[i][j]].SetPpid(i);
    }
  }

}
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetNeighborSearchCids(const vector<pair<CID,TASKWEIGHT> >& clusterweights) {
  PID myid = get_thread_id();
  int nprocs = get_num_threads();
  for(int i=target_np; i<clusterweights.size(); ++i) {
    if(i%nprocs == myid) neighbors.push_back(clusterweights[i].first);
  }
  if(neighbors.size() <2) {
    cout<<"Number of clusters in PID "<<myid<<" is less than 2. Cannot swap! Increase the number of clusters for random swapping."<<endl;
    exit(1);
  }
#ifdef DEBUG_MAPPING
  cout<<"PID "<<myid<<" neighbors search space CIDs: ("<<endl;
  for(int i=0; i<neighbors.size(); ++i) {
    cout<<" "<<neighbors[i];
  }
  cout<<" )"<<endl;
#endif
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
double
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
Randmized_Mapping(int niter) {
  int myid = stapl::get_thread_id();
  int nprocs = stapl::get_num_threads();
  mymakespan.first=myid;
  cout<<"cluster size: "<<schedule.size()<<endl;
  vector<pair<CID,TASKWEIGHT> > clusterweights;
  //sort clusters
/*   Setup_Mapping_Blevel(clusterweights); */
  Setup_Mapping(clusterweights);
  InitialRandomMapping(clusterweights);
  //partition search space to each scheduling process
  GetNeighborSearchCids(clusterweights);

  //annote annoted_graph with initial mapping pids
  Schedule s2 = MakeNewSchedule();  //based on the mappingvec
  AnnoteGraphwithPid(s2);           //based on Schedule s2

  //===========================================
  //neighborhood search, iteration limit: #niter
  //===========================================
  mymakespan.second=SwappingPids(niter);
  //===========================================
  cout<<"PID "<<myid<<" before final reduce results:"
      <<mymakespan.first<<" "<<mymakespan.second<<endl;
  pair<PID,TASKWEIGHT> bestmksp;
  reduce_rmi(&mymakespan, &bestmksp, getHandle(),
	     &this_type::minmksp, true);
/*  cout<<"PID "<<myid<<" final reduce results:" */
/*       <<bestmksp.first<<" "<<bestmksp.second<<endl; */
#ifdef DEBUG_MAPPING
  if(myid == bestmksp.first) {
    cout<<"Best Mapped Schedule in Proc "<<myid<<endl;
    mapped_schedule=MakeNewSchedule();
    mapped_schedule.DisplaySchedule();
    cout<<"Final Makespan "<<endl;
    cout<<bestmksp.second<<endl;
  }
#endif
  return bestmksp.second;
};

//===================================================================
//  private Methods
//===================================================================

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
double
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
SwappingPids(int niter) {

  double mksp=GetMappedMakespan();  //based on mappingvec
/* #ifdef DEBUG_MAPPING */
  cout<<"Makespan after the initial random mapping:"<<endl;
  cout<<mksp<<endl;
/* #endif */

  vector<pair<pair<PID,int>, pair<PID,int> > > acceptedswaps;
  int nit = 0; //count total number of exchanges
  int success=0; //number of success swaps made
  int search_limit = niter/get_num_threads();

  while(nit < search_limit) {  //number of exchanges limited

    pair<pair<PID,int>, pair<PID,int> > tpr;
    double tmksp = DoOneSwap(acceptedswaps,tpr);
#ifdef DEBUG_MAPPING
    cout<<"myid "<<get_thread_id()<<" random swap: "
	<<tmksp<<endl;   
#endif

    if(tmksp < mksp) {
      success++;
      mksp = tmksp;
      acceptedswaps.push_back(tpr);
      cout<<"myid "<<get_thread_id()<<" a random good swap: "
	  <<tmksp<<" #success "<<success<<endl;   
    } else {
      CID tcid = tpr.first.second;
      tpr.first.second=tpr.second.second;
      tpr.second.second=tcid;
      Swapping(tpr);  //change back to old mappingvec
      UpdateGraph(tpr); //change back to old annoted_graph
    }

    //exchange best mappingvec after certain number of success count
    int maxsuc=0;
    reduce_rmi(&success, &maxsuc,getHandle(),&this_type::max1, true);
/*     cout<<"myid "<<get_thread_id()<<" max success: " */
/* 	<<maxsuc<<" #success "<<success<<endl;  */
    int threathold = 4;
    if(maxsuc >= threathold) {
      threathold = threathold/2;
      if(threathold == 0) threathold = 1;
      ExchangeBestMappingVec(mksp);
      nit++;
      success=0;
      acceptedswaps.clear();
    } 
  }
 return mksp;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
ExchangeBestMappingVec(TASKWEIGHT& mksp) {
  mymakespan.second = mksp;
  pair<PID,TASKWEIGHT> bestmksp;
  reduce_rmi(&mymakespan, &bestmksp, getHandle(),&this_type::minmksp, true);
  mksp = bestmksp.second;
  cout<<"PID "<<get_thread_id()<<" intermediate reduce results:"
      <<bestmksp.first<<" "<<bestmksp.second<<endl;
  rmi_fence();
  if(get_thread_id() != bestmksp.first) mappingvec.clear();
  if(get_thread_id() == bestmksp.first) DistributeBestSched();  
  rmi_poll();
  rmi_fence();
#ifdef DEBUG_MAPPING
  cout<<"myid: "<<get_thread_id()<<" distributed mapping vec:"<<endl;
  DisplayMappingVec();
#endif

  if(get_thread_id() != bestmksp.first) {
    Schedule ss=MakeNewSchedule(); //based on new mappingvec
    AnnoteGraphwithPid(ss);
    SetSchedulePpid();
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
DistributeBestSched() {
  //only called by the process with the best schedule!!!
  for(int i=0; i<mappingvec.size(); ++i) {
    for(int j=0; j<get_num_threads(); ++j) {
      if(get_thread_id() != j) 
	async_rmi(j, getHandle(), &this_type::SetMappingVec,mappingvec[i]);
    }
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
SetMappingVec(const vector<CID>& in) {
  mappingvec.push_back(in);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
double
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
DoOneSwap(vector<pair<pair<PID,int>, pair<PID,int> > >& acceptedswaps,
	  pair<pair<PID,int>, pair<PID,int> >& tpr) {
  tpr = SelectRandomSwap(acceptedswaps);
  UpdateGraph(tpr);
  Swapping(tpr);
  TASKWEIGHT tmksp = GetMappedMakespan();
  return tmksp;
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
pair<pair<PID,int>, pair<PID,int> >
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
SelectRandomSwap(vector<pair<pair<PID,int>, 
		 pair<PID,int> > >& acceptedswaps) {
  pair<pair<PID,int>, pair<PID,int> > tpr;
  pair<PID,int> pr1(-1,-1);
  pair<PID,int> pr2(-1,-1);
  int count = 0;
  do {
    //randomly choose two different clusters from two different mapping procs
    while(pr1.first == pr2.first) {
      unsigned int seed1 = time(NULL);
      srand(seed1);
      pr1.second = neighbors[(int) (rand() % neighbors.size())];
      pr1.first = (schedule.tasksetvector())[pr1.second].GetPpid();
      do {
	unsigned int seed2 = time(NULL);
	srand(seed2);
	pr2.second = neighbors[(int) (rand() % neighbors.size())];
	pr2.first = (schedule.tasksetvector())[pr2.second].GetPpid();
      } while(pr1.second == pr2.second); 
    }

#ifdef DEBUG_MAPPING
     cout<<"random pid cid pair one:"<<pr1.first<<" "<<pr1.second<<endl;
     cout<<"random pid cid pair two:"<<pr2.first<<" "<<pr2.second<<endl;
#endif

    tpr.first = pr1; 
    tpr.second= pr2;
    count++;
    if(count >= TOTALTRY) break;
  } while(MyFind(acceptedswaps,tpr)); 
  return tpr;
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
pair<pair<PID,int>, pair<PID,int> >
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
SelectRandomSwap_old(vector<pair<pair<PID,int>, pair<PID,int> > >&
		     acceptedswaps) {
  pair<pair<PID,int>, pair<PID,int> > tpr;
  pair<PID,int> pr1(-1,-1);
  pair<PID,int> pr2(-1,-1);
  int count = 0;
  do {
    while(pr1.first == pr2.first) {
      unsigned int seed1 = time(NULL);
      srand(seed1);
      pr1.first = (int) (rand() % machineinfo->nodesize);
      unsigned int seed2 = time(NULL);
      srand(seed2);
      pr2.first = (int) (rand() % machineinfo->nodesize);
    }
//     cout<<"Two random pids:"<<pr1.first<<" "<<pr2.first<<endl;

    unsigned int seed = time(NULL);
    srand(seed);
    if(mappingvec[pr1.first].size() != 1) {
      int ti = (int) (rand() % mappingvec[pr1.first].size());
      pr1.second = mappingvec[pr1.first][ti];
    } else {
      pr1.second = mappingvec[pr1.first][0];
    }
    if(mappingvec[pr2.first].size() != 1) {
      int tj = (int) (rand() % mappingvec[pr2.first].size());
      pr2.second = mappingvec[pr2.first][tj];
    } else {
      pr2.second =  mappingvec[pr2.first][0];
    }
//     cout<<"Two random cids:"<<pr1.second<<" "<<pr2.second<<endl;

    tpr.first = pr1; 
    tpr.second=pr2;
    count++;
    if(count >= TOTALTRY) break;
  } while(MyFind(acceptedswaps,tpr)); 
  return tpr;
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
Swapping(const pair<pair<PID,int>, pair<PID,int> >& tpr) {

  int rpid1=tpr.first.first;
  int rcid1 = tpr.first.second;
  int rpid2=tpr.second.first;
  int rcid2 = tpr.second.second;
  
  schedule.tasksetvector()[rcid1].SetPpid(rpid2);
  schedule.tasksetvector()[rcid2].SetPpid(rpid1);

  UpdateMappingVector(rpid1,rcid1,rcid2);
  UpdateMappingVector(rpid2,rcid2,rcid1);
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
UpdateMappingVector(PID rpid, CID rcid1, CID rcid2) {
  for(int i=0; i<mappingvec[rpid].size(); ++i) {
    if(mappingvec[rpid][i] == rcid1) {
      mappingvec[rpid][i] = rcid2;
      return;
    }
  }
  cout<<"ERROR! rcid1 "<<rcid1<<" not found in mappingvec with PID "
      <<rpid<<endl;
  exit(1);
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
UpdateGraph(const pair<pair<PID,int>, pair<PID,int> >& tpr) {
  int rpid1=tpr.first.first;
  int rcid1 = tpr.first.second;
  int rpid2=tpr.second.first;
  int rcid2 = tpr.second.second;
  IVID di;
  for(di = schedule.tasksetvector()[rcid1].taskset.begin(); 
      di != schedule.tasksetvector()[rcid1].taskset.end(); di++) {
    (annoted_graph.v)[*di].data.SetProcId(rpid2);
  }
  for(di = schedule.tasksetvector()[rcid2].taskset.begin(); 
      di != schedule.tasksetvector()[rcid2].taskset.end(); di++) {
    (annoted_graph.v)[*di].data.SetProcId(rpid1);
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AnnoteGraphwithCid() {
  int i=0;
  for(GII si = schedule.tasksetvector().begin(); 
       si != schedule.tasksetvector().end(); si++,i++) {
    for(CIVID di = si->second.taskset.begin(); 
	di != si->second.taskset.end(); di++) {
      (annoted_graph.v)[*di].data.SetClusterId(i); //NOTE: i is pid in schedule
                                                   //NOT ppid
    }
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
MakeNewSchedule() {
	  
  Schedule s2;
  vector<vector<CID> >::iterator ci;
  for(ci = mappingvec.begin(); ci!=mappingvec.end(); ci++) {
    IVID vii = ci->begin();
    vector<VID> tmpv = schedule.tasksetvector()[*vii].taskset;
    for(IVID vi = ci->begin()+1; vi!=ci->end(); vi++) {
      for(CIVID ivi = schedule.tasksetvector()[*vi].taskset.begin(); 
	  ivi!=schedule.tasksetvector()[*vi].taskset.end(); ivi++) {
	tmpv.push_back(*ivi);
      }
    }
    s2.AddTaskSet(tmpv);
  }
  return s2;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AnnoteGraphwithPid(Schedule& ss) {
  for (GII si = ss.tasksetvector().begin(); 
       si != ss.tasksetvector().end(); si++) {
    PID tpid = si->second.GetPpid(); //NOTE: ppid is used, NOT pid in schedule
    for(CIVID di = si->second.taskset.begin(); 
	di != si->second.taskset.end(); di++) {
      (annoted_graph.v)[*di].data.SetProcId(tpid);
    }
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AnnoteGraphwithProcId(Schedule& ss) {
  for (GII si = ss.tasksetvector().begin(); 
       si != ss.tasksetvector().end(); si++) {
    PID tpid = si->second.GetPid(); //NOTE:  pid in schedule
    for(CIVID di = si->second.taskset.begin(); 
	di != si->second.taskset.end(); di++) {
      (annoted_graph.v)[*di].data.SetProcId(tpid);
    }
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AnnoteGraphwithPid() {
  AnnoteGraphwithPid(schedule);
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetMakespan() {
  AnnoteGraphwithProcId(schedule);  //using the ppid in schedule
      EvaluateCosts();  //based on annoted graph
  TASKWEIGHT mksp = GetMakespan(schedule);
  return mksp;
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetMappedMakespan() {
  Schedule s2 = MakeNewSchedule();  //based on mappingvec
  EvaluateCosts();          //based on annoted_graph
  TASKWEIGHT mksp = GetMakespan(s2);  //based on annoted_graph & s2
  return mksp;
};


template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
BlevelOrdering(Schedule& s2) {
  for(GII gi = s2.tasksetvector().begin(); 
      gi!=s2.tasksetvector().end(); gi++) {
    vector<pair<VID,VID> > pvec;
    for(IVID ti = gi->second.taskset.begin(); 
	ti!=gi->second.taskset.end(); ti++) {
      pair<VID,VID> np(*ti, (int)blevel_list[*ti]);
      pvec.push_back(np);
    }
    stable_sort(pvec.begin(),pvec.end(),__TDS<pair<VID,VID> >());
    gi->second.taskset.erase(gi->second.taskset.begin(),
			     gi->second.taskset.end());
    vector<pair<VID,VID> >::iterator si;
    for(si = pvec.begin(); si!=pvec.end(); si++){
      gi->second.taskset.push_back(si->first);
    }
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GetMakespan(Schedule& s2) {
/*   annoted_graph.DisplayGraph(); */
/*   s2.DisplaySchedule(); */
  vector<TASKWEIGHT> proc_endtime(s2.size(), 0.);
  vector<deque<IVID> > readytasks(s2.size());
  BlevelOrdering(s2);
/*   s2.DisplaySchedule(); */
  int i=0;
  for(GII gi = s2.tasksetvector().begin(); 
      gi!=s2.tasksetvector().end(); gi++,++i) {
    IVID ti = gi->second.taskset.begin(); 
    if(indegree_list[*ti] == 0) {
      TI vv = annoted_graph.v.begin()+*ti;
      proc_endtime[i] = proc_endtime[i]+vv->data.GetCompCost();
      vv->data.SetFinishT(proc_endtime[i]);
      for(EGI ei=vv->edgelist.begin(); ei!=vv->edgelist.end(); ++ei) {
	indegree_list[ei->vertex2id]--;
      }
      readytasks[i].push_back((ti+1));
/*       cout<<"PID "<<i<<"push "<<*(ti+1)<<endl; */
    } else {
      readytasks[i].push_back(ti);
/*       cout<<"PID "<<i<<"push "<<*(ti)<<endl; */
    }
  }

  int count =0;
  bool done = true;
  i=0;
  while(count != s2.size()) {
    done = true;
    if(!readytasks[i].empty()) {
      done = false;
      IVID ti = readytasks[i].front();
/*        cout<<"front "<<*ti<<" indegree "<<indegree_list[*ti]<<endl;  */
      if(indegree_list[*ti] <= 0) {
	TASKWEIGHT tout = FindLatestOutQPred(*ti);
	TASKWEIGHT start = tout>proc_endtime[i]?tout:proc_endtime[i];
	TI vv = annoted_graph.v.begin()+*ti;
	for(EGI ei=vv->edgelist.begin(); ei!=vv->edgelist.end(); ++ei) {
	  indegree_list[ei->vertex2id]--;
	}
	proc_endtime[i] = start + vv->data.GetCompCost();
	vv->data.SetFinishT(proc_endtime[i]);
/* 	cout<<"finish time of Proc "<<i<<" is "<<proc_endtime[i]<<endl; */
/* 	cout<<"PID "<<i<<"pop "<<*ti<<endl; */
	if((++ti)!= (s2.tasksetvector()[i]).taskset.end()) {
	  readytasks[i].push_back(ti);
/* 	  cout<<"PID "<<i<<"push "<<*(ti)<<endl; */
	}
/* 	else cout<<"not pushed."<<endl; */
	readytasks[i].pop_front();
      }
    }
    i=(++i)%(s2.size());
/*     cout<<"i "<<i<<endl; */
    if(done) count++; else count = 0; 
/*     cout<<"count:"<<count<<endl; */
  }

  s2.makespan = *max_element(proc_endtime.begin(),proc_endtime.end());
  return s2.makespan;
};



template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
TASKWEIGHT
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
FindLatestOutQPred(const VID& _vid) {               
  TI  v1 = annoted_graph.v.begin()+_vid;
  PID p1 = v1->data.GetProcId();
  TASKWEIGHT st=0.;
  for (EGI ee = v1->predecessors.begin(); ee<v1->predecessors.end();ee++) {
    TI v2 = annoted_graph.v.begin()+ee->vertex2id;
    PID p2 = v2->data.GetProcId();
    if(p2 != p1) {
      TASKWEIGHT cost =  v2->data.GetFinishT() + 
	(v2->GetEdgeWeight(_vid)).GetCommCost();
      if(cost > st) st = cost;
    }
  }
  return st;
};

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
EvaluateCosts() {
  TI vi;
  double startup,gh,speed;
  TASKWEIGHT totalcompcost=0;
  COMMWEIGHT totalcommcost=0;
  for(vi=annoted_graph.v.begin(); vi!=annoted_graph.v.end(); vi++) {
    PID pid = vi->data.GetProcId();
    speed = machineinfo->GetProcSpeed(systemlevel,systemid,pid);
    TASKWEIGHT twt = speed*vi->data.GetTaskWeight();
/*     cout<<"speed: "<<speed<<endl; */
    totalcompcost += twt;
    vi->data.SetCompCost(twt);
    for(EGI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ei++) {
      PID p2id = annoted_graph.v[ei->vertex2id].data.GetProcId();
      COMMWEIGHT cwt(0);
      if(pid != p2id) {
	machineinfo->GetCommPara(systemlevel,systemid,pid,p2id,startup,gh);
	cwt = startup + gh*ei->weight.GetMessageSize();
/* 	cout<<"startup "<<startup<<" gh "<<gh<<endl; */
      }
      ei->weight.SetCommCost(cwt);
      totalcommcost += cwt;
    }
  }
};

//======================================================================
//
// Use List-based Load Balancing (LLB) to map UNC schedule to bounded 
// number of processors
//
//======================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
Schedule
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
MapbyLLB()
{
  typedef pair<TASKWEIGHT, VID> READYTASK;
  typedef pair<TASKWEIGHT, double> IDLEPROC;

   Schedule& unc_schedule = schedule;
  // Check if number of clusters > target_np
  if (target_np >= unc_schedule.GetNumTaskSets()) {
    cout << "In MapbyLLB: no merging required,";
    cout << " target_np >= number of clusters." << endl;
    return unc_schedule;
  }

  // Find b-levels for all vertices wrt. a UNC schedule
  // do not count same processor edges
  /*   AssignBlevelsUNC(unc_schedule); */ //moved to constructor, used in
   //TaskOrdering too.


  // Find all ready unmapped tasks
  priority_queue< READYTASK > ru_pqueue;
  int nvertices = annoted_graph.GetVertexCount();
  PROC_list.reserve(nvertices);
  FT_list.reserve(nvertices);

  for (int vid = 0; vid < nvertices; vid++) {
    vector<VID> tmpv;
    annoted_graph.GetPredecessors(vid,tmpv);
    indegree_list.push_back( tmpv.size() );
/*     cout<<" indegree "<<indegree_list[vid]<<endl;  */
    if ( tmpv.size() == 0 ) {
      //unmapped ready tasks
      ru_pqueue.push( make_pair(blevel_list[vid], vid) );
    }
    PROC_list.push_back( -1 );
    FT_list.push_back( -1 );
  }

  // Initialize all procs to idle
  // vector of earliest idle time and processing cost of each proc
  vector< IdleProc > idle_procs;   
  //ready mapped tasks, initially emtpy
  vector< priority_queue< READYTASK > > rm_pqueues; 
  //assume Proc Cost is sorted in ascending order
  for (PID pid = 0; pid < target_np; pid++) {
    double pcost = machineinfo->GetProcSpeed(systemlevel,systemid,pid);
    idle_procs.push_back( IdleProc(0.0, pcost,pid) );
    priority_queue< READYTASK > new_queue;
    rm_pqueues.push_back( new_queue );
    mapped_schedule.AddTaskSet();
  }

  // Schedule tasks
  int remaining_tasks = nvertices;
  READYTASK rm_task, ru_task;
  VID sch_taskid;
  while ( remaining_tasks > 0 ) {
    // Find most urgent ready unmapped task
    if ( ! ru_pqueue.empty() ) {
      ru_task = ru_pqueue.top();
    } else {
      ru_task.first = -1;
      ru_task.second = -1;
    }

    // Find earliest idle proc first based on idle time, second based on speed
    IdleProc earliestproc = *min_element(idle_procs.begin(), idle_procs.end(),_LESS<IdleProc>());
/*     cout<<"earliestproc: "<<earliestproc.itime<<" with id "<<earliestproc.pid<<endl; */
    // Find most urgent ready mapped task on proc epid
    PID epid = earliestproc.pid;
    priority_queue< READYTASK > &ref_rmpq = rm_pqueues[epid];
    if ( ! ref_rmpq.empty() ) {
      rm_task = ref_rmpq.top();
    } else {
      rm_task.first = -1;
      rm_task.second = -1;
    }
    
    // Determine which task t to schedule
    if (rm_task.second != -1 && ru_task.second != -1) {
      if (rm_task.first > ru_task.first) {
        sch_taskid = rm_task.second;
        ref_rmpq.pop();
      } else {
        sch_taskid = ru_task.second;
        ru_pqueue.pop();
      }
    } else if (rm_task.second == -1 && ru_task.second != -1) {
      sch_taskid = ru_task.second;
      ru_pqueue.pop();
    } else if (ru_task.second == -1 && rm_task.second != -1) {
      sch_taskid = rm_task.second;
      ref_rmpq.pop();
    } else {
      // Schedule most urgent ready task from other procs
      TASKWEIGHT max_pr = 0.0;
      for (PID pid = 0; pid < target_np; pid++) {
        if ( pid != epid ) {
          priority_queue< READYTASK > &ref_pq = rm_pqueues[pid];
          if ( ! ref_pq.empty() ) {
            if (max_pr < ref_pq.top().first) {
              max_pr = ref_pq.top().first;
              sch_taskid = ref_pq.top().second;
            }
          }
        }
      }
      // There has to be a ready task, right?
      epid = PROC_list[sch_taskid];
      rm_pqueues[ epid ].pop();
    }
    
    // Schedule task t on proc p (add task to p's tasklist)
    mapped_schedule.AddTask(epid, sch_taskid);

    // If t is not mapped, map all tasks in t's cluster on p
    GII ploc;
    CIVID vloc, vend;
    if ( PROC_list[sch_taskid] == -1 ) {
      const TaskSet &ref_ts = unc_schedule.GetTaskSetbyVID(sch_taskid);
      CIVID ts_iter = ref_ts.taskset.begin();
      CIVID ts_end = ref_ts.taskset.end();
      for (; ts_iter != ts_end; ts_iter++)
        PROC_list[*ts_iter] = epid;
    }

    // Add new ready tasks to ready task lists
    //   if task has owner, place on owner's ready mapped queue
    //   else place on ready unmapped queue
    vector<VID> succs;
    annoted_graph.GetSuccessors(sch_taskid,succs);
    vend = succs.end();
    for (vloc = succs.begin(); vloc != vend; vloc++) {
      indegree_list[*vloc]--;
      if ( indegree_list[*vloc] == 0 ) {
        READYTASK new_task( blevel_list[*vloc], *vloc );
        if ( PROC_list[*vloc] == -1 ) {
          ru_pqueue.push( new_task );
        } else {
          rm_pqueues[ PROC_list[*vloc] ].push( new_task );
        }
      }
    }

    // Update next idle time for epid and end time for task
    vector<VID> preds;
    annoted_graph.GetPredecessors(sch_taskid,preds);
    TASKWEIGHT taskwt = annoted_graph.GetData(sch_taskid).GetTaskWeight();
    double pcost = machineinfo->GetProcSpeed(systemlevel,systemid,PROC_list[sch_taskid]);

    if ( preds.size() == 0 ) {
      FT_list[sch_taskid] = earliestproc.itime + taskwt*pcost;
    } else {
      TASKWEIGHT maxendtime = earliestproc.itime;
      vend = preds.end();
      PID task_owner = PROC_list[sch_taskid];
      for ( vloc = preds.begin(); vloc != vend; vloc++ ) {
        if ( PROC_list[*vloc] != task_owner ) {
	  PID pre_pid = PROC_list[*vloc];
	  double startup, gh;
	  machineinfo->GetCommPara(systemlevel,systemid,
				   pre_pid,task_owner,startup,gh);
          COMMWEIGHT commwt = 
	    annoted_graph.GetEdgeWeight(*vloc, sch_taskid).GetMessageSize()
	    * gh + startup;
          maxendtime = max( maxendtime, FT_list[*vloc] + commwt );
        }
      }
      FT_list[sch_taskid] = maxendtime + taskwt * pcost;
/*       if(sch_taskid == 6)  */
/* 	cout<<"Max endtime "<<maxendtime<< " taskwt: "<<taskwt */
/* 	    <<" pid :"<< earliestproc.pid<< " pcost "<<earliestproc.pcost<<endl; */
    }
    idle_procs[epid].itime = FT_list[sch_taskid]; 
    remaining_tasks--;
  }
  
  // Cleanup
  rm_pqueues.erase( rm_pqueues.begin(), rm_pqueues.end() );

  TASKWEIGHT mksp  = *max_element(FT_list.begin(),FT_list.end());
  cout<<"latest proc finish time: "<<mksp<<endl;

  for(int i=0; i<PROC_list.size(); ++i) {
    PID pid = PROC_list[i];
    (annoted_graph.v)[i].data.SetProcId(pid);
  }
  EvaluateCosts();

/*   for(int i=0; i<FT_list.size(); ++i) { */
/*     cout<<"ftime of "<<i<<" is "<<FT_list[i]<<endl; */
/*     cout<<"proc of "<<i<<" is "<<PROC_list[i]<<endl; */
/*   } */
  const string & tnm="-LLB";
  mapped_schedule.SetName(unc_schedule.GetName()+tnm); 
  return mapped_schedule; 
}

// Assign B-levels for each node wrt. a UNC schedule
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
AssignBlevelsUNC(Schedule &sch)
{
  // Get topological sort
  vector<VID> tsort;
  TopologicalSort(annoted_graph,tsort);

  // Initialize for computing b-levels
  int nvertices = annoted_graph.GetVertexCount();
  blevel_list.reserve(nvertices);
  blevel_list.assign(nvertices, 0.0);

  // Assign b-levels (including edge weights)
  vector<VID>::reverse_iterator rev, rend;
  CIVID child, send;
  // vector<VID> succs;
  TASKWEIGHT maxval, taskwt;
  COMMWEIGHT edge_wt;
  VID parentid, childid;
  for (rev = tsort.rbegin(), rend = tsort.rend(); rev != rend; rev++) {
    parentid = *rev;
    vector<VID> succs;
    annoted_graph.GetSuccessors(parentid,succs);
    maxval = 0;
    for (child = succs.begin(), send = succs.end(); child != send; child++) {
      childid = *child;
      // This call is expensive
      //      if ( sch.IsSameProcessor(parentid, childid) ) {
      // if (clusterID_list[parentid] == clusterID_list[childid]) {
      if (sch.IsSameProcessor(parentid, childid)) {
        edge_wt = 0.0;
      } else {
        edge_wt = annoted_graph.GetEdgeWeight(parentid, childid).GetMessageSize();
      }
      if (edge_wt + blevel_list[childid] > maxval)
        maxval = edge_wt + blevel_list[childid];
    }
    taskwt = annoted_graph.GetData(parentid).GetTaskWeight();
    blevel_list[parentid] = taskwt + maxval;
  }
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
void
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
DisplayMappingVec() {
  cout<<"Mapping Vector:"<<endl;
  for(int i=0; i<mappingvec.size(); ++i) {
    cout<<" PID "<<i<<" (CIDs:";
    for(int j=0; j<mappingvec[i].size(); ++j)
      cout<<" "<<mappingvec[i][j];
    cout<<" )"<<endl;
  }
}


template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
double
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
GA_Mapping(int ntiter, int niter) {

  double mksp=INFINITE;
 /*   Schedule bestmappedschedule;
  Schedule bestsched;
  TASKWEIGHT keepcompcost,keepcommcost;
  for(int i=0;i<ntiter; i++) {

    //initial random mapping, first p clusters assigned to each proc
    //rest random
    int j=0;
    unsigned int seed = time(NULL);
    srand(seed);
    for(GII si = schedule.tasksetvector().begin(); 
	si != schedule.tasksetvector().end();
	si++, j++) {
      if(j<machineinfo->nodesize) si->second.SetPpid(j);
      else {
	int rdn = rand();
	int rpid = rdn%machineinfo->nodesize;
	si->second.SetPpid(rpid);
      } 
    }
    InitializeMappingVec();
    AnnoteGraphwithPid();

    double tmksp=SwappingPids(niter);
    if(tmksp < mksp) {
      mksp = tmksp;
      bestsched = schedule;
      bestmappedschedule = mapped_schedule;
	  keepcompcost = totalcompcost;
	  keepcommcost = totalcommcost;

    }
  }

  schedule = bestsched;
  mapped_schedule = bestmappedschedule;
  AnnoteGraphwithPid();
#ifdef DEBUG_MAPPING
  cout<<"Annoted Graph:"<<endl;
  annoted_graph.DisplayGraph();
  cout<<"Best Mapped Schedule"<<endl;
  mapped_schedule.DisplaySchedule();
  totalcompcost = keepcompcost;
  totalcommcost = keepcommcost;
  cout<<"Totalcomp/commcost:"<<totalcompcost<<" "<<totalcommcost<<endl;
  cout<<"Final Makespan "<<endl;
#endif
cout<<mksp<<endl; */
  return mksp;
}

template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
bool
TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
MyFind(const vector<pair<pair<PID,int>, pair<PID,int> > >& acceptedswaps,
       const pair<pair<PID,int>, pair<PID,int> >& tpr) {
  vector<pair<pair<PID,int>, pair<PID,int> > >::const_iterator vpi;
  for(vpi=acceptedswaps.begin(); vpi!=acceptedswaps.end(); vpi++){
    if((vpi->first == tpr.first && 
	vpi->second == tpr.second) || 
       (vpi->first == tpr.second && 
	vpi->second == tpr.first ) ) return true;
  }
  return false;
};


}  //end namespace

#endif
