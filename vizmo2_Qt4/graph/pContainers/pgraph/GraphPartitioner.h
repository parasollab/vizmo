/*!
	\file GraphPartitioner.h
	\author Gabriel Tanase	
	\date Apr. 9, 03
	\ingroup stapl
	\brief A collection of partition methods for parallel graph datastructure

*/
#ifndef _STAPL_GRAPH_PARTITIONER
#define _STAPL_GRAPH_PARTITIONER
extern "C" {
#include "metis.h"
}

#include "Graph.h"
namespace stapl{
template <class UGRAPH>
class GraphPartitioner{
  int NP;//number of processors
 public:
  GraphPartitioner(){
    NP = stapl::get_num_threads();
  }
  /**
   *The worst partition; For testing purposes;
   */
  int ModuloPartition(int _nr_of_verts, map<VID,PID>& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 1 && _nr_of_verts > 0);
    for(int i=0;i<_nr_of_verts;i++){
      _partition[i]= i % NP;
    }
    return OK;
  }

  int ModuloPartition(int _nr_of_verts, map<PID,vector<VID> >& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 1 && _nr_of_verts > 0);
    for(int i=0;i<_nr_of_verts;i++){
      _partition[i % NP].push_back(i);
    }
    return OK;
  }

  int ModuloPartition(int _nr_of_verts, map<VID,PID>& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    assert(_nr_of_verts > 0);
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    for(int i=0;i<_nr_of_verts;i++){
      _partition[i]= _active[i % na];
    }
  }
  int ModuloPartition(int _nr_of_verts, map<PID,vector<VID> >& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    assert(_nr_of_verts > 0);
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    for(int i=0;i<_nr_of_verts;i++){
      _partition[_active[i % na] ].push_back(i);
    }
  }
  
  
  /**
   *Simple but good enough for some input cases(2d/3d meshes);
   */
  int BlockPartition(int _nr_of_verts, map<VID,PID>& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 1 && _nr_of_verts > 0);
    for(int i=0;i<_nr_of_verts;i++){
      _partition[i] = (i * NP) / _nr_of_verts;
    }
    return OK;
  }

  int BlockPartition(int _nr_of_verts, map<PID,vector<VID> >& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 1 && _nr_of_verts > 0);
    for(int i=0;i<_nr_of_verts;i++){
      _partition[(i * NP) / _nr_of_verts].push_back(i);
    }
    return OK;
  }

  int BlockPartition(int _nr_of_verts, map<VID,PID>& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    assert(_nr_of_verts > 0);
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    for(int i=0;i<_nr_of_verts;i++){
      _partition[i]= _active[(i * na) / _nr_of_verts];
    }
  }
  int BlockPartition(int _nr_of_verts, map<PID,vector<VID> >& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    assert(_nr_of_verts > 0);
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    for(int i=0;i<_nr_of_verts;i++){
      _partition[_active[(i * na) / _nr_of_verts] ].push_back(i);
    }
  }



  /************************************************************
    ADD interface to CHACO
  *************************************************************/
  int ChacoPartition(int _nr_of_verts, char* chacofile, map<VID,PID>& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 2);
    assert(_nr_of_verts > 0);

    char * actual_file_name=new char[100];
    sprintf(actual_file_name,"%s.chaco.%d",chacofile,stapl::get_num_threads());

    printf("Reading chaco partition from file [%s]\n",actual_file_name);
    ifstream _myistream(actual_file_name);
  
    if(!_myistream.is_open()){
      cout<<"Error Opening Input File "<<actual_file_name<<endl;
      return ERROR;
    }

    //????
    int temp;
    for(int i=0;i < _nr_of_verts; i++){
      _myistream>>temp;
      _partition[i] = temp;
    }
    return OK;
  }

  int ChacoPartition(int _nr_of_verts, char* chacofile, map<PID,vector<VID> >& _partition){
    //_partition is a map from vertex if to processor id
    assert(NP >= 2);
    assert(_nr_of_verts > 0);

    char * actual_file_name=new char[100];
    sprintf(actual_file_name,"%s.chaco.%d",chacofile,stapl::get_num_threads());

    printf("Reading chaco partition from file [%s]\n",actual_file_name);
    ifstream _myistream(actual_file_name);
  
    if(!_myistream.is_open()){
      cout<<"Error Opening Input File "<<actual_file_name<<endl;
      return ERROR;
    }

    //????
    int temp;
    for(int i=0;i < _nr_of_verts; i++){
      _myistream>>temp;
      _partition[temp].push_back(i);
    }
    return OK;
  }

  int ChacoPartition(char* chacofile, map<VID,PID>& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    
    return OK;
  }

  int ChacoPartition(char* chacofile, map<PID,vector<VID> >& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    
    return OK;
  }

  /************************************************************
    ADD interface to METIS
  *************************************************************/
  /**
   *Metis Partitioner
   */
  int MetisPartition(UGRAPH& _g, map<VID,PID>& _partition,int np=-1){
    //_partition is a map from vertex if to processor id

    int i;

    assert(NP >= 1);
    //convert the graph into metis format;
    //for more on the meaning of these variables please read metis manual;
    int n = 0;//number of vertices;
    idxtype* xadj;
    idxtype* adjncy;
    idxtype* vwgt;
    idxtype* adjwgt;
    int wgtflag;
    int numflag;
    int nparts;
    int options[10];
    int edgecut;
    idxtype* part;


    //copy the graph from our format to METIS format
    //!!!  Modify such that we don't care about UGRAPH's vids

    //here we count the number of vertices and edges; this should be available in the 
    //graph datastructure but ReadExo is not correct; or at least the input files
    //I have are not correct;
    int ne = 0;
    for(typename UGRAPH::VI it = _g.begin();it != _g.end();it++){
      n++;
      for(typename UGRAPH::EI ei = it->edgelist.begin();ei != it->edgelist.end();ei++){
	ne++;
      }
    }

    cout<<"partition the graph using METIS :vertices "<<n<<"edges "<<ne<<endl;

    xadj = (int*) safe_malloc((n+1)*sizeof(int));
    part = (int*) safe_malloc(n*sizeof(int));
    adjncy = (int*) safe_malloc(ne * sizeof(int));

    int ofs = 0;
    for(typename UGRAPH::VI it = _g.begin();it != _g.end();it++){
      xadj[it->vid] = ofs;
      for(typename UGRAPH::EI ei = it->edgelist.begin();ei != it->edgelist.end();ei++){
	adjncy[ofs]=ei->vertex2id;
	ofs++;
      }
    }
    xadj[n]=ofs;

    printf("verts=%d edges=%d\n",n,ne);
 
    //for(i=0;i<n+1;i++){
    //  cout<<xadj[i]<<" "<<endl;
    //}
    //cout<<"convewrted"<<endl;
    //for(i=0;i<_g.GetNumEdges();i++){
    //  cout<<adjncy[i]<<" "<<endl;
    //}

    cout<<"start partition:"<<NP<<endl;
    vwgt = NULL;//all vertices are with weight 1
    adjwgt=NULL;//all edges are with weight 1
    wgtflag = 0;//all weights are 1
    numflag =0; //using C style(start with zero)

    if(np == -1)
      nparts = NP;//number of parts to be generated
    else
      nparts = np;

    options[0] = 0;//no options

    METIS_PartGraphRecursive(&n, 
			xadj,
			adjncy,
			vwgt,
			adjwgt,
			&wgtflag,
			&numflag,
			&nparts, 
			options,
			&edgecut,
			part);
    
    cout<<"after partition"<<endl;
    for(i=0;i<n;i++){
      _partition[i] = part[i];
    }

    free(xadj);
    free(adjncy);
    free(part);

    return OK;
  }

  int MetisPartition(UGRAPH& _g, map<PID,vector<VID> >& _partition){
    //!!! we can make the above method to return the metis partition and 
    //    convert these methods such that they only convert metis to the required format;
    //_partition is a map from vertex if to processor id
    assert(NP >= 1);
    map<VID,PID> _temp;
    this->MetisPartition(_g,_temp);
    //now convert _temp to _partition
    for(int i=0; i<_g.GetNumVerts(); ++i){
      _partition[_temp[i]].push_back(i);
    }
    return OK;
  }

  //another interface --ann
  int MetisPartition(UGRAPH& _g, map<PID,vector<VID> >& _partition, int np){
    //!!! we can make the above method to return the metis partition and 
    //    convert these methods such that they only convert metis to the required format;
    //_partition is a map from vertex if to processor id
/*     assert(NP >= 1); */
    map<VID,PID> _temp;
    this->MetisPartition(_g,_temp, np);
    //now convert _temp to _partition
    for(int i=0; i<_g.GetVertexCount(); ++i){
      _partition[_temp[i]].push_back(i);
    }
    return OK;
  }

  int MetisPartition(UGRAPH& _g, map<VID,PID>& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    int na = _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    map<VID,PID> _temp;
    this->MetisPartition(_g,_temp,na);
    //now convert _temp to _partition
    for(int i=0; i<_g.GetNumVerts(); ++i){
      _partition[i] = _active[ _temp[i] ];
    }
    return OK;

  }

  int MetisPartition(UGRAPH& _g, map<PID,vector<VID> >& _partition,vector<PID>& _active){
    //_partition is a map from vertex if to processor id
    int na= _active.size();
    if(na <= 0) {
      cout<<"ERROR:: GraphPartitioner::ModuloPartition() invalid mask of active processors"<<endl;
      return ERROR;
    }
    map<VID,PID> _temp;
    this->MetisPartition(_g,_temp,na);
    //now convert _temp to _partition
    for(int i=0; i<_g.GetNumVerts(); ++i){
      _partition[_active[_temp[i]] ].push_back(i);
    }
    return OK;
  }


};
}//end namespace
#endif
