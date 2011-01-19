#ifndef PSCCEXECUTOR
#define PSCCEXECUTOR

#include "../../common/Defines.h"

#include "pSCCutils.h"
#include "pGraphTraversal.h"

#include <assert.h>

//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////

namespace stapl{

//#define OLDSCCREM
//#define REMBEFOREMARK
//#define STAPL_PSSCG_REMOTE
//#define _LCC

//////////////////////////////////////////////////////////////
//                        MARK visitors
//////////////////////////////////////////////////////////////
template <class PGRAPH>
class _stapl_pscc_mark_forward_executor: public visitor_base<PGRAPH>{
  int pivot_id;
  PGRAPH* pg;
  public:  
    _stapl_pscc_mark_forward_executor(){
      pg = NULL;
    }

  _stapl_pscc_mark_forward_executor(PGRAPH* _pg,int _id){
    //cout<<"Visitor Mark instantiated"<<endl; 
    pg       = _pg;
    pivot_id = _id;
  }

  void set_pivot_info(int _id){
    pivot_id = _id;
  }

  inline int vertex (typename PGRAPH::VI vi) {
    //for every vertex I'll try to decrement the out variable ;

    if((vi->data.in==0)||(vi->data.out==0)) return -1;
    //vi->data.forward[pivot_id] = pivot_val;

    //here I have to color all successors with the pivot_val I have
    for(typename PGRAPH::EI ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
      int pivval = vi->data.GetPivotForward(pivot_id); 
      if(pivval != -1){
	
	//if(vi->vid == 4816||vi->vid == 4838||ei->vertex2id == 4354)
	//cout<<"-22->[[from "<<vi->vid<<"to"<<ei->vertex2id<<" pivd:"<<pivot_id<<" pival:"<<pivval<<"]"<<endl;
	//cout<<"set vertex from "<<vi->vid<<" to "<<ei->vertex2id<<endl;
	pg->pSetVertexField(ei->vertex2id,
			    &taskSCC::SetPivotForward,
			    pivot_id,pivval);
      }
    }
    return OK;
  }

  void define_type(stapl::typer &t) {
    cout<<"!!! Define_type for visitor called"<<endl;
    t.local(pivot_id);
  }

};


template <class PGRAPH>
class _stapl_pscc_mark_back_executor: public visitor_base<PGRAPH>{
  int pivot_id;
  PGRAPH* pg;

  public:  
  _stapl_pscc_mark_back_executor(PGRAPH* _pg,int _id){
    //cout<<"Visitor Mark backward instantiated"<<endl; 
    pg       = _pg;
    pivot_id = _id;
  }

  _stapl_pscc_mark_back_executor(){
    pg       = NULL;
  }

  void set_pivot_info(int _id){
    pivot_id  = _id;
  }

  inline int vertex (typename PGRAPH::VI vi) {
    //for every vertex I'll try to decrement the out variable ;

    
    if((vi->data.in==0)||(vi->data.out==0)) return -1;

    //if(vi->vid == 4002 || vi->vid == 3980)
    //cout<<"back problem?"<<vi->vid<<"piv id"<<pivot_id<<"piv val"<<vi->data.GetPivotBack(pivot_id)<<endl;

    //if(pivot_id == 0)
      //cout<<"backward going trough "<<vi->vid<<"for pivot"<<vi->data.GetPivotBack(pivot_id)<<endl; 

    //vi->data.back[pivot_id] = pivot_val;
    for(typename PGRAPH::EI ei = vi->predecessors.begin();ei != vi->predecessors.end();ei++){

      int pivval = vi->data.GetPivotBack(pivot_id); 
      //if(vi->vid == 3980||vi->vid ==4464||vi->vid==4001)
      //cout<<"back problem?"<<vi->vid<<"to"<<ei->vertex2id<<":"<<pivval<<"pivid"<<pivot_id<<endl;

      if(pivval != -1){

	//if(ei->vertex2id == 4751)
	//cout<<"[from "<<vi->vid<<"to"<<ei->vertex2id<<" pivd:"<<pivot_id<<" pival:"<<pivval<<"]"<<endl;

	pg->pSetVertexField(ei->vertex2id,
			    &taskSCC::SetPivotBack,
			    pivot_id,pivval);
      }
    }
    vi->data.mark = 0;
    return OK;
  }

  void define_type(stapl::typer &t) {
    cout<<"!!! Define_type for visitor called"<<endl;
    t.local(pivot_id);
  }
};

/////////////////////////////////////////////////////////////////////////
  //                            pSCC class<core algorithm>
/////////////////////////////////////////////////////////////////////////

template <class PGRAPH>
class pSCCExecutor: public BasePObject{
  typedef pGraph<PDG<taskSCC,dummyw>,
    PMG<taskSCC,dummyw>, 
    PWG<taskSCC,dummyw>, taskSCC,dummyw> PGRAPH_SCC;
  typedef stapl::linear_boundary<typename PGRAPH_SCC::VI> linear_boundary;

  typedef stapl::pRange<linear_boundary,PGRAPH_SCC,TOPODDG<PGRAPH_SCC> > pGraphTOPOpRange;
  typedef stapl::pRange<linear_boundary,PGRAPH_SCC,BFSDDG<PGRAPH_SCC> > pGraphBFSpRange;
  typedef stapl::pRange<linear_boundary,PGRAPH_SCC,BFSDDGReverse<PGRAPH_SCC> > pGraphBFSReversepRange;

  typedef stapl::defaultScheduler scheduler;
  
  vector< vector <vector<pair<int,int> > > > finalSCC;
  vector<map<int,set<VID> > >                localSCC;
  vector<vector<VID> >                       _compSCC;
    //for every trimer we have a vector of vids 

  int maxsz;

  int myid;
  int NP;
  int pvf; 
  int elems;
  int recv;
  double tt;	


  //switches to employ different optimizations
  bool OLDSCCREM;
  bool LCCMULTI;
  bool REMOVEBEFOREMARK;
  //bool STAPL_PSSCG_REMOTE;


 public:
  pSCCExecutor(bool sccreuse=false, bool localcc=true, bool rbm=false){
    OLDSCCREM = sccreuse;
    LCCMULTI  = localcc;
    REMOVEBEFOREMARK = rbm;
    this->register_this(this);
    myid = stapl::get_thread_id();
    NP = stapl::get_num_threads();
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
			      &pSCCExecutor<PGRAPH>::_collect,k,SCC[k][j]); 
	    //pssc.collect(scc[k][j]);
	  }
	  //printf("\n");
	}
      }
      stapl::rmi_fence();
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
     for(vit = SCC[i].begin();vit != SCC[i].end();++vit){
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
	 stapl::async_rmi(_n.locpid(),this->getHandle(),&pSCCExecutor<PGRAPH>::_compactSCC,arg);
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
     for(vit = SCC[i].begin();vit != SCC[i].end();++vit){
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
	 stapl::async_rmi(0,this->getHandle(),&pSCCExecutor<PGRAPH>::_compactSCCLeader,arg);
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
    map<int,set<VID> >::iterator mit;
    set<VID> _clone;
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

    PGRAPH_SCC::VI vi;
    int i;

    stapl::timer t; 
    t = stapl::start_timer();

    vector<VID> start_in;
    vector<VID> start_out; 

    _stapl_pscc_trim_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_trim_back<PGRAPH_SCC>  vis_back;
    vector<_stapl_pscc_trim_forward<PGRAPH_SCC> > vvisfwd;

    //                       Create pRanges
    
    vector<pGraphTOPOpRange> vpr(_trimers.size());
    stapl::rmi_fence();
    
    for(i=0;i < _trimers.size();i++){
      TOPODDG<PGRAPH_SCC>* ddg = new TOPODDG<PGRAPH_SCC>(_trimers[i]->sccg);
      stapl::rmi_fence();
      
      pGraphTOPOpRange pgpr(&_trimers[i]->sccg);
      stapl::rmi_fence();
      
      pgpr.set_ddg(ddg);
      
      for(vi = _trimers[i]->sccg.local_begin();vi != _trimers[i]->sccg.local_end(); vi++){
	pGraphTOPOpRange* ppr = new pGraphTOPOpRange(vi->vid, 
						     myid, 
						     &_trimers[i]->sccg,
						     linear_boundary(vi,vi+1),
						     10000,
						     &pgpr);
	pgpr.add_subrange(*ppr);
      }
      vpr[i] = pgpr;
    }

    for(i=0;i<_trimers.size();i++){
      vvisfwd.push_back(vis_fwd);      
    }//for every trimer
    
    p_TOPO_pRange<PGRAPH_SCC, 
                  pGraphTOPOpRange, 
                  _stapl_pscc_trim_forward<PGRAPH_SCC> > (vpr , vvisfwd);

    stapl::rmi_fence();
    stapl::stop_timer(t);
    
  }//end trim method

  int mark_method_traversal(vector<trimer<PGRAPH>* >& _trimers,bool reuse){

    PGRAPH_SCC::VI vi;
    int vid;
    int j,global;
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
	_trimers[i]->FindMultiPivots();
      }  

      if(_trimers[i]->pivots.size() == 0){
	finished++;
      }
      else{
	//printf("inside %d for vertex %d\n",stapl::get_thread_id(),global);
	for(j=0;j<_trimers[i]->pivots.size();j++){
	  cout<<"start on pos"<<myid<<" pivot "<<_trimers[i]->pivots[j]<<endl;
	  pvf++;
	  vis_fwd.set_pivot_info(myid,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j],vis_fwd,myid,false);
	  
	  vis_back.set_pivot_info(myid,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j], vis_back, myid, true);
	}
	//printf("mark started %d for pivot %d [%d %d]\n",stapl::get_thread_id(),global,vi->data.in,vi->data.out);
      }    
    }//for every trimer/graph

    double t2 = stapl::stop_timer(t);    
    //printf("find pivots %f\n",t2);

    tt+=t2;

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::reduce_rmi<pSCCExecutor, int>(&finished, 
				 &global, 
				 getHandle(),
				 &pSCCExecutor::_MIN, 
				 true);
    //printf(" minimum %d\n",global); 
    return global==_trimers.size();    
  }//                         END MARK TRAVERSAL


  int mark_method(vector<trimer<PGRAPH>* >& _trimers, bool reuse){

    PGRAPH_SCC::VI vi;
    int vid, i,j, global;
    int finished = 0;
    stapl::timer t;

    t = stapl::start_timer();

    for(i=0;i<_trimers.size();i++){
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
	_trimers[i]->FindMultiPivots();
      }
      if(_trimers[i]->pivots.size() == 0){
	finished++;
      }
    }//for every trimer/graph

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::reduce_rmi<pSCCExecutor, int>(&finished, 
				 &global, 
				 getHandle(),
				 &pSCCExecutor::_MIN, 
				 true);

    //if there is work left
    if(global != _trimers.size()){//mark   
      //                               Create pRanges

      //cout<<"                inside if()"<<endl;

      vector<pGraphBFSpRange> vpr(NP * _trimers.size());
      vector<pGraphBFSReversepRange> vprrev(NP * _trimers.size());

      vector<_stapl_pscc_mark_forward_executor<PGRAPH_SCC> > vvisfwd(NP * _trimers.size());
      vector<_stapl_pscc_mark_back_executor<PGRAPH_SCC> > vvisback(NP * _trimers.size());

      stapl::rmi_fence();

      int k=0;

      for(i=0;i < _trimers.size();i++){

	for(k=0;k<NP;k++){
	  //here I create a visitor corresponding to every pivot
	  _stapl_pscc_mark_forward_executor<PGRAPH_SCC>  vis_fwd(&(_trimers[i]->sccg),k);
	  _stapl_pscc_mark_back_executor<PGRAPH_SCC>     vis_back(&(_trimers[i]->sccg),k);
	  vvisfwd[i*NP+k]  = vis_fwd;
	  vvisback[i*NP+k] = vis_back;
	}

	BFSDDG<PGRAPH_SCC>* ddg = new BFSDDG<PGRAPH_SCC>(_trimers[i]->sccg, _trimers[i]->pivots);
	BFSDDGReverse<PGRAPH_SCC>* ddgrev = new BFSDDGReverse<PGRAPH_SCC>(_trimers[i]->sccg, _trimers[i]->pivots);
	stapl::rmi_fence();
	
	pGraphBFSpRange pgpr(&_trimers[i]->sccg);
	pGraphBFSReversepRange pgprrev(&_trimers[i]->sccg);
	stapl::rmi_fence();
	
	pgpr.set_ddg(ddg);
	pgprrev.set_ddg(ddgrev);
	//                        !!!!!!!!!
	// In the next for loop I can skip the vertices that are marked dead 
	//                        !!!!!!!!!

	for(vi = _trimers[i]->sccg.local_begin();vi != _trimers[i]->sccg.local_end(); vi++){
	  pGraphBFSpRange* ppr = new pGraphBFSpRange(vi->vid, 
					       myid, 
					       &_trimers[i]->sccg,
					       linear_boundary(vi,vi+1),
					       10000,
					       &pgpr);
	  pGraphBFSReversepRange* pprrev = new pGraphBFSReversepRange(vi->vid, 
					       myid, 
					       &_trimers[i]->sccg,
					       linear_boundary(vi,vi+1),
					       10000,
					       &pgprrev);
	  pgpr.add_subrange(*ppr);
	  pgprrev.add_subrange(*pprrev);
	}
	for(k=0;k<NP;k++){
	  vpr[i*NP+k]    = pgpr;	
	  vprrev[i*NP+k] = pgprrev;
	  if(k != myid){
	    vpr[i+k].get_ddg().clearStart();
	    vprrev[i+k].get_ddg().clearStart();
	  }
	}
      }
      //the pranges are created now call pBFS


      //first go forward
      for(i=0;i<_trimers.size();i++){
	for(j=0;j<_trimers[i]->pivots.size();j++){
	  cout<<"trimer "<<_trimers[i]->pivots[j]<<endl;
	  _trimers[i]->sccg.pSetVertexField(_trimers[i]->pivots[j],
					    &taskSCC::SetPivotForward,
					    myid,_trimers[i]->pivots[j]);
	}
      }

      stapl::rmi_fence();

      p_BFS_pRange_EQ<PGRAPH_SCC, 
	pGraphBFSpRange, 
	_stapl_pscc_mark_forward_executor<PGRAPH_SCC> > (vpr , vvisfwd);      

      //if(myid == 0) cout<<"START REVERSE"<<endl;

      //second go backward
      for(i=0;i<_trimers.size();i++){
	for(j=0;j<_trimers[i]->pivots.size();j++){

	  //cout<<"start on pos"<<myid<<" pivot "<<_trimers[i]->pivots[j]<<endl;

	  _trimers[i]->sccg.pSetVertexField(_trimers[i]->pivots[j],
					    &taskSCC::SetPivotBack,
					    myid,_trimers[i]->pivots[j]);
	}
      }

      stapl::rmi_fence();

      p_BFSReverse_pRange_EQ<PGRAPH_SCC, 
	pGraphBFSReversepRange, 
	_stapl_pscc_mark_back_executor<PGRAPH_SCC> > (vprrev , vvisback);
      stapl::rmi_fence();
    }

    double t2 = stapl::stop_timer(t);    
    tt+=t2;
    
    return global == _trimers.size();    
  }//                         END MARK NEW


  template <class TRIMGRAPH>
  int dfs_pred(TRIMGRAPH& _g,vector<VID>& _scc,typename TRIMGRAPH::VI _start){

    typename TRIMGRAPH::VI vi;
    typename TRIMGRAPH::EI ei;
    //color vertices with white since they are GRAY already !!!!
    _start->data.mark = WHITE;
    _scc.push_back(_start->vid);
    
    for(ei = _start->predecessors.begin();ei !=  _start->predecessors.end();ei++){
      if(!_g.IsVertex(ei->vertex2id,&vi)){
	//cout<<"Warning:: inside pSCC::dfs_pred method"<<endl;
	continue;
      }
      
      if((vi->data.in==0)||(vi->data.out==0)) continue;

      if(vi->data.mark != WHITE){
	dfs_pred(_g,_scc,vi);
      }
    }
    return OK;
  }

  template <class TRIMGRAPH>
  int dfs_succ(TRIMGRAPH& _g, typename TRIMGRAPH::VI _start, vector<typename TRIMGRAPH::VI>& ft){

    typename TRIMGRAPH::VI vi;
    typename TRIMGRAPH::EI ei;
    //color vertices with white since they are GRAY already !!!!
    _start->data.mark = GRAY;
    
    
    for(ei = _start->edgelist.begin();ei !=  _start->edgelist.end();ei++){
      if(!_g.IsVertex(ei->vertex2id,&vi)){
	//cout<<"Warning:: inside pSCC::dfs_succ method"<<endl;
	continue;
      }

      if((vi->data.in==0)||(vi->data.out==0)) continue;

      if(vi->data.mark == WHITE){
	dfs_succ(_g,vi,ft);
      }
    }

    ft.push_back(_start);

    return OK;
  }

  template<class TRIMGRAPH>
  int sequentialSCC(vector<trimer<TRIMGRAPH>*>&  _trimers,vector<set<int> >& nonlocalCC,vector<vector<pair<VID,int> > >& SCC){
    
    PGRAPH_SCC::VI vi;
    int i, tr_id;
    set<int>::iterator sit;
    vector<PGRAPH_SCC::VI> ft;//finish times
    int tmp;
    tmp = 0;
    vector<VID> _scc;

    //vector<vector<VID> > to_delete(_trimers.size());


    // mark variable inside vertex datastructure has to be set on 0 before calling this
    //method;
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      ft.clear();
      _scc.clear();

      //_filterVisitorSCC<PGRAPH_SCC, vector<PGRAPH_SCC::VI> > _vis(&ft,scc_no);

      //cout<<"ft before "<<ft.size()<<endl;

      for(vi = _trimers[tr_id]->sccg.local_begin();
	  vi != _trimers[tr_id]->sccg.local_end();vi++){

	if((vi->data.in==0)||(vi->data.out==0)) continue;

	if(vi->data.mark != WHITE) continue;

	sit = nonlocalCC[tr_id].find(vi->data.ccno);
	if(sit != nonlocalCC[tr_id].end()){
	  //the vertex is part of a cc with remote edges
	  vi->data.mark = GRAY;
	  continue;
	}
	//here the cc is local so we can start the sequential scc
	dfs_succ(_trimers[tr_id]->sccg,vi,ft);
	//cout<<"sz"<<ft.size()<<endl;
      }
      //for all vertices in ft call dfs_pred
      
      //to_delete[tr_id].resize(ft.size());

      for(i=ft.size()-1;i>=0;i--){
	//to_delete[tr_id][i] = ft[i]->vid;
	
	if(ft[i]->data.mark != WHITE){
	  _scc.clear();
	  dfs_pred(_trimers[tr_id]->sccg,_scc,ft[i]);
	  //cout<<"SCC size"<<_scc[scc_no].size()<<endl;
	  if(_scc.size() > 1){
	    for(int j=0;j<_scc.size();j++){
	      pair<VID,int> tempp(_scc[j], _scc[0]);
	      SCC[tr_id].push_back(tempp);
	    }
	  } 
	}
	ft[i]->data.in = 0;
      }

      
      tmp += ft.size();

    }//for every trimer
    
    if(stapl::get_thread_id() == 0) cout<<"ELIMINATED:"<<tmp<<endl;
    
    /*
    stapl::rmi_fence();
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      _trimers[tr_id]->sccg.pDeleteVertices(to_delete[tr_id]);
    }
    */
    return OK;
  }

  /**
   * the trim method will mark as dead some vertices; there is an
   * option to remove them before starting the mark method and the
   * next method perform this
   */
  int removeBeforeMark(vector<trimer<PGRAPH>*>& _trimers){
    vector<VID>  to_delete;
    int          i, tr_id;
    stapl::timer t;
    int          ttt=0;
    int          trash = 0;
    PGRAPH_SCC::VI vi;

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
    stapl::stop_timer(t);
    return OK;
 }


  int removeLocalSCC(vector<trimer<PGRAPH>*>& _trimers,vector<vector<pair<VID,int> > >& SCC){
    PGRAPH_SCC::VI vi,vi2;
    PGRAPH_SCC::EI ei;	
    int tr_id,tcnt;
    vector<set<int> > nonlocalCC;
    vector<set<int> > nonlocalpreCC;
    vector<set<int> > nonlocalsuccCC;

    nonlocalCC.resize(_trimers.size());
    nonlocalsuccCC.resize(_trimers.size());
    nonlocalpreCC.resize(_trimers.size());


    set<VID>::iterator __first1,__last1,__first2,__last2;
        
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      for(vi = _trimers[tr_id]->sccg.local_begin();
	  vi != _trimers[tr_id]->sccg.local_end();++vi){
	vi->data.mark = WHITE;
	for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	  if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
	    //there is a remote edge from current vertex;
	    //the CC associated with this vertex is not local;
	    nonlocalsuccCC[tr_id].insert(vi->data.ccno);
	      }
	}
	for(ei = vi->predecessors.begin();ei != vi->predecessors.end();ei++){
	  if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
	    //there is a remote edge from current vertex;
	    //the CC associated with this vertex is not local;
	    nonlocalpreCC[tr_id].insert(vi->data.ccno);
	  }
	}
      }
      //nonlocalCC[tr_id] = nonlocalpreCC[tr_id];
      
      
      __first1 = nonlocalsuccCC[tr_id].begin();
      __last1  = nonlocalsuccCC[tr_id].end();
      __first2 = nonlocalpreCC[tr_id].begin();
      __last2  = nonlocalpreCC[tr_id].end();
      while (__first1 != __last1 && __first2 != __last2)
	if (*__first1 < *__first2)
	  ++__first1;
	else if (*__first2 < *__first1)
	  ++__first2;
	else {
	  nonlocalCC[tr_id].insert(*__first1);
	  ++__first1;
	  ++__first2;
	}
      //set_intersection(nonlocalsuccCC[tr_id].begin(),nonlocalsuccCC[tr_id].end(),
      //	 nonlocalpreCC[tr_id].begin(), nonlocalpreCC[tr_id].end(),
      //	 nonlocalCC[tr_id].begin());
    }//for every trimer
    
    cout<<"NONLOCAL SIZE:"<<nonlocalCC[0].size()<<endl;

    // print info gathered
    set<int>::iterator sit;
    tcnt = 0;
    
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      for(vi = _trimers[tr_id]->sccg.local_begin();
	  vi != _trimers[tr_id]->sccg.local_end();++vi){
	sit = nonlocalCC[tr_id].find(vi->data.ccno);
	if(sit == nonlocalCC[tr_id].end()){
	  // cout<<"CCNO "<<vi->data.ccno<<"local"<<endl;
	  tcnt++;
	  
	}
      }
    }
    
    if (stapl::get_thread_id() == 0) 
      cout<<"local:"<<stapl::get_thread_id()<<"vertices:"<<tcnt<<endl;
    
    //here I have to call the sequentiall SCC
    if(tcnt > 0) sequentialSCC(_trimers,nonlocalCC,SCC);
    return OK;
  }

  int checkSCCFromPreviousStep(vector<trimer<PGRAPH>*>& _trimers, 
			       vector<vector<pair<VID,int> > >& SCC, bool reuse){
    int i,j;
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
    return OK;
  }


  int initTraversalInfo(vector<trimer<PGRAPH>*>&  _trimers){
    int i;

    PGRAPH_SCC::VI vi;
    
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
    //fill up the information for in and out dependencies
    for(i=0;i<_trimers.size();i++){
      for(vi = _trimers[i]->sccg.local_begin();vi!=_trimers[i]->sccg.local_end();vi++){
	vi->data.in = vi->predecessors.size();
	vi->data.out = vi->edgelist.size();
	vi->data.reset_fb();
      }
    }
    return OK;
  }

  //////////////////////////////////////////////////////////////////////
  //                    The main method of pSCC class
  //////////////////////////////////////////////////////////////////////

  int run(vector<trimer<PGRAPH>*>&  _trimers,
	  vector<vector<pair<VID,int> > >& SCC,int aggf,bool reuse){

    typedef pBaseGraph<taskSCC,dummyw> PBASE_SCC;
    
    PGRAPH_SCC::EI ei;
    PGRAPH_SCC::VI vi;
    PGRAPH_SCC::VI vi2;

    taskSCC data1,data2;

    bool finished=false;
    int iteration = 0;
    
    //timers
    double t0, t1, t2, t3,t4,t5,t6,t7,t8,t9,treuse;    
    double setupt, trt, mkt, sdelt, delt, tt0,tt1,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9;
    tt0=tt1=tt2=tt3=tt4=tt5=tt6=tt7=tt8=tt9=0.0;
    setupt=trt=mkt=sdelt=delt=0.0;
    
    vector<VID> to_delete;
    vector<VID> del_edgef;
    vector<VID> neighbours;
    
    int i,j,k,pivt;
    int tcnt,globaltcnt;

    //to keep track of CC number
    int CCNO;
    map<int,int> pre_ccno;
    map<int,int> succ_ccno;
    map<int,int>::iterator mit;

    int tr_id;
    stapl::timer t;
    int ttt;

    int trash;

    t = stapl::start_timer();

    SCC.clear();
    SCC.resize(_trimers.size());
    int myid = stapl::get_thread_id();

    //needed because the constructors has to finish
    stapl::set_aggregation(aggf);

    stapl::rmi_fence();
    t0 = stapl::stop_timer(t);

    treuse = 0;
    trash = 0;

    
    ////////////////////////////////////////////////////////////
    //                      code to remove old SCC
    ////////////////////////////////////////////////////////////
    if(OLDSCCREM){
      t = stapl::start_timer();
      if(reuse == true){
	checkSCCFromPreviousStep(_trimers,SCC,reuse);
      }
      treuse = stapl::stop_timer(t);
    }

    CCNO = 0;
    while(!finished){
      iteration++;

      finished = true;
      if(myid == 0) cout<<"iteration --------------"<<iteration<<endl;

  
      //stapl::set_aggregation(aggf);
      //_trimers[0]->DisplayGraph();
      t = stapl::start_timer();
       initTraversalInfo(_trimers);
      stapl::rmi_fence();
      t4 = stapl::stop_timer(t);


      //                       HERE trimer is called
      t = stapl::start_timer();
      trim_method(_trimers);
      t5 = stapl::stop_timer(t);    

      //int tcn = 0;
      //for(tr_id=0;tr_id<_trimers.size();tr_id++){
      //for(vi = _trimers[tr_id]->sccg.local_begin();
      //    vi != _trimers[tr_id]->sccg.local_end();++vi){
      //  if(vi->data.in == 0 || vi->data.out==0){
      //    tcn++;
      //  }
      //}
      //}
      //cout<<"TRIM has marked as dead "<<tcn<<" vertices"<<endl;


      //                    local connected component check
      //here I have to add to nonlocalCC the CC that are not local;
      //it is easier to identify them

      t = stapl::start_timer();

      if(LCCMULTI)
	if(iteration > 1) {
	  removeLocalSCC(_trimers,SCC);
	}

      stapl::rmi_fence();
      t6 = stapl::stop_timer(t);    

      if (REMOVEBEFOREMARK)
	removeBeforeMark(_trimers);

      t = stapl::start_timer();
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.ResetTraversals();
      }
      stapl::rmi_fence();
      
      
      //_trimers[0]->sccg.pDisplayGraph();

      //                       HERE marker is called
      finished = mark_method(_trimers,reuse);
      t7 = stapl::stop_timer(t);      
      stapl::rmi_fence();

      
      //if(myid ==0)
      //for(vi = _trimers[0]->sccg.local_begin();
      //  vi != _trimers[0]->sccg.local_end();++vi){
      // if(vi->data.in == 0 || vi->data.out==0) continue;
      // cout<<"-->"<<vi->vid<<":>"<<vi->data.in<<" "<<vi->data.out<<"||"<<vi->data.forward[0]<<" "<<vi->data.back[0]<<"||"<<vi->data.forward[1]<<" "<<vi->data.back[1]<<endl;
      //}
      
      //////////////////////////////////////////////////////////////////////
      //printf("take out\n");
      //here I have to take out from the graph the dead vertices and the SCC
      //////////////////////////////////////////////////////////////////////

      t = stapl::start_timer();
      
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete.clear();	
	pre_ccno.clear();
	succ_ccno.clear();
	for(vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();++vi){
	  //if dead vertex add to to_delete
	  if(vi->data.in == 0 || vi->data.out==0){
	    to_delete.push_back(vi->vid);
	    continue;
	  }
	  //the node is alive;
	  //check if it is part of the SCC
	  //the second condition in the next if is to take out 
	  //the Rem nodes;
	  for(pivt=0;pivt<vi->data.forward.size();pivt++){
	    if((vi->data.forward[pivt] == vi->data.back[pivt]) 
	       && (vi->data.back[pivt] != -1)){
	      
	      SCC[tr_id].push_back(pair<VID,int>(vi->vid,vi->data.forward[pivt]));
	      to_delete.push_back(vi->vid);
	      break;
	    }
	  }//for every choosen pivot

	  if(LCCMULTI){
	    //                                  CCNO
	    
	    //here I update the ccno field for every vertex according with the
	    //marking done for the current pivot; the update is done using the local CCNO
	    //pre_ccno succ_ccno
	    if(vi->data.forward[myid] != -1 && vi->data.back[myid] != -1 && vi->data.forward[myid]!= vi->data.back[myid] ){
	      cout<<"ERROR"<<endl;
	    }
	    if(vi->data.forward[myid] != -1 && vi->data.back[myid] == -1){
	      mit = pre_ccno.find(vi->data.ccno);
	      if(mit == pre_ccno.end()){
		pre_ccno[vi->data.ccno] = CCNO + 1;
		vi->data.ccno = CCNO + 1;
		CCNO++;
	      }
	      else{
		//the cc is already created; just assign the ccno to this vertex
		vi->data.ccno = mit->second;
	      }
	    }
	    if(vi->data.forward[myid] == -1 && vi->data.back[myid] != -1){
	      mit = succ_ccno.find(vi->data.ccno);
	      if(mit == succ_ccno.end()){
		succ_ccno[vi->data.ccno] = CCNO + 1;
		vi->data.ccno = CCNO + 1;
		CCNO++;
	      }
	      else{
		//the cc is already created; just assign the ccno to this vertex
		vi->data.ccno = mit->second;
	      }
	    }
	  }//end if multicc
	  	  
	  //after we delete the vertices that are dead or in scc
	  //we have to remove edges between REM and FWD and BACK
	  
	  data1 = vi->data;
	  
	  //printf("posible rem candidate %d\n",vi->vid);
	  //vi belongs to remaining
	  del_edgef.clear();
	  neighbours.clear();
	  neighbours.resize(vi->edgelist.size());
	  j = 0;
	  for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){     
	    neighbours[j] = ei->vertex2id;
	    j++;
	  }
	  for(j=0;j<neighbours.size();j++){
#ifdef STAPL_PSSCG_REMOTE
	    if(!_trimers[tr_id]->sccg.IsVertex(neighbours[j],&vi2)){
	      continue;
	    }
#endif
	    data2 = _trimers[tr_id]->sccg.GetData(neighbours[j]);

	    if(data2.in == 0 || data2.out==0) {
	      continue;
	    }
	 
	    //for(k=0;k < data1.forward.size();k++){
	    //  cout<<"-|"<<vi->vid<<"["<<data1.forward[k]<<":"<<data1.back[k]<<"]"<<endl;
	    //  cout<<"-|"<<vi->vid<<"["<<data2.forward[k]<<":"<<data2.back[k]<<"]"<<endl;
	    //}

	    if(vi->vid == 3870 && neighbours[j] == 3408) {
	      //cout<<"Edge DELETED......."<<k<<endl;
	      for(int kk=0;kk < data1.forward.size();kk++){
		cout<<"-|"<<vi->vid<<"["<<data1.forward[kk]<<":"<<data1.back[kk]<<"]"<<endl;
		cout<<"-|"<<neighbours[j]<<"["<<data2.forward[kk]<<":"<<data2.back[kk]<<"]"<<endl;
	      }
	    }
	    for(k=0;k < data1.forward.size();k++){
	      //break between rem and (pred,succ)
	      if(data1.back[k] == -1 && data1.forward[k] == -1){
		if(data2.back[k] != -1 || data2.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		  if(vi->vid == 3870 && neighbours[j] == 3408) {
		    cout<<"Edge DELETED......."<<k<<endl;
		    for(int kk=0;kk < data1.forward.size();kk++){
		      cout<<"-|"<<vi->vid<<"["<<data1.forward[kk]<<":"<<data1.back[kk]<<"]"<<endl;
		      cout<<"-|"<<neighbours[j]<<"["<<data2.forward[kk]<<":"<<data2.back[kk]<<"]"<<endl;
		    }
		  }
		  //break;  //jump
		}      
	      }
	      if(data2.back[k] == -1 && data2.forward[k] == -1){
		if(data1.back[k] != -1 || data1.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge

		  if(vi->vid == 3870 && neighbours[j] == 3408) cout<<"Edge DELETED.......-2"<<endl;

		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	      //break between pred and succ
	      if(data1.back[k] == -1 && data1.forward[k] != -1){
		if(data2.back[k] != -1 && data2.forward[k] == -1){

		  if(vi->vid == 3870 && neighbours[j] == 3408) cout<<"Edge DELETED.......-3"<<endl;

		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	      //break between pred and succ
	      if(data1.back[k] != -1 && data1.forward[k] == -1){
		if(data2.back[k] == -1 && data2.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge

		  if(vi->vid == 3870 && neighbours[j] == 3402) cout<<"Edge DELETED.......-4"<<endl;

		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	    }//for k
	    //here
	  }//for every edge
	  //efficient way to delete edges
	  for(j=0;j<del_edgef.size();j++){
	    //if(vi->vid == 2563 && del_edgef[j] == 3025) cout<<"Edge DELETED......."<<endl; 
	    vi->DeleteXEdges(del_edgef[j],1);
	  }
	  //printf("[%d] Delete %d vertices\n",iteration,to_delete.size());

	  //!!!! STRANGE #endif
	}//for every vertex in the current trimer
	//here we have to remove edges between pred and scc
	//and edges between succ and scc (both directions)
     	//printf("[%d] Delete %d vertices\n",iteration,to_delete.size());

	//printf("delete vertices NOT efficient\n");
	//here I need to implement a faster version for pDelete

	//            ??????????????   MOVE delete out of the loop
	stapl::rmi_fence();

	_trimers[tr_id]->sccg.pDeleteVertices(to_delete);

	
	//stapl::rmi_fence();

      }//for every trimer/graph

      stapl::rmi_fence();

      t8 = stapl::stop_timer(t);


      t = stapl::start_timer();
      //rescan the graphs associated with trim/mark
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.UpdateMap();
      }
      t9 = stapl::stop_timer(t);
      
      if(LCCMULTI){
	for(tr_id=0;tr_id<_trimers.size();tr_id++){
	  for(vi = _trimers[tr_id]->sccg.local_begin();vi != _trimers[tr_id]->sccg.local_end();++vi){
	    for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	      if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
		continue;
	      }
	      //here vi2 contains the right stuff
	      if(vi->data.ccno != vi2->data.ccno){
		cout<<"ERROR setting ccno:iter="<<iteration<<":"<<vi->data.ccno<<":"<<vi2->data.ccno;
		cout<<"["<<vi->data.forward[myid]<<":"<<vi->data.back[myid]<<"] ["<<vi2->data.forward[myid]<<":"<<vi2->data.back[myid]<<"]";
		cout<<"[["<<vi->vid<<","<<vi2->vid<<"]]"<<endl;
	      }
	    }
	  }
	}
      }//if LCCMULTI
    
      tt0+=t0;//update ccno
      tt1+=t1;//clear pred
      tt2+=t2;//async set
      tt3+=t3;//graph set deps
      tt4+=t4;//user set dep
      tt5+=t5;//trim
      tt6+=t6;//sequentiall scc
      tt7+=t7;//reset and trim
      tt8+=t8;//remove scc
      tt9+=t9;//update map

      if(stapl::get_thread_id()==0)
	printf("FINAL  Elapsed [iteration %d]\n clear pred=%f \n async set=%f \n graph set dep=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset_trim  %f\n remove %f\n update %f\n update ccno %f\n",iteration,t1,t2,t3,t4,t5,t6,t7,t8,t9,t0);

      stapl::rmi_fence();
    }//while !finished
    stapl::rmi_fence();
    //printf("pivots for %d -> %d [%d] [%d][sent %d recv %d] [mktt %f]\n",
    //	   stapl::get_thread_id(),pvf,tr.f,tr.b,elems,recv,tt);
    //if(stapl::get_thread_id()==0) 
    
    printf("FINAL  Elapsed [iteration %d]\n clear pred=%f \n async set=%f \n graph set dep=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset_trim  %f\n remove %f\n update %f\n update ccno %f\n",iteration,tt1,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9,tt0);
     return OK;    
  }


};


}//end namespace stapl
#endif