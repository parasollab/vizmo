#include <stdlib.h>
#include "timing.h"

#include "Graph.h"
#include "GraphAlgo.h"

#include "task.h"

/************************************************************

Next are a few typedefs that can be usefull when you want to 
instantiate a specific type of graph. First of all a graph 
is templated such that we can have directed/undirected, weighted/unweighted
allow multiple edges or not. 

Case1) Directed graph, allow multiple edges, unweighted;
The user doesn't have acces to methods related with weight;

   typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;

Case2)Undirected, multiple edges, unweighted
   typedef Graph<UG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;

Case3) Directed non multiple edges nonweighted
   typedef Graph<DG<Task>, NMG<Task>, NWG<Task>, Task> GRAPH;

Case4)Directed, multiple edges, weighted
The user has to specify the weight

   typedef Graph<DG<Task,Weight2>, MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;

*************************************************************/

template <class T>
struct __s1 : public binary_function<T, T, bool> {
  bool operator()(T x, T y) { return x.first < y.first; }
};

template <class G>
class visitor: public visitor_base<G>{
  //this class is used by DFS/BFS
public:  
  visitor(G& g){
    cout<<"visitor created"<<endl;
  };
  inline int vertex (typename G::VI v) {
    //cout<<"Vertex "<<v->vid<<" "<<v->data<<endl;
    return 1;
  }
  inline int edge (typename G::VI v, typename G::EI e) {
    cout<<"Edge "<<e->vertex2id<<endl;
    return 1;
  }
  inline int finish_vertex (typename G::VI v) {
    cout<<"Vertex "<<v->vid<<" "<<v->data<<endl;
    return 1;
  }
};

template <class T, class W>
class mygraph: public Graph<UG<T,W>, MG<T,W>, WG<T,W>, T,W>{
public:
  mygraph(){
    //cout<<"good"<<endl;
  }
};

int main(void){
  WtEdgeType<int,int> w;
  int i,j,k;
  double elapsed;
  
  typedef Graph<DG<Task,Weight2>, 
    MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<DG<Task,Weight2>, 
    MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> DIRG;
  
  
  
  //typedef Graph<UG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;
  //  typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> DIRG;
  
  GRAPH g;
  GRAPH::VI vi;
  GRAPH::EI ei;
  g.ReadExoGraph("in");
  Weight2  w2(1);
  //Weight2  w3(78.78);
  //pair<Weight2,Weight2> pw(w2,w3);

  //g.ChangeEdgeWeight(8,14,pw);

  for(vi = g.begin(); vi != g.end(); ++vi){
    for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
      ei->weight = w2;
    }
  }
  //g.DisplayGraph();
  
  for(int ll=0;ll<g.size();ll++){

    vector<VID> cc;
    int kk=GetCC(g,ll,cc);
    cout<<"number of vertices in this cc:"<<kk<<endl;
    
    //call dijkstra 1
    vector<pair<Task,Weight2> > _path;
    vector<pair<Task,Weight2> > _path1;
    cout<<"start Dijkstra vector...";
    /*
    stapl::start_timer();
    for(i=0;i<kk;i++)
      j = FindPathDijkstra(g,0,cc[i],_path);
    elapsed = stapl::stop_timer();
    cout<<"Elapsed:"<<elapsed<<endl;

    //call dijkstra 2
    cout<<"start Dijkstra map...";
    stapl::start_timer();
    for(i=0;i<kk;i++)
      k = FindPathDijkstra1(g,0,cc[i],_path1);
        elapsed = stapl::stop_timer();
    cout<<"Elapsed:"<<elapsed<<endl;
    */

    for(i=0;i<kk;i++){
      j = FindPathDijkstra(g,0,cc[i],_path);
      k = FindPathDijkstra1(g,0,cc[i],_path1);
      if(j != k )
	cout<<"ERROR while comparing two dijkstra-s"<<endl;
      for(int ll =0;ll<j;ll++)
	if(!(_path[ll].first == _path1[ll].first))
	  cout<<"ERROR while testing Dijkstra; The path is different at index:"<<ll<<endl;
    }
    cout<<"length of the path "<<k<<endl;
    
  }

  return 1;

  vector<int> vs;
  FindVIDPathBFS(g,2748,3870,vs);
  cout<<"START 2748"<<endl;
  for(i=0;i<vs.size();i++){
    cout<<vs[i]<<"->"<<endl;
  }
  return 1;


  vector<vector<VID> > vt;
  //scc(g,vt);
  
  int cnt =0;
  
  for(i=0;i<vt.size();i++){
    if(vt[i].size()>1){
      cout<<"cc no "<<i<<endl;
      for(j=0;j<vt[i].size();j++){
	//cout<<vt[i][j]<<" "<<endl; 
	cnt++;
      }
      //cout <<endl;
    }
  }
  
  cout<<"TOTAL:"<<cnt<<endl;

  //i=g.GetVertexOutDegree(1);
  //cout<<i<<endl;
    //float tt = stapl::stop_timer();
  //cout<<"time_new :"<<tt<<endl;
  return 1;
}
