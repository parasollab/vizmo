#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include "pGraph.h"
#include "pGraphAlgo.h"
#include "pSCC.h"


int NP;
int SZ;

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
    printf("--------> %d %d\n",v->vid,_g->myid);
    return 1;
  }
};


void stapl_main(int argc,char** argv){

  char crude[100];
  char dummy[100];
  typedef pGraph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> PGRAPH;
      
  using namespace stapl;

  int id=stapl::get_thread_id();
  NP=stapl::get_num_threads();
 


  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);

  printf("main started\n");


  PGRAPH g;
  stapl::rmi_fence();
  Task data;
  int j; 

  vector<int> to_go;
  for(int i=0; i<50000; ++i){
    to_go.push_back(getOwner(i));  
  }
  printf("The file to be split [%s]\n",argv[2]);
  g.Split_Crude(argv[2],to_go);

  sprintf(crude,"%s_%d_",argv[2],NP); 

  g.pFileWrite(crude);
}
