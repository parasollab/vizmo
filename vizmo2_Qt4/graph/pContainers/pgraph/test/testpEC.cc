#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "../../../include/pContainers/pgraph/pEC.h"
#include "../../../include/pContainers/pgraph/GraphPartitioner.h"


#include <sys/time.h>

using namespace stapl;

int SZ, NP;

void stapl_main(int argc, char** argv)
{
  typedef Graph<DG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task, Weight2> DGRAPH;
  typedef Graph<UG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task, Weight2> UGRAPH;
  typedef pGraph<PDG<Task,Weight2>, PMG<Task,Weight2>, PWG<Task,Weight2>, Task, Weight2> PDGRAPH;

  Task data;
  Weight2 wt;

  int i=2;
  int id=stapl::get_thread_id();
  NP = stapl::get_num_threads();

  printf("main started\n");

  SZ = atoi(argv[1]);
  
  PDGRAPH pg;
  stapl::rmi_fence();

  //simple partition
  map<VID, PID> _pp;
  for(int r=0; r<SZ; r++)
    _pp[r] = (r*NP)/SZ; 
  
/*
  stapl::GraphPartitioner<DGRAPH> GP;
  DGRAPH _dg;
  _dg.ReadDotGraph(argv[2]);
  map<VID,PID> _pp;
  GP.MetisPartition(_dg, _pp);
*/

  stapl::set_aggregation(50);	//?????
  
  pg.Split_Crude_Dot<DGRAPH>(argv[2], _pp, 0);	//?????

  stapl::rmi_fence();
  pg.pFileWrite("pgout", 10);
//  pg.pDisplayGraph();
  stapl::rmi_fence();

  int ret = pFindEC<PDGRAPH>(pg);
  stapl::rmi_fence();
  if (ret == -1) 
	  cout<<"THREAD #"<<stapl::get_thread_id()<<" says: NO EC!"<<endl;
  else 
	  cout<<"THREAD #"<<stapl::get_thread_id()<<" says: HAS EC!"<<endl;

  stapl::rmi_fence();
  cout<<"THREAD #"<<stapl::get_thread_id()<<" FINISH!!!"<<endl;
  return ;

}
