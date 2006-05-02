#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include "Graph.h"
#include "../pGraph.h"
//#include "../pGraphAlgo.h"
//#include "../pSCCmulticc.h"
#include "../pSCCsingle.h"

#include <strings.h>

using namespace stapl;

int NP;
int SZ;

void stapl_main(int argc,char** argv){

  char crude[100];
  char dummy[100];
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;
  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;

  using namespace stapl;

  int id=stapl::get_thread_id();
  NP=stapl::get_num_threads();
  sprintf(crude,"crude%d",id); 
  sprintf(dummy,"dummy%d",id);
  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);

  stapl::rmi_fence();
  Task data;
  int i,n_pgraphs; 

  vector<PGRAPH*> vpg(60); 
  stapl::rmi_fence();
 

  if(id==0) printf("Using aggregation factor [%s]\n",argv[3]);
  int aggf = atoi(argv[3]);
  

  int fromdir = atoi(argv[4]);
  if(id==0) printf("Using from direction %d \n",fromdir);

  int ndirs = atoi(argv[5]); 
  if(id==0) printf("Using %d directions\n",ndirs);

  n_pgraphs = ndirs - fromdir;

  stapl::set_aggregation(aggf);
  stapl::set_poll_rate(100);

  for(i=fromdir;i<ndirs;i++){
    bzero(crude,100);
    sprintf(crude,"%s%d",argv[2],i);
    if(id==0) printf("Read file [%s]\n",crude);
    vpg[i-fromdir] = new PGRAPH;
    stapl::rmi_fence();
    //shortcut for aCC
    PGRAPH* & __x=vpg[i-fromdir];
    //__x->Split_Crude_Dot<GRAPH>(argv[2],_pp,0);
    //__x->Split_Crude_Exo<GRAPH>(crude,_pp,0);
    __x->pFileRead(crude, 10);
  }

  stapl::rmi_fence();

  stapl::timer t;
  double t1;

  vector<map<VID,vector<VID> > >  SCC;
  vector<vector<pair<VID,VID> > > Edges;
  pSCCWorkFunction<PGRAPH> pf;

  t = stapl::start_timer();

  pSCC_single(vpg,n_pgraphs,Edges,SCC,&pf,aggf,false);

  t1 = stapl::stop_timer(t);      

  printf("Elapsed time %f\n",t1);
  return;
}



/*
  stapl::GraphPartitioner<UGRAPH> GP;
  map<VID,PID> _pp;

  PGRAPH pp1;
  pp1.AddPart();

  //UGRAPH _ug;
  //_ug.ReadExoGraph(argv[2]);
  //GP.MetisPartition(_ug,_pp);

  //GP.BlockPartition(SZ,_pp);
  //GP.ChacoPartition(SZ,argv[2],_pp);
  //map<VID,PID>::iterator mit = _pp.begin();
  //while(mit != _pp.end()){
  //  cout<<mit->second<<endl;
  //  mit++;
  //}
  

*/


//set MPI_DEBUG_CONT=1
//break modified_dcsc
