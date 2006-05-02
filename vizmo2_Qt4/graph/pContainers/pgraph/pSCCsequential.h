
#include "GraphAlgo.h"
#include "../pvector/pvector.h"


namespace stapl{


class taskSCCsequential {
public:
  
  taskSCCsequential() {};

  ~taskSCCsequential() {};

  inline bool operator==(const taskSCCsequential &t) const {  
    return  true;
  }

  void define_type(stapl::typer &t){
  }
};

inline istream& operator >> (istream& s, taskSCCsequential& t) { 
  return s;
};

inline ostream& operator << (ostream& s, const taskSCCsequential& t) {
  //return s <<"["<<t.in<<","<<t.out<<"]";
  return s <<" ";
};

class dummywsequential {
public:
  dummywsequential() {};
  dummywsequential(int _e){
  }  
  ~dummywsequential() {}
  inline bool operator== (const dummywsequential &weight){  
    return  true;
  }
  void define_type(stapl::typer &t){
  }
};

inline istream& operator >> (istream& s, dummywsequential& w) { 
  return s;
};

inline ostream& operator << (ostream& s, const dummywsequential& w) {
  return s << "dummy weight";
};


template <class PGRAPH>
class trimerSequential{
 public:
  typedef pGraph<PDG<taskSCCsequential,dummywsequential>,
    PMG<taskSCCsequential,dummywsequential>, 
    PWG<taskSCCsequential,dummywsequential>, 
    taskSCCsequential,dummywsequential> PGRAPH_SCC;

  PGRAPH* pg;
  
  PGRAPH_SCC sccg;//the pgraph to work on it
  pvector<int> tt;//the termination times
  pvector<int> scc;//the termination times


  int maxvid;
  int NP;
  vector<VID> pivots;

  
  //connstructor/destructor
  trimerSequential(){}

  trimerSequential(PGRAPH* _pg){
    typename PGRAPH::EI ei;

    taskSCCsequential t;
    dummywsequential  dw;

    NP = stapl::get_num_threads();
    int myid = stapl::get_thread_id();

    stapl::rmi_fence();
    //this fence is here because I want the constructor of the sccg be finished
    //before I'll start calling methods on it;

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
	sccg.AddEdge(vi->vid,ei->vertex2id,dw);
      }
    }
    //the vertices and edges were copied in the loop above;
    //printf("CONSTRUCTOR FINISHED\n");
  }


  void DisplayGraph(){
    sccg.pDisplayGraph();
  }

  ~trimerSequential(){
    sccg.local_clear();
  }
};



template <class _GRAPH,class TRIMER>
class _stapl_pscc_terminationtimes:public visitor_base<_GRAPH>{

  TRIMER* tr;
  int SZ;

  public:
  _stapl_pscc_terminationtimes(TRIMER* _tr,int _sz){
    tr = _tr;
    SZ = _sz;
  }

  inline int vertex (typename _GRAPH::VI vi) {
    return OK;
  }

  inline int finish_vertex (typename _GRAPH::VI vi, int tt) {

    tr->tt.SetElement(SZ - tt,vi->vid);
    return OK;
  }

  void define_type(stapl::typer &t) {
  }
};


template <class _GRAPH,class TRIMER>
class _stapl_pscc_markingscc:public visitor_base<_GRAPH>{

  TRIMER* tr;
  int scc_no;

  public:
  _stapl_pscc_markingscc(TRIMER* _tr){
    tr = _tr;
  }

  _stapl_pscc_markingscc& operator=(const _stapl_pscc_markingscc &other) {
    scc_no = other.scc_no;
  }

  void SetSCCID(int _id){
    scc_no = _id;
  }

  inline int vertex (typename _GRAPH::VI vi) {
    tr->scc.SetElement(vi->vid,scc_no);//value, where
    return OK;
  }

  inline int finish_vertex (typename _GRAPH::VI vi, int tt) {
    return OK;
  }
  void define_type(stapl::typer &t) {
    t.dynamicOffset(tr, tr, 0);
    t.local(scc_no);
  }

};



template <class PGRAPH>
class pSCCsequential: public BasePObject{
 public:
  
  vector< vector <vector<pair<int,int> > > > finalSCC;
  int maxsz;


  int myid;

  int pvf; 
  int elems;
  int recv;
  double tt;	


  pSCCsequential(){
    this->register_this(this);
    myid = stapl::get_thread_id();
  }

  ~pSCCsequential(){
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
  }

  //use trough rmi to collect all SCC
  void _collect(int tr, pair<VID,int> p){   
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
			      &pSCCsequential<PGRAPH>::_collect,k,SCC[k][j]); 
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

  void SetToken(){
    
  }

  int run(vector<trimerSequential<PGRAPH>*>&  _trimers,
	  vector<vector<pair<VID,int> > >& SCC,int aggf){
    
    typedef pGraph<PDG<taskSCCsequential,dummywsequential>,
      PMG<taskSCCsequential,dummywsequential>, 
      PWG<taskSCCsequential,dummywsequential>, 
      taskSCCsequential,dummywsequential> PGRAPH_SCC;
    typedef pBaseGraph<taskSCCsequential,dummywsequential> PBASE_SCC;
    
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi;
    taskSCC data1,data2; 
    int sz;
    int myid = stapl::get_thread_id();
    int NP = stapl::get_num_threads();
    int i;


    SCC.clear();
    SCC.resize(_trimers.size());

    //for one trimer only;
    for(int tr_id=0;tr_id < _trimers.size();tr_id++){
      //set the SCC id for all vertices 
      for(vi = _trimers[tr_id]->sccg.local_begin(); vi != _trimers[tr_id]->sccg.local_end(); vi++){
	_trimers[tr_id]->scc.AddElement(-1,vi->vid);//value,vid
      }
      sz = _trimers[tr_id]->sccg.size();
      
      _trimers[tr_id]->sccg.pSetPredecessors();
      stapl::rmi_fence();

      //set the termination time to 0;
      if(myid!=NP-1){
	for(i=0;i<sz / NP;i++){
	  //cout<<"adding "<<i+myid*(sz/NP)<<endl;
	  _trimers[tr_id]->tt.AddElement(0,i+myid*(sz/NP));
	}
      }
      else{
	//the last processor
	for(i=(NP-1)*(sz / NP); i<sz ;i++){
	  //cout<<"adding "<<i<<endl;
	  _trimers[tr_id]->tt.AddElement(i,i);
	}
      }

      _stapl_pscc_terminationtimes<PGRAPH_SCC,trimerSequential<PGRAPH> > _vis_termination(_trimers[tr_id],sz);
      _stapl_pscc_markingscc<PGRAPH_SCC,trimerSequential<PGRAPH> > _vis_marking(_trimers[tr_id]);

      /*   
	   i = 0;
	   for(pvector<int>::iterator pvi=_trimers[tr_id]->tt.local_begin();pvi!=_trimers[tr_id]->tt.local_end();pvi++){
	   //cout<<"@@@@"<<stapl::get_thread_id()<<":"<<*pvi<<endl;
	   cout<<"FFF"<<stapl::get_thread_id()<<":"<<_trimers[tr_id]->tt.GetElement(i++)<<endl;
	   }
	   stapl::rmi_fence();
      */

      pDFSAllFunctor<PGRAPH_SCC,_stapl_pscc_terminationtimes<PGRAPH_SCC,trimerSequential<PGRAPH> > >(_trimers[tr_id]->sccg , _vis_termination);


    
      //here I have to call DFS on the transpose pGraph
      //the starting vertices are choosen according to the termination time computed 
      //in the previous loop;
      pDFSAllTransposeFunctor<PGRAPH_SCC,_stapl_pscc_markingscc<PGRAPH_SCC,trimerSequential<PGRAPH> > >(_trimers[tr_id]->sccg , _vis_marking,_trimers[tr_id]->tt);


      //here I transfer the values stored in pvector t SCC;
      if(myid!=NP-1){
	for(i=0;i<sz / NP;i++){
	  int ccno = _trimers[tr_id]->scc.GetElement(i+myid*(sz/NP));
	  if(ccno != -1)
	    SCC[tr_id].push_back(pair<int,int>(i+myid*(sz/NP),ccno));
	}
      }
      else{
	//the last processor
	for(i=(NP-1)*(sz / NP); i<sz ;i++){
	  int ccno = _trimers[tr_id]->scc.GetElement(i);
	  if(ccno != -1)
	    SCC[tr_id].push_back(pair<int,int>(i,ccno));
	}
      }

      //for(pvector<int>::iterator pvi=_trimers[tr_id]->scc.local_begin();pvi!=_trimers[tr_id]->scc.local_end();pvi++){
      //cout<<">>>>>>"<<stapl::get_thread_id()<<":"<<*pvi<<endl;
      //}
    }
  }


};//end class pSCCsequential


}//end namespace stapl
