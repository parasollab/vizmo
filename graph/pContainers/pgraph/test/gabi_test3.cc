#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include "../pGraph.h"
#include "../pGraphAlgo.h"
#include "../pSCC.h"
#include "../pSCCgeneric.h"
#include "../GraphPartitioner.h"

#include <strings.h>

int NP;
int SZ;

using namespace stapl;

void readChacoPartition(char* file_name, int size, vector<int>& mp){
  char * actual_file_name=new char[100];
  sprintf(actual_file_name,"%s.chaco.%d",file_name,stapl::get_num_threads());
  printf("Reading metis partition from file [%s]\n",actual_file_name);
  ifstream _myistream(actual_file_name);
  
  if(!_myistream.is_open()){
    cout<<"Error Opening Input File "<<actual_file_name<<endl;
    return;
  }
  int temp;
  for(int i=0;i<size;i++){
    _myistream>>temp;
    mp.push_back(temp);
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
  int i; 

  vector<PGRAPH*> vpg(60); 
  stapl::GraphPartitioner<UGRAPH> GP;
  map<VID,PID> _pp;
  map<VID,PID> _pp1;

  vector<PID> active0;

  for(i=0;i<NP/2;i++){
    active0.push_back(i);
    cout<<"Active_0:"<<i<<endl;
  }
  //active0.push_back(1);
 
  vector<PID> active1;
  for(i=NP/2;i<NP;i++){
    active1.push_back(i);
    cout<<"Active_1:"<<i<<endl;
  }
  //active1.push_back(2);
  //active1.push_back(3);


  GP.BlockPartition(SZ,_pp,active0);
  GP.BlockPartition(SZ,_pp1,active1);

  map<VID,PID>::iterator mit = _pp1.begin();
  //while(mit != _pp1.end()){
  //  cout<<mit->second<<endl;
  //  mit++;
  //}
  

  stapl::rmi_fence();
 

  if(id==0) printf("Using aggregation factor [%s]\n",argv[3]);
  int aggf = atoi(argv[3]);
  

  int fromdir = atoi(argv[4]);
  if(id==0) printf("Using from direction %d \n",fromdir);

  int ndirs = atoi(argv[5]); 
  if(id==0) printf("Using %d directions\n",ndirs);
  stapl::set_aggregation(aggf);
  for(i=fromdir;i<ndirs;i++){
    bzero(crude,100);
    sprintf(crude,"suite22/c22_%d",i*3);
    if(id==0) printf("Read file [%s]\n",crude);
    vpg[i-fromdir] = new PGRAPH;
    //stapl::rmi_fence();
    if(i%2 == 0){
      PGRAPH* & __x=vpg[i-fromdir];
      __x->Split_Crude_Exo<GRAPH>(crude,_pp,0);
      //vpg[i-fromdir]->Split_Crude_Exo<GRAPH>(crude,_pp,0);
    }
    else{
      PGRAPH* & __x=vpg[i-fromdir];
      __x->Split_Crude_Exo<GRAPH>(crude,_pp1,0);
      //vpg[i-fromdir]->Split_Crude_Exo<GRAPH>(crude,_pp1,0);
    }
  }

  stapl::rmi_fence();
  //stapl::set_aggregation(aggf);  

  //return;
  stapl::timer t;
  double t1;

  printf("set traversal number start\n");
  vector<vector<pair<VID,int> > > SCC;

  

  vector<trimer<PGRAPH>* > vtr;
  for(i=fromdir;i<ndirs;i++){
    trimer<PGRAPH>* tr1 = new trimer<PGRAPH>(vpg[i-fromdir]);
    vtr.push_back(tr1);
    tr1->sccg.SetTraversalNumber(NP);
  }
  printf("set traversal number STOP\n");
  

  //pSCC<PGRAPH,trimer<PGRAPH> > pscc;
  pSCCgeneric<PGRAPH> pscc;

  stapl::rmi_fence();
  printf("start scc\n");
  t = stapl::start_timer();
  pscc.run(vtr,SCC,aggf);
  t1 = stapl::stop_timer(t);      
  printf("Elapsed time %f\n",t1);

  stapl::rmi_fence();
  pscc.collect(SCC,vtr.size(),SZ);
  pscc.dump("res",0);

  //g.pFileWrite("out_4_",12);  
  return;
}
