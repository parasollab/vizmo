/*!
        \file test_pgraphalgo.cc
	\author Gabriel Tanase	
        \date Feb. 24, 04
        \ingroup stapl
        \brief Test suite for pGraph algorithms.
	\mainly the algorithms required to parallelize protein folding codes

*/
//#undef STAPL_DEBUG
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "pContainers/pgraph/pGraphAlgo.h"

using namespace stapl;

int NP,SZ;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}

template <class G>
class visitor_test: public visitor_base<G>{
  //this class is used by DFS/BFS
  //G* _g;
public:  
  //visitor(){
    //_g = &g;
  //}
  inline int vertex (typename G::VI v) {
    timeval tv;
    gettimeofday(&tv,NULL);
    //printf("%d @@ %d\n",tv.tv_usec,v->vid);
    return 1;
  }
  inline int finish_vertex(typename G::VI v,int tt){
    return 1;
  }

  void define_type(stapl::typer &t) {
    //cout<<"Ask Nathan if this is correct"<<endl;
    //t.local(_g);
  }

};



void stapl_main(int argc,char** argv){


  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data,data1;
  Weight2 wt;

  int i,j,tmp,localedges;   
  int id=stapl::get_thread_id();
  vector<int> vertices;
  NP=stapl::get_num_threads();
  int myid=stapl::get_thread_id();
  PGRAPH::VI vi,vi1;
  PGRAPH::EI ei,ei1;
  PGRAPH::CVI cvi;

  PGRAPH pg,pg_copy;
  stapl::rmi_fence();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  stapl_print("testing AddVertex...");
  for(i=0;i<SZ;i++){
    tmp = i+myid*SZ;
    data.SetWeight(tmp);
    j = pg_copy.AddVertex(data);
    j = pg.AddVertex(data);
    vertices.push_back(j);
  }
  stapl::rmi_fence();
  //here we should have NP*SZ vertices
  for (i=0;i<SZ*NP;i++){
    if(!pg.IsVertex(i)){
      cout<<"ERROR while testing AddVertex/IsVertex"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  for(vi = pg.local_begin(); vi != pg.local_end();++vi){
    vi->data.SetWeight(vi->vid);
  }
  for(vi = pg_copy.local_begin(); vi != pg_copy.local_end();++vi){
    vi->data.SetWeight(vi->vid);
  }

  localedges = 0;

  //                       the graph will be a ring
  stapl_print("testing AddEdge...");
  for (i=0;i<SZ;i++){
    tmp = vertices[i] * ((vertices[i] + 1)%(NP*SZ));
    wt.SetWeight(tmp);
    pg.AddEdge(vertices[i],(vertices[i]+1)%(NP*SZ),wt);
    localedges++;
  }
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
    if(!pg.IsEdge(vertices[i],(vertices[i]+1)%(NP*SZ)))
      cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
    //if(!pg.IsEdge( (vertices[i]+1)%(NP*SZ),vertices[i]) )
    //cout<<"ERROR while testing AddEdge/IsEdge(2)"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  //pg.pDisplayGraph();
  
  vector<VID> succ;
  vector<VID> succ1;
  stapl_print("testing DFS_traversal...");
  visitor_test<PGRAPH> vis1;
  //tmp = 5 ;
  //tmp = pDFSfunctor (pg, vis1, tmp);
  pg.SetTraversalNumber(NP);
  pg.ResetTraversals();
  tmp = pg.local_begin()->vid;
  //tmp = 2;
  pg.DFS_traversal(tmp,vis1,myid,false);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing IsSameCC...");
  pIsSameCC sameCC;
  stapl::rmi_fence();
  for(vi = pg.local_begin();vi != pg.local_end(); ++vi){
    bool t = sameCC.IsSameCC(pg,vi->vid,0);
    if(t != true)
      cout<<"ERROR IsSameCC Didn't find a path on thread:"<<myid<<"for:"<<vi->vid<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing IsSameCCData...");
  for(vi = pg.local_begin();vi != pg.local_end(); ++vi){
    data = pg.GetData(0);
    bool t = sameCC.IsSameCCData(pg,vi->data,data);
    if(t != true)
      cout<<"ERROR IsSameCCData Didn't find a path on thread:"<<myid<<"for:"<<vi->vid<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetCC...");
  pGetCC<PGRAPH> getCC;
  stapl::rmi_fence();
  for(vi = pg.local_begin();vi != pg.local_end(); ++vi){
    tmp = getCC.GetCC(pg,vi->vid,succ);
    if(tmp != NP*SZ)
      cout<<"ERROR Didn't find right CC :"<<myid<<"for:"<<vi->vid<<endl;
    if(succ.size() != NP*SZ)
      cout<<"ERROR(2) Didn't find right CC :"<<myid<<"for:"<<vi->vid<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetCCData...");
  vector<Task> vdata;
  stapl::rmi_fence();
  for(vi = pg.local_begin();vi != pg.local_end(); ++vi){
    tmp = getCC.GetCC(pg,vi->data,vdata);
    if(tmp != NP*SZ)
      cout<<"ERROR Didn't find right CC Data :"<<myid<<"for:"<<vi->vid<<endl;
    if(succ.size() != NP*SZ)
      cout<<"ERROR(2) Didn't find right CC Data:"<<myid<<"for:"<<vi->vid<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetCCcount...");
  pGetCCcount<PGRAPH> pgccc;
  cout<<"thread id"<<myid<<":"<<pgccc.GetCCcount(pg)<<endl;
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetCCstats...");
  pvector<pair<int,VID> > stats;
  pGetCCStats<PGRAPH> pgs;
  cout<<"STATS id"<<myid<<":"<<pgs.GetCCStats(pg,stats)<<endl;
  stapl_print("Passed\n");

  stapl_print("testing pDisplayCCstats...");
  pDisplayCCStats(pg,pgs);
  stapl_print("Passed\n");

  stapl_print("testing pDisplayCC...");
  pDisplayCC(pg,getCC,pg.local_begin()->vid);
  stapl::rmi_fence();
  stapl_print("Passed\n");
  
  cout<<"Finished:"<<myid<<endl;
  return;
}
