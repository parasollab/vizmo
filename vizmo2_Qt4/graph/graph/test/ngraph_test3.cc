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

#define VERTEX Task
#define WEIGHT Weight2

int main(void){
  WtEdgeType<int,int> w;
  int i,j,k;

  
  typedef Graph<UG<Task,Weight2>, 
    NMG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
  typedef Graph<DG<Task,Weight2>, 
  NMG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> DIRG;



  //typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> GRAPH;
  //typedef Graph<DG<Task>, MG<Task>, NWG<Task>, Task> DIRG;

  GRAPH c3,c2;

  GRAPH::VI vi;
  GRAPH::EI ei;


  mygraph<Task,Weight2> g;

  Task data(1);
  Weight2  w3(1);
  k=5;

  c3.ReadDotGraph("in");
  for(vi = c3.begin(); vi != c3.end(); ++vi){
    vi->data.SetTaskWeight(vi->vid);
    for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
      ei->weight = w3;
    }
  }
  cout<<"Start"<<endl;
  stapl::start_timer();
  //test the graph's methods
  c3.IsDirected();c3.IsMulti();c3.IsWeighted();
  Task d1(1);
  Weight2 w1(1),w2(2);
  for(j=0;j < 4;j++){
    //methods from Graph.h
    vector<Task> v1;
    v1.push_back(data);
    v1.push_back(d1);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(data);
    c2.AddVertex(v1);
    
    cout<<c2.size()<<endl;
    
    cout<<"First "<< c2.AddEdge(0,1,w1)<<endl;
    cout<<"Second "<< c2.AddEdge(0,2,w1)<<endl;
    c2.AddEdge(c2.begin()->data,(c2.begin() + 3)->data,w1);
    pair<Weight2,Weight2> p;
    p.first = w1;
    p.second = w2;
    c2.AddEdge(1,2,p);
    c2.AddEdge(c2.begin()->data,(c2.begin() + 4)->data,p);
    int ne;
    c2.DisplayGraph();
  
    c2.DisplayVertexAndEdgelist(15);
    c2.DeleteVertex(15);
    c2.DisplayGraph();
    k=c2.getVertIDs();
    c2.setVertIDs(k);
    
    //the next two are not available in the old one
    cout<<"Run GetEdges"<<endl;
    
    vector<pair<VID,VID> > edges;
    ne = c2.GetEdges(edges);
    //cout<<"number of edges "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges[i].first<<" -> "<<edges[i].second<<endl;
    //}
    vector< pair< VERTEX, VERTEX > > edges1;
    ne = c2.GetEdgesVData(edges1);
    //cout<<"number of edges1 "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges1[i].first<<" -> "<<edges1[i].second<<endl;
    //} 
    
    vector< pair< pair< VID, VID >, WEIGHT > > edges2;
    ne = c2.GetEdges(edges2);
    cout<<"number of edges2 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges2[i].first.first<<" -> "<<edges2[i].first.second<<" "<<edges2[i].second<<endl;
    }
    vector< pair< pair< VERTEX, VERTEX >, WEIGHT > > edges3;
    ne = c2.GetEdgesVData(edges3);
    cout<<"number of edges3 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges3[i].first.first<<" -> "<<edges3[i].first.second<<" "<<edges3[i].second<<endl;
    }
    vector<VID> vd1;
    vd1.push_back(100);
    vd1.push_back(101);
    vd1.push_back(102);
    c2.AddPath(vd1, w1);

    //c2.ChangeEdgeWeight(0,2,w2);
    //    c2.ChangeEdgeWeight(c2.begin()->data,(c2.begin() + 1)->data,w2);
    c2.DeleteVertex(i);
    c2.DisplayGraph();
    c2.DisplayVertices();
    c2.DisplayVertex(i);
    // not in WMDiGc2.DisplayVertexandEdgelist(i);
    
    //!!!c2.DeleteVertex(v1);
    for(vi = c3.begin(); vi != c3.end(); vi++) {	 
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++) {
      }
    }
  }
  //this are not in original graph for directed
  for(j=0;j < 4;j++){
  
    vector<Task> v1;
    v1.push_back(data);
    v1.push_back(d1);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(data);
    c3.AddVertex(v1);
    
    cout<<c3.size()<<endl;
    
    cout<<"First "<< c3.AddEdge(0,1,w1)<<endl;
    cout<<"Second "<< c3.AddEdge(0,2,w1)<<endl;
    c3.AddEdge(c3.begin()->data,(c3.begin() + 3)->data,w1);
    pair<Weight2,Weight2> p;
    p.first = w1;
    p.second = w2;
    c3.AddEdge(1,2,p);
    c3.AddEdge(c3.begin()->data,(c3.begin() + 4)->data,p);
    int ne;
    c3.DisplayGraph();
  
    c3.DisplayVertexAndEdgelist(15);
    c3.DeleteVertex(15);
    c3.DisplayGraph();
    k=c3.getVertIDs();
    c3.setVertIDs(k);
  
    //is edge
    cout<<c3.IsEdge(0,j)<<endl;
    cout<<c3.IsEdge(c3.begin()->data,(c3.begin() + 3)->data);
    cout<<c3.IsEdge(0,j,w1);
    cout<<c3.IsEdge(c3.begin()->data,(c3.begin() + 3)->data,w1);
 
    vector< pair<pair<VID,VID>,Weight2> > iedges;
    ne = c3.GetIncidentEdges(j, iedges);
    for(i=0;i<ne;i++){
      cout<<iedges[i].first.first<<" -> "<<iedges[i].first.second<<" "<<iedges[i].second<<endl;
    }
    vector< pair<pair<Task,Task>,Weight2> > iedgesw;
    ne = c3.GetIncidentEdgesVData(j, iedgesw);
    for(i=0;i<ne;i++){
      cout<<iedges[i].first.first<<" -> "<<iedges[i].first.second<<" "<<iedges[i].second<<endl;
    }
    //weighted methods;
    cout<<c3.GetEdgeWeight(0,j)<<endl;
    cout<<c3.GetEdgeWeight(c3.begin()->data , (c3.begin() + 3)->data)<<endl;
   
    c3.DeleteWtEdge(0, j , w1);
    c3.DeleteWtEdge(c3.begin()->data , (c3.begin() + 3)->data, w1);
    c3.DisplayGraph();
    
    //the next two are not available in the old one
    cout<<"Run GetEdges"<<endl;
    
    vector<pair<VID,VID> > edges;
    ne = c3.GetEdges(edges);
    //cout<<"number of edges "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges[i].first<<" -> "<<edges[i].second<<endl;
    //}
    vector< pair< VERTEX, VERTEX > > edges1;
    ne = c3.GetEdgesVData(edges1);
    //cout<<"number of edges1 "<<ne<<endl;
    //for(i=0;i<ne;i++){
    //  cout<<edges1[i].first<<" -> "<<edges1[i].second<<endl;
    //} 
    
    vector< pair< pair< VID, VID >, WEIGHT > > edges2;
    ne = c3.GetEdges(edges2);
    cout<<"number of edges2 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges2[i].first.first<<" -> "<<edges2[i].first.second<<" "<<edges2[i].second<<endl;
    }
    vector< pair< pair< VERTEX, VERTEX >, WEIGHT > > edges3;
    ne = c3.GetEdgesVData(edges3);
    cout<<"number of edges3 "<<ne<<endl;
    for(i=0;i<ne;i++){
      cout<<edges3[i].first.first<<" -> "<<edges3[i].first.second<<" "<<edges3[i].second<<endl;
    }
    
    vector<VID> vd1;
    vd1.push_back(100);
    vd1.push_back(101);
    vd1.push_back(102);
    c3.AddPath(vd1, w1);
    c3.DeleteVertex(j);
    c3.DisplayGraph();
    c3.DisplayVertices();
    c3.DisplayVertex(j);
    // not in WMDiGc3.DisplayVertexandEdgelist(i);
    
    //!!!c3.DeleteVertex(v1);
    for(vi = c3.begin(); vi != c3.end(); vi++) {	 
      for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++) {
      }
    }
  }
  float tt = stapl::stop_timer();
  cout<<endl<<"time_new :"<<tt<<endl;
  c3.WriteGraph("titi1");
  return 1;
}
