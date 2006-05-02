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


typedef Graph<DG<Task,Weight2>, 
  MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
typedef Graph<DG<Task,Weight2>, 
  MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> DIRG;



//typedef Graph<UG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;
//  typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> DIRG;

  GRAPH c3;

  GRAPH::VI vi;
  GRAPH::EI ei;


  mygraph<Task,Weight2> g;

  Task data;
  Weight2  w2(1);
  k=5;

  c3.ReadDotGraph("in");

  for(vi = c3.begin(); vi != c3.end(); ++vi){
    vi->data.SetTaskWeight(vi->vid);
    for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
      ei->weight = w2;
    }
  }
  cout<<"Start"<<endl;
  stapl::start_timer();
  for(i=0;i < k;i++){

    for(vi = c3.begin(); vi != c3.end(); vi++) {	 
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++) {
      }
    }
    cout<<"new DFS"<<endl;
    DIRG g7;
    DFS(c3,g7,i);
    //for(vi = g.begin(); vi != g.end(); ++vi){
    //vi->data.SetTaskWeight(vi->vid);
    //for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
    //ei->weight = w2;
    //}
    //}
    g7.DisplayGraph();

    cout<<"VERTEX"<<endl;
    g7.EraseGraph();
    DFS(c3,g7,(c3.begin()+1)->data);
    g7.DisplayGraph();
    
    cout<<"DFS VID"<<endl;
    vector<VID> v;
    DFSVID(c3,i,v);
    sort(v.begin(),v.end());
    for(j=0;j < v.size();j++){
      cout<<v[j]<<endl;
    }
    cout<<"VERTEX"<<endl;
    DFSVID(c3,(c3.begin()+1)->data,v);
    sort(v.begin(),v.end());
    for(j=0;j < v.size();j++){
      cout<<v[j]<<endl;
    }
    cout<<"DFS returns the forest"<<endl;
    vector<DIRG> v41;
    DFS(c3,v41);
    for(j=0;j < v41.size();j++){
      v41[j].DisplayGraph();
    }
    cout<<"IsCycle "<<IsCycle(c3)<<endl;
    
    cout<<"GetBackEdges"<<endl;
    vector<pair<VID,VID> > v31;
    GetBackedge(c3, v31);
    stable_sort(v31.begin(), v31.end(), __s1<pair<VID,VID> >());
    //for(j=0;j < v31.size();j++){
    //cout<<v31[j].first<<" "<<v31[j].second<<endl;
    //}
    
    cout<<"new BFS"<<endl;
    DIRG g6;
    BFS(c3,g6,i);
    g6.DisplayGraph();
    cout<<"VERTEX"<<endl;
    BFS(c3,g6,(c3.begin()+1)->data);
    g6.DisplayGraph();

    cout<<"BFSVID"<<endl;
    vector<VID> v4;
    BFSVID(c3,i,v4);
    sort(v4.begin(),v4.end());
    for(j=0;j < v4.size();j++){
      cout<<v4[j]<<endl;
    }
    cout<<"VERTEX"<<endl;
    BFSVID(c3,(c3.begin()+1)->data,v4);
    sort(v4.begin(),v4.end());
    for(j=0;j < v4.size();j++){
      cout<<v4[j]<<endl;
    }
  }
  
  float tt = stapl::stop_timer();
  //cout<<"time_new :"<<tt<<endl;
  c3.WriteGraph("titi1");
  return 1;
}
