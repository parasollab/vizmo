#ifndef PSCCGENERIC
#define PSCCGENERIC

#include "../../common/Defines.h"
#include "Graph.h"
#include "GraphAlgo.h"
#include "task.h"
#include "runtime.h"

#include "pSCC.h"

#include <assert.h>
//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////



namespace stapl{

//#define OLDSCCREM
#define REMBEFOREMARK
#define STAPL_PSSCG_REMOTE

template <class _GRAPH>
class _stapl_pscc_trim_forward{
  public:
  _stapl_pscc_trim_forward(){
    //cout<<"Visitor Trim Forward Instantiated"<<endl; 
  };

  inline int vertex (typename _GRAPH::VI vi) {
    vi->data.SetToZeroIn();
    return OK;
  }

  void define_type(stapl::typer &t) {
  }
};


template <class _GRAPH>
class _stapl_pscc_trim_back{
  public:
  _stapl_pscc_trim_back(){
    //cout<<"Visitor Trim Back Instantiated"<<endl; 
  };

  inline int vertex (typename _GRAPH::VI vi) {
    vi->data.SetToZeroOut();
    return OK;
  }

  void define_type(stapl::typer &t) {
  }
};


template <class _GRAPH>
class _stapl_pscc_mark_forward{//: public visitor_base<_GRAPH>{
  int pivot_id;
  int pivot_val;
  public:  
  _stapl_pscc_mark_forward(){
    //cout<<"Visitor Mark instantiated"<<endl; 
  };

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
  void define_type(stapl::typer &t) {
    t.local(pivot_id);
    t.local(pivot_val);
  }

};


template <class _GRAPH>
class _stapl_pscc_mark_back{
  int pivot_id;
  int pivot_val;
  public:  
  _stapl_pscc_mark_back(){
    //cout<<"Visitor Mark backward instantiated"<<endl; 
  };

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

  void define_type(stapl::typer &t) {
    t.local(pivot_id);
    t.local(pivot_val);
  }
};



template <class PGRAPH>
class pSCCgeneric: public BasePObject{
 public:
  
  vector< vector <vector<pair<int,int> > > > finalSCC;

  vector<map<int,set<VID> > > localSCC;
  vector<vector<VID> > _compSCC;//for every trimer we have a vector of vids 

  int maxsz;

  int myid;
  int pvf; 
  int elems;
  int recv;
  double tt;	

  pSCCgeneric(){
    this->register_this(this);
    myid = stapl::get_thread_id();
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
			      &pSCCgeneric<PGRAPH>::_collect,k,SCC[k][j]); 
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

  void _compactSCC(_StaplTriple<int,VID,VID> _arg){
    vector<pair<VID,int> >::iterator vit;
    map<int,set<VID> >::iterator mit;
    set<VID> temp;
    int i;
    //for every trimer
    mit = localSCC[_arg.first].find(_arg.second);
    if(mit == localSCC[_arg.first].end()){
      //new entry; just added
      temp.clear();
      temp.insert(_arg.third);
      localSCC[_arg.first][_arg.second] = temp;
    }
    else{
      mit->second.insert(_arg.third);
    }
  }

  int compactSCC(vector<trimer<PGRAPH>*>&  _trimers, vector<vector<pair<VID,int> > >& SCC){
   
   //collect information relevant to this processors
   vector<pair<VID,int> >::iterator vit;
   map<int,set<VID> >::iterator mit;
   map<int,set<VID> > dummy;
   set<VID> temp;

   int i;

   for(i=0;i<_trimers.size();i++){
     localSCC.push_back(dummy);
   }

   stapl::rmi_fence();

   for(i=0;i<_trimers.size();i++){
     //for every trimer
     for(vit = SCC[i].begin();vit != SCC[i].end();vit++){
       if(_trimers[i]->sccg.IsLocal(vit->second)){//if the scc leader is here
	 //add to the map in the entry corresponding to the scc leader 
	 //the vertex id 
	 mit = localSCC[i].find(vit->second);
	 if(mit == localSCC[i].end()){
	   //new entry; just added
	   temp.clear();
	   //temp.push_back(vit->first);
	   temp.insert(vit->first);
	   localSCC[i][vit->second] = temp;
	 }
	 else{
	   mit->second.insert(vit->first);
	 }
       }//if local
       else{//remote
	 Location _n=_trimers[i]->sccg.Lookup(vit->second);
	 stapl_assert(_n.ValidLocation(),"Invalid id for the second vertex while SETPredecessors");   
	 //trimer_id, SCC_id, vertex_id
	 _StaplTriple<int,VID,VID> arg(i,vit->second, vit->first);
	 stapl::async_rmi(_n.locpid(),this->getHandle(),&pSCCgeneric<PGRAPH>::_compactSCC,arg);
       }//else remote
     }//for every vertex in pSCC
   }//for every trimer

   stapl::rmi_fence();

   //print the compact SCC 
   //vector<map<int,vector<VID> > > localSCC;
   /*
   for(i=0;i<localSCC.size();i++){
     for(mit=localSCC[i].begin();mit != localSCC[i].end();mit++){
       for(set<VID>::iterator it2 = mit->second.begin();it2 != mit->second.end(); it2++){
	 cout<<"SCC id no:"<<mit->first<<" "<<*it2<<endl;
       } 
     }
   }
   */

   _compSCC.resize(localSCC.size());
   for(i=0;i<localSCC.size();i++){
     for(mit=localSCC[i].begin();mit != localSCC[i].end();mit++){
       if(mit->second.size() > 1){
	 for(set<VID>::iterator it2 = mit->second.begin();it2 != mit->second.end(); it2++){
	   _compSCC[i].push_back(*it2);
	 }//for
       }//if size
     }//for mit
   }
   stapl::rmi_fence();
   return OK;
  }//end method

  void _compactSCCLeader(_StaplTriple<int,VID,VID> _arg){
    vector<pair<VID,int> >::iterator vit;
    map<int,set<VID> >::iterator mit;
    set<VID> temp;
    int i;
    //for every trimer
    mit = localSCC[_arg.first].find(_arg.second);
    if(mit == localSCC[_arg.first].end()){
      //new entry; just added
      temp.clear();
      temp.insert(_arg.third);
      localSCC[_arg.first][_arg.second] = temp;
    }
    else{
      mit->second.insert(_arg.third);
    }
  }

  int compactSCCLeader(int _trs, vector<vector<pair<VID,int> > >& SCC){
   
   //collect information relevant to this processors
   vector<pair<VID,int> >::iterator vit;
   map<int,set<VID> >::iterator mit;
   map<int,set<VID> > dummy;
   set<VID> temp;
   int i;

   for(i=0;i<_trs;i++){
     localSCC.push_back(dummy);
   }

   stapl::rmi_fence();

   for(i=0;i < _trs;i++){
     //for every trimer
     for(vit = SCC[i].begin();vit != SCC[i].end();vit++){
       if(stapl::get_thread_id() == 0){
	 //add to the map in the entry corresponding to the scc leader 
	 //the vertex id 
	 mit = localSCC[i].find(vit->second);
	 if(mit == localSCC[i].end()){
	   //new entry; just added
	   temp.clear();
	   temp.insert(vit->first);
	   localSCC[i][vit->second] = temp;
	 }
	 else{
	   mit->second.insert(vit->first);
	 }
       }//if local
       else{//remote
	 //trimer_id, SCC_id, vertex_id
	 _StaplTriple<int,VID,VID> arg(i,vit->second, vit->first);
	 stapl::async_rmi(0,this->getHandle(),&pSCCgeneric<PGRAPH>::_compactSCCLeader,arg);
       }//else remote
     }//for every vertex in pSCC
   }//for every trimer

   stapl::rmi_fence();

   //print the compact SCC 
   //vector<map<int,vector<VID> > > localSCC;

   for(i=0;i<localSCC.size();i++){
     for(mit=localSCC[i].begin();mit != localSCC[i].end();mit++){
       for(set<VID>::iterator it2 = mit->second.begin();it2 != mit->second.end(); it2++){
	 cout<<"SCC id no:"<<mit->first<<" "<<*it2<<endl;
       } 
     }
   }
  }

  void localCheckForward(vector<trimer<PGRAPH>* >& _trimers,
			 int trimer_id, VID start, set<VID>& oldSCC){
    typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    PGRAPH_SCC::VI vi;
    PGRAPH_SCC::EI ei;
    set<VID>::iterator si;

    if(!_trimers[trimer_id]->sccg.IsVertex(start,&vi)) {
      //cout<<"WARNING::pSSC::localCheckForward: vertex inexistent"<<endl;
      return ;
    }
    oldSCC.erase(start);
    if(oldSCC.size()==0) return;
    //here we have to consider successors
    for(ei = vi->edgelist.begin();ei != vi->edgelist.end(); ei++){
      si = oldSCC.find(ei->vertex2id);
      if(si == oldSCC.end()) continue;
      //here ei->vertex2id is in oldSCC
      localCheckForward(_trimers, trimer_id, ei->vertex2id, oldSCC);
      if(oldSCC.size() == 0) return;
    }
    
  }

  void localCheckBack(vector<trimer<PGRAPH>* >& _trimers, 
		      int trimer_id,VID start, set<VID>& oldSCC){
    typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    PGRAPH_SCC::VI vi;
    PGRAPH_SCC::EI ei;
    set<VID>::iterator si;

    if(!_trimers[trimer_id]->sccg.IsVertex(start,&vi)) {
      cout<<"WARNING::pSSC::localCheckForward: vertex inexistent"<<endl;
      return ;
    }
    oldSCC.erase(start);
    if(oldSCC.size()==0) return;
    //here we have to consider successors
    for(ei = vi->predecessors.begin();ei != vi->predecessors.end(); ei++){
      si = oldSCC.find(ei->vertex2id);
      if(si == oldSCC.end()) continue;
      //here ei->vertex2id is in oldSCC
      localCheckBack(_trimers, trimer_id, ei->vertex2id, oldSCC);
      if(oldSCC.size() == 0) return;
    }
  }

  int removeSCC(vector<trimer<PGRAPH>* >& _trimers,vector<vector<VID> >& reusedSCC){
    //the SCC are stored in localSCC container
    
    //for every trimer;
    // for every SCC check if the SCC is still valid for the new vector of trimers
    //   if valid remove it else go ahead wit the next one 

    //vector<map<int,vector<VID> > > localSCC;
    cout<<"REMOVE SCC"<<endl;

    int i;
    vector<pair<VID,int> >::iterator vit;
    map<int,set<VID> >::iterator mit;
    set<VID> _clone;
    bool flag=false;
    
    vector<VID> to_delete;


    reusedSCC.clear();


    for(i=0;i<localSCC.size();i++){//for every trimer
      to_delete.clear();
      reusedSCC.push_back(to_delete);
      for(mit=localSCC[i].begin();mit != localSCC[i].end();mit++){    
	//here mit->first is the SCC id
	//mit->second is the SET with vertices id
	//...
	_clone = mit->second;
	localCheckForward(_trimers, i, mit->first, _clone);
	if(_clone.size() != 0) continue;
	
	_clone = mit->second;
	localCheckBack(_trimers, i, mit->first, _clone);
	if(_clone.size() != 0) continue;

	//here we are if the SCC id was found to be valid in the new pGraph
	for(set<VID>::iterator it2 = mit->second.begin();it2 != mit->second.end(); it2++){
	  to_delete.push_back(*it2);
	}
	reusedSCC[i].push_back(mit->first);
      }//for every old SCC
      //delete from trimer all old SCC that are current

      //for(int j=0;j<to_delete.size();j++){
      //cout<<stapl::get_thread_id()<<"::OLDSCC delete "<<to_delete[j]<<endl;
      //}
      cout<<"REUSED:" << to_delete.size() << endl;
      _trimers[i]->sccg.pDeleteVertices(to_delete);

    }//for every trimer

    cout<<"reusedSCC size:"<<reusedSCC[0].size()<<endl;

    stapl::rmi_fence();
    cout<<"ADD CODE to add the valid SCC to the SCC vector passed to run as argument"<<endl;
    return OK;
  }//end remove SCC

  void _MAX(int* in, int* inout) { *inout = (*in>*inout)?*in:*inout; }
  void _MIN(int* in, int* inout) { *inout = (*in<*inout)?*in:*inout; }

  void trim_method(vector<trimer<PGRAPH>* >& _trimers){
    typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;

    int i,j;
    stapl::timer t;
    int ii;// to be erased

    t = stapl::start_timer();

    vector<VID> start_in;
    vector<VID> start_out; 

    _stapl_pscc_trim_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_trim_back<PGRAPH_SCC>  vis_back;
    
    for(i=0;i<_trimers.size();i++){

      _trimers[i]->sccg.GetZeroDependencies(start_in,start_out);
      //stapl::rmi_fence();

      //cout<<"[["<<start_in.size()<<","<<start_out.size()<<"]]"<<endl;
      //for(ii=0;ii<start_in.size();ii++) cout <<"--->"<< start_in[ii]<<endl;
      //for(ii=0;ii<start_out.size();ii++) cout <<"---<"<< start_out[ii]<<endl;

      for(j=0;j<start_in.size();j++){
	_trimers[i]->sccg.TOPO_traversal(start_in[j],vis_fwd,0,false);
      }
      for(j=0;j<start_out.size();j++){
	_trimers[i]->sccg.TOPO_traversal(start_out[j],vis_back,0,true);
      }
    }//for every trimer
    stapl::rmi_fence();
    
  }//end trim method



  int mark_method(vector<trimer<PGRAPH>* >& _trimers,bool reuse){
typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    PGRAPH_SCC::VI vi;
    int vid;
    int global;
    trimer<PGRAPH>* temp;

    int finished = 0;
    stapl::timer t;

    t = stapl::start_timer();

    _stapl_pscc_mark_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_mark_back<PGRAPH_SCC>  vis_back;

    for(int i=0;i<_trimers.size();i++){
      vid = -1;
      if(reuse && _compSCC.size() == _trimers.size() ){

	//cout<<"REUSE"<<endl;

	//here set the vid to the last element in _compSCC
	while(_compSCC[i].size() > 0){
	  vid = _compSCC[i].back();
	  _compSCC[i].pop_back();
	  //cout<<"checking::::"<<vid<<endl;

	  if(_trimers[i]->sccg.IsVertex(vid,&vi))
	    break;

	  vid = -1;
	}
      }
      if(vid == -1){
	_trimers[i]->FindPivots();
	//cout<<"marker's pivots size "<<_trimers[i]->pivots.size()<<endl;
	if(_trimers[i]->pivots.size() == 0){
	  for(vi = _trimers[i]->sccg.local_begin();vi!=_trimers[i]->sccg.local_end();vi++){
	    if(vi->data.in != 0 && vi->data.out != 0) break;
	  }
	  //if no vertices available skip;
	  if(vi == _trimers[i]->sccg.local_end()) {
	    //printf("End detected\n");
	    vid = -1;
	    finished++;
	  }
	  else vid = vi->vid;
	}
	else{
	  vid = _trimers[i]->pivots[_trimers[i]->pivots.size()/2];
	  //cout<<"VID:"<<vid<<endl;
	  if(!_trimers[i]->sccg.IsVertex(vid,&vi)){
	    printf("ERROR: class mark pivot inexistent\n");
	  }
	}    
      }  

      global = vid;
      if(global != -1){
	//printf("inside %d for vertex %d\n",stapl::get_thread_id(),global);
	pvf++;
	vis_fwd.set_pivot_info(myid,global);
	_trimers[i]->sccg.BFS_traversal_EQ(global,vis_fwd,myid,false);
	
	vis_back.set_pivot_info(myid,global);
	_trimers[i]->sccg.BFS_traversal_EQ(global,vis_back,myid,true);

	//printf("mark started %d for pivot %d [%d %d]\n",stapl::get_thread_id(),global,vi->data.in,vi->data.out);
      }    
    }//for every trimer/graph

    double t2 = stapl::stop_timer(t);    
    //printf("find pivots %f\n",t2);

    tt+=t2;

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::reduce_rmi<pSCCgeneric, int>(&finished, 
				 &global, 
				 getHandle(),
				 &pSCCgeneric::_MIN, 
				 true);
    //printf(" minimum %d\n",global); 
    return global==_trimers.size();    
  }

  int run(vector<trimer<PGRAPH>*>&  _trimers,vector<vector<pair<VID,int> > >& SCC,int aggf,bool reuse){
    typedef pGraph<PDG<taskSCC,dummyw>,PMG<taskSCC,dummyw>, PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
    typedef pBaseGraph<taskSCC,dummyw> PBASE_SCC;
    
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi;
    PGRAPH_SCC::VI vi2;

    taskSCC data1,data2;

    bool finished=false;
    int iteration = 0;
    
    double t0, t1, t2, t3,t4,t5,t6,treuse;
    
    double setupt, trt, mkt, sdelt, delt, tt4,tt5;

    setupt=trt=mkt=sdelt=delt=0.0;
    
    vector<VID> to_delete;
    vector<VID> del_edgef;
    vector<VID> del_edgeb;

    
    int i,j,k,pivt;
    int tr_id;
    stapl::timer t;
    int ttt;

    int trash;

    t = stapl::start_timer();

    SCC.clear();

    trim<PGRAPH,trimer<PGRAPH> > tr(_trimers);
    mark<PGRAPH,trimer<PGRAPH> > mk(_trimers);
    SCC.resize(_trimers.size());
    int myid = stapl::get_thread_id();

    //needed because the constructors has to finish

    stapl::set_aggregation(aggf);

    stapl::rmi_fence();
    t0 = stapl::stop_timer(t);

    treuse = 0;
    trash = 0;
    ////////////////////////////////////////////// code to remove old SCC
#ifdef OLDSCCREM
    t = stapl::start_timer();
    if(reuse == true){
      
      vector<vector<VID> > reusedSCC;
      
      //clear the vectores where the predecessors are stored
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.WarmCache();
	_trimers[i]->sccg.ClearPredecessors();
      }
      stapl::rmi_fence();
      
      //fill the predecessors list
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.pAsyncSetPredecessors();
      }
      stapl::rmi_fence();    
      //this function  will run once and unset the other reuse mechanism
      if(reuse == true) this->removeSCC(_trimers, reusedSCC);
      reuse=false;
      cout<<"REUSEDSCC size for trimer 0 "<<reusedSCC[0].size()<<endl;
      //here add code to add the reused scc to the SCC
      for(i=0;i<reusedSCC.size();i++){
	for(j=0;j < reusedSCC[i].size();j++){
	  //reusedSCC[i][j] is the SCC no
	  //vector<map<int,set<VID> > > localSCC;  
	  map<int, set<VID> >::iterator msit = localSCC[i].find(reusedSCC[i][j]);
	  if(msit == localSCC[i].end()) {
	    cout<<"WARNING::inside #ifdef OLDSCCREM invalid scc id"<<endl;
	    continue;
	  }
	  set<VID>::iterator sit = msit->second.begin();
	  while( sit != msit->second.end() ){
	    pair<VID,int> tempp(*sit, msit->first);
	    SCC[i].push_back(tempp);
	    sit++;
	  }
	}
      }
    }
    treuse = stapl::stop_timer(t);
#endif
    ////////////////////////////////////////////// end remove

    while(!finished){
      iteration++;

      finished = true;
      //printf("trim started --------------%d>\n",iteration);
  
      //stapl::set_aggregation(aggf);
      //_trimers[0]->DisplayGraph();
      int in_c=0,out_c=0;

      t = stapl::start_timer();
      //clear the vectores where the predecessors are stored
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.WarmCache();
	_trimers[i]->sccg.ClearPredecessors();
      }
      stapl::rmi_fence();
      t1 = stapl::stop_timer(t);

      t = stapl::start_timer();
      //fill the predecessors list
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.pAsyncSetPredecessors();
      }
      stapl::rmi_fence();
      t2 = stapl::stop_timer(t);


      t = stapl::start_timer();
      //for every trimer set the vectors storing the dependencies;
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.SetDependencies(0);	
      }
      stapl::rmi_fence();
      t3 = stapl::stop_timer(t);

      t = stapl::start_timer();
      //fill up the information for in and out dependencies
      for(i=0;i<_trimers.size();i++){
	for(vi2 = _trimers[i]->sccg.local_begin();vi2!=_trimers[i]->sccg.local_end();vi2++){
	  vi2->data.in = vi2->predecessors.size();
	  vi2->data.out = vi2->edgelist.size();
	  vi2->data.reset_fb();
	  in_c++;
	}
      }
      stapl::rmi_fence();
      t4 = stapl::stop_timer(t);
      
      //printf("ITERATION %d# [%d] t1=%f t2=%f t3=%f t4=%f\n",iteration,in_c,t1,t2,t3,t4);

      //                       HERE trimer is called
      t = stapl::start_timer();
      //tr.run();
      trim_method(_trimers);
      t2 = stapl::stop_timer(t);    


#ifdef REMBEFOREMARK
      ttt =0;
      t = stapl::start_timer();
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete.clear();	
	for(vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();vi++){
	  //if dead vertex add to to_delete
	  if(vi->data.in == 0 || vi->data.out==0){
	    to_delete.push_back(vi->vid);
	    trash++;
	    ttt++;
	  }
	}

	_trimers[tr_id]->sccg.pDeleteVertices(to_delete);

      }//for every trimer

      stapl::rmi_fence();

      //rescan the graphs associated with trim/mark
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.UpdateMap();
      }

      //clear the vectores where the predecessors are stored
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.WarmCache();
	_trimers[i]->sccg.ClearPredecessors();
      }
      stapl::rmi_fence();

      //fill the predecessors list
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.pAsyncSetPredecessors();
      }
      stapl::rmi_fence();

      //for every trimer set the vectors storing the dependencies;
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.SetDependencies(0);	
      }
      stapl::rmi_fence();
      treuse += stapl::stop_timer(t);
      //cout<<"Iteration:delete"<<iteration<<":"<< ttt <<endl;
#endif

      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.ResetTraversals();
      }
      stapl::rmi_fence();

      //                       HERE marker is called
      t = stapl::start_timer();
      //finished = mk.run();
      finished = mark_method(_trimers,reuse);
      t3 = stapl::stop_timer(t);    

      //finished = true;
      //stapl::set_aggregation(1);
      
      stapl::rmi_fence();
      //printf("take out\n");
      //here I have to take out from the graph the dead vertices and the SCC

      t = stapl::start_timer();

      tt4=tt5=0;

      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete.clear();	
	for(vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();vi++){
	  //if dead vertex add to to_delete
	  if(vi->data.in == 0 || vi->data.out==0){
	    to_delete.push_back(vi->vid);
	    continue;
	  }
	  //the node is alive;
	  //check if it part of the SCC
	  //the second condition in the next if is to take out 
	  //the Rem nodes;
	  for(pivt=0;pivt<vi->data.forward.size();pivt++){
	    //cout<<vi->vid<<"["<<vi->data.forward[pivt]<<":"<<vi->data.back[pivt]<<"]"<<endl;
	    if((vi->data.forward[pivt] == vi->data.back[pivt]) 
	       && (vi->data.back[pivt] != -1)){
	      SCC[tr_id].push_back(pair<VID,int>(vi->vid,vi->data.forward[pivt]));
	      to_delete.push_back(vi->vid);
	      // cout<<"SCC considered:"<<vi->vid<<":"<<vi->data.forward[pivt]<<":"<<pivt<<":"<<vi->data.forward[3]<<vi->data.back[3]<<endl;
	      break;
	    }
	  }//for every choosen pivot

	 
	  //!!!! STRANGE #ifdef STAPL_PSSCG_REM

	  //after we delete the vertices that are dead or in scc
	  //we have to remove edges between rem and forw and trim

	  //cout<<"REMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"<<endl;
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
#ifdef STAPL_PSSCG_REMOTE
	      if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
		  continue;
	      }
#endif
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
#ifdef STAPL_PSSCG_REMOTE
	      if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
		  continue;
	      }
#endif 
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
	      vi->DeleteXEdges(del_edgef[j],1);
	    }
	  }
	  //printf("[%d] Delete %d vertices\n",iteration,to_delete.size());

	  //!!!! STRANGE #endif
	}//for every vertex in the current trimer

	t4 = stapl::stop_timer(t);

    
	t = stapl::start_timer();
	//here we have to remove edges between pred and scc
	//and edges between succ and scc (both directions)
     	//printf("[%d] Delete %d vertices\n",iteration,to_delete.size());

	//printf("delete vertices NOT efficient\n");
	//here I need to implement a faster version for pDelete
	//stapl::rmi_fence();

	_trimers[tr_id]->sccg.pDeleteVertices(to_delete);

	//stapl::rmi_fence();

	//stapl::set_aggregation(1);

	t5 = stapl::stop_timer(t);    	
	tt4+=t4;
	tt5+=t5;
      }//for every trimer/graph

      stapl::rmi_fence();


      t = stapl::start_timer();
      //rescan the graphs associated with trim/mark
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.UpdateMap();
      }
      t6 = stapl::stop_timer(t);
      
      setupt +=t1;
      trt +=t2;
      mkt+=t3;
      sdelt+=tt4;
      delt+=tt5;
      //if(stapl::get_thread_id()==0) 
      //printf("[%d]Elapsed [iteration %d] setup=%f trim=%f mark=%f search_del=%f delete=%f\n",
      //       stapl::get_thread_id(),iteration,t1,t2,t3,tt4,tt5);

    }//while !finished
    stapl::rmi_fence();
    //printf("pivots for %d -> %d [%d] [%d][sent %d recv %d] [mktt %f]\n",
    //	   stapl::get_thread_id(),pvf,tr.f,tr.b,elems,recv,tt);
    //if(stapl::get_thread_id()==0) 
    printf("FINAL  Elapsed [iteration %d] setup=%f trim=%f mark=%f search_del=%f delete=%f constructor=%f [time preprocess %f][trim deleted %d]\n",
	     iteration,setupt,trt,mkt,sdelt,delt,t0,treuse,trash);
    return OK;    
  }


};


}//end namespace stapl
#endif
