#ifndef _PSCCUTILS
#define _PSCCUTILS

#include "../../common/Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "runtime.h"
#include <assert.h>

//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////

namespace stapl{

#define PSCC_SEEN   0
#define PSCC_UNSEEN 1
#define PSCC_ERASED 2

#define SCC_DEAD -100

#ifndef STAPL_PSCC_FWD
#define STAPL_PSCC_FWD 0
#endif

#ifndef STAPL_PSCC_BACK
#define STAPL_PSCC_BACK 1
#endif

#define STAPL_PSCC_REM
#define STAPL_PSCC_REMOTE

template <class GRAPH>
class _stapl_pscc_visitor_cycles: public visitor_base<GRAPH>{
  //  vector<pair<VID,VID> >* _back_edges;
  vector<VID>* _back_edges;
  public:  
  _stapl_pscc_visitor_cycles(GRAPH& g, vector<VID>* _v){
    _back_edges = _v;
  };
  inline int back_edge (typename GRAPH::VI vi, typename GRAPH::EI ei){
    _back_edges->push_back(ei->vertex2id);
    return EARLY_QUIT;
  }  
};

class taskSCC {
public:
  
  int ccno;
  int mark;
  int localid;
  int in; //in dependencies
  int out;//out dependencies
  
  //vectors for multiple directions(pivots)
  vector<int> forward;
  vector<int> back;

  //int forward;
  //int back;

  
  taskSCC() {
    in = out = -1;
    ccno=0;
  }

  taskSCC(int _vwt) {
    in = out = _vwt;
    ccno=0;
    //!! remember to initialize forward backward
  };

  ~taskSCC() {};

  void define_type(stapl::typer &t){
    t.local(ccno);
    t.local(mark);
    t.local(localid);
    t.local(in);
    t.local(out);
    t.local(forward);
    t.local(back);
  }

  //===================================================================
  // Operators
  //===================================================================
  inline int decrement_in(){
    if(in == 0) return SCC_DEAD;
    in = in - 1;
    return in;
  }

  inline int decrement_out(){
    if(out == 0) return SCC_DEAD;
    out = out - 1;
    return out;
  }

  inline bool operator== (const taskSCC &task) const {  
    //not used just for requirements
    return  (in == task.in);
  };
  
  inline taskSCC& operator= (const taskSCC &task) {
    //not used just for requirements
    ccno   =task.ccno;
    mark   = task.mark;
    localid = task.localid;
    in     =task.in;
    out    =task.out;
    forward=task.forward;
    back   =task.back;
    mark   =task.mark;
    return *this;
  }

  void SetToZeroIn(){
    in = 0;
  }

  void SetToZeroOut(){
    out = 0;
  }

  void reset(){
    in = out  = -1;
    for(int i=0;i<forward.size();i++){
      forward[i] = -1;
      back[i] = -1;
    }
  }
  void reset_fb(){
    for(int i=0;i<forward.size();i++){
      forward[i] = -1;
      back[i]    = -1;
    }
  }

  int SetPivotForward(int _pivid, int _pivval){

    if(in == 0 || out == 0) return OK;
    forward[_pivid] = _pivval;
    return OK;
  }

  int SetPivotBack(int _pivid, int _pivval){

    if(in == 0 || out == 0) return OK;
    back[_pivid] = _pivval;
    return OK;
  }

  int GetPivotForward(int _pivid){
    return forward[_pivid];
  }

  int GetPivotBack(int _pivid){
    return back[_pivid];
  }

};

inline istream& operator >> (istream& s, taskSCC& t) { 
  s >> t.in;
  return s;
};

inline ostream& operator << (ostream& s, const taskSCC& t) {
  //return s <<"["<<t.in<<","<<t.out<<"]";
  return s <<"li:"<<t.localid;
};

class edgeSCC {
public:
  int status;

  edgeSCC() {
    status = PSCC_UNSEEN;
  };
  edgeSCC(int _s){
    status = _s;
  }  
  ~edgeSCC() {}
  inline bool operator== (const edgeSCC &weight){
    
    return  status == weight.status;
  }
  inline edgeSCC& operator= (const edgeSCC &weight){
    status = weight.status;
    return *this;
  }
  void define_type(stapl::typer &t){
  }
};

inline istream& operator >> (istream& s, edgeSCC& w) { 
  return s;
};

inline ostream& operator << (ostream& s, const edgeSCC& w) {
  return s << w.status <<" ";
};


template <class PGRAPH>
class trimer{
 public:
  typedef pGraph<PDG<taskSCC,edgeSCC>,PMG<taskSCC,edgeSCC>, PWG<taskSCC,edgeSCC>, taskSCC,edgeSCC> PGRAPH_SCC;

  PGRAPH* pg;
  PGRAPH_SCC sccg;
  int maxvid;
  int NP;
  vector<VID> pivots;

  

  //connstructor/destructor
  trimer(){}

  trimer(PGRAPH* _pg){
    typename PGRAPH::EI ei;
    taskSCC t;
    edgeSCC dw;
    NP = stapl::get_num_threads();
    int myid = stapl::get_thread_id();

    stapl::rmi_fence();
    //this fence is here because I want the constructor of the sccg be finished
    //before I'll start calling methods on it;

    t.forward.resize(NP,-1);
    t.back.resize(NP,-1);
    pg = _pg;
    
    //the next call is very important; I spent like 1 hour to figure it out;
    //and it is not the first time;
    //sccg.v.reserve(pg->local_size());

    //printf("Constructor of the trimer begin\n");

    for(typename PGRAPH::VI vi = pg->local_begin();vi!=pg->local_end();vi++){
      //printf("for every vertex\n");
      sccg.AddVertex(t,vi->vid,myid);
    }

    //dist init here
    stapl::rmi_fence();
    //this->sccg.initializeDistribution();

    //this fence is here because I want all vertices to be added before starting
    //adding edges; the previous fence can be taken out since the AddVertex
    //step is local only;
    stapl::rmi_fence();

    for(typename PGRAPH::VI vi = pg->local_begin();vi!=pg->local_end();vi++){
      for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	t.in  = vi->predecessors.size();   
	t.out = vi->edgelist.size();	
	sccg.AddEdge(vi->vid,ei->vertex2id,dw);
      }
    }
    //the vertices and edges were copied in the loop above;
    //printf("CONSTRUCTOR FINISHED\n");
  }

  void setpGraph(PGRAPH* _pg){
    typename PGRAPH::EI ei;
    taskSCC t;
    edgeSCC dw;
    NP = stapl::get_num_threads();
    int myid = stapl::get_thread_id();

    //clean the trimer before setting a new pgraph to work with
    sccg.ErasePGraph();

    stapl::rmi_fence();
    //this fence is here because I want the constructor of the sccg be finished
    //before I'll start calling methods on it;
    t.in = 1; t.out = 1;
    t.forward.resize(NP,-1);
    t.back.resize(NP,-1);
    pg = _pg;
    
    //the next call is very important; I spent like 1 hour to figure it out;
    //and it is not the first time;
    //sccg.v.reserve(pg->local_size());

    //printf("Constructor of the trimer begin\n");

    for(typename PGRAPH::VI vi = pg->local_begin();vi!=pg->local_end();vi++){
      //printf("for every vertex\n");
      sccg.AddVertex(t,vi->vid,myid);
    }

    //this fence is here because I want all vertices to be added before starting
    //adding edges; the previous fence can be taken out since the AddVertex
    //step is local only;
    stapl::rmi_fence();
    for(typename PGRAPH::VI vi = pg->local_begin();vi!=pg->local_end();vi++){
      for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	t.in  = vi->predecessors.size();   
	t.out = vi->edgelist.size();	
	sccg.AddEdge(vi->vid,ei->vertex2id,dw);
      }
    }
    //the vertices and edges were copied in the loop above;
    //printf("CONSTRUCTOR FINISHED\n");
    stapl::rmi_fence();
  }

  void DisplayGraph(){
    sccg.pDisplayGraph();
  }

  ~trimer(){
    sccg.local_clear();
  }
  
  void FindPivots(){
    pivots.clear();

    _stapl_pscc_visitor_cycles<PGRAPH_SCC> _vis(sccg,&pivots);
    int temp = DFS_functor_EQ(sccg,_vis);
    
    //printf("pivots number ########%d \n",pivots.size()); 
    
    //int start = rand() % sccg.local_size();
    /*
    PGRAPH_SCC::VI vi;
    for(vi = sccg.local_begin(); vi!=sccg.local_end();vi++){
      if(vi->data.in != 0 && vi->data.out != 0) pivots.push_back(vi->vid);;
    }
    */
    //if(vi!= sccg._end())
    //pivots.push_back(vi->vid);
  }
  /** 
   *find a pivot per CC
   */
  void FindMultiPivots(){
    pivots.clear();
    map<int,vector<int> >           candidates;
    map<int,vector<int> >::iterator mit;
    vector<int> temp;
    int i;
    PGRAPH_SCC::VI vi;
    for(vi = sccg.local_begin(); vi!=sccg.local_end();vi++){
      if(vi->data.in != 0 && vi->data.out != 0){
	mit = candidates.find(vi->data.ccno);
	if(mit == candidates.end()){
	  temp.push_back(vi->vid);
	  candidates[vi->data.ccno] = temp;
	  temp.clear();
	}
	else{
	  mit->second.push_back(vi->vid);
	}
      }
    }
    pivots.resize(candidates.size());
    i=0;
    for(mit = candidates.begin();mit != candidates.end(); mit++){
      pivots[i] = mit->second[mit->second.size() / 2];
      //cout<<"ccno pivot ["<<mit->first<<":"<<pivots[i]<<"]"<<endl;
      i++;
    }
  }

  int FindMultiPivotsSingle(int _ccnos){
    //from Will's code
    vector<int> localwork(_ccnos);
    vector<int> localrank(_ccnos);
    vector<int> totalprocs(_ccnos);    
    vector<int> pivotcandidate(_ccnos);
    map<int,vector<int> >           candidates;
    map<int,vector<int> >::iterator mit;
    vector<int> temp;
    int i, ccid;
    PGRAPH_SCC::VI vi;

    pivots.resize(_ccnos);
    for(ccid = 0;ccid < _ccnos; ccid++){
      localwork[ccid]  =  0;
      localrank[ccid]  =  0;
      totalprocs[ccid] =  0;
      pivotcandidate[ccid] = -1;
    }


    for(vi = sccg.local_begin(); vi!=sccg.local_end();vi++){
      if(vi->data.in != 0 && vi->data.out != 0){

	localwork[vi->data.ccno] = 1;

	mit = candidates.find(vi->data.ccno);
	if(mit == candidates.end()){
	  temp.push_back(vi->vid);
	  candidates[vi->data.ccno] = temp;
	  temp.clear();
	}
	else{
	  mit->second.push_back(vi->vid);
	}
      }
    }

    for(mit = candidates.begin();mit != candidates.end(); mit++){
      pivotcandidate[mit->first] = mit->second[mit->second.size() / 2];
    }

    stapl::rmi_fence();
    stapl::rmi_fence();

#ifdef _STAPL_THREAD
    #warning Not implemented yet... ANY CODE USING IT WILL BE WRONG - JKP/02Dec2004
    stapl_assert(0,"Not implemented yet... ANY CODE USING IT WILL BE WRONG - JKP/02Dec2004");
#else
    MPI_Scan(&localwork[0],&localrank[0],_ccnos, MPI_INT,MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&localwork[0],&totalprocs[0],_ccnos,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
#endif
    stapl::rmi_fence();

    /* DETERMINE IF PIVOT SHOULD BE ON THIS PROCESSOR... IF SO, ASSIGN
     * THE PIVOT INDEX IN M->MESH[IMESH].PIVOT 
     */

    //if(candidates.size() == 0) return -1;

    i=0;
    for(ccid = 0;ccid < _ccnos; ccid++){      
      if(localwork[ccid]==0) {
	localrank[ccid] = -1;
      }
      
      if(totalprocs[ccid]!=0) { 
	if(((totalprocs[ccid]/2)+1) == localrank[ccid]) {
	  pivots[i] = pivotcandidate[ccid];
	  i++;
	}
      }
    }
    return i; //the number of pivots
  }//end method
};

/////////////////////////////////////////////////
// Visitors for trim step - forward and backward
/////////////////////////////////////////////////

template <class _GRAPH>
class _stapl_pscc_trim_forward : public visitor_base<_GRAPH>{
  public:
  _stapl_pscc_trim_forward(){}

  inline int vertex (typename _GRAPH::VI vi) {
    vi->data.SetToZeroIn();
    return OK;
  }

  void define_type(stapl::typer &t) {}
};


template <class _GRAPH>
class _stapl_pscc_trim_back : public visitor_base<_GRAPH>{
  public:
  _stapl_pscc_trim_back(){}

  inline int vertex (typename _GRAPH::VI vi) {
    vi->data.SetToZeroOut();
    return OK;
  }

  void define_type(stapl::typer &t) {}
};

//////////////////////////////////////////////
//      MARK visitors
//////////////////////////////////////////////
template <class _GRAPH>
class _stapl_pscc_mark_forward : public visitor_base<_GRAPH>{
  int pivot_id;
  int pivot_val;
  public:  
  _stapl_pscc_mark_forward(){}

  void set_pivot_info(int _id,int _val){
    pivot_id  = _id;
    pivot_val = _val;
  }
  inline int vertex (typename _GRAPH::VI vi) {
    //for every vertex I'll try to decrement the out variable ;
    if((vi->data.in==0)||(vi->data.out==0)) return -1;
    vi->data.forward[pivot_id] = pivot_val;
    return OK;
  }
  inline int tree_edge(typename _GRAPH::VI vi, typename _GRAPH::EI ei){
    ei->weight.status = PSCC_SEEN;
    return OK;
  }
  void define_type(stapl::typer &t) {
    t.local(pivot_id);
    t.local(pivot_val);
  }
};

template <class _GRAPH>
class _stapl_pscc_mark_back : public visitor_base<_GRAPH>{
  int pivot_id;
  int pivot_val;
  public:  
  _stapl_pscc_mark_back(){}

  void set_pivot_info(int _id,int _val){
    pivot_id  = _id;
    pivot_val = _val;
  }

  inline int vertex (typename _GRAPH::VI vi) {
    //for every vertex I'll try to decrement the out variable ;
    if((vi->data.in==0)||(vi->data.out==0)) return -1;
    vi->data.back[pivot_id] = pivot_val;
    return OK;
  }
  inline int tree_edge(typename _GRAPH::VI vi, typename _GRAPH::EI ei){
    ei->weight.status = PSCC_SEEN;
    return OK;
  }
  void define_type(stapl::typer &t) {
    t.local(pivot_id);
    t.local(pivot_val);
  }
};


  template <class PGRAPH>
  class pSCCWorkFunction{
  public:

     pair<VID,VID> operator()(pair<const VID,vector<VID> >& scc,trimer<PGRAPH>& trimer){

       //remove all back edges; this is a good aproach;

      //here I have to find the edge that has to be cut
      //typename trimer<PGRAPH>::PGRAPH_SCC::VI vi;
      pGraph<PDG<taskSCC,edgeSCC>,PMG<taskSCC,edgeSCC>, PWG<taskSCC,edgeSCC>, taskSCC,edgeSCC>::VI vi;
      pGraph<PDG<taskSCC,edgeSCC>,PMG<taskSCC,edgeSCC>, PWG<taskSCC,edgeSCC>, taskSCC,edgeSCC>::EI ei;
      PARTID partid;

      if(scc.second.size() == 1){
	//self cycle
	return pair<VID,VID>(scc.first,scc.first);
      }

      for(int i=0;i<scc.second.size();i++){
	if(!trimer.sccg.IsLocal(scc.second[i],partid)){
	  //here we are if the vertex is not local;
	  for(int j=0;j<scc.second.size();j++){
	    if(trimer.sccg.IsVertex(scc.second[j],&vi)){
	     for(ei = vi->edgelist.begin(); ei != vi->edgelist.end();ei++){
	       if(ei->vertex2id == scc.second[i]) return pair<VID,VID>(vi->vid, scc.second[i]);
	     } 

	     for(ei = vi->predecessors.begin(); ei != vi->predecessors.end();ei++){
	       if(ei->vertex2id == scc.second[i]) return pair<VID,VID>(scc.second[i],vi->vid);
	     }
	    }
	  }
	}
      }
      //if there is no cross edge
      if(trimer.sccg.IsVertex(scc.first,&vi)){
	//cout<<"Local"<<endl;
	for(int i=0;i<scc.second.size();i++){
	  for(ei = vi->edgelist.begin(); ei != vi->edgelist.end();ei++){
	    if(ei->vertex2id == scc.second[i] && ei->vertex2id != scc.first) 
	      return pair<VID,VID>(vi->vid, scc.second[i]);
	  } 	  
	}
      }
      
      cout<<"Error pSCCWorkFunction:: Edge not found for vertex"<<scc.first<<endl;
      //for(int i=0;i<scc.second.size();i++){
      //cout<<" "<<scc.second[i];
      //}
      //cout<<endl;
      return pair<VID,VID>(-1,-1);
    }
  };



/*
template <class GRAPH,class CONTAINER>
class _filterVisitorSCC: public visitor_base<GRAPH>{
  CONTAINER* finishtimes;
  int ccno;
  int finish;
 public:  
  _filterVisitorSCC(CONTAINER* _v,int _ccno){
   finishtimes = _v;
   //finishtimes->resize(g.size());
   finish = 0;
   ccno = _ccno;
  }
 
  int finish_vertex(typename GRAPH::VI vi){
    //(*finishtimes)[vi->vid].first  = vi->vid;
    vi->data.mark = BLACK;
    finishtimes->push_back(vi);
    return OK;
  }
};
*/




}//end namespace stapl
#endif
