#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "../../../include/pContainers/pgraph/pST.h"


#include <sys/time.h>

using namespace stapl;

int SZ, NP;

void stapl_main(int argc, char** argv)
{
  typedef Graph<DG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task, Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task, Weight2> UGRAPH;
  typedef pGraph<PDG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task, Weight2> PDGRAPH;
  typedef pGraph<PUG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task, Weight2> PUGRAPH;

  Task data;
  Weight2 wt;

  int i=2;
  int id=stapl::get_thread_id();
  NP = stapl::get_num_threads();

  printf("main started\n");

  SZ = atoi(argv[1]);
  
  PUGRAPH pg;
  PUGRAPH::VI vi;
  stapl::rmi_fence();

  //simple partition
  map<VID, PID> _pp;
  for(int r=0; r<SZ; r++)
    _pp[r] = (r*NP)/SZ; 

  stapl::set_aggregation(50);	//?????
  
  pg.Split_Crude_Dot<UGRAPH>(argv[2], _pp, 0);	//?????

  stapl::rmi_fence();

//  pg.pDisplayGraph();
  stapl::rmi_fence();

  // ST test 
  PUGRAPH  st;

  timeval tv1,tv2, elapsed;
  gettimeofday(&tv1, NULL);
  //int ret = pFindMST<PUGRAPH>(pg, st);
  int ret = pFindST<PUGRAPH>(pg, st);
  gettimeofday(&tv2, NULL);
  
  if (tv1.tv_usec > tv2.tv_usec) { tv2.tv_usec += 10000000; tv2.tv_sec--;}
  elapsed.tv_usec = tv2.tv_usec - tv1.tv_usec;
  elapsed.tv_sec = tv2.tv_sec - tv1.tv_sec;

  cout<<"# "<<get_thread_id()<<" TIME: "<<(elapsed.tv_sec*1000000+elapsed.tv_usec)<<endl;


  stapl::rmi_fence();
  return ;

}
