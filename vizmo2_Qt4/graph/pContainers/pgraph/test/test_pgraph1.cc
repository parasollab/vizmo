#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "pGraphAlgo.h"


#include <sys/time.h>

using namespace stapl;

int SZ,NP;

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

  void define_type(stapl::typer &t){
  }
};


#include "pContainers/pgraph/SubpGraphIterator.h"

void 	Sub_pGraph_Iterator_Test()
{


     	typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;
     	typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;



	if(false)
	{
		GRAPH g;
		g.AddVertex(Task(100),0);	g.AddVertex(Task(101),1);	g.AddVertex(Task(102),2);	g.AddVertex(Task(103),3);	g.AddVertex(Task(104),4);
		g.AddEdge(0,1,Weight2(1000));	g.AddEdge(0,2,Weight2(2000));	g.AddEdge(2,3,Weight2(3000));	g.AddEdge(1,4,Weight2(4000));
		g.DisplayGraph();

	        vector<pair<VID,VID> > bdry;
	        pair<VID,VID> p;
	/*        p.first=1; p.second=2; bdry.insert(p);	p.first=9; p.second=8; bdry.insert(p);	p.first=24; p.second=23; bdry.insert(p); */

		//Construct the SubpGraph which represents only the boundary of the SubpGraph
	        //SubpGraph<GRAPH> subgraph(&g,bdry,0);

		//Iterator for SubpGraph
	        //SubpGraphIterator< SubpGraph<GRAPH> > RealIt;

		//Iterate through SubpGraph
	        //for(RealIt=subgraph.begin(); RealIt!=subgraph.end();    RealIt++)
		//{                cout<<"VISITING "<<RealIt.GetVID()<<endl;        }
	}

	PGRAPH pg;	
	if(stapl::get_thread_id()==0)
	{

		cout<<" Testing Sub-pGraph Iterator "<<endl;
		
		Task t;
		t=Task(100);

		pg.AddVertex(t,0);
		t=Task(101);

		pg.AddVertex(t,1);
		t=Task(102);

		pg.AddVertex(t,2);

		t=Task(103);
		pg.AddVertex(t,3);

		t=Task(104);
		pg.AddVertex(t,4);

		t=Task(105);
		pg.AddVertex(t,5);

		t=Task(106);
		pg.AddVertex(t,6);

		t=Task(107);
		pg.AddVertex(t,7);

		t=Task(108);
		pg.AddVertex(t,8);
		
		t=Task(109);
		pg.AddVertex(t,9);


		pg.AddEdge(0,1);		
		pg.AddEdge(0,2);
		pg.AddEdge(2,1);
		pg.AddEdge(2,5);
		pg.AddEdge(2,3);
		pg.AddEdge(1,4);
		pg.AddEdge(5,3);
		pg.AddEdge(5,6);
		pg.AddEdge(5,1);
		pg.AddEdge(3,4);
		pg.AddEdge(3,1);
		pg.AddEdge(4,7);
		pg.AddEdge(4,8);
		pg.AddEdge(6,7);
		pg.AddEdge(7,9);
		pg.AddEdge(8,9);


		



 	       vector<pair<VID,VID> > bdry;
  	      	pair<VID,VID> p;
		p.first=5; p.second=6;                bdry.push_back(p);
		p.first=3; p.second=7;                bdry.push_back(p);
		p.first=4; p.second=7;                bdry.push_back(p);
		p.first=4; p.second=8;                bdry.push_back(p);
		
		//Construct the SubpGraph which represents only the boundary of the SubpGraph
 	        //SubpGraph<PGRAPH> subgraph(&pg,bdry,0);

		//Iterator for SubpGraph
	        //SubpGraphIterator< SubpGraph<PGRAPH> > RealIt;

		//SubpGraphIterator< SubpGraph<PGRAPH> >::value_type  pp(1000);
		//cout<<pp<<endl;

		//Iterate through SubpGraph
	        //for(RealIt=subgraph.begin(); RealIt!=subgraph.end();    RealIt++)
 	        //{
  	        //      cout<<"Visiting Vertex "<<RealIt.GetVID()<<endl;
		//}

		cout<<" Testing Sub-pGraph Iterator :: OVER :: "<<endl;

	}


stapl::rmi_fence();	
}

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}


void stapl_main(int argc,char** argv){

  Sub_pGraph_Iterator_Test();

  char crude[100];
  char dummy[100];

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data;
  Weight2 wt;

  int sz,sz1,i=2;   
  int id=stapl::get_thread_id();
  NP=stapl::get_num_threads();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);
  printf("Reading graph from file %s\n",argv[2]);

  stapl_print("testing default pGraph  constructor...");
  PGRAPH pg;
  PGRAPH pg1;
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //simple partition
  map<VID,PID> _pp;
  for(int r=0;r<SZ;r++)
    _pp[r] = (r * NP) / SZ;

  stapl::set_aggregation(50);
  
  stapl_print("testing read from file and distribute...");
  pg.Split_Crude_Dot<GRAPH>(argv[2],_pp,0);
  pg1.Split_Crude_Dot<GRAPH>(argv[2],_pp,0);
  stapl_print("Passed\n");

  visitor<PGRAPH> vis(pg);
  visitor<PGRAPH> vis1(pg1);
  //p_DFS_functor<PGRAPH>(pg,vis,1);

  vector<PGRAPH*> vpg;
  vector<visitor<PGRAPH>* > vvis;
  vector<VID> vstart; 
  vpg.push_back(&pg);
  vpg.push_back(&pg1);
  vvis.push_back(&vis);  
  vvis.push_back(&vis1);
  vstart.push_back(0);
  vstart.push_back(0);
  //pBFS<PGRAPH,visitor<PGRAPH> >(pg,vis,0);
  //p_BFS_functor<PGRAPH,visitor<PGRAPH> >(vpg,vvis,vstart);
  stapl::rmi_fence();
  //test pDeleteVertex
  //if(id == 0){
  //  pg.pDeleteVertex(2);
  //}
  //stapl::rmi_fence();
  //if(id == 0){
  //  pg.pDisplayGraph();
  //  pg.DisplayMaps();
  //}
  //pg.pDeleteEdge(1,3);
  //p_DFS_functor<PGRAPH>(pg,vis,0);


  stapl::rmi_fence();


  pg.pFileWrite("pgout",10);

  //pg.pDisplayGraph();
  //pg.DisplayMaps();
  stapl::rmi_fence();
  //pg1.pFileRead("pgout",10);
  //stapl::rmi_fence();
  //pg1.pFileWrite("pg1out",10);

  //test the const iterator isVertex
  PGRAPH::VI pi = pg.local_begin();
  if(pi != pg.local_end()){}
  pi++;
  //for(pi=pg.begin();pi!=pg.end();pi++){}
  //  cout<<"Data "<<pi->data<<endl;
  //}
  cout<<"IsEdge "<<pg.IsEdge(0,3)<<endl;

  PGRAPH::EI ei;
  if(pg.IsEdge(0,1,&pi,&ei)){
    assert(pi->vid == 0);
    assert(ei->vertex2id == 1);
  }
  
  vector<pair<VID,VID> > edges;
  sz = pg.GetEdges(edges);
  assert(sz == edges.size());

  vector<pair<pair<VID,VID>,Weight2> > wedges;
  sz1 = pg.GetEdges(wedges);
  assert(sz1 == wedges.size());
  assert(edges.size() == wedges.size());
  stapl::rmi_fence();
  cout<<"finished"<<endl;
  return;
}



