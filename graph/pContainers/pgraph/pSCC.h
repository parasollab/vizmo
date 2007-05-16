#ifndef _PSCC
#define _PSCC

#include "../../common/Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "task.h"
#include "runtime.h"

#include <assert.h>
//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////

namespace stapl{


#define STAPL_DEAD -100

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
  int in; //in dependencies
  int out;//out dependencies
  
  //vectors for multiple directions

  vector<int> forward;
  vector<int> back;

  //int forward;
  //int back;

  
  taskSCC() {
    ccno=0;
  }
  taskSCC(int _vwt) {
    in = out = _vwt;
    ccno=0;
    //!! remember to initialize forward backward
  };

  ~taskSCC() {};

  void define_type(stapl::typer &t){
    t.local(in);t.local(out);
    t.local(forward);t.local(back);
  }

  //===================================================================
  // Operators
  //===================================================================
  inline int decrement_in(){
    if(in == 0) return STAPL_DEAD;
    in = in - 1;
    return in;
  }

  inline int decrement_out(){
    if(out == 0) return STAPL_DEAD;
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
      back[i] = -1;
    }
  }


};

inline istream& operator >> (istream& s, taskSCC& t) { 
  s >> t.in;
  return s;
};

inline ostream& operator << (ostream& s, const taskSCC& t) {
  //return s <<"["<<t.in<<","<<t.out<<"]";
  return s <<" ";
};

class dummyw {
public:
  dummyw() {};
  dummyw(int _e){
  }  
  ~dummyw() {}
  inline bool operator== (const dummyw &weight){  
    return  true;
  }
  inline dummyw& operator= (const dummyw &weight){
    return *this;
  }
  void define_type(stapl::typer &t){
  }
};

inline istream& operator >> (istream& s, dummyw& w) { 
  return s;
};

inline ostream& operator << (ostream& s, const dummyw& w) {
  return s << "dummy weight";
};


template <class PGRAPH>
class trimer{
 public:
  typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;

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
    dummyw dw;
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
    dummyw dw;
    NP = stapl::get_num_threads();
    int myid = stapl::get_thread_id();

    //clean the trimer before setting a new pgraph to work with
    sccg.ErasePGraph();

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
};


template <class PGRAPH,class TR>
class trim: public BasePObject{  
 public:
  typedef pGraph<PDG<taskSCC,dummyw>,
                 PMG<taskSCC,dummyw>, 
                 PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
  typedef map<VID,int> MAP;
  int f,b;

  vector<TR*> trimers;
  //vertex vi , index and direction(fwd,back)
  list<_StaplTriple<typename PGRAPH_SCC::VI,int,int> > QT;//queue used for trimer traversals
  bool inside;

  trim(){
    cout<<"Default constructor will not give you correct results"<<endl;
  }

  trim(vector<TR*> _trs){
    //see if I can avoid the copy here
    trimers = _trs;
    f=0; b=0;
    register_this(this);
  }

  void rescan(){     
    for(int i=0;i<trimers.size();i++){

      trimers[i]->sccg.pSetPredecessors();

      for(typename PGRAPH_SCC::VI vi = trimers[i]->sccg.local_begin();vi!=trimers[i]->sccg.local_end();vi++){
	//printf("for every vertex\n");
	vi->data.reset_fb();	
	vi->data.in = vi->predecessors.size();
	vi->data.out = vi->edgelist.size();
	if(vi->data.in  == 0)  {
	  f++;
	  // cout <<"--->"<< vi->vid<<endl;
	  addQueue(vi,i); 
	}
	if(vi->data.out == 0) {
	  b++;
	  //cout <<"---<"<< vi->vid<<endl;
	  addQueueBack(vi,i);
	  
	}
      }//for every vertex
    }//for every trimer
  }

  void rescan_back(){     
    for(int i=0;i<trimers.size();i++){
      //      trimers[i]->sccg.SetPredecessors();
      for(typename PGRAPH_SCC::VI vi = trimers[i]->sccg.local_begin();
	  vi!=trimers[i]->sccg.local_end();vi++){
	//printf("for every vertex\n");	
	if(vi->data.out == 0) {
	  printf("BAcKWARD................%d %d\n",vi->vid, i);
	  b++;
	  addQueueBack(vi,i);
	}
      }//for every vertex
    }//for every trimer
  }

  void addQueue(PGRAPH_SCC::VI vi,int index){
    QT.push_back(_StaplTriple<typename PGRAPH_SCC::VI,int,int>(vi,index,STAPL_PSCC_FWD));
  }

  void addQueueBack(PGRAPH_SCC::VI vi,int index){
    QT.push_back(_StaplTriple<typename PGRAPH_SCC::VI,int,int>(vi,index,STAPL_PSCC_BACK));
  }

  void addQueueFront(PGRAPH_SCC::VI vi,int index){
    QT.push_front(_StaplTriple<typename PGRAPH_SCC::VI,int,int>(vi,index,STAPL_PSCC_FWD));
  }

  void addQueueBackFront(PGRAPH_SCC::VI vi,int index){
    QT.push_front(_StaplTriple<typename PGRAPH_SCC::VI,int,int>(vi,index,STAPL_PSCC_BACK));
  }

  _StaplTriple<typename PGRAPH_SCC::VI,int,int> popQueue(){
    _StaplTriple<typename PGRAPH_SCC::VI,int,int> temp = QT.front();
    QT.pop_front();
    return temp;
  }
  
  inline int size(){
    return QT.size();
  }

  void remoteAdd(VID vid,int index){
    PGRAPH_SCC::VI vi;
    if(!trimers[index]->sccg.IsVertex(vid,&vi)){
      cout<<"ERROR: while an rmi method invoked: trim::remoteAdd; Invalid vid "<<vid<<"for trim no "<<index<<" on node "<<stapl::get_thread_id()<<endl;
      return;
    }
    //here vi is initialized properly;
    int ret = vi->data.decrement_in();
    if(ret == ERROR) return;
    if(ret == STAPL_DEAD ) return;//do nothing; ignore the vertex
    if(ret == 0){
      //add u to q
      //if(vi->vid==2989) printf("------------> in is zero remote %d\n",stapl::get_thread_id());
      addQueue(vi,index);
       //here I have to call the trim_loop
      //?????????????????????????????????
      //see if I can use a flag to avoid the recursivity
      if (inside == false) trimLoop();
    }
  }

  void remoteAddBack(VID vid,int index){
    PGRAPH_SCC::VI vi;
    if(!trimers[index]->sccg.IsVertex(vid,&vi)){
      printf("ERROR: while an rmi method invoked: trim::remoteAddBack\n");
      return;
    }
    //here vi is initialized properly;
    int ret = vi->data.decrement_out();
    if(ret == ERROR) return;
    if(ret == STAPL_DEAD ) return;//do nothing; ignore the vertex
    if(ret == 0){
      //add u to q
      addQueueBack(vi,index);

      //here I have to call the trim_loop
      //see if I can use a flag to avoid the recursivity
      if (inside == false) trimLoop();
    }
  }

  void trimLoop(){
    //sweep to the graph associated with this trimer
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi;
    //vi , index, direction

    _StaplTriple<typename PGRAPH_SCC::VI,int,int> v;
    inside = true;
    vector<VID> succs;
    vector<VID> preds;
    //printf("start loop for %d  ==========q size %d\n",index,QT.size());
    while(size() > 0){//while there are more vertices
      //pop a vertex from Q
      v = popQueue();
      if(v.third == STAPL_PSCC_FWD){
	trimers[v.second]->sccg.GetSuccessors(v.first,succs);
	for(int v2id=0;v2id<succs.size();v2id++){
	  if(!trimers[v.second]->sccg.IsVertex(succs[v2id],&vi)){
	    //the vertex is remote
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    Location _n=trimers[v.second]->sccg.Lookup(succs[v2id]);
	    if(!_n.ValidLocation()){
	      cout<<"ERROR:: Inside trim::trimLoop"<<succs[v2id]<<" "<<stapl::get_thread_id()<<endl;
	    }
	    stapl_assert(_n.ValidLocation(),"Invalid vertex id inside trim::trimLoop ");
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &trim::remoteAdd,succs[v2id],v.second);
	    continue;
	  }
	  //the vertex is local
	  //here vi is initialized correctly
	  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  //!!if(vi->data.out == 0) continue;//the node is dead due to forward trim

	  int ret = vi->data.decrement_in();
	  if(ret == ERROR) {inside=false;return;}
	  if(ret == STAPL_DEAD ) continue;//do nothing; ignore the vertex
	  if(ret == 0){
	    //add u to q
	    addQueueFront(vi,v.second);
	  }
	}
      }
      else{
	//cout<<"predecessors....1"<<endl;
	trimers[v.second]->sccg.GetPredecessors(v.first,preds);
	for(int v2id=0;v2id<preds.size();v2id++){
	  //cout<<"predecessors....2"<<endl;
	  if(!trimers[v.second]->sccg.IsVertex(preds[v2id],&vi)){
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    Location _n=trimers[v.second]->sccg.Lookup(preds[v2id]);
	    if(!_n.ValidLocation()){
	      cout<<"ERROR:: Inside trim::trimLoop"<<preds[v2id]<<" "<<stapl::get_thread_id()<<endl;
	    }
	    stapl_assert(_n.ValidLocation(),"Invalid vertex id inside trim::trimLoop ");
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &trim::remoteAddBack,preds[v2id],v.second);
	    continue;
	  }
	  //the vertex is local
	  //here vi is initialized correctly
	  
	  //HERE IAm and I think the next line is wrong because it stops advancing for sme nodes
	  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  //!!if(vi->data.in == 0) continue;//the node is dead due to forward trim

	  int ret = vi->data.decrement_out();
	  if(ret == ERROR) {inside=false;return;}
	  if(ret == STAPL_DEAD ) continue;//do nothing; ignore the vertex
	  if(ret == 0){
	    //add u to q
	    addQueueBackFront(vi,v.second);
	  }
	}
      }
    }//end while
    inside=false;
  }//end method trim_loop 



  int run(){
    //first init the trimers with vertices having 0 indegree
    //done by default in the constructor;
    //second start sweeping on any trimer that has vertices available
    //-----forward
    inside = false;
    //rescan succs/preds 
    rescan();
    /*
    for(typename PGRAPH_SCC::VI vi = trimers[0]->sccg.local_begin();vi!=trimers[0]->sccg.local_end();vi++){
      for(typename PGRAPH_SCC::EI ei = vi->predecessors.begin();ei != vi->predecessors.end();ei++){
	printf("%d %d\n",vi->vid,ei->vertex2id);
      }
    }
    */

    //trimers[0]->DisplayGraph();
    //cout<<"REscan finished"<<endl;
    trimLoop();
    stapl::rmi_fence();
    return OK;
  }

  int run_back(){
    inside = false;
    rescan_back();
    trimLoop();
    stapl::rmi_fence();
    return OK;
  }
};


template <class PGRAPH,class TR>
class mark: public BasePObject{  
 public:
  typedef pGraph<PDG<taskSCC,dummyw>,
                 PMG<taskSCC,dummyw>, 
                 PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
  typedef map<VID,int> MAP;

  vector<TR*> trimers;
  
  //next we have vertex pointer, index_trimer, direction, pivot_id and pivot_value
  list<_StaplPenta<typename PGRAPH_SCC::VI,int, int,int,int> > QM;//queue used for forward traversal
  
  bool inside;
  int globalc;
  int myid;

  int pvf; 
  int pvc;

  int elems;
  int recv;
  double tt;	
 
  mark(){
    cout<<"Default constructor will not give you correct results"<<endl;
  }

  mark(vector<TR*> _trs){
    //see if I can avoid the copy here
    trimers = _trs;
    myid = stapl::get_thread_id();

    //counters;
    pvf=0;
    pvc=0;
    elems=0;
    recv=0;
    tt = 0.0;
    register_this(this);
  }

  void addMark(PGRAPH_SCC::VI vi,int index, int direction,int pivot_id,VID pivot_val){
    QM.push_back(_StaplPenta<PGRAPH_SCC::VI,int,int,int,VID>(vi,index,direction,pivot_id,pivot_val));
  }
 
  void addMarkFront(PGRAPH_SCC::VI vi,int index, int direction,int pivot_id,VID pivot_val){
    QM.push_front(_StaplPenta<PGRAPH_SCC::VI,int,int,int,VID>(vi,index,direction,pivot_id,pivot_val));
  }
  
  _StaplPenta<PGRAPH_SCC::VI,int,int,int,VID> popQueueMark(){
    _StaplPenta<PGRAPH_SCC::VI,int,int,int,VID> temp = QM.front();
    QM.pop_front();
    return temp;
  }

  inline int sizeMark(){
    return QM.size();
  }

  void remoteAddMarkFwd(_StaplQuad<VID,int,int,int> _arg){
    PGRAPH_SCC::VI vi;
    VID vid = _arg.first;
    int index = _arg.second;
    int pivot_id = _arg.third;
    int pivot_val = _arg.fourth;

    recv++;

    if(!trimers[index]->sccg.IsVertex(vid,&vi)){
      //printf("ERROR: while an rmi method invoked: mark::remoteAddMarkFwd\n");
      return;
    }
    //check if the vertex is dead
    if(vi->data.in==0||vi->data.out==0) return;

    if(vi->data.forward[pivot_id] == -1) 
      addMark(vi,index,STAPL_PSCC_FWD,pivot_id,pivot_val);

    if(inside==false) markLoop();
  }

  void remoteAddMarkBack(_StaplQuad<VID,int,int,int> _arg){
    PGRAPH_SCC::VI vi;
    VID vid       = _arg.first;
    int index     = _arg.second;//which trimer
    int pivot_id  = _arg.third;
    int pivot_val = _arg.fourth;

    //assert(vid <10164);
    recv++;

    if(!trimers[index]->sccg.IsVertex(vid,&vi)){
      //printf("ERROR: while an rmi method invoked: mark::remoteAddMarkFwd\n");
      return;
    }
    //check if the vertex is dead
    if(vi->data.in==0||vi->data.out==0) return;

    if(vi->data.back[pivot_id] == -1) 
      addMark(vi,index,STAPL_PSCC_BACK,pivot_id,pivot_val);

    if(inside==false) markLoop();
  }

  
  void markLoop(){
    //sweep to the graph associated with this trimer
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi;
    vector<VID> succs;
    vector<VID> preds;

    stapl::timer t;
    double t2;
    t = stapl::start_timer();

    //printf("mark loop started %d for pivot %d\n",tr->sizeMark(),pivot);
    inside = true;
    while(sizeMark() > 0){

      globalc++;
      //next we have [vertex pointer, index_trimer, direction, pivot_id and pivot_value]
      _StaplPenta<PGRAPH_SCC::VI,int,int,int,VID> v = popQueueMark();
      
      //next we have v as vertex pointer, direction, pivot_id and pivot_value

      //assert(v.first->vid <10164);

      if (v.third == STAPL_PSCC_FWD){
	v.first->data.forward[v.fourth] = v.fifth;
      }
      else{
	v.first->data.back[v.fourth] = v.fifth;
	//v.first->data.back = pivot;
      }
      //go trough predecessors/succesors correspondingly
      if(v.third == STAPL_PSCC_FWD){
	//forward ; go trough successors
	trimers[v.second]->sccg.GetSuccessors(v.first,succs);
	for(int v2id=0;v2id<succs.size();v2id++){
	  //for every successor
	  //if(v.first->vid == 15||v.first->vid==20) 
	  //  printf("s->[%d->%d]%d[%d]\n",v.first->vid,succs[v2id],stapl::get_thread_id(),v.fourth);

	  //if(!tr->sccg.IsVertex(ei->vertex2id,&vi)){
	  if(!trimers[v.second]->sccg.IsVertex(succs[v2id],&vi)){
	    //remote
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    //printf("remote %d\n",vi->vid);
	    _StaplQuad<VID,int,int,int> arg(succs[v2id],v.second,v.fourth,v.fifth);

	    //Check here !!!                                              INCORRECT
	    elems++;

	    //-check this line !!!!  vi->data.forward[v.fourth] = v.fifth;

	    Location _n=trimers[v.second]->sccg.Lookup(succs[v2id]);
	    if(!_n.ValidLocation()){
	      cout<<"ERROR:: Inside trim::trimLoop"<<succs[v2id]<<" "<<stapl::get_thread_id()<<endl;
	    }
	    stapl_assert(_n.ValidLocation(),"Invalid vertex id inside mark::markLoop ");
	    t2 = stapl::stop_timer(t);		
	    tt+=t2;
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &mark::remoteAddMarkFwd,arg); 
	    t = stapl::start_timer();
	    continue;
	  }
	  //vertex local
	  //check if the vertex is dead
	  if((vi->data.in==0)||(vi->data.out==0)) continue;
	  if(vi->data.forward[v.fourth] == -1){
	    addMarkFront(vi,v.second,STAPL_PSCC_FWD,v.fourth,v.fifth);
	    vi->data.forward[v.fourth] = v.fifth;
	  }
	}
      }
      else{
	//backward: go trough predecessors
	int wh=0;
	trimers[v.second]->sccg.GetPredecessors(v.first,preds);
	for(int v2id=0;v2id<preds.size();v2id++){
	  //for every successor
	  //if(v.first->vid == 15||v.first->vid==20) 
	  //  printf("p->[%d->%d]%d[%d]\n",v.first->vid,succs[v2id],stapl::get_thread_id(),v.fourth);

	  if(!trimers[v.second]->sccg.IsVertex(preds[v2id],&vi)){
	    //remote
	    //here add trough an rmi the vertex in the other 
	    //processor queue of ready vertices
	    if(preds[v2id]==20)
	      printf("STAPL_PSCC_REMOTEEEEEE back %d\n",preds[v2id]);
	    elems++;
	    _StaplQuad<VID,int,int,int> arg(preds[v2id],v.second,v.fourth,v.fifth);

	    //Check here !!!                                              INCORRECT
	    //check this line  !!!!! vi->data.back[v.fourth] = v.fifth;

	    Location _n=trimers[v.second]->sccg.Lookup(preds[v2id]);
	    if(!_n.ValidLocation()){
	      cout<<"ERROR:: Inside trim::trimLoop"<<preds[v2id]<<" "<<stapl::get_thread_id()<<endl;
	    }
	    stapl_assert(_n.ValidLocation(),"Invalid vertex id inside mark::markLoop ");
	    stapl::async_rmi( _n.locpid(), 
			      getHandle(),
			      &mark::remoteAddMarkBack,arg); 
	    continue;
	  }
	  wh++;
	  //vertex local
	  //here vi is u;
	  //check if the vertex is dead
	  if(vi->data.in==0||vi->data.out==0) continue;
	  //printf("local back %d %d\n",vi->vid,vi->data.back);
	  if(vi->data.back[v.fourth] == -1){
	    addMarkFront(vi,v.second,STAPL_PSCC_BACK,v.fourth,v.fifth);
	    vi->data.back[v.fourth] = v.fifth;
	  }
	}
      }
    }
    inside = false;
    t2 = stapl::stop_timer(t);		
    tt+=t2;
  }

  void _MAX(int* in, int* inout) { *inout = (*in>*inout)?*in:*inout; }
  void _MIN(int* in, int* inout) { *inout = (*in<*inout)?*in:*inout; }

 
  bool run(){
    PGRAPH_SCC::VI vi;
    int vid;
    int global;

    int finished = 0;
    inside = false;
    stapl::timer t;

    globalc=0;
    t = stapl::start_timer();

    for(int i=0;i<trimers.size();i++){

 
      trimers[i]->FindPivots();
      //cout<<"marker's pivots size "<<trimers[i]->pivots.size()<<endl;

      if(trimers[i]->pivots.size() == 0){
	for(vi = trimers[i]->sccg.local_begin();vi!=trimers[i]->sccg.local_end();vi++){
	  if(vi->data.in != 0 && vi->data.out != 0) break;
	}
	//if no vertices available skip;
	if(vi == trimers[i]->sccg.local_end()) {
	  //printf("End detected\n");
	  vid = -1;
	  finished++;
	}
	else {
	  vid = vi->vid;
	  pvf++;
	}
      }
      else{
	vid = trimers[i]->pivots[trimers[i]->pivots.size()/2];
	//cout<<"VID:"<<vid<<endl;
	if(!trimers[i]->sccg.IsVertex(vid,&vi)){
	  printf("ERROR: class mark pivot inexistent\n");
	}
      }      
      global = vid;

      if(/*global == vid &&*/ global != -1){
	//printf("inside %d\n",stapl::get_thread_id());
	//-if(global > trimers[i]->maxvid) global = global - CYCLE;

	pvf++;
	addMark(vi,i,STAPL_PSCC_FWD,myid,global);
	addMark(vi,i,STAPL_PSCC_BACK,myid,global);

	//printf("mark started %d for pivot %d [%d %d]\n",stapl::get_thread_id(),global,vi->data.in,vi->data.out);
	
      }    
    }//for every trimer/graph

    double t2 = stapl::stop_timer(t);    
    //printf("find pivots %f\n",t2);


    tt+=t2;
    //!!!!!!!!!
    markLoop();

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::reduce_rmi<mark, int>(&finished, 
				 &global, 
				 getHandle(),
				 &mark::_MIN, 
				 true);
    //printf(" minimum %d\n",global); 
    return global==trimers.size();
  }
};


template <class PGRAPH,class TR>
class pSCC: public BasePObject{
 public:
  vector< vector <vector<pair<int,int> > > > finalSCC;
  int maxsz;

  pSCC(){
    register_this(this);
  }
  //use trough rmi to collect all SCC

  void _collect(int tr, pair<VID,int> p){   
    //if(p.first==3451) printf("------------> considered coll\n");
    finalSCC[tr][p.second].push_back(p);
  }

  void collect(vector<vector<pair<VID,int> > >& SCC, int nrtr, int maxsz){
    int i,j,k;
    int id = stapl::get_thread_id();

    finalSCC.resize(nrtr);
    if(id==0){
      for(i=0;i<finalSCC.size();i++){
	finalSCC[i].resize(maxsz);
      }
    }

    if(id==0){
	for(k=0;k<SCC.size();k++){
	  for(j=0;j<SCC[k].size();j++){
	    //printf("[%d %d]",SCC[k][j].first,SCC[k][j].second);
	    _collect(k,SCC[k][j]);
	  }
	  //printf("\n");
	}
      }
      else{
	for(k=0;k<SCC.size();k++){
	  for(j=0;j<SCC[k].size();j++){
	    stapl::async_rmi( 0, 
			      this->getHandle(),
			      &pSCC<PGRAPH,trimer<PGRAPH> >::_collect,k,SCC[k][j]); 
	    //pssc.collect(scc[k][j]);
	  }
	  //printf("\n");
	}
      }
      stapl::rmi_fence();
      //}//for every trimer
  }

  void dump(char* file_name,int tr){
    if(stapl::get_thread_id()==0){
      char * actual_file_name=new char[100];
      sprintf(actual_file_name,"%s.scc",file_name);
      printf("writing scc to file [%s]\n",actual_file_name);
      ofstream _myistream(actual_file_name);
      
      if(!_myistream.is_open()){
	cout<<"Error opening output file "<<actual_file_name<<endl;
	return;
      }
      int temp;
      for(int i=0;i<finalSCC[tr].size();i++){
	if(finalSCC[tr][i].size()>1){
	  for(int j=0;j<finalSCC[tr][i].size();j++){
	    _myistream<<finalSCC[tr][i][j].first<<" "<<finalSCC[tr][i][j].second<<endl;
	  }
	}
      }
      _myistream.close();
    }
  }



  int run(vector<TR*>&  _trimers,vector<vector<pair<VID,int> > >& SCC,int aggf){
    typedef pGraph<PDG<taskSCC,dummyw>,
                   PMG<taskSCC,dummyw>, 
                   PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi2;
    taskSCC data1,data2;

    bool finished=false;

    int iteration = 0;
    int trashc=0;

    double t1, t2, t3,t4,t5;
    
    double trt, mkt, sdelt, delt;

    trt=mkt=sdelt=delt=0.0;
    
    vector<VID> to_delete;
    vector<VID> del_edgef;
    vector<VID> del_edgeb;

    
    int j,k,pivt;
    int tr_id;
    stapl::timer t;
    SCC.clear();

    trim<PGRAPH,trimer<PGRAPH> > tr(_trimers);
    mark<PGRAPH,trimer<PGRAPH> > mk(_trimers);
    SCC.resize(_trimers.size());
    
    int myid = stapl::get_thread_id();

    //needed because the constructors has to finish
    stapl::rmi_fence();
    
    while(!finished){
      iteration++;

      finished = true;
      //      printf("trim started -------------->%d\n",iteration);
  
      stapl::set_aggregation(aggf);
      //_trimers[0]->DisplayGraph();

      t = stapl::start_timer();
      tr.run();
      t1 = stapl::stop_timer(t);    
  
      trashc = 0;
      for(typename PGRAPH_SCC::VI vi_t = _trimers[0]->sccg.local_begin();vi_t!=_trimers[0]->sccg.local_end();vi_t++){
	//printf("it%d %d [%d %d]\n",iteration,vi_t->vid, vi_t->data.in, vi_t->data.out);
	if(vi_t->data.in == 0  ||  vi_t->data.out==0) trashc++;
      } 
      
      //printf("===%d===========%d\n",iteration,trashc);
      // _trimers[0]->DisplayGraph();

      trashc = 0;

      for(typename PGRAPH_SCC::VI vi_t = _trimers[0]->sccg.local_begin();vi_t!=_trimers[0]->sccg.local_end();vi_t++){
	//printf("it%d %d [%d %d]\n",iteration,vi_t->vid, vi_t->data.in, vi_t->data.out);
	if(vi_t->data.in == 0  ||  vi_t->data.out==0) trashc++;
      } 

      //printf("===%d===========%d\n",iteration,trashc);


      //here I have to choose a pivot among vertices that are still
      //alive
      //printf("mark started\n");
      //stapl::set_aggregation(aggf);
      t = stapl::start_timer();
      finished = mk.run();
      t2 = stapl::stop_timer(t);    
      //stapl::set_poll_rate(1);
      //printf("------globalc %d\n",mk.globalc);
      //finished = true;
      //stapl::set_aggregation(1);
      
      //stapl::rmi_fence();
      //printf("take out\n");
      //here I have to take out from the graph the dead vertices and the SCC
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete.clear();
	t = stapl::start_timer();
	if(mk.sizeMark()>0){
	  printf("ERROR there are elements left in the queues\n");
	}
	
	for(PGRAPH_SCC::VI vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();vi++){
	  //printf("for every vertex\n");
	  //if dead remove
	  //SCC.push_back();
 
	  if(vi->data.in == 0 || vi->data.out==0){
	    to_delete.push_back(vi->vid);
	    continue;
	  }

	  //the node is alive;
	  //check if it part of the SCC
	  //the second condition in the next if is to take out 
	  //the Rem nodes;
	  for(pivt=0;pivt<vi->data.forward.size();pivt++){
	    if((vi->data.forward[pivt] == vi->data.back[pivt]) 
	       && (vi->data.back[pivt] != -1)){
	      SCC[tr_id].push_back(pair<VID,int>(vi->vid,vi->data.forward[pivt]));
	      to_delete.push_back(vi->vid);
	      // cout<<"SCC considered:"<<vi->vid<<":"<<vi->data.forward[pivt]<<":"<<pivt<<":"<<vi->data.forward[3]<<vi->data.back[3]<<endl;
	      break;
	    }
	  }//for every choosen pivot

	  

#ifdef STAPL_PSCC_REM
	  //after we delete the vertices that are dead or in scc
	  //we have to remove edges between rem and forw and trim
	  data1 = vi->data;
	  for(pivt=0;pivt < data1.forward.size();pivt++){
	    if(data1.back[pivt] == -1 && data1.forward[pivt] == -1) break;
	  }
	  //if at least one position is -1 on forw and back (untouched by marking)
	  if(pivt < data1.forward.size()){
	    //printf("posible rem candidate %d\n",vi->vid);
	    //vi belongs to remaining
	    del_edgef.clear();
	    for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){	      
	      //#ifdef STAPL_PSCC_REMOTE
	      if(!_trimers[tr_id]->sccg.IsLocal(ei->vertex2id)){
		  continue;
	      }
	      //#endif
	      data2 = _trimers[tr_id]->sccg.GetData(ei->vertex2id);

	      if(data2.in == 0 || data2.out==0) continue;
	 
	      for(k=pivt;k < data1.forward.size();k++){
		if(data1.back[k] == -1 && data1.forward[k] == -1){
		  if(data2.back[k] != -1 || data2.forward[k] != -1){
		    //the dest node is not in remaining; delete the edge
		    del_edgef.push_back(ei->vertex2id);
		    //break;  //jump
		  }      
		}        
	      }//for k
	      //here
	    }//for every edge

	    for(ei = vi->predecessors.begin();ei != vi->predecessors.end();ei++){
	      //#ifdef STAPL_PSCC_REMOTE
	      if(!_trimers[tr_id]->sccg.IsLocal(ei->vertex2id)){
		  continue;
	      }
	      //#endif 
	      data2 = _trimers[tr_id]->sccg.GetData(ei->vertex2id);

	      if(data2.in == 0 || data2.out==0) continue;

	      //printf("Check edge %d\n",ei->vertex2id);

	      //here vi2 is initialized with the second vertex
	      for(k=pivt;k < data1.forward.size();k++){
		if(data1.back[k] == -1 && data1.forward[k] == -1){
		  if(data2.back[k] != -1 || data2.forward[k] != -1){
		    _trimers[tr_id]->sccg.DeleteEdge(ei->vertex2id,vi->vid);
		    //break;  //jump
		  }      
		}        
	      }//for k
	      //here
	    }
	    //efficient way to delete edges
	    for(j=0;j<del_edgef.size();j++){
	      vi->DeleteXEdges(vi->vid,1);
	    }
	  }
#endif
	  //reset the data for the next iteration
	  //vi->data.reset_fb();
	}
	t5 = stapl::stop_timer(t);    
	t = stapl::start_timer();

	//here we have to remove edges between pred and scc
	//and edges between succ and scc (both directions)

	/*
	printf("Delete %d vertices\n",to_delete.size());
	stapl::rmi_fence();
	for(vector<VID>::iterator it=to_delete.begin();it!=to_delete.end();it++){
	  _trimers[tr_id]->sccg.DeleteVertex(*it);
	}
	stapl::rmi_fence();
	//_trimers[tr_id]->DisplayGraph();
	*/

	//printf("delete vertices NOT efficient\n");
	//here I need to implement a faster version for pDelete
	stapl::rmi_fence();
	_trimers[tr_id]->sccg.pDeleteVertices(to_delete);

	stapl::rmi_fence();

	//stapl::set_aggregation(1);

	t3 = stapl::stop_timer(t);    	
      }//for every trimer/graph

      //stapl::rmi_fence();
      /*
      t = stapl::start_timer();
      //rescan the graphs associated with trim/mark
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.UpdateMap();
      }
      t4 = stapl::stop_timer(t);
      */

      //if(stapl::get_thread_id()==0) 
      //	printf("Elapsed [iteration %d] trim=%f mark=%f search_del=%f delete=%f rescan=%f\n",
      //       iteration,t1,t2,t5,t3,t4);
      trt +=t1;
      mkt+=t2;
      sdelt+=t5;
      delt+=t3;
    }//while !finished
    stapl::rmi_fence();
    if (stapl::get_thread_id()==0) 
     	printf("Elapsed [iterations %d] trim=%f mark=%f search_del=%f delete=%f \n", iteration,trt,mkt,sdelt,delt);

    printf("pivots for %d -> %d [%d] [%d][sent %d recv %d] [mktt %f]\n",
	   stapl::get_thread_id(),mk.pvf,tr.f,tr.b,mk.elems,mk.recv,mk.tt);
    return OK;    
  }
};


}//end namespace stapl
#endif
