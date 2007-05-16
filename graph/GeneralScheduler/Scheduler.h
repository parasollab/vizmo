/*!
	\file Scheduler.h
	\author Ping An	
	\date Jan. 28, 02
	\ingroup scheduler
	\brief Generic scheduler w/ parallel/hierarchical scheduling

	Base class Scheduler 

	0	Partitioning/clustering with general scheduling algorithms
	1       Allocation/mapping onto physical processors 
	        with general mapping algs.
	2       Aggregate into communication units on each processor
	3       Ordering tasks in each communication unit

	4	Function to get optimal makespan (critical path)
	5	Schedule correctness checking 
		(not scheduled or scheduled more than once)
	6       Estimate Makespan

	should get a ddg from prange, and output cdg to prange 
	to do distribution and execution

	Input:
	  a Graph (with format defined by Graph.h & Task.h);
	  MachineInfo

	Output:
	  Coarse Dependence Graph (CDG)

	Note:
	  input graph is preprocessed by calling GraphPreprocessing() in the
	  constructor for cycle detection, set predecessors, compute level 
	  info etc.
*/
#ifndef Scheduler_h
#define Scheduler_h

#include <sys/time.h>
#include "TaskScheduler.h"
#include "TaskAllocator.h"

namespace scheduler {

//===================================================================
//===================================================================
template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
class SchedulerType
{

 public:
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

  typedef  TaskScheduler<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    TaskScheduler_type;
  typedef  TaskAllocator<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    TaskAllocator_type;
  typedef  SchedulerGraphMethods<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO> 
    SchedulerGraphMethods_type;

//===================================================================
//  Data
//===================================================================
protected:

	MachineInfo* machineinfo;
	WtDiGraph ddg;

//===================================================================
//  Constructors and Destructor
//===================================================================
public:
        SchedulerType();
	SchedulerType(MachineInfo*);
	SchedulerType(const WtDiGraph&, MachineInfo*);
        ~SchedulerType();

//===================================================================
//  Other Methods
//===================================================================
	MachineInfo* GetMachineInfo() const { return machineinfo;};
	void SetMachineInfo(MachineInfo* _m) { machineinfo=_m;};

/* 	const WtDiGraph& GetDDG() const { return ddg; }; */
	WtDiGraph& GetDDG() { return ddg; };
	
	void SetDDG(const WtDiGraph _g) { 
	  ddg = _g;  
/*	  SchedulerGraphMethods_type sgm;
	  sgm.GraphPreprocessing(ddg);
*/	};

	Schedule Clustering(GenClustAlgType, int np=0);

	double Allocating(GenAllocAlgType,const Schedule&, WtDiGraph&
			  ,Schedule& newschedule,
			  int ntiter=0, int niter=0);

/* 	vector<VID> Ordering(const WtDiGraph&); */

	/*! Aggregate graph into a graph of chunks, based 
	   on aggregation factor vector, the number of nodes in each chunk is
	   non-uniform, but the aggregation factor vector is the same 
	   across processors
	*/
/* 	virtual CWtDiGraph Aggregation(const vector<int>&, */
/* 					  const WtDiGraph&) = 0; */

/* 	double GetMakeSpan(const CWtDiGraph) {return 0;}; */


private:

};


  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  SchedulerType() {
  }

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  SchedulerType(MachineInfo* _m)  { 
    machineinfo= _m; 
  }

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  SchedulerType(const WtDiGraph& _g, MachineInfo* _m) {
    ddg = _g;
    machineinfo = _m;
  }

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  ~SchedulerType() {
  }

//===================================================================
//  public Methods
//===================================================================
  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  Schedule
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  Clustering(GenClustAlgType _p, int np) {
    SchedulerGraphMethods_type sgm;
    map<VID,VID> vidmap;
    WtDiGraph nddg=ddg;
    bool trans = sgm.GraphTransform(ddg,nddg,vidmap);    
    sgm.GraphPreprocessing(nddg);
/*      nddg.DisplayGraph();  */
    TaskScheduler_type tsched(nddg);
    Schedule s1;
    double elapsed;
    timer tm;
    switch(_p) {
    case WFM:
      //    s1=tsched.WaveFront_BFS();
      break;
    case CPM:
      s1=tsched.CriticalPathClustering();
      break;
    case DSC:
      s1=tsched.DominantSequence();
      break;
    case FLB:
      s1=tsched.FastLoadBalance(np);
      break;
    case FCP:
      tsched.AssignALAPLevel();
      tm = stapl::start_timer();
      s1=tsched.FastCriticalPath(np);
      elapsed = stapl::stop_timer(tm);
      // cout<<"scheduling time:"<<elapsed<<endl;
      break;
    case HEM:
      //    s1=tsched.HeavyEdgeMerge();
      break;
    case OTHER_CLUST:
      //    s1=*user_defined_cluster();
      break;
    case NIL_CLUST:
      //    using DSC if no preference
      s1=tsched.DominantSequence();
      break;
    default:
      cout<<"\nClustering Choice is not supported"<<endl;
      exit(1);
      break;
    }

    if(trans) s1.ScheduleTransform(vidmap);

#if STAPL_DEBUG
    s1.DisplaySchedule();
#endif
    return s1;
  }

  template<class TASKWEIGHT, class COMMWEIGHT, class TASKINFO,class COMMINFO>
  double
  SchedulerType<TASKWEIGHT,COMMWEIGHT,TASKINFO,COMMINFO>::
  Allocating(GenAllocAlgType _p,const Schedule& s, WtDiGraph& agraph,
	     Schedule& newschedule,
	     int ntiter, int niter) {
    TaskAllocator_type talloc(s,machineinfo,ddg);
    double mksp=-1;
    switch(_p) {
    case RAND:
      mksp=talloc.Randmized_Mapping(niter);
/*       newschedule = talloc.GetMappedSchedule(); */
/*       agraph = talloc.GetAnnotedGraph(); */
      return mksp;
      break;
     case GA:
      mksp=talloc.GA_Mapping(ntiter,niter);
      newschedule = talloc.GetMappedSchedule();
      agraph = talloc.GetAnnotedGraph();
      return mksp;
      break;
    case LLB:
      newschedule = talloc.MapbyLLB();
      mksp = talloc.GetMakespan(newschedule);
      agraph = talloc.GetAnnotedGraph();
      return mksp;
      break;
    case OTHER_ALLOC:
      newschedule = talloc.MapbyLLB();
      mksp = talloc.GetMakespan(newschedule);
      agraph = talloc.GetAnnotedGraph();
      return mksp;
      break;
    case NIL_ALLOC:
      //use LLB if no preference
/*       newschedule = talloc.MapbyLLB(); */
      mksp = talloc.GetMakespan();
      agraph = talloc.GetAnnotedGraph();
      return mksp;
      break;
    default:
      cout<<"\nMapping choice is not supported"<<endl;
      exit(1);
      break;
    }
    return mksp;
  }

} //end namespace
#endif
