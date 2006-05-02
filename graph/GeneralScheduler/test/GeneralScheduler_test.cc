/*!
  @ingroup stapl

  Testing General Scheduler Methods

  NOTE:
  The library is under construction

  Currently Testing Schedulin/clustering methods:
  CPM, DSC, FLB, FCP

  Should add tests for Mapping mthods LLB and GA in the future

  @author Ping An
   copyright Texas A&M University 2004/11/06
*/

#include "SchedulerDefines.h"
#include "Scheduler.h"

#define TASKWEIGHT double
#define COMMWEIGHT double
#define MAX_NPROCS 100

using namespace scheduler;
typedef SchedulerType<TASKWEIGHT,COMMWEIGHT,double,double> Scheduler;
typedef Scheduler::WtDiGraph MyWtDiGraph;

void stapl_main(int argc,char** argv){

  if(argc < 5) {
    cout<< "Usage: exe" <<endl;
    cout<< "System size:"<<endl;
    cout<< "Benchmark Graph?"<<endl;
    cout<< "ddg name"<<endl;
    cout<< "Clustering_method:"<<endl;
    cout<< "1:CPM 2:DSC 3:FLB 4:FCP" <<endl; 
    exit(1);
  }
  
  //=================================================
  ///Building up the system graph to schedule the DAG on
  //=================================================
  int tnp = atoi(argv[1]);

  int i,j;
  int nnprocs =0;
  Sys0 m0[MAX_NPROCS];
  for(i=0; i<tnp; i++) {
    m0[i].systemid=nnprocs++;
    //can give a random number for cpu speed
    m0[i].speed =i+1;
    m0[i].level=0;
    m0[i].leadpid = m0[i].systemid;
  }

  Sys1 m1;
  m1.systemid = 0;
  m1.nodesize=tnp;
  m1.level = 1;
  for(i=0;i<tnp;i++) { 
    m1.commgraph.AddVertex( m0[i] );
  }
  m1.leadpid = (m1.commgraph.v.begin())->data.leadpid;
  for(i=0;i<tnp;i++) {
    for(j=i+1;j<tnp;j++) {
      //can give a random number for network speed
      m1.commgraph.AddEdge(i,j,j);
    }
  }

  //=================================================
  ///Read in the DAG from file
  ///if bg == 1, read a benchmark graph (index starts from 1)
  ///else read a normal graph @ref BaseGraph.h
  //=================================================
		      
  MyWtDiGraph dag;
  int bg = atoi(argv[2]);
  if(bg==0) dag.ReadGraph(argv[3]);
  else dag.ReadBenchMarkGraph(argv[3]);

  //=================================================
  ///Schedule the DAG and check correctness of the schedule
  //=================================================
  Scheduler sched(dag,&m1);

  GenClustAlgType ct = (GenClustAlgType) atoi(argv[4]);
  Schedule s1 = sched.Clustering(ct,tnp);

#ifdef CHECKSCHEDULE
  vector<VID> missvid;
  vector<VID> dupvid;
  vector<VID> vidlist;
  dag.GetVerticesVID(vidlist);

   if(!s1.CheckSchedule(vidlist,missvid,dupvid)) {
    IVID iv;
    for(iv = missvid.begin(); iv!=missvid.end(); iv++ )
      cout<<"Missing "<<*iv<<endl;
    for(iv = dupvid.begin(); iv!=dupvid.end(); iv++ )
      cout<<"Duplicating "<<*iv<<endl;
  } else {
    cout<<"Correct Schedule. Clustering result"<<endl;
    s1.DisplaySchedule();
  }
#endif

   TaskAllocator<TASKWEIGHT,COMMWEIGHT,double,double> alloc_sched(s1,&m1,dag);

   // double makespan = alloc_sched.GetMakespan();
  //alloc_sched.GetAnnotedGraph().DisplayGraph();
   // cout<<"Makespan is :"<<makespan<<endl;

}

