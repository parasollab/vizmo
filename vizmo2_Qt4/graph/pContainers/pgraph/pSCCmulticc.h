#ifndef PSCCMULTI
#define PSCCMULTI

#include "../../common/Defines.h"
#include "runtime.h"
#include "pSCCutils.h"

#include <assert.h>
//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////



namespace stapl{


template <class PGRAPH>
class pSCCmulticc: public BasePObject{

  typedef pGraph<PDG<taskSCC,dummyw>,
    PMG<taskSCC,dummyw>, 
    PWG<taskSCC,dummyw>, 
    taskSCC,dummyw> PGRAPH_SCC;

  vector< vector <vector<pair<int,int> > > > finalSCC;
  vector<map<int,set<VID> > >                localSCC;
  vector<vector<VID> >                       _compSCC;
  //for every trimer we have a vector of vids 

  int maxsz;
  int myid;
  int pvf; 
  int elems;
  int recv;
  double tt;	

  //switches to employ different optimizations
  bool OLDSCCREM;
  bool LCCMULTI;
  bool REMOVEBEFOREMARK;
  //bool STAPL_PSSCG_REMOTE;

  //timers
  stapl::timer t;
  double t0, t1, t2, t3,t4,t5,t6,t7,t8,t9,treuse;
  double setupt, trt, mkt, sdelt, delt, tt0,tt1,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9;

 public:
  pSCCmulticc(bool sccreuse=false, bool localcc=true, bool rbm=false){
    OLDSCCREM = sccreuse;
    LCCMULTI  = localcc;
    REMOVEBEFOREMARK = rbm;
    this->register_this(this);
    myid = stapl::get_thread_id();
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
			    &pSCCmulticc<PGRAPH>::_collect,k,SCC[k][j]); 
	  //pssc.collect(scc[k][j]);
	}
	//printf("\n");
      }
    }
    stapl::rmi_fence();
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
    map<int,set<VID> >::iterator mit;
    set<VID> temp;
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
	 stapl::async_rmi(_n.locpid(),this->getHandle(),&pSCCmulticc<PGRAPH>::_compactSCC,arg);
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
	 stapl::async_rmi(0,this->getHandle(),&pSCCmulticc<PGRAPH>::_compactSCCLeader,arg);
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

    int i,j;
    stapl::timer t;

    t = stapl::start_timer();

    vector<VID> start_in;
    vector<VID> start_out; 

    _stapl_pscc_trim_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_trim_back<PGRAPH_SCC>  vis_back;
    
    for(i=0;i<_trimers.size();i++){

      _trimers[i]->sccg.GetZeroDependencies(start_in,start_out);
      //stapl::rmi_fence();

      for(j=0;j<start_in.size();j++){
	_trimers[i]->sccg.TOPO_traversal(start_in[j],vis_fwd,0,false);
      }
      for(j=0;j<start_out.size();j++){
	_trimers[i]->sccg.TOPO_traversal(start_out[j],vis_back,0,true);
      }
    }//for every trimer

    stapl::rmi_fence();
    stapl::stop_timer(t);    
  }//end trim method


  void trim_method_algo(vector<trimer<PGRAPH>* >& _trimers){

    int i;
    stapl::timer t;
   
    t = stapl::start_timer();

    vector<PGRAPH_SCC*> pgs(_trimers.size());
    vector<_stapl_pscc_trim_forward<PGRAPH_SCC>* >  vvis_fwd(_trimers.size());
    vector<_stapl_pscc_trim_back<PGRAPH_SCC>* > vvis_back(_trimers.size());

    for(i=0;i<_trimers.size();i++){
      pgs[i] = &_trimers[i]->sccg;
      vvis_fwd[i] = new _stapl_pscc_trim_forward<PGRAPH_SCC>();
      vvis_back[i] = new _stapl_pscc_trim_back<PGRAPH_SCC>();
    }

    pTOPOFwdBack<PGRAPH_SCC,
      _stapl_pscc_trim_forward<PGRAPH_SCC>,
      _stapl_pscc_trim_back<PGRAPH_SCC> >(pgs, vvis_fwd, vvis_back);

    stapl::rmi_fence();
    for(i=0;i<_trimers.size();i++){
     delete vvis_fwd[i];
     delete vvis_back[i];
    }
  }//end trim method


  int mark_method(vector<trimer<PGRAPH>* >& _trimers,bool reuse){

    PGRAPH_SCC::VI vi;
    int vid;
    int j,global;

    int finished = 0;
    stapl::timer t;

    t = stapl::start_timer();

    _stapl_pscc_mark_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_mark_back<PGRAPH_SCC>  vis_back;

    for(int i=0;i<_trimers.size();i++){
      //_trimers[i]->sccg.StartCounting();
      vid = -1;
      if(reuse && _compSCC.size() == _trimers.size() ){
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
	  pvf++;
	  vis_fwd.set_pivot_info(myid,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j],vis_fwd,myid,false);
	  
	  vis_back.set_pivot_info(myid,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j], vis_back, myid, true);
	}
      }    
    }//for every trimer/graph

    double t2 = stapl::stop_timer(t);    
    //printf("find pivots %f\n",t2);

    tt+=t2;

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::rmi_fence();
    stapl::reduce_rmi<pSCCmulticc, int>(&finished, 
				 &global, 
				 getHandle(),
				 &pSCCmulticc::_MIN, 
				 true);
    //printf(" minimum %d\n",global); 
    for(int i=0;i<_trimers.size();i++){
      _trimers[i]->sccg.StopCounting();
    }
    return global==_trimers.size();    
  }//                         END MARK


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
    int i, tr_id, tmp;
    set<int>::iterator sit;
    vector<PGRAPH_SCC::VI> ft;//finish times
    vector<VID> _scc;

    //int scc_no;
    //vector<vector<VID> > to_delete(_trimers.size());

    // mark variable inside vertex datastructure has to be set on 0 before calling this
    //method;
    tmp = 0;
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      ft.clear();
      _scc.clear();
      //scc_no = 0;

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
    
    //if (stapl::get_thread_id() == 0) 
    //cout<<"local:"<<stapl::get_thread_id()<<"vertices:"<<tcnt<<endl;
    
    //here I have to call the sequentiall SCC
    if(tcnt > 0) sequentialSCC(_trimers,nonlocalCC,SCC);
    return OK;
  }

  int checkSCCFromPreviousStep(vector<trimer<PGRAPH>*>& _trimers, 
			       vector<vector<pair<VID,int> > >& SCC,bool reuse){
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
    stapl::timer t_l;
    PGRAPH_SCC::VI vi;

    t_l = stapl::start_timer();
    for(i=0;i<_trimers.size();i++){
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
    t3 = stapl::stop_timer(t_l);   
    return OK;
  }

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
    tt0=tt1=tt2=tt3=tt4=tt5=tt6=tt7=tt8=tt9=0.0;
    setupt=trt=mkt=sdelt=delt=0.0;
    
    vector<vector<VID> > to_delete(_trimers.size());
    vector<VID> del_edgef;
    vector<VID> neighbours;
    
    int i,j,k,pivt;
    //to keep track of CC number
    int CCNO;
    hash_map<int,int> pre_ccno;
    hash_map<int,int> succ_ccno;
    hash_map<int,int>::iterator mit;

    int tr_id;
    
    
    t = stapl::start_timer();

    SCC.clear();
    SCC.resize(_trimers.size());
    int myid = stapl::get_thread_id();

    //needed because the constructors has to finish
    stapl::set_aggregation(aggf);
    stapl::rmi_fence();
    t0 = stapl::stop_timer(t);

    treuse = 0;

    //    Remove SCCs using informations from a previous run of the algorithm
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
      //_trimers[0]->DisplayGraph();


      t = stapl::start_timer();
       initTraversalInfo(_trimers);

       if(iteration == 1){
	 for(i=0;i<_trimers.size();i++){
	   _trimers[i]->sccg.WarmCache();
	 }
       }
      stapl::rmi_fence();
      t4 = stapl::stop_timer(t);
      
      //                       TRIM
      t = stapl::start_timer();
       trim_method(_trimers);
      t5 = stapl::stop_timer(t);    

      //                    Local Connected Component check
      //here I have to add to nonlocalCC the CC that are not local;
      //it is easier to identify them
      t = stapl::start_timer();
      if(LCCMULTI)
	if(iteration > 1) {
	  removeLocalSCC(_trimers,SCC);
	}
      stapl::rmi_fence();
      t6 = stapl::stop_timer(t);    

      if(REMOVEBEFOREMARK)
	removeBeforeMark(_trimers);

      //                       MARK
      t = stapl::start_timer();

      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.ResetTraversals();
      }
      stapl::rmi_fence();
      finished = mark_method(_trimers,reuse);
      stapl::rmi_fence();
      t7 = stapl::stop_timer(t);      

      //                       END MARK
      
      //      Remove dead vertices, the SCCs found and the edges between
      //      REM/SUCC/PRED
      t = stapl::start_timer();      
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete[tr_id].clear();	
	pre_ccno.clear();
	succ_ccno.clear();
	for(vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();++vi){
	  //if dead vertex add to to_delete
	  if(vi->data.in == 0 || vi->data.out==0){
	    to_delete[tr_id].push_back(vi->vid);
	    continue;
	  }
	  //the node is alive;
	  //check if it is part of the SCC
	  //the second condition in the next if is to take out 
	  //the Rem nodes;
	  for(pivt=0;pivt<vi->data.forward.size();pivt++){
	    //cout<<vi->vid<<"["<<vi->data.forward[pivt]<<":"<<vi->data.back[pivt]<<"]"<<endl;
	    if((vi->data.forward[pivt] == vi->data.back[pivt]) 
	       && (vi->data.back[pivt] != -1)){
	      SCC[tr_id].push_back(pair<VID,int>(vi->vid,vi->data.forward[pivt]));
	      to_delete[tr_id].push_back(vi->vid);
	      // cout<<"SCC considered:"<<vi->vid<<":"<<vi->data.forward[pivt]<<":"<<pivt<<":"<<vi->data.forward[3]<<vi->data.back[3]<<endl;
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
	  }//if local cc check
	
	  
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
	 
	    for(k=0;k < data1.forward.size();k++){
	      //break between rem and (pred,succ)
	      if(data1.back[k] == -1 && data1.forward[k] == -1){
		if(data2.back[k] != -1 || data2.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		}      
	      }
	      if(data2.back[k] == -1 && data2.forward[k] == -1){
		if(data1.back[k] != -1 || data1.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	      //break between pred and succ
	      if(data1.back[k] == -1 && data1.forward[k] != -1){
		if(data2.back[k] != -1 && data2.forward[k] == -1){
		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	      //break between pred and succ
	      if(data1.back[k] != -1 && data1.forward[k] == -1){
		if(data2.back[k] == -1 && data2.forward[k] != -1){
		  //the dest node is not in remaining; delete the edge
		  del_edgef.push_back(neighbours[j]);
		  //break;  //jump
		}      
	      }
	    }//for k
	    //here
	  }//for every edge
	  //efficient way to delete edges
	  for(j=0;j<del_edgef.size();j++){
	    vi->DeleteXEdges(del_edgef[j],1);
	  }
	  //printf("[%d] Delete %d vertices\n",iteration,to_delete.size());

	  //!!!! STRANGE #endif
	}//for every vertex in the current trimer
	//here we have to remove edges between pred and scc
	//and edges between succ and scc (both directions)
     	//printf("[%d] Delete %d vertices\n",iteration,to_delete.size());
	//            ??????????????   MOVE delete out of the loop
	
	//stapl::rmi_fence();

      }//for every trimer/graph

      stapl::rmi_fence();
      t8 = stapl::stop_timer(t);

      t = stapl::start_timer();
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.pDeleteVertices(to_delete[itr]);
      }
      stapl::rmi_fence();
      t0 = stapl::stop_timer(t);

      t = stapl::start_timer();
      //rescan the graphs associated with trim/mark
      for(int itr=0;itr<_trimers.size();itr++){
	_trimers[itr]->sccg.UpdateMap();
      }
      t9 = stapl::stop_timer(t);

      /*
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
      */

      tt0+=t0;//update ccno
      tt1+=t1;//clear pred
      tt2+=t2;//warm cache
      tt3+=t3;//graph initialization
      tt4+=t4;//user set dep(tt2 + tt3)
      tt5+=t5;//trim
      tt6+=t6;//sequentiall scc
      tt7+=t7;//reset and mark
      tt8+=t8;//remove scc
      tt9+=t9;//update map
      //for(tr_id=0;tr_id<_trimers.size();tr_id++){
      //_trimers[tr_id]->sccg.PrintCounter();
      //}
      
      //if(stapl::get_thread_id()==0)
      //printf("FINAL  Elapsed [iteration %d]\n \n warm cache=%f \n graph initialization=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset_trim  %f\n remove %f\n update %f\n update ccno %f\n",iteration,t2,t3,t4,t5,t6,t7,t8,t9,t0);
      stapl::rmi_fence();
    }//while !finished
    stapl::rmi_fence();
    //printf("pivots for %d -> %d [%d] [%d][sent %d recv %d] [mktt %f]\n",
    //	   stapl::get_thread_id(),pvf,tr.f,tr.b,elems,recv,tt);
    //if(stapl::get_thread_id()==0) 
    
    printf("FINAL  Elapsed [iteration %d]\n warm up cache =%f \n graph initialization=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset and mark  %f\n remove scc %f\n update %f\n update ccno %f\n",iteration,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9,tt0);
    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      cout<<tr_id<<" ";
      _trimers[tr_id]->sccg.PrintCounter();
    }
    return OK;    
  }

};//end pSCCmulti class

}//end namespace stapl
#endif
