#ifndef PSCCSINGLE
#define PSCCSINGLE

#include "../../common/Defines.h"
#include "runtime.h"
#include "pSCCutils.h"

#include <assert.h>
//////////////////////////////////////////////////
//pgraph algorithms  STRONGLY CONNECTED COMPONENTS
//////////////////////////////////////////////////

#define NPARTS 3

#define RIDX   0
#define SIDX   1
#define PIDX   2
#define SCCIDX 3

namespace stapl{


template <class PGRAPH>
class pSCCsingle: public BasePObject{

  typedef pGraph<PDG<taskSCC,edgeSCC>,
    PMG<taskSCC,edgeSCC>, 
    PWG<taskSCC,edgeSCC>, 
    taskSCC,edgeSCC> PGRAPH_SCC;

  vector<map<VID,vector<VID> > >*            finalSCC;
  vector<map<int,set<VID> > >                localSCC;
  vector<vector<VID> >                       _compSCC;

  vector<trimer<PGRAPH>*>* ptrimers;

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

  //to keep track of CC number
  vector<int> ccno;

  //timers
  stapl::timer t;
  double t0, t1, t2, t3,t4,t5,t6,t7,t8,t9,treuse;
  double setupt, trt, mkt, sdelt, delt, tt0,tt1,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9;

 public:
  pSCCsingle(bool sccreuse=false, bool localcc=true, bool rbm=false){
    OLDSCCREM = sccreuse;
    LCCMULTI  = localcc;
    REMOVEBEFOREMARK = rbm;
    this->register_this(this);
    myid = stapl::get_thread_id();
  }

  //use trough rmi to collect all SCC
  void _collect(int tr, pair<VID,vector<int> > p){
    //finalSCC[tr][p.second].push_back(p);
    map<VID,vector<int> >::iterator scci;
    scci = (*finalSCC)[tr].find(p.first);
    if (scci == (*finalSCC)[tr].end()){
      (*finalSCC)[tr].insert(pair<VID,vector<int> >(p.first,p.second));
    }
    else{
      scci->second.insert(scci->second.end(),p.second.begin(), p.second.end());
    }
  }

  void collect(vector<map<VID,vector<int> > >& SCC, int nrtr, int maxsz){
    int i,j,k;
    map<VID,vector<int> >::iterator scci;
    int id = stapl::get_thread_id();
    if(id != 0){
      for(k=0;k<SCC.size();k++){
	for(scci=SCC[k].begin();scci != SCC[k].end();scci++){
	  pair<VID,vector<int> > p = *scci;
	  stapl::async_rmi( 0, 
			    this->getHandle(),
			    &pSCCsingle<PGRAPH>::_collect,k,p); 
	}
	//printf("\n");
      }
    }
    stapl::rmi_fence();
  }

  void dump(char* file_name,int tr){

    map<VID,vector<int> >::iterator scci;

    if(stapl::get_thread_id()==0){
      char * actual_file_name=new char[100];
      sprintf(actual_file_name,"%s.%d.scc",file_name,tr);
      printf("writing scc to file [%s]\n",actual_file_name);
      ofstream _myistream(actual_file_name);
      if(!_myistream.is_open()){
	cout<<"Error opening output file "<<actual_file_name<<endl;
	return;
      }
      for(scci = (*finalSCC)[tr].begin();scci != (*finalSCC)[tr].end();scci++){
	if(scci->second.size() > 1){
	  for(int j=0;j<scci->second.size();j++){
	    _myistream<<scci->first<<" "<<scci->second[j]<<endl;
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
	 stapl::async_rmi(_n.locpid(),this->getHandle(),&pSCCsingle<PGRAPH>::_compactSCC,arg);
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
	 stapl::async_rmi(0,this->getHandle(),&pSCCsingle<PGRAPH>::_compactSCCLeader,arg);
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
    vector<VID> start_in;
    vector<VID> start_out; 

    t = stapl::start_timer();

    _stapl_pscc_trim_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_trim_back<PGRAPH_SCC>  vis_back;
    
    for(i=0;i<_trimers.size();i++){
      _trimers[i]->sccg.GetZeroDependencies(start_in,start_out);
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

  void trim_forward(vector<trimer<PGRAPH>* >& _trimers){

    int i;
    stapl::timer t;

    t = stapl::start_timer();
    _stapl_pscc_trim_forward<PGRAPH_SCC>  vis_fwd;    
    for(i=0;i<_trimers.size();i++){
      _trimers[i]->sccg.TOPO_traversal(vis_fwd,0,false);
    }//for every trimer

    stapl::rmi_fence();
    stapl::stop_timer(t);    
  }//end trim method

  void trim_backward(vector<trimer<PGRAPH>* >& _trimers){

    int i;
    stapl::timer t;

    t = stapl::start_timer();
    _stapl_pscc_trim_back<PGRAPH_SCC>  vis_back;
    
    for(i=0;i<_trimers.size();i++){
      _trimers[i]->sccg.TOPO_traversal_EQ(vis_back,0,true);
    }//for every trimer

    stapl::rmi_fence();
    stapl::stop_timer(t);    
  }//end trim method


  int mark_method(vector<trimer<PGRAPH>* >& _trimers,bool reuse){

    PGRAPH_SCC::VI vi;
    int vid;
    int j,global;
    vector<int> npivots(_trimers.size(),0);

    int finished = 0;
    stapl::timer t;

    t = stapl::start_timer();

    _stapl_pscc_mark_forward<PGRAPH_SCC>  vis_fwd;
    _stapl_pscc_mark_back<PGRAPH_SCC>  vis_back;

    for(int i=0;i<_trimers.size();i++){
	npivots[i] = _trimers[i]->FindMultiPivotsSingle(ccno[i]);
    }

    for(int i=0;i<_trimers.size();i++){
      
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

      //if(npivots[i] == -1){
      //cout<<stapl::get_thread_id()<<"---------------------------------------taken:"<<_trimers[i]->sccg.local_size() <<endl;
      //_trimers[i]->sccg.ErasePGraph();
      //finished++;
      //} else 
      
      if(npivots[i] == 0){
	finished++;
      }
      else{
	//printf("inside %d for vertex %d\n",stapl::get_thread_id(),global);
	for(j=0;j<npivots[i];j++){
	  //cout<<"PIVOT::"<<_trimers[i]->pivots[j]<<endl;
	  pvf++;
	  vis_fwd.set_pivot_info(0,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j],vis_fwd,0,false);
	  
	  vis_back.set_pivot_info(0,_trimers[i]->pivots[j]);
	  _trimers[i]->sccg.BFS_traversal_EQ(_trimers[i]->pivots[j], vis_back, 0, true);
	}
	//cout<<"MARK::"<<_trimers[i]->sccg._nins<<endl;
      }    
    }//for every trimer/graph

    double t2 = stapl::stop_timer(t);    
    //printf("find pivots %f\n",t2);

    tt+=t2;

    //printf("wait fence %d\n",stapl::get_thread_id());    
    stapl::rmi_fence();
    stapl::rmi_fence();
    stapl::reduce_rmi<pSCCsingle, int>(&finished, 
				 &global, 
				 getHandle(),
				 &pSCCsingle::_MIN, 
				 true);
    //printf(" minimum %d\n",global); 
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
  int sequentialSCC(vector<trimer<TRIMGRAPH>*>&  _trimers,
		    vector<set<int> >& nonlocalCC,
		    vector<map<VID,vector<int> > >& SCC){

    PGRAPH_SCC::VI vi;
    int i, tr_id, tmp;
    set<int>::iterator sit;
    vector<PGRAPH_SCC::VI> ft;//finish times
    vector<VID> _scc;
    map<VID,vector<int> >::iterator scci;

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
	      scci = SCC[tr_id].find(_scc[0]);
	      if (scci == SCC[tr_id].end()){
		//new scc
		vector<int> temp;
		temp.push_back(_scc[j]);
		SCC[tr_id][_scc[0]] = temp;
		//cout<<"lSCC considered:"<<_scc[j]<<":"<<_scc[0]<<endl;
	      }
	      else{
		//some vertices of scc are there
		scci->second.push_back(_scc[j]);
		//cout<<"l2SCC considered:"<<_scc[j]<<":"<<_scc[0]<<endl;
	      }
	      //SCC[tr_id].push_back(tempp);
	    }
	  } 
	}
	//next mark node as dead such that mark will skip 
	ft[i]->data.in = 0;
      }
      tmp += ft.size();
    }//for every trimer
    //cout<<"ELIMINATED"<<tmp<<endl;
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
      //_trimers[i]->sccg.WarmCache();
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


  int removeLocalSCC(vector<trimer<PGRAPH>*>& _trimers,
		     vector<map<VID,vector<int> > >& SCC){
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
      
      //perform the union of non locall CC
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
    

    //cout<<"NONLOCAL SIZE:"<<nonlocalCC[0].size()<<endl;
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

    if(tcnt > 0)
      sequentialSCC(_trimers,nonlocalCC,SCC);

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



  void pscc_partition(vector<trimer<PGRAPH>*>&  _trimers, pSCCWorkFunction<PGRAPH>* pf){

    int CCNO,i,tr_id;
    PGRAPH_SCC::VI vi;
    int nparts;
    vector<int> l_subsize;
    vector<int> g_subsize;

    for(tr_id=0;tr_id<_trimers.size();tr_id++){
      
      CCNO = ccno[tr_id];
      if(CCNO == 0 ) continue;
      if(pf == NULL){
	nparts = NPARTS;
      }
      else{
	nparts = NPARTS + 1;
      }

      l_subsize.resize(CCNO * nparts , 0);
      g_subsize.resize(CCNO * nparts , 0);

      //cout<<"resizing for :"<< CCNO*NPARTS << endl;
      //for(i=0;i<CCNO * NPARTS;i++){
      //l_subsize[i] = 0;
      //}

      for(vi = _trimers[tr_id]->sccg.local_begin();
	  vi != _trimers[tr_id]->sccg.local_end();++vi){
	//if dead vertex add to to_delete

	if(vi->data.in == 0 || vi->data.out==0){
	  continue;
	}

	//succ component
	if(vi->data.forward[0] != -1 && vi->data.back[0] == -1){
	  l_subsize[vi->data.ccno + SIDX * CCNO]+=1;
	}
	//pred component
	if(vi->data.forward[0] == -1 && vi->data.back[0] != -1){
	  l_subsize[vi->data.ccno + PIDX * CCNO]+=1;
	}
	//rem component
	if(vi->data.forward[0] == -1 && vi->data.back[0] == -1){
	  l_subsize[vi->data.ccno + RIDX * CCNO]+=1;
	}

	//current scc with an edge removed is another distinct component;
	//it will separate succ and pred
	if(pf != NULL){  
	  if(vi->data.forward[0] != -1 && vi->data.back[0] != -1 && 
	    vi->data.forward[0] == vi->data.back[0]){
	    l_subsize[vi->data.ccno + SCCIDX * CCNO]+=1;
	  }
	}
      }
      //reduce rmi
#ifdef _STAPL_THREAD
      #warning Reduce not implemented yet... ANY CODE USING IT WILL BE WRONG - JKP/02Dec2004
      stapl_assert( 0, "Reduce not implemented yet... ANY CODE USING IT WILL BE WRONG - JKP/02Dec2004");
#else
      MPI_Allreduce(&l_subsize[0],&g_subsize[0],CCNO*nparts,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
#endif

      int newcc=0;
      for(i=0;i < CCNO*nparts; i++){
	if(g_subsize[i] > 1) {
	  //if(freelist.size() > 0) {
	  //g_subsize[i] = freelist.back();
	  //freelist.pop_back();
	  //}
	  //else{
	  //g_subsize[i] = CCNO+newcc;
	  //newcc++;
	  //}
	  l_subsize[i] = newcc++;
	}
      }
      //now I have to update all vertices with the new ccno
	
      for(vi = _trimers[tr_id]->sccg.local_begin();
	  vi != _trimers[tr_id]->sccg.local_end();++vi){
	//if dead vertex add to to_delete
	if(vi->data.in == 0 || vi->data.out==0){
	  continue;
	}
	
	//                                  CCNO
	//create new connected components corresponding to pred,succ,rem
	//and when pf is defined to current scc;
	//here I update the ccno field for every vertex according with the
	//marking done for the current pivot; the update is done using the local CCNO
	//pre_ccno succ_ccno
	//this is relative to the pivots corresponding to this thread;
	if(vi->data.forward[0] != -1 && vi->data.back[0] != -1 && vi->data.forward[0]!= vi->data.back[0] ){
	  cout<<"ERROR::While generating new connected components"<<endl;
	}
	int temp = vi->data.ccno;
	
	//succ component
	if(vi->data.forward[0] != -1 && vi->data.back[0] == -1){
	  //if (temp + SIDX*CCNO >= l_subsize.size()){
	  //cout<<"ERRORRRRRRRRRRRRRR1"<<endl;
	  //}
	  vi->data.ccno = l_subsize[temp + SIDX * CCNO];
	  //cout<<":::PArtition:"<<vi->vid<<"|"<<vi->data.ccno<<"old"<<temp<<":index:"<<temp+SIDX*CCNO<<endl;
	}
	//pred component
	if(vi->data.forward[0] == -1 && vi->data.back[0] != -1){
	  //if (temp + PIDX*CCNO >= l_subsize.size()){
	  //cout<<"ERRORRRRRRRRRRRRRR2"<<endl;
	  //}
	  vi->data.ccno = l_subsize[temp + PIDX * CCNO];
	  //cout<<":::PArtition:"<<vi->vid<<"|"<<vi->data.ccno<<"old"<<temp<<":index:"<<temp+PIDX*CCNO<<endl;
	}
	//rem
	if(vi->data.forward[0] == -1 && vi->data.back[0] == -1){
	  //if (temp + PIDX*CCNO >= l_subsize.size()){
	  //cout<<"ERRORRRRRRRRRRRRRR3"<<endl;
	  //}
	  vi->data.ccno = l_subsize[temp + RIDX * CCNO];
	  //cout<<":::PArtition:"<<vi->vid<<"|"<<vi->data.ccno<<"old"<<temp<<":index:"<<temp+PIDX*CCNO<<endl;
	}

	//current scc with an edge removed is another distinct component;
	//it will separate succ and pred
	if(pf != NULL){  
	  if(vi->data.forward[0] != -1 && vi->data.back[0] != -1 && 
	     vi->data.forward[0] == vi->data.back[0]){
	    vi->data.ccno = l_subsize[temp + SCCIDX * CCNO];
	  }
	}
	
      }//for every vertex in the current trimer
      ccno[tr_id] = newcc;
    }//for every trimer/graph   
    l_subsize.clear();
    g_subsize.clear();
  }

  void DeletePredecessor(_StaplTriple<int,VID,VID> args){
    PGRAPH_SCC::VI vi;

    if(!(*ptrimers)[args.first]->sccg.IsVertex(args.second,&vi)){
      cout<<"ERROR:: Invalid vertex in pSCC::DeletePredecessorEdge"<<endl;
      return;
    }
    vi->DeleteXPredEdges(args.third,1);
    vi->DeleteXEdges(args.third,1);
    vi->SetDependencies(0);
  }

  void CheckEdgePred(_StaplSix<int,VID,VID,VID,VID,int> args){
    PGRAPH_SCC::VI vi;
    taskSCC data1;
    taskSCC data2;
    bool _action = false;

    if(!(*ptrimers)[args.first]->sccg.IsVertex(args.second,&vi)){
      cout<<"ERROR:: Invalid vertex in pSCC::CheckEdge : "<<args.second<<"from vertex"<<args.third<<endl;
      return;
    }
    data1 = vi->data;
    if(data1.in == 0 || data1.out==0) {
      return;
    }
   
    int k=0;
    if(data1.back[k] != args.fourth || data1.forward[k] != args.fifth){
      //the dest node is not in remaining; delete the edge
      vi->DeleteXEdges(args.third,1);
      vi->DeleteXPredEdges(args.third,1);
      vi->SetDependencies(0);
      _action = true;
    }      
    if(_action){
      _StaplTriple<int,VID,VID> _args(args.first, args.third, vi->vid);
      stapl::async_rmi(args.sixth,this->getHandle(),
		       &pSCCsingle<PGRAPH>::DeletePredecessor,_args);
    }
  }

  void DeleteSuccessor(_StaplTriple<int,VID,VID> args){
    PGRAPH_SCC::VI vi;

    if(!(*ptrimers)[args.first]->sccg.IsVertex(args.second,&vi)){
      cout<<"ERROR:: Invalid vertex in pSCC::DeletePredecessorEdge"<<endl;
      return;
    }
    vi->DeleteXEdges(args.third,1);
    vi->DeleteXPredEdges(args.third,1);
    vi->SetDependencies(0);
  }

  void CheckEdgeSucc(_StaplSix<int,VID,VID,VID,VID,int> args){
    PGRAPH_SCC::VI vi;
    taskSCC data1;
    taskSCC data2;
    bool _action = false;

    if(!(*ptrimers)[args.first]->sccg.IsVertex(args.second,&vi)){
      cout<<"ERROR:: Invalid vertex in pSCC::CheckEdge : "<<args.second<<"from vertex"<<args.third<<endl;
      return;
    }
    data1 = vi->data;
    if(data1.in == 0 || data1.out==0) {
      return;
    }
   
    int k=0;
    if(data1.back[k] != args.fourth || data1.forward[k] != args.fifth){
      //the dest node is not in remaining; delete the edge
      vi->DeleteXPredEdges(args.third,1);
      vi->DeleteXEdges(args.third,1);
      vi->SetDependencies(0);
      _action = true;
    }      
    if(_action){
      _StaplTriple<int,VID,VID> _args(args.first, args.third, vi->vid);
      stapl::async_rmi(args.sixth,this->getHandle(),
		       &pSCCsingle<PGRAPH>::DeleteSuccessor,_args);
    }
  }

  int initTraversalInfo(vector<trimer<PGRAPH>*>&  _trimers, int iteration){
    int i;
    stapl::timer t_l;
    PGRAPH_SCC::VI vi,viend;
    PGRAPH_SCC::EI ei;
    t_l = stapl::start_timer();
    //clear the vectores where the predecessors are stored
    //for(i=0;i<_trimers.size();i++){
    //_trimers[i]->sccg.WarmCache();
    //}
    //stapl::rmi_fence();
    if(iteration == 1){
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.ClearPredecessors();
	//_trimers[i]->sccg.WarmCache();
      }
      stapl::rmi_fence();
    }
    t2 = stapl::stop_timer(t_l);
   
    t_l = stapl::start_timer();

    if(iteration == 1){
      //fill the predecessors list
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.pAsyncSetPredecessors();
      }
      stapl::rmi_fence();

      //for every trimer set the vectors storing the dependencies;
      for(i=0;i<_trimers.size();i++){
	_trimers[i]->sccg.SetDependencies(0);	
      }

    }
    t3 = stapl::stop_timer(t_l);       

    //for every trimer set the vectors storing the dependencies;
    /*
    for(i=0;i<_trimers.size();i++){
      _trimers[i]->sccg.SetDependencies(0);	
    }
    */

    return OK;
  }

  void _AddToMap(int tr_id,pair<VID,VID> _args){
    map<VID,vector<VID> >::iterator scci;
    scci = (*finalSCC)[tr_id].find(_args.first);
    if (scci == (*finalSCC)[tr_id].end()){
      //new scc
      vector<int> temp;
      temp.push_back(_args.second);
      (*finalSCC)[tr_id][_args.first] = temp;
    }
    else{
      //some vertices of scc are there
      scci->second.push_back(_args.second);
    }
  }

  void AddToMap(pSCCWorkFunction<PGRAPH>* pf,vector<trimer<PGRAPH>*>&  _trimers, int tr_id,VID sccno, VID vid){
    PARTID partid;
    if(pf == NULL){
      _AddToMap(tr_id,pair<VID,VID>(sccno,vid));
    }
    else{
      if(_trimers[tr_id]->sccg.IsLocal(sccno,partid)){
	_AddToMap(tr_id,pair<VID,VID>(sccno,vid));
      }
      else{
	//remote
	Location _n=_trimers[tr_id]->sccg.Lookup(sccno);
	//if(!_n.ValidLocation()){
	//}
	stapl_assert(_n.ValidLocation(),"Invalid sccno in pSCC::AddToMap");
	_StaplPair<VID,VID> args(sccno,vid);
	stapl::async_rmi(_n.locpid(),this->getHandle(),
			 &pSCCsingle<PGRAPH>::_AddToMap,tr_id,args);
	
      }
    }
  }
  int run(vector<trimer<PGRAPH>*>&  _trimers,
	  vector<vector<pair<VID,VID> > >& Edges,
	  vector<map<VID,vector<VID> > >& SCC,
	  pSCCWorkFunction<PGRAPH>* pf=NULL, int aggf=50,bool reuse=false){

    typedef pBaseGraph<taskSCC,edgeSCC> PBASE_SCC;
    

    PGRAPH_SCC::EI ei,eiend;
    PGRAPH_SCC::VI vi,viend;
    PGRAPH_SCC::VI vi2;

    taskSCC data1,data2;

    bool finished=false;
    int iteration = 0;   
 
    ptrimers = &_trimers;

    //timers
    double _e_trim, _e_mark, _e_part;
    _e_trim = _e_mark = _e_part = 0.0;

    tt0=tt1=tt2=tt3=tt4=tt5=tt6=tt7=tt8=tt9=0.0;
    setupt=trt=mkt=sdelt=delt=0.0;
    
    vector<vector<VID> > to_delete(_trimers.size());

    vector<VID> del_edgef;
    vector<VID> neighbours;
    
    int i,j,k,pivt;
    int vid;

    map<VID,vector<VID> >::iterator scci;

    int tr_id;
    pair<double,double> dtp,t_dtp(0.0,0.0);    

    
    t = stapl::start_timer();
    
    finalSCC = &SCC;
    SCC.clear();
    SCC.resize(_trimers.size());
    ccno.resize(_trimers.size(),1);

    if(pf != NULL){
      Edges.clear();
      Edges.resize(_trimers.size());
    }

    int myid = stapl::get_thread_id();

    //needed because the constructors has to finish
    stapl::set_aggregation(aggf);

    stapl::rmi_fence();
    t0 = stapl::stop_timer(t);

    treuse = 0;

    //    Remove SCCs using informations from a previous run of the algorithm
    /*
    if(OLDSCCREM){
      t = stapl::start_timer();
      if(reuse == true){
	checkSCCFromPreviousStep(_trimers,SCC,reuse);
      }
      treuse = stapl::stop_timer(t);
    }
    */

    while(!finished){
      iteration++;
      finished = true;
      //if(myid == 0) cout<<"iteration"<<iteration<<endl;

      t = stapl::start_timer();
       initTraversalInfo(_trimers,iteration);
       stapl::rmi_fence();
      t4 = stapl::stop_timer(t);
      
      //if (myid == 1)
      //_trimers[0]->DisplayGraph();
      //_trimers[0]->sccg.DisplayDistributionInfo();



      //                       TRIM
#ifdef ARMI_STATS
    stapl::reset_armi_stats();
#endif
      t = stapl::start_timer();
       trim_method(_trimers);
       stapl::rmi_fence();
      t5 = stapl::stop_timer(t); 
#ifdef ARMI_STATS
    _e_trim += stapl::get_armi_elapsed(S_TOTAL);
#endif

      //                    Local Connected Component check
      //here I have to add to nonlocalCC the CC that are not local;
      //it is easier to identify them

      //                       UNCOMENT THIS               !!!!!!!!!!!!!!!
      //if(LCCMULTI)
      //if(iteration > 1) {
      //  removeLocalSCC(_trimers,SCC);
      //}
      //stapl::rmi_fence();
      

      if(REMOVEBEFOREMARK)
	removeBeforeMark(_trimers);

      //                       MARK
      t = stapl::start_timer();
      for(i=0;i<_trimers.size();i++){
	//_trimers[i]->sccg.ResetTraversals();
	for(vi = _trimers[i]->sccg.local_begin();
	    vi != _trimers[i]->sccg.local_end();++vi){
	  //if dead vertex add to to_delete	  
	  if(vi->data.in == 0 || vi->data.out==0){
	    continue;
	  }
	  vi->ResetTraversal(0);
	  vi->data.forward[0] = vi->data.back[0] = -1;
	}
      }
#ifdef ARMI_STATS
    stapl::reset_armi_stats();
#endif
      stapl::rmi_fence();
      t9 = stapl::stop_timer(t);
      t = stapl::start_timer();

      finished = mark_method(_trimers,reuse);
      stapl::rmi_fence();
#ifdef ARMI_STATS
    _e_mark += stapl::get_armi_elapsed(S_TOTAL);
#endif
      t7 = stapl::stop_timer(t);      
      if (finished) break;
      //                       END MARK      
      //      Remove dead vertices, the SCCs found and the edges between
      //      REM/SUCC/PRED

      t = stapl::start_timer();

#ifdef ARMI_STATS
      stapl::reset_armi_stats();
#endif
      pscc_partition(_trimers,pf);
      stapl::rmi_fence();
      t6 = stapl::stop_timer(t);    

      t = stapl::start_timer();      
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	to_delete[tr_id].clear();	
	
	for(vi = _trimers[tr_id]->sccg.local_begin();
	    vi != _trimers[tr_id]->sccg.local_end();++vi){

	  //if dead vertex add to to_delete

	  if(vi->data.in == 0 || vi->data.out==0){
	    //to_delete[tr_id].push_back(vid);
	    continue;
	  }

	  vi->SetDependencies();

	  //here I check only marked vertices
	  //there is another alternative to check REM
	  if(vi->data.back[0] == -1 && vi->data.forward[0] == -1){
	    continue;
	  }

	  vid = vi->vid;

	  //the node is alive;
	  //check if it is part of the SCC
	  pivt =0;
	  if((vi->data.forward[pivt] == vi->data.back[pivt])){
	    if(pf == NULL){
	      AddToMap(pf,_trimers,tr_id,vi->data.forward[pivt],vid);
	      //if the function is not specifed the normal scc will run	    
	      to_delete[tr_id].push_back(vid);
	    }
	    else{
	      //keep the scc where the pivot lives
	      AddToMap(pf,_trimers,tr_id,vi->data.forward[pivt],vid);
	    }
	  }
	  
	  //after we delete the vertices that are dead or in scc
	  //we have to remove edges between REM and FWD and BACK
	  data2 = vi->data;
	  
	  //printf("posible rem candidate %d\n",vi->vid);
	  //vi belongs to remaining
	  //del_edgef.clear();
	  if(data2.forward[0] != -1){
	    //neighbours.clear();
	    neighbours.resize(vi->predecessors.size());

	    j = 0;
	    for(ei = vi->predecessors.begin();ei != vi->predecessors.end();ei++){     
	      neighbours[j] = ei->vertex2id;
	      j++;
	    }
	    //eiend = vi->predecessors.end();
	    //for(ei = vi->predecessors.begin();ei != eiend ;++ei){
	    for(int np=0;np<j;np++){
	      //if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
	      if(!_trimers[tr_id]->sccg.IsVertex(neighbours[np],&vi2)){
		Location _n=_trimers[tr_id]->sccg.Lookup(neighbours[np]);
		if(!_n.ValidLocation()){
		  cout<<"problem with pred "<<neighbours[np]<<":"<<np<<" from vertex "<<vid<<"on trim:"<<tr_id<<endl;
		}
		stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSCC::run");
		_StaplSix<int,VID,VID,VID,VID,int> args(tr_id,neighbours[np],vid,data2.back[0],data2.forward[0],myid);
		stapl::async_rmi(_n.locpid(),this->getHandle(),
				 &pSCCsingle<PGRAPH>::CheckEdgePred,args);
		continue;
	      }
	      
	      //data2 = _trimers[tr_id]->sccg.GetData(neighbours[np]);
	      data1 = vi2->data;
	      
	      if(data1.in == 0 || data1.out==0) {
		continue;
	      }

	      int k=0;
	      if(data1.back[k] != data2.back[k] || data1.forward[k] != data2.forward[k]){
		//the dest node is not in remaining; delete the edge
		vi2->DeleteXEdges(vid,1);
		vi2->SetDependencies(0);
		vi->DeleteXPredEdges(vi2->vid,1);
		vi->SetDependencies(0);
	      }
	      //here
	    }//for every edge
	  }
	  else{
	    //neighbours.clear();
	    neighbours.resize(vi->edgelist.size());
	    j = 0;
	    for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){     
	      neighbours[j] = ei->vertex2id;
	      j++;
	    }
	    for(int np=0;np < j;np++){
	      //if(!_trimers[tr_id]->sccg.IsVertex(ei->vertex2id,&vi2)){
	      if(!_trimers[tr_id]->sccg.IsVertex(neighbours[np],&vi2)){
		Location _n=_trimers[tr_id]->sccg.Lookup(neighbours[np]);
		stapl_assert(_n.ValidLocation(),"Invalid id for the vertex in pSCC::run");
		_StaplSix<int,VID,VID,VID,VID,int> args(tr_id,neighbours[np],vid,data2.back[0],data2.forward[0],myid);
		stapl::async_rmi(_n.locpid(),this->getHandle(),
				 &pSCCsingle<PGRAPH>::CheckEdgeSucc,args);
		continue;
	      }
	      
	      //data2 = _trimers[tr_id]->sccg.GetData(neighbours[np]);
	      data1 = vi2->data;
	      
	      if(data1.in == 0 || data1.out==0) {
		continue;
	      }

	      int k=0;
	      if(data1.back[k] != data2.back[k] || data1.forward[k] != data2.forward[k]){
		//the dest node is not in remaining; delete the edge
		vi2->DeleteXPredEdges(vid,1);
		vi2->SetDependencies(0);
		vi->DeleteXEdges(vi2->vid,1);
		vi->SetDependencies(0);
	      }
	      //here
	    }//for every edge
	  }
	}//for every vertex in the current trimer
      }//for every trimer/graph

      stapl::rmi_fence();

      //delete the edges to break the cycles
      if(pf != NULL){
	//for all trimers
	for(tr_id=0;tr_id<_trimers.size();tr_id++){
	  //for all SCC found
	  for(scci = SCC[tr_id].begin(); scci != SCC[tr_id].end(); scci++){
	    //cout<<"checking for "<<scci->first<<":";
	    //for(i=0;i<scci->second.size();i++){
	    //cout<<" "<<scci->second[i];
	    //}
	    //cout<<endl;
	    pair<VID,VID> edge = (*pf)(*scci , *(_trimers[tr_id]));
	    if (edge.first != -1){
	      //_trimers[tr_id]->sccg.DeleteEdge(edge.first,edge.second);
	      _trimers[tr_id]->sccg.DeleteEdgeAndPredecessor(edge.first,edge.second);
	      Edges[tr_id].push_back(edge);
	      //cout<<"Edge Eliminated:"<<edge.first<<":"<<edge.second<<endl;
	    }
	  }
	  //now the SCC[tr_id] is cleaned
	  SCC[tr_id].clear();
	}
      }

      stapl::rmi_fence();
      t8 = stapl::stop_timer(t);
      t = stapl::start_timer();
      for(tr_id=0;tr_id<_trimers.size();tr_id++){
	dtp = _trimers[tr_id]->sccg.pSimpleDeleteVertices(to_delete[tr_id]);
	//t_dtp.first += dtp.first;
	//t_dtp.second += dtp.second;
      }
      //stapl::rmi_fence();
      t0 = stapl::stop_timer(t);

      //t = stapl::start_timer();

      //rescan the graphs associated with trim/mark
      //for(int itr=0;itr<_trimers.size();itr++){
      //_trimers[itr]->sccg.UpdateMap();
      //}

      //t9 = stapl::stop_timer(t);
      /*
      if(LCCMULTI){
	for(tr_id=0;tr_id<_trimers.size();tr_id++){
	  for(vi = _trimers[tr_id]->sccg.local_begin();vi != _trimers[tr_id]->sccg.local_end();++vi){
	    for(ei = vi->edgelist.begin();ei != vi->edgelist.end();ei++){
	      if(!_trimers[tr_id]->sccg.IsVertex(neighbours[np],&vi2)){
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
      tt9+=t9;//reset for mark

      //if(stapl::get_thread_id()==0)
      //printf("FINAL  Elapsed [iteration %d]\n \n warm cache=%f \n graph initialization=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset_trim  %f\n remove %f\n update %f\n deleteVertices %f\n",iteration,t2,t3,t4,t5,t6,t7,t8,t9,t0);

      //stapl::rmi_fence();
#ifdef ARMI_STATS
    _e_part += stapl::get_armi_elapsed(S_TOTAL);
#endif
    }//while !finished
    stapl::rmi_fence();

    /*
    if(stapl::get_thread_id()==0) {    
      printf("FINAL  Elapsed [iteration %d]\n warm up cache =%f \n graph initialization=%f\n user set dep=%f\n trim=%f\n seq scc=%f\n reset and mark  %f\n remove scc %f\n reset for mark %f\n deleteVertices %f\n",iteration,tt2,tt3,tt4,tt5,tt6,tt7,tt8,tt9,tt0);
      cout<<"DELETE: delete edges"<<t_dtp.first<<"compact:"<<t_dtp.second<<endl;
      printf("[%d]Communication time trim=[%f] mark = [%f] partition = [%f]\n",myid, _e_trim, _e_mark,_e_part);
    }
    */
    return OK;    
  }

};//end pSCCsingle class

template <class PGRAPH>
  void pSCC_single(vector<PGRAPH*>&  _vpg, int n_pgraphs,
	  vector<vector<pair<VID,VID> > >& Edges,
	  vector<map<VID,vector<VID> > >& SCC,
	  pSCCWorkFunction<PGRAPH>* pf=NULL, int aggf=50,bool reuse=false){
  stapl::timer t;
  double elapsed;
  int i;

  vector<trimer<PGRAPH>* > vtr;
  for(i=0;i<n_pgraphs;i++){
    trimer<PGRAPH>* tr1 = new trimer<PGRAPH>(_vpg[i]);
    tr1->sccg.SetTraversalNumber(stapl::get_num_threads());
    vtr.push_back(tr1);
  }

  pSCCsingle<PGRAPH> pscc;
  stapl::rmi_fence();
  t = stapl::start_timer();
  pscc.run(vtr,Edges,SCC,pf,aggf,false);
  elapsed = stapl::stop_timer(t);      
  //cout<<"Elapsed time pSCC only:"<<elapsed<<endl;
#ifdef _PSCC_CORRECTNES
  stapl::rmi_fence();
  pscc.collect(SCC,vtr.size(),SZ);
  stapl::rmi_fence();

  for(i=0;i<vtr.size();i++){
    pscc.dump("res",i);
  }
  stapl::rmi_fence();
#endif
  for(i=0;i<vtr.size();i++){
    delete vtr[i];
  }
}
}//end namespace stapl
#endif
