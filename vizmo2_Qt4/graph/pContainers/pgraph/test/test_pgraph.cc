/*!
        \file test_pgraph.cc
	\author Gabriel Tanase	
        \date Feb. 16, 04
        \ingroup stapl
        \brief Test suite for pGraph class. All methods from pGraph class instantiated
	\and tested for correctness. These are mainly methods to add/get vertices, edges
	int 
	IsDirected () const
	int 
	IsWeighted () const
	int 
	IsMulti () const
	int 
	AddEdge (VID _v1, VID _v2, WEIGHT _weight=WEIGHT(-1))
	int 
	AddEdge (VID _v1, VID _v2, pair< WEIGHT, WEIGHT > &_p)
	int 
	GetEdges (vector< pair< VID, VID > > &edges)
	int 
	GetEdgesVData (vector< pair< VERTEX, VERTEX > > &edges)
	int 
	GetEdges (vector< pair< pair< VID, VID >, WEIGHT > > &edges)
	int 
	GetEdgesVData (vector< pair< pair< VERTEX, VERTEX >, WEIGHT > > &edges)
	template<class GRAPH> void 
	void 
	pFileWrite (const char *file_name, int num_chars)
	void 
	pFileRead (const char *file_name, int num_chars)

	//these are from pBaseGraph
	GID 
	AddVertex()
	int 
	GetVertices (pcontainer< VID > &_verts) const
	int 
	GetVertices (pcontainer< VI > &_verts) const
	int 
	GetVertices (pcontainer< VERTEX > &_verts) const
*/
#include <runtime.h>
#include "Defines.h"
#include "task.h"
#include <assert.h>
#include "Graph.h"
#include "pGraph.h"
#include "pArray.h"

#include <sys/time.h>

using namespace stapl;

int SZ,NP;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}


void stapl_main(int argc,char** argv){

  typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<UG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> UGRAPH;
  typedef pGraph<PDG<Task,Weight2>,PMG<Task,Weight2>, PWG<Task,Weight2>, Task,Weight2> PGRAPH;


  Task data;
  Weight2 wt;
  int i,j,tmp,localedges;   
  int myid=stapl::get_thread_id();
  vector<int> vertices;
  NP=stapl::get_num_threads();

  printf("main started\n");

  SZ = atoi(argv[1]);
  printf("SIZE %d\n",SZ);

  stapl_print("testing default pGraph  constructor...");
  PGRAPH pg;
  PGRAPH pg1;
  PGRAPH::VI vi;

  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing pGraph properties...");
  stapl_assert(pg.IsDirected(),"is directed failed");
  stapl_assert(pg.IsMulti(),"is multi failed");
  stapl_assert(pg.IsWeighted(),"is weighted failed");
  stapl_print("Passed\n");

  stapl_print("testing AddVertex...");
  for(i=0;i<SZ;i++){
    data.SetWeight(i);
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
  //pg.pDisplayGraph();
  //stapl::rmi_fence();
  //return;

  for(vi = pg.local_begin(); vi != pg.local_end();++vi){
    vi->data.SetWeight(vi->vid);
  }

  localedges = 0;
  stapl_print("testing AddEdge...");
  for (i=0;i<SZ;i++){
      for (j=SZ*NP-1;j>=0;j--){
	wt.SetWeight(vertices[i] * j);
	pg.AddEdge(vertices[i],j,wt);
	localedges++;
      }
  }
  stapl::rmi_fence();
  for (i=0;i<SZ;i++){
      for (j=SZ*NP-1;j>=0;j--){
	if(!pg.IsEdge(vertices[i],j))
	  cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
      }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetVertices(VIDs...");
  pArray<VID> verts;
  stapl::rmi_fence();
  tmp = pg.GetVertices(verts);
  //for(i=0;i<tmp;i++){
    //cout<<myid<<":index:"<<i<<":val"<<verts[i]<<endl;
  //}
  stapl::rmi_fence();
  stapl_print("Passed\n");
  
  stapl_print("testing GetVertices(VERTEX...");
  pArray<Task> vertsD;
  stapl::rmi_fence();
  tmp = pg.GetVertices(vertsD);
  for(i=0;i<tmp;i++){
    if(verts[i] != vertsD[i].GetWeight()){
      cout<<"ERROR while testing GetVertices,GetVerticesData"<<endl;
      cout<<myid<<":index:"<<i<<":val"<<vertsD[i].GetWeight()<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");


  stapl_print("testing GetAllEdges(VIDs...");
  pArray<pair<pair<VID,VID>,Weight2> > pedges;
  pair<pair<VID,VID>,Weight2> tp;
  stapl::rmi_fence();
  tmp = pg.GetAllEdges(pedges);
  for(i=0;i<tmp;i++){
    tp = pedges[i];
    if(tp.first.first*tp.first.second != tp.second.GetWeight())
      cout<<"ERROR while testing GetAllEdges"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetAllEdges(VIDs...");
  pArray<pair<pair<Task,Task>,Weight2> > edgesD;
  pair<pair<Task,Task>,Weight2> tpp;
  stapl::rmi_fence();
  tmp = pg.GetAllEdgesVData(edgesD);
  for(i=0;i<tmp;i++){
    tpp = edgesD[i];
    if(tpp.first.first.GetWeight() * tpp.first.second.GetWeight() != tpp.second.GetWeight())
      cout<<"ERROR while testing GetAllEdgesVData"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  /*
    //this test case is not supported yet;
  stapl_print("testing GetVertices <VI>...");
  vector<PGRAPH::VI> vert_vi;
  tmp = pg.GetVertices(vert_vi); 
  //cout<<tmp<<" "<<vert_test.size()<<" "<<vertices.size()<<endl;
  stapl_assert(tmp == vert_vi.size(),"ERROR while testing GetVertices_vis(1)");
  stapl_assert(tmp == vertices.size(),"ERROR while testing GetVertices_vis(2)");
  for (i=0;i<SZ;i++){
    if(i != vert_vi[i]->data.GetWeight()){
      cout<<"ERROR while testing GetVertices_vis(3): "<<vertices[i]<<":"<<vert_vi[i]->data.GetWeight()<<endl;
    }
  }
  stapl_print("Passed\n");
  */
  stapl_print("testing GetEdges...");
  vector< pair< VID, VID > > edges;
  tmp = pg.GetEdges(edges); 
  stapl_assert(tmp == edges.size(),"ERROR while testing GetEdges(1)");
  stapl_assert(tmp == localedges,"ERROR while testing GetEdges(2)");
  for (i=0;i<tmp;i++){
    if(!pg.IsEdge(edges[i].first, edges[i].second)){
      cout<<"ERROR while testing GetEdges(3): "<<edges[i].first<<":"<<edges[i].second<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing GetEdges<Task,Task>...");
  vector< pair< Task, Task > > edges_data;
  tmp = pg.GetEdgesVData(edges_data); 
  stapl_assert(tmp == edges_data.size(),"ERROR while testing GetEdges<Task,Task>(1)");
  stapl_assert(tmp == localedges,"ERROR while testing GetEdges<Task,Task>(2)");
  for (i=0;i<tmp;i++){
    if(edges[i].first !=  edges_data[i].first.GetWeight()){
      cout<<"ERROR while testing GetEdges<Task,Task>(3): "<<edges[i].first<<":"<<edges_data[i].first.GetWeight()<<endl;
    }
  }
  stapl_print("Passed\n");


  stapl_print("testing GetEdges_w...");
  // I am consideringas an invariant the fact that the weight of the edge
  // is the product of the vertices that identifies the edge
  vector< pair< pair< VID, VID >, Weight2 > > edges_w;
  tmp = pg.GetEdges(edges_w); 
  stapl_assert(tmp == edges_w.size(),"ERROR while testing GetEdges_w(1)");
  stapl_assert(tmp == localedges,"ERROR while testing GetEdges_w(2)");
  for (i=0;i<tmp;i++){
    if(!pg.IsEdge(edges_w[i].first.first, edges_w[i].first.second)){
      cout<<"ERROR while testing GetEdges_w(3): "<<endl;
    }
    if(edges_w[i].second.GetWeight() != edges_w[i].first.first * edges_w[i].first.second){
      cout<<"ERROR while testing GetEdges_w(4): "<<endl;
    }
  }
  stapl_print("Passed\n");


  stapl_print("testing GetEdges_vw...");
  // I am consideringas an invariant the fact that the weight of the edge
  // is the product of the vertices that identifies the edge
  vector< pair< pair< Task, Task >, Weight2 > > edges_vw;
  tmp = pg.GetEdgesVData(edges_vw); 
  stapl_assert(tmp == edges_w.size(),"ERROR while testing GetEdges_vw(1)");
  stapl_assert(tmp == localedges,"ERROR while testing GetEdges_vw(2)");
  for (i=0;i<tmp;i++){
    if(edges_data[i].first.GetWeight() != edges_vw[i].first.first.GetWeight()){
      cout<<"ERROR while testing GetEdges_vw(3): "<<endl;
    }
    if(edges_data[i].second.GetWeight() != edges_vw[i].first.second.GetWeight()){
      cout<<"ERROR while testing GetEdges_vw(3): "<<endl;
    }
    if(edges_vw[i].second.GetWeight() != edges_w[i].first.first * edges_w[i].first.second){
      cout<<"ERROR while testing GetEdges_vw(4): "<<endl;
    }
  }
  stapl_print("Passed\n");

  stapl_print("testing SetPGraphwGraph...");
  PGRAPH pg3;
  GRAPH sequentialgraph;
  map<PID,vector<VID> > part;

  for(j=0;j<NP;j++){
    vector<VID> vs;
    for(i=0;i<SZ;i++){
      VID vd = sequentialgraph.AddVertex(data);
      vs.push_back(vd);
    }
    part[j] = vs;
  }

  for(i=0;i<NP * SZ;i++){
    for(j=NP*SZ-1;j>=0;j--){
      sequentialgraph.AddEdge(i,j,wt);
    }
  }
  stapl::rmi_fence();
  //pg3.SetPGraphwGraph(sequentialgraph);
  pg3.SetPGraphwGraph(sequentialgraph,part);
  pg3.CheckPGraphwGraph(sequentialgraph);
  stapl_print("Passed\n");  

  stapl_print("testing write to distributed file...");
  pg.pFileWrite("test_pgraph.out",15);
  stapl_print("Passed\n");

  stapl_print("testing read from distributed file...");
  pg1.pFileRead("test_pgraph.out",15);

  //compare pg1 with pg;
  PGRAPH::VI vi1 = pg.begin();
  PGRAPH::VI vi2 = pg1.begin();
  for(;vi1 != pg.end();++vi1,++vi2){
    if(vi1->vid != vi2->vid) cout<<"ERROR while testing read from file(vid)"<<endl;
    if(vi1->data.GetWeight() != vi2->data.GetWeight()) 
      cout<<"ERROR while testing read from file(data)"<<endl;
    //testing edges
    PGRAPH::EI ei1 = vi1->edgelist.begin();
    PGRAPH::EI ei2 = vi2->edgelist.begin();
    for(;ei1 != vi1->edgelist.end();++ei1,++ei2){
      if(ei1->vertex2id != ei2->vertex2id)
	cout<<"ERROR while testing read from file(edges vids)"<<endl;
      if(ei1->weight.GetWeight() != ei2->weight.GetWeight())
	cout<<"ERROR while testing read from file(edges data)"<<endl;
    }
  }
  stapl_print("Passed\n");
  return;
}



