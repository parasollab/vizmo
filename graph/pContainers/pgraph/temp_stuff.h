#ifndef TMPSTUFF
#define TMPSTUFF
//// TEMPORARY thing

const static int a[10]={0,1,1,1,0,1,0,1,0,1};
//int a[10]={3,1,2,0,2,1,3,3,2,0};
/* static int _ai=0; */
/* inline int get_next(int np){     */
/*   _ai++; */
/*   return (_ai % np); */
/*   //return a[_ai-1]; */
/* } */

/*Encapsulates the parameters for AddEdgeRemote
 */
template <class WEIGHT>
class _AER_param{
public:
  WEIGHT _w;
  VID _v1id;
  VID _v2id;
  PID _pid;   //the owner of the remote vid

  _AER_param(){
  }
  _AER_param(VID vid1, VID vid2,WEIGHT& w, PID n){
    _w = w;
    _v1id = vid1;
    _v2id = vid2;
    _pid = n;
  }

  void define_type(stapl::typer& t){
    t.local(_w);
    t.local(_v1id);
    t.local(_v2id);
  }
};

/*
   old PMETHODS from pGraph

  int pAddVertex(VERTEX& _v){
    //??? problem; it is possible to have inconsistency if you use this
    //AddVertex and the one where the vid is specified;
    
    VID vid = -1;
    if(token){//the token is mine; i'll do the insert;
      
      vid = graph.AddVertex(_v);	
      quanta--;
      if(quanta == 0){
	//pass the token to the next procs
	token = false;
	int nextproc = (myid+1) % nprocs;
	int tmp1=stapl::sync_rmi(nextproc,
			     getHandle(),
			     &PGRAPH::_sendToken,graph.getVertIDs());
      }
    }
    stapl::rmi_fence();
    //the threads should return the same value or local values;
    //right now I return local values
    return vid;
  }
  
  int _sendToken(VID vert){
    token = true;
    quanta = QUANTA;
    graph.setVertIDs(vert);
    return OK;
  }


*/


  //*********************************************************
  //*****************  different implementation SetWeightField
  //*********************************************************
  
  /**
   * This method will be used when the user will write on the weight 
   * corresponding to the edge; Some edges are between vertices that lives on 
   * two different processors; In that case I have to update the weight on the edge 
   * corresponding to the ghost node on the remote processor
   * @param VID v1 iterator to the vertex one of the edge
   * @param VID v2 iterator to the edge(second vertex and weight)
   * @param method is the method that will be called on the WEIGHT class
   * @param arg is the argument that will be passed to the method
   */


/*

  template <class T>
  int SetWeightField_2(VID v1, VID v2,int (WEIGHT::* method)(T),T arg){
    
    //this one assumes that all th threads are calling with the same parameters
    //if only one thread calls is *NOT* OK;
    int RET; 
    VI vi1, vi2;
    EI ei;

    if(IsVertex(v1, &vi1) && IsVertex(v2, &vi2)){
      //here we are if both vertices exist; we have them in vi1 and vi2
      //if(!vi1->IsGhost()){
      for(ei = vi1->edgelist.begin(); ei != vi1->edgelist.end(); ei++){
	if(ei->vertex2id == v2){
	  WEIGHT_TYPE* pwt = &(ei->weight);
	  RET = (pwt->*(method))(arg);
	  if(RET < 0) return RET;
	}
      }
      
      if(!graph.IsDirected()){//undirected
	for(ei = vi2->edgelist.begin(); ei != vi2->edgelist.end(); ei++){
	  if(ei->vertex2id == v1){
	    // one_SetWeightField(vi2, ei, method, arg);
	    WEIGHT_TYPE* pwt = &(ei->weight);
	    RET = (pwt->*(method))(arg);
	    if(RET < 0) return RET;
	  }
	}
      }//end for
    }
    stapl::rmi_fence();
    return RET;
  }

*/

 /**
   *This function is internal to pGraph; It will migrate a vertex
   *_v from were it lives to the node _n; It takes care of moving all
   *the ghost nodes and edges linked to it;
   */

/*

  this version assumes that all the threads are running;
  this makes sense since the redistribution is a global phase were 
  everybody should be involved;

  int pMigrateVertex(VID _v,PID _n){
    //This function could be called by all the processors
    //That makes sure that the ghost verices are updated too.
    //If the ghosts neednt be updated call the function _RemoteMigrateVertex
    
    //Migrates the vertex _v to node _n
    
    //This works like this. 
    //1. Makes a list of ToEdges
    //2. Makes a list of FromEdges
    //4. Adds the vertex Remotely
    //5. Calls the _AddRemoteEdge for all the deleted edges
    
    //6. Calls the MapOwner and updates the location of the Vertex

    //!!!!! needs to be checked to see if everything is updated correctly
    //the maps inside distribution
    int i;
    VI vi;
    if(IsVertex(_v,&vi)){
      if(!vi->IsGhost()){
	//Only the processor with the master vertex need to do 
	//points 1 to 6 described in the  beginning of this function
	vector<pair<VID,WEIGHT> > ToEdges; 
	vector<pair<VID,WEIGHT> > FromEdges;

	vi->GetAllEdges(FromEdges);//edges that come from _v
	
	if(graph.IsDirected())
	  graph.GetAllEdgesToV(_v,ToEdges);//edges that go to _v
			
	//cout<<"In MASTER Processor: "<<stapl::get_thread_id()<<" Handling MOVING of: "<<_v<<"   "<<vi->IsGhost()<<"  "<<GRAPH::IsGhost(_v)<<endl;
	//cout<<"From Edges"<<endl;
	//for(int i=0; i<FromEdges.size(); ++i)
	//{
	//  cout<<FromEdges[i].first<<"["<<FromEdges[i].second.Weight()<<"]  ";
	//}
	//cout<<endl;

	//cout<<"To Edges"<<endl;
	//for(int i=0; i<ToEdges.size(); ++i)
	//{
	//  cout<<ToEdges[i].first<<"["<<ToEdges[i].second.Weight()<<"]  ";
	//}
	//cout<<endl;
	VERTEX VDATA=vi->data;
	int ref_count=0;
	
	//add the node to be moved on the destination processor
	//converting eventualy from ghost to real; 
	_AddRemoteVertexGtoR(VDATA,_v,_n);

	for(i=0; i<FromEdges.size(); ++i){
	  //v -> from
	  VID current = FromEdges[i].first;
	  PID owner = dist.Lookup(current);
	  if(owner == myid){
	    ref_count++;//the node will remain alive as ghost
	  }
	  else{
	    pDeleteEdge(_v,current);//this call is not optimal
	    //I know that the edge exists so we can skip some of the 
	    //checks that this function is performing
	    //the above function will delete also the "current"
	    //vertex if the reference count goes to zero
	  }
	  //here we have to add the edge remotely
	  
	  _StaplTriple<VERTEX,VID,PID> 
	    _parg(VDATA, FromEdges[i].first, owner);
	  int tmp4=stapl::sync_rmi(_n,
				   getHandle(),
				     &PGRAPH::_AddGhostVertexRemote,_parg);
	  
	  //the data inside ghost nodes is not valid anymore
	  _StaplTriple<VID,VID,WEIGHT> 
	    _par(_v, FromEdges[i].first, FromEdges[i].second);
	  int tmp3=stapl::sync_rmi(_n,
				   getHandle(),
				   &PGRAPH::_SimpleAddRemoteEdge,_par);
	}
	
	for(i=0; i<ToEdges.size(); ++i){
	  //To -> v
	  VID current = ToEdges[i].first;
	  PID owner = dist.Lookup(current);
	  if(owner == myid){
	    ref_count++;//the node will remain alive as ghost
	  }
	  else{
	    pDeleteEdge(_v,current);//this call is not optimal
	    //I know that the edge exists so we can skip some of the 
	    //checks that this function is performing
	    //the above function will delete also the "current"
	    //vertex if the reference count goes to zero
	  }  
	  _StaplTriple<VERTEX,VID,PID> 
	    _parg(VDATA, ToEdges[i].first, owner);
	  int tmp4=stapl::sync_rmi(_n,
				   getHandle(),
				   &PGRAPH::_AddGhostVertexRemote,_parg);
	  
	  //the data inside ghost nodes is not valid anymore
	  _StaplTriple<VID,VID,WEIGHT> 
	    _par(ToEdges[i].first,_v, FromEdges[i].second);
	  int tmp3=stapl::sync_rmi(_n,
				   getHandle(),
				   &PGRAPH::_SimpleAddRemoteEdge,_par);
	  
	}
	//here if ref_count is zero it means that the node will remain alive 
	//as a ghost
	if(ref_count == 0){
	  graph.DeleteVertex(_v);
	}
	else{
	  //the node will remain here as ghost
	  vi->SetGhostInfo(ref_count);
	                         //id, where is the real, refcount
	}

	_StaplPair<VID,PID> _y;
	_y.first=_v; //Global ID of the vertex
	_y.second=_n;  //The actual location vertex
	dist._Add2LocationCache(_y);
			
      }
      else
      {
	//Now This is node with a ghost copy of the vertex
			
	//cout<<"In GHOST Processor: "<<stapl::get_thread_id()<<" Handling UPDATING of: "<<_v<<endl;

	if(myid != _n) vi->pid=_n; 
	//Just change the processor to which the ghost node is pointing to.
	//This will happen only if the pMigrateVertex is called by all 
	//the processors
      }
    }
    stapl::rmi_fence();
    return 0;
  }
*/

#endif
