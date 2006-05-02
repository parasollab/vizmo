#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"

#include "pGraph.h"
#include "pGraphAlgo.h"
#include "pArray.h"

#include <sys/time.h>

using namespace stapl;

int SZ,NP;

int getOwner(VID _v){
  //return (_v % NP);

  //for m91
  if(NP==2){
    //--- np = 2
    if(_v < SZ/2) return 0;
    else return 1;
  }
  else if(NP==4){
    //- np=4
    if(_v < SZ/4) return 0;
    else if (_v < 2*SZ/4) return 1;
    else if (_v < 3*SZ/4) return 2;
    else return 3;
  }
  else if(NP==6){
    //- np = 6
    if(_v < SZ/6) return 0;
    else if (_v < 2*SZ/6) return 1;
    else if (_v < 3*SZ/6) return 2;
    else if (_v < 4*SZ/6) return 3;
    else if (_v < 5*SZ/6) return 4;
    else return 5;
  }
  else if(NP==8){
    //- np = 8
    if(_v < SZ/8) return 0;
    else if (_v < 2*SZ/8) return 1;
    else if (_v < 3*SZ/8) return 2;
    else if (_v < 4*SZ/8) return 3;
    else if (_v < 5*SZ/8) return 4;
    else if (_v < 6*SZ/8) return 5;
    else if (_v < 7*SZ/8) return 6;
    else return 7;
  }
  else return 0;
}

template <class G>
class visitor: public visitor_base<G>{
  //this class is used by DFS/BFS
  G* _g;
public:  
  visitor(G& g){
      _g = &g;
  };
  inline int vertex (typename G::VI v) {
    timeval tv;
    gettimeofday(&tv,NULL);
    printf("%d --> %d %d %d\n",tv.tv_usec,v->vid,stapl::get_thread_id(),_g->getHandle());
    return 1;
  }
};


void stapl_main(int argc,char** argv){

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;
  typedef GraphPart<GRAPH,Weight2> GPART;


  Task data;
  Weight2 wt;

  int i=2;   
  int id=stapl::get_thread_id();
  NP=stapl::get_num_threads();

  printf("mpirun -np P test_adddel N\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);


  GRAPH g;
  PGRAPH pg;
  PGRAPH pg1;
  stapl::rmi_fence();


  //first measure the overhead of the pgraph compared with graph
  
  stapl::timer tm;
  double elapsed;
  int s;
  
  tm = stapl::start_timer();
  for(i=0;i<SZ;i++){
    pg.AddVertex(data);
  }
  elapsed = stapl::stop_timer(tm);
  stapl::rmi_fence();
  cout<<"Add vertices elapsed time par:"<<elapsed<<endl;

  tm = stapl::start_timer();
  for(i=0;i<SZ;i++){
    g.AddVertex(data);
  }
  elapsed = stapl::stop_timer(tm);
  cout<<"Add vertices elapsed time seq:"<<elapsed<<endl;

  pArray<VID> pverts;
  vector<VID> verts;
  stapl::rmi_fence();

  tm = stapl::start_timer();
  s=pg.GetVertices(pverts);
  pArray<VID>::iterator vit = pverts.local_begin();
  for(i=0;i < pverts.local_size();i++){
    pg.DeleteVertex(*vit);
    ++vit;
  }
  elapsed = stapl::stop_timer(tm);
  cout<<"Delete  vertex time par:"<<elapsed<<endl;
  stapl::rmi_fence();
  verts.clear();


  tm = stapl::start_timer();
  s=g.GetVerticesVID(verts);
  for(i=0;i<s;i++){
    g.DeleteVertex(verts[i]);
  }
  elapsed = stapl::stop_timer(tm);
  stapl::rmi_fence();
  cout<<"Delete vertex time par:"<<elapsed<<endl;

  tm = stapl::start_timer();
  for(i=0;i<SZ;i++){
    pg.AddVertex(data);
  }
  elapsed = stapl::stop_timer(tm);
  stapl::rmi_fence();
  cout<<"Add vertices elapsed time par:"<<elapsed<<endl;

  //pg.pDisplayGraph();

  return;
}



