#include <stdlib.h>


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



	typedef Graph<DG<Task,Weight2>,MG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH;
	typedef Graph<UG<Task,Weight2>, NMG<Task,Weight2>, WG<Task,Weight2>, Task,Weight2> GRAPH2;

	GRAPH g;
	GRAPH2 g2;
	GRAPH::VI vi;
	GRAPH::EI ei;
	Task data;
	Weight2  w2(1);

	g.ReadDotGraph("in");
	for(vi = g.begin(); vi != g.end(); ++vi){
	  vi->data.SetTaskWeight(vi->vid);
	  for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
	    ei->weight = w2;
	  }
	}

	g.WriteGraph("new1");
	
	g2.ReadDotGraph("in");
	for(vi = g2.begin(); vi != g2.end(); ++vi){
	  vi->data.SetTaskWeight(vi->vid);
	  for(ei = vi->edgelist.begin(); ei != vi->edgelist.end(); ei++){
	    ei->weight = w2;
	  }
	}
	g2.WriteGraph("new2");

bool alltest=true;
bool graphtest=true;
if((alltest) || (graphtest))
{
	cout<<"*********START : Printig the GRAPH1*********"<<endl;
	g.DisplayGraph();
	cout<<"*********END   : Printig the GRAPH1*********"<<endl;

	cout<<"*********START : Printig the GRAPH2*********"<<endl;
	//g2.DisplayGraph();
	cout<<"*********END   : Printig the GRAPH2*********"<<endl;
}




vector<Task> v_tasks1;
g.GetVerticesData(v_tasks1);

vector<Task> v_tasks2;
g2.GetVerticesData(v_tasks2);

//TESTING DIRECTED GRAPH 
for(int i=0; i<g.GetVertexCount(); ++i)
{
	if(graphtest)
	cout<<g.GetVertexOutDegree(i)<<endl;
	
	if(graphtest)	
	{
		vector<VID> result;
		g.GetSuccessors(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	if(graphtest)		
	{
	
		vector<Task> result;
		g.GetSuccessorsDATA(i,result);
		
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" SuccData= "<<result[j].GetTaskWeight()<<endl;
		}
	}

	if(graphtest)	
		g.SetPredecessors();
	if(graphtest)		
	{
		vector<VID> result;
		g.GetPredecessors(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	if(graphtest)		
	{
	
		vector<Task> result;
		g.GetPredecessorsDATA(i,result);
		
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" SuccData= "<<result[j].GetTaskWeight()<<endl;
		}
	}
}

	if(graphtest)		
	{
		vector<VID> result;
		g.GetSources(result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<" j: "<<j<<" Source = "<<result[j]<<endl;
		}
	}
	
	if(graphtest)		
	{
		vector<VID> result;
		g.GetSinks(result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<" j: "<<j<<" Sink = "<<result[j]<<endl;
		}
	}


//TESTING DIRECTED GRAPH OVER

//TESTING UNDIRECTED GRAPH

for(int i=0; i<g.GetVertexCount(); ++i)
{
	if(graphtest)
		cout<<g2.GetVertexOutDegree(i)<<endl;
		
	if(graphtest)
	{
		vector<VID> result;
		g2.GetAdjacentVertices(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" Succ= "<<result[j]<<endl;
		}
	}
	
	
	if(graphtest)
	{
	
		vector<Task> result;
		g2.GetAdjacentVerticesDATA(i,result);
		
		for(int j=0; j<result.size(); ++j)
		{
			//cout<<"i: "<<i<<" j: "<<j<<" SuccData= "<<result[j].GetTaskWeight()<<endl;
		}
	}
	
	if(graphtest)
	{
		vector< pair<VID,VID> > result;	
		g2.GetIncidentEdges(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first<<"  Second: "<<result[j].second<<endl;
		}
	}
	
	if(graphtest)
	{
		vector< pair<pair<VID,VID>,Weight2> > result;	
		g2.GetIncidentEdges(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first<<"  Second: "<<result[j].first.second<<" Data:="<<result[j].second.Weight()<<endl;
		}
	}
			

	
	if(graphtest)
	{
		vector< pair<Task,Task> > result;	
		g2.GetIncidentEdgesVData(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.GetTaskWeight()<<"  Second: "<<result[j].second.GetTaskWeight()<<endl;
		}
	}
	
	if(graphtest)
	{
		vector< pair<pair<Task,Task>,Weight2> > result;	
		g2.GetIncidentEdgesVData(i,result);
		for(int j=0; j<result.size(); ++j)
		{
			cout<<"i: "<<i<<" j: "<<j<<" IncEdge.first= "<<result[j].first.first.GetTaskWeight()<<"  Second: "<<result[j].first.second.GetTaskWeight()<<" Data:="<<result[j].second.Weight()<<endl;
		}
	}
		
	 
}

for(int counter=0; counter<1; counter++)	
{

//TESTING UNDIRECTED GRAPH
	if(graphtest)
	{
		g2.AddEdge(counter+(random()%10)*counter,counter+(random()%10)*counter,Weight2(counter*1000+(counter*(random()%100)))); 
		g2.AddEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter],Weight2(counter*1000+(counter*(random()%100)))); 

		pair<Weight2,Weight2> tmp1;
		tmp1.first=Weight2(counter*1000+(counter*(random()%100)));
		tmp1.second=Weight2(counter*1000+(counter*(random()%100)));
		pair<Weight2,Weight2> tmp2;
		tmp2.first=Weight2(counter*1000+(counter*(random()%100)));
		tmp2.second=Weight2(counter*1000+(counter*(random()%100)));

		g2.AddEdge(counter+(random()%10)*counter,counter+(random()%10)*counter,tmp1); 
		g2.AddEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter],tmp2); 
	
		g2.DeleteEdge(counter+(random()%10)*counter,counter+(random()%10)*counter);
		int e1=counter+(random()%10)*counter;
		int e2=counter+(random()%10)*counter;
		Weight2 tw1=g2.GetEdgeWeight(e1,e2);
		g2.DeleteWtEdge(e1,e2,tw1);
	
	
		g2.DeleteEdge(v_tasks2[counter+(random()%10)*counter],v_tasks2[counter+(random()%10)*counter]);
		int e3=counter+(random()%10)*counter;
		int e4=counter+(random()%10)*counter;
		Weight2 tw2=g2.GetEdgeWeight(e1,e2);
		g2.DeleteWtEdge(v_tasks2[e1],v_tasks2[e2],tw2);
		
		g2.DisplayGraph();
	
	}
	if(alltest)
	{
		cout<<" TESTIGN BFS: START "<<endl;
		GRAPH result,result2;
		true_BFS(g,result,counter);
  		result.DisplayGraph();
		BFS(g,result2,counter);
 		result2.DisplayGraph();
		cout<<" TESTIGN BFS: END "<<endl;

	}
	
	if(alltest)
	{
		cout<<"TESTING BFS VID"<<endl;
		vector<VID> result;
		BFSVID(g,counter,result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"TESTING BFS VID OVER"<<endl;
	}
	
	
	if(alltest)
	{
		cout<<"TESTING BFS VID2"<<endl;
		vector<VID> result;
		BFSVID(g,v_tasks1[counter],result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"TESTING BFS VID OVER2"<<endl;
	}
	
	
//NOTE :: //Last -1 not 1e-232 : For next two blocks

	if(alltest)
	{
		cout<<"FIND PATH BFS TESTING"<<endl;		
		vector< pair<Task,Weight2> > result;
		FindPathBFS(g,counter,2*counter,result);
		for(int i=0; i<result.size(); ++i)
		{
			if(i!=result.size()-1)
				cout<<result[i].first.GetTaskWeight()<<"  "<<result[i].second.Weight()<<endl;
			else
				cout<<result[i].first.GetTaskWeight()<<"  "<<endl;
			
		}
		cout<<"FIND PATH BFS OVER"<<endl;
		
	}
	
	if(alltest)
	{
		cout<<"FIND PATH BFS TESTING2"<<endl;		
		vector< pair<Task,Weight2> > result;
		FindPathBFS(g,v_tasks1[counter],v_tasks1[2*counter],result);
		for(int i=0; i<result.size(); ++i)
		{

			if(i!=result.size()-1)
				cout<<result[i].first.GetTaskWeight()<<"  "<<result[i].second.Weight()<<endl;
			else
				cout<<result[i].first.GetTaskWeight()<<"  "<<endl;
		}
		cout<<"FIND PATH BFS OVER2"<<endl;
		
	}
	if(alltest)
	{
		cout<<"FIND VID PATH BFS TESTING"<<endl;		
		vector<VID> result;
		FindVIDPathBFS(g,counter,2*counter,result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"FIND VID PATH BFS OVER"<<endl;
	}
	

	if(alltest)
	{
	
	
		cout<<"IsSameCC Test1 : START"<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*2<<"? "<<IsSameCC(g2,counter,counter*2)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*3<<"? "<<IsSameCC(g2,counter,counter*3)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*4<<"? "<<IsSameCC(g2,counter,counter*4)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*5<<"? "<<IsSameCC(g2,counter,counter*5)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*6<<"? "<<IsSameCC(g2,counter,counter*6)<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*7<<"? "<<IsSameCC(g2,counter,counter*7)<<endl;
		cout<<"IsSameCC Test1 : END"<<endl;
	}

//NOTE::The answer given by the next block may be different if the Task weights are the same	
	if(alltest)
	{
	
		cout<<"IsSameCC Test2 : START"<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*2<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*2])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*3<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*3])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*4<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*4])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*5<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*5])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*6<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*6])<<endl;
		cout<<"\nis it "<<counter<<"  "<<counter*7<<"? "<<IsSameCC(g2,v_tasks2[counter],v_tasks2[counter*7])<<endl;
		cout<<"IsSameCC Test2 : END"<<endl;
	}
	
	if(alltest)
	{
		cout<<"Testing GetCC1"<<endl;
	
		vector<VID> result;
		GetCC(g2,counter,result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i]<<endl;
		}
		cout<<"Testing GetCC1 end"<<endl;
	}
	
	if(alltest)
	{
		cout<<"Testing GetCC2"<<endl;
		vector<Task> result;
		GetCC(g2,v_tasks2[counter],result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<result[i].GetTaskWeight()<<endl;
		}
		cout<<"Testing GetCC1 end"<<endl;
	}
	
	if(alltest)
	{
		cout<<"Testing GetCCEdges1"<<endl;
		vector< pair<VID,VID> > result ;
		GetCCEdges(g2,result,counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first<<" S: "<<result[i].second<<endl;
		}
	
		cout<<"Testing GetCCEdges1 OVER"<<endl;
	
	}
	
	if(alltest)
	{
	
		cout<<"Testing GetCCEdges2"<<endl;
		vector< pair<pair<VID,VID>, Weight2> > result ;
		GetCCEdges(g2,result,counter);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first.first<<" S: "<<result[i].first.second<<"  W: "<<result[i].second.Weight()<<endl;
		}
	
		cout<<"Testing GetCCEdges2 OVER"<<endl;
	
	
	}
	
	if(alltest)
	{
		cout<<"Testing GetCCEdgesVData1"<<endl;
		vector< pair< pair<Task,Task>,Weight2 > > result;
		GetCCEdgesVData(g2,result,counter);	
	
		for(int i=0; i<result.size(); ++i)
		{

			cout<<"i: "<<i<<" F: "<<result[i].first.first.taskwt<<" S: "<<result[i].first.second.taskwt<<"  W: "<<result[i].second.Weight()<<endl;
		}
	
		cout<<"Testing GetCCEdgesVData Over1"<<endl;
	
	}
	
	if(alltest)
	{
		cout<<"TESTING GetEdgesByCCVDataOnly1"<<endl;
		vector< vector< pair< pair<Task,Task>,Weight2 > > > result;
		GetEdgesByCCVDataOnly(g2,result);	

	
		for(int i=0; i<result.size(); ++i)
		{
		
			for(int j=0; j<result[i].size(); ++j)
			{
				
				cout<<"i: "<<i<<" j:"<<j<<" F: "<<result[i][j].first.first.taskwt<<" S: "<<result[i][j].first.second.taskwt<<endl;//"  W: "<<result[i][j].second.Weight()<<endl;
			}
		}
		cout<<"TESTING GetEdgesByCCVDataOnly OVER1"<<endl;
	}
	
	if(alltest)
	{

		cout<<"Testing GetCCStats1"<<endl;
		vector< pair<int,VID> > result;
		GetCCStats(g2,result);
		for(int i=0; i<result.size(); ++i)
		{
			cout<<"i: "<<i<<" F: "<<result[i].first<<" S: "<<result[i].second<<endl;
		}
		cout<<"Testing GetCCStats OVER"<<endl;
	}
	
	
	if(alltest)
	{
		cout<<"Testing GetCC Count"<<endl;
		cout<<GetCCcount(g2)<<endl;
		cout<<"Testing GetCC Count : OVER"<<endl;
	}
	
	
	if(alltest)
	{
	
		GRAPH result;
		//DijkstraSSSP(g,result,counter);
		//result.DisplayGraph();
	
	}
	
	if(alltest)
	{
	
		GRAPH result;
		//DijkstraSSSP(g2,result,counter);
		//result.DisplayGraph();
	
	}
	
	if(alltest)	
	{
		
		vector< pair<Task,Weight2> > result;
		//FindPathDijkstra(g,counter,counter*2,result);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}
	
	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//FindPathDijkstra(g2,counter,counter*2,result);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
	}

	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		//FindPathDijkstra(g,v_tasks1[counter],v_tasks1[counter*2],result);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
		
	}
	
	if(alltest)
	{
		
		vector< pair<Task,Weight2> > result;
		FindPathDijkstra(g2,v_tasks1[counter],v_tasks1[counter*2],result);
		double totaldist=0;
		for(int i=0; i<result.size(); ++i)
		{
			
			//cout<<"Second : "<<result[i].second.Weight()<<endl;
			totaldist+=result[i].second.Weight();
		
		}
	}


}
	return 1;
}
