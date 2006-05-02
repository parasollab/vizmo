/* Tester classes for basic methods of sequential Graph
 * inheritted from GraphTester
 */
#include "commontester.h"

using namespace stapl;


/* (0) Tester_Simple : Test some simple methods of graph: size, 
 * GetEdgeCount, IsVertex, IsEdge, EraseGraph 
 */
template <class GRAPH>
class Tester_Simple : public GraphTester<GRAPH> {

public:

  Tester_Simple() {flag='b';}

  char* GetName() { return "Tester_Simple"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_Simple *t = new Tester_Simple; t->flag=this->flag; return t; }

  void ShowUsage() {
    GraphTester<GRAPH>::ShowUsage();
  }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 3;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
	//this tester is simple, there's no main difference between performance test and correctness test
	//so just write them together in Test()
    int tmp;

    starttimer;
    typename GRAPH::VI vi;
    typename GRAPH::EI ei;

    GRAPH newsg = sg;
    newsg.EraseGraph();

    counter=0;
    /* 0.1 empty */
    MethodNames.push_back("empty");
    bool emptyflag=false;
#ifdef VERBOSE
      stapl_print("Testing empty ... ");
#endif
    if_want_perf {
      starttimer;
      emptyflag=newsg.v.empty();
      newstoptimer;
    }
    if_want_corr {
      if (!(newsg.v.empty()) ) {
	cout<<"ERROR while testing EraseGraph/empty"<<endl;
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 0.2 size */
    MethodNames.push_back("size()");
    int size;
#ifdef VERBOSE    
    stapl_print("Testing size() for "); 
    stapl_print(sg.size()); stapl_print(" vertices...");
#endif
    if_want_perf {
      starttimer;
      size=sg.size();
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 0.3 GetEdgeCount */
    MethodNames.push_back("GetEdgeCount()");
#ifdef VERBOSE    
    stapl_print("Testing GetEdgeCount() for "); 
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      int ecount=sg.GetEdgeCount();
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
    stapl_print("\nTester_Simple tested.\n");
#endif
  }
};

/* (1) Tester_AddVertex : Tester for Graph's method AddVertex;
 * this method can be called directly, not depending on any other methods
 * @v_num given number of vertices to be added into graph
 * In our testing, we add v_num vertices to the graph, each vertex having index(0~v_num) as its weight.
 */
template <class GRAPH>
class Tester_AddVertex : public GraphTester<GRAPH> {
  int v_num;

public:
  Tester_AddVertex() {flag='b'; v_num=0;}
  Tester_AddVertex(int _v_num) {flag='b'; v_num=_v_num;}

  char* GetName() { return "Tester_AddVertex"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_AddVertex *t = new Tester_AddVertex; t->flag=this->flag; t->v_num=this->v_num; return t; }

  void ParseParams(char* s) {
	//the incoming string is such as: "n 20": neither performance nor correctness  & add 20 vertices
	//OR : "p 20": report performance & work on current graph & add 20 vertices
    char *space = " "; char *p;

    if(!(p=strtok(s,space))) { 
      ShowUsage(); 
      exit(1); 
    }
    if (strcmp(p, "-quiet")==0) {
      quiet=true; 
      flag = strtok(NULL,space)[0];
    } else {
      quiet = false;
      flag = p[0];
    }
    if (!(flag=='p'||flag=='c'||flag=='b'))  { 
      ShowUsage(); 
      exit(1); 
    }
    if (p=strtok(NULL,space)) v_num=atoi(p);
    return;
  }  

  void SetTestNo() {
    testno = 3;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;

    Task datum; vector<Task> data;
    typename GRAPH::VI vi;
    for (int i=0; i<v_num; i++) {
      datum.SetWeight(i);
      data.push_back(datum);
    }
  
    GRAPH newsg = sg;

    counter = 0;
    /* 1.1 AddVertex(1): AddVertex(VERTEX& ) */
    MethodNames.push_back("AddVertex(1) - AddVertex(VERTEX&)" );
#ifdef VERBOSE    
    stapl_print("Testing AddVertex(VERTEX&) for ");
    stapl_print(v_num); stapl_print(" vertices ...");
#endif
    if_want_perf {
//      sg=newsg;
      sg.EraseGraph();
      starttimer;
      for (int i=0; i<v_num; i++)  {
        tmp = sg.AddVertex(data[i]);
      }
      newstoptimer;
    } 
    if_want_corr {
      sg.EraseGraph();
      for (int i=0; i<v_num; i++)  tmp = sg.AddVertex(data[i]);
      for (int i=0; i<v_num; i++) {
        if (!sg.IsVertex(i,&vi) ) {
          cout<<"ERROR while testing AddVertex(VERTEX&)/IsVertex (1) i="<<i<<" is not vertex"<<endl;
   	  break;
	}
  	if (vi->data.GetWeight() != data[i].GetWeight()) {
          cout<<"ERROR while testing AddVertex(VERTEX&)/IsVertex (2)"<<endl;
   	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 1.2 AddVertex(2): AddVertex(vector<VERTEX>&) */
    MethodNames.push_back("AddVertex(2) - AddVertex(vector<VERTEX>&)" );
#ifdef VERBOSE    
    stapl_print("Testing AddVertex(vector<VERTEX>&) for ");
    stapl_print(v_num); stapl_print(" vertices ...");
#endif
    if_want_perf {
      sg.EraseGraph();
      starttimer;
      tmp = sg.AddVertex(data);
      newstoptimer;
    } 
    if_want_corr {
      sg.EraseGraph();
      tmp = sg.AddVertex(data);
      for (int i=0; i<v_num; i++) {
        if (!sg.IsVertex(i,&vi) ) {
          cout<<"ERROR while testing AddVertex(vector<VERTEX>&)/IsVertex (1)"<<endl;
   	  break;
	}
  	if (vi->data.GetWeight() != data[i].GetWeight()) {
          cout<<"ERROR while testing AddVertex(vector<VERTEX>&)/IsVertex (2)"<<endl;
   	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 1.3 AddVertex(3): AddVertex(VERTEX&, VID) */
    MethodNames.push_back("AddVertex(3) - AddVertex(VERTEX&, VID)" );
#ifdef VERBOSE    
    stapl_print("Testing AddVertex(VERTEX&,VID) for ");
    stapl_print(v_num); stapl_print(" vertices ...");
#endif
    //main testing part...
    if_want_perf {
      sg=newsg;
      starttimer;
      for (int i=0; i<v_num; i++) {
        tmp = sg.AddVertex(data[i],i);
      }
      newstoptimer;
    } 
    if_want_corr {
      sg=newsg;
      for (int i=0; i<v_num; i++)  tmp = sg.AddVertex(data[i],i);
      for (int i=0; i<v_num; i++) {
        if (!sg.IsVertex(i,&vi) ) {
          cout<<"ERROR while testing AddVertex(VERTEX&,VID)/IsVertex (1)"<<endl;
   	  break;
	}
  	if (vi->data.GetWeight() != data[i].GetWeight()) {
          cout<<"ERROR while testing AddVertex(VERTEX&,VID)/IsVertex (2)"<<endl;
   	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
    stapl_print("\nTester_AddVertex tested.\n");
#endif
  }
};

/* (2) Tester_DeleteVertex : Tester for pGraph's method DeleteVertex(VID) and pDeleteVertices(vector<VID>&);
 * must call AddVertex first
 * @v_num number of vertices to be deleted from graph on each thread
 */
template <class GRAPH>
class Tester_DeleteVertex : public GraphTester<GRAPH> {

public:
  Tester_DeleteVertex() {flag='b'; }

  char* GetName() { return "Tester_DeleteVertex"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_DeleteVertex *t = new Tester_DeleteVertex; t->flag=this->flag;  return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    GRAPH newsg=sg;

    vector<VID> myvids;
    vector<Task> verts;
    for(typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      myvids.push_back(vi->vid);
      verts.push_back(vi->data);
    }

    counter = 0;
    /* 2.1 DeleteVertex(1): DeleteVertex(VID) */
    MethodNames.push_back("DeleteVertex(1) -  DeleteVertex(VID)");
    VID vid;
#ifdef VERBOSE    
    stapl_print("Testing DeleteVertex(VID) for...");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      sg=newsg;
      starttimer;
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
        sg.DeleteVertex(*it);
      }
//cout<<"seq: Deleted "<<myvids.size()<<" vertices"<<endl;
      newstoptimer;
    }
    if_want_corr {
      sg=newsg;
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
          vid = *it;
          sg.DeleteVertex(vid);
      }
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
	vid=*it;
        if (sg.IsVertex(vid)) {
          cout<<"ERROR while testing DeleteVertex(VID) "<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 2.2 DeleteVertex(2): DeleteVertex(VERTEX&) */
    MethodNames.push_back("DeleteVertex(2) -  DeleteVertex(VERTEX&)");
#ifdef VERBOSE    
    stapl_print("Testing DeleteVertex(VERTEX&) for...");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      sg=newsg;
      starttimer;
      for (vector<Task>::iterator it=verts.begin(); it!=verts.end(); ++it) {
         Task datum = *it;
        sg.DeleteVertex(datum);
      }
      newstoptimer;
    }
    if_want_corr {
      sg=newsg;
      for (vector<Task>::iterator it=verts.begin(); it!=verts.end(); ++it) {
         Task datum = *it;
        sg.DeleteVertex(datum);
      }
      for (vector<Task>::iterator it=verts.begin(); it!=verts.end(); ++it) {
        Task datum = *it;
        if (sg.IsVertex(datum)) {
          cout<<"ERROR while testing DeleteVertex(VERTEX&) - "<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
    stapl_print("\nTester_DeleteVertex tested.\n");
#endif
  }
};

/* (3) Tester_IsVertex : Tester for pGraph's method IsVertex 
 * must call AddVertex first
 */
template <class GRAPH>
class Tester_IsVertex : public GraphTester<GRAPH> {

public:
  Tester_IsVertex() {flag='b';}

  char* GetName() { return "Tester_IsVertex"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_IsVertex *t = new Tester_IsVertex; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  
  
  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;

    counter = 0;
    /* 3.1 IsVertex(1): IsVertex(VID) */
    MethodNames.push_back("IsVertex(1) - IsVertex(VID)" );
#ifdef VERBOSE    
    stapl_print("Testing IsVertex(VID) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<sg.size(); i++)  {
        tmp = sg.IsVertex(i);
      }
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
//#ifdef VERBOSE    
    stapl_print("Done IsVertex(1).\n"); 
//#endif

    counter++;
    /* 3.2 IsVertex(2): IsVertex(VID, VI*) */
    MethodNames.push_back("IsVertex(2): IsVertex(VID, VI*)");
    typename GRAPH::VI vi;
#ifdef VERBOSE    
    stapl_print("Testing IsVertex(VID,VI*) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<sg.size(); i++)  {
        tmp = sg.IsVertex(i,&vi);
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<sg.size(); i++) {
        tmp = sg.IsVertex(i,&vi);
        if (vi->vid!=i) {
          cout<<"ERROR while testing IsVertex(VID&,VI*)"<<endl;
	  break;
	}
      }
    }
//#ifdef VERBOSE    
    stapl_print("Done IsVertex(2).\n"); 
//#endif

    counter++;

    Task datum; vector<Task> data;
    for (int i=0; i<sg.size(); i++) {
      datum.SetWeight(i);
      data.push_back(datum);
    }
    /* 3.3 IsVertex(3): IsVertex(VERTEX&,VI*) */
    MethodNames.push_back("IsVertex(3) - IsVertex(VERTEX&,VI)" );
#ifdef VERBOSE    
    stapl_print("Testing IsVertex(VERTEX&,VI*) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<sg.size(); i++)  {
        tmp = sg.IsVertex(data[i], &vi);
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<sg.size(); i++) {
	if (!sg.IsVertex(data[i],&vi))  {
          cout<<"ERROR while testing IsVertex(VERTEX&,VI*)"<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done IsVertex(3)\n");
#endif 

    counter++;
    /* 3.4 IsVertex(4): IsVertex(VERTEX&) */
    MethodNames.push_back("IsVertex(3): IsVertex(VERTEX&)");
#ifdef VERBOSE    
    stapl_print("Testing IsVertex(VERTEX&) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<sg.size(); i++)  {
        tmp = sg.IsVertex(data[i]);
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<sg.size(); i++) {
	if (!sg.IsVertex(data[i])) {
          cout<<"ERROR while testing IsVertex(VERTEX&)"<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done IsVertex(4)\n"); 
    //finish
    stapl_print("\nTester_IsVertex tested.\n");
#endif

  }
};

/* (4) Tester_GetVertices : Tester for Graph's methods GetVerticesVID
 * these methods depend on AddVertex - assume user will guarantee that
 */
template <class GRAPH>
class Tester_GetVertices : public GraphTester<GRAPH> {

public:

  Tester_GetVertices() {flag='b'; }

  char* GetName() { return "Tester_GetVertices";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_GetVertices *t = new Tester_GetVertices; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  
  
  void SetTestNo() {
    GraphTester<GRAPH>::SetTestNo();
  }

  void Test(GRAPH& sg) {
    int tmp;
    vector<VID> verts;

    counter = 0;
    MethodNames.push_back("GetVerticesVID(vector<VID>&)");
#ifdef VERBOSE    
    stapl_print("Testing GetVerticesVID(vector<VID>&) for a graph with ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      verts.clear();
      starttimer;
      tmp = sg.GetVerticesVID(verts);
      newstoptimer;
    }
    if_want_corr {
      verts.clear();
      tmp = sg.GetVerticesVID(verts);
      for (int i=0; i<tmp; i++) {
	if (!sg.IsVertex(verts[i])) {
	  cout<<"ERROR while testing GetVerticesVID "<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
    stapl_print("\nTester_GetVertices passed testing.\n");
#endif
  }

};

/* (5) Tester_AddEdge : Tester for pGraph's method AddEdge(VID, VID, Weight2&);
 * this method depends on addVertex - only when graph has vertices, AddEdge can be called - We assume user will take this into account
 * @deg the number of edges of each vertex in the graph
 * each vertex having index(0~v_num) as its weight. Then add deg edges for each vertex, each edge having its 2 vertices' weights
 * multiplied as the edge's weight.
 *
 */
template <class GRAPH>
class Tester_AddEdge : public GraphTester<GRAPH> {

  int deg;

public:

  Tester_AddEdge() {flag='b'; deg=1;}
  Tester_AddEdge(int _deg) {flag='b'; deg=_deg;}

  char* GetName() { return "Tester_AddEdge"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_AddEdge *t = new Tester_AddEdge; t->flag=this->flag; t->deg=this->deg; return t; }

  void ParseParams(char* s) {
	//the incoming string is such as: "n 20": neither performance nor correctness  & add 20 edges
	//OR : "p 20": report performance & work on current graph & add 20 edges
    char *space = " "; char *p;

    if(!(p=strtok(s,space))) { 
      ShowUsage(); 
      exit(1); 
    }
    if (strcmp(p, "-quiet")==0) {
      quiet=true; 
      flag = strtok(NULL,space)[0];
    } else {
      quiet = false;
      flag = p[0];
    }
    if (!(flag=='p'||flag=='c'||flag=='b'))  { 
      ShowUsage(); 
      exit(1); 
    }
    if (p=strtok(NULL,space)) deg=atoi(p);
    return;
  }  

  void SetTestNo() {
    testno = 1; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  } 

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    GRAPH newsg = sg;

    typename GRAPH::VI vi;
    Weight2 wt,wt1,wt2; vector<Weight2> wts; vector<pair<Weight2,Weight2> > wtpairs;
    int SZ=sg.size();
    int maxj = SZ-1; int minj = SZ-deg;
    for (vi=sg.begin(); vi!=sg.end(); ++vi) {
      vi->data.SetWeight(vi->vid);
      for (int j=maxj; j>=minj; j--) {
        wt.SetWeight(vi->data.GetWeight()*j);
        wts.push_back(wt);
	wt1.SetWeight(vi->data.GetWeight()*j);
	wt2.SetWeight(vi->data.GetWeight()*j);
        pair<Weight2,Weight2> tmppair;
	tmppair.first=wt1; tmppair.second=wt2;
	wtpairs.push_back(tmppair);
      }
    }
    int i=0; 

    vector<_StaplTriple<VID,VID,Weight2> > edgesvec;
    GenEdge(edgesvec, sg, deg);

    counter = 0;
    /* 5.1 AddEdge(1): AddEdge(VID,VID,WEIGHT) */
    MethodNames.push_back("AddEdge(1) - AddEdge(VID,VID,WEIGHT)" );
#ifdef VERBOSE    
    stapl_print("Testing AddEdge(VID,VID,WEIGHT) for ");
    stapl_print(deg); stapl_print(" edges/vertex for a graph with ");
    stapl_print(newsg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      sg=newsg;
      starttimer;
      for(int i=0; i<edgesvec.size(); i++) {
	tmp = sg.AddEdge(edgesvec[i].first, edgesvec[i].second, edgesvec[i].third);
      }
/*
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          tmp = sg.AddEdge(vi->vid,j,wts[i]);
          i++;
        }
      }
*/
      newstoptimer;
    }
    if_want_corr {
      //i=0;	
      sg=newsg;
      for(int i=0; i<edgesvec.size(); i++) {
	tmp = sg.AddEdge(edgesvec[i].first, edgesvec[i].second, edgesvec[i].third);
      }
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
       if (vi->vid < SZ-5) {
        for (int j=vi->vid+deg;  j>vi->vid; j--) {
          if (!sg.IsEdge(vi->vid, j) ) {
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
	}
      }     
/*
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          tmp = sg.AddEdge(vi->vid,j,wts[i]);
          i++;
        }
      }
*/
/*
      if (sg.IsDirected()) {
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          if (!sg.IsEdge(vi->vid, j) ) {
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
      }     
     } else {
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          if (!(sg.IsEdge(vi->vid, j)) || !(sg.IsEdge(j,vi->vid)) ){
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
      }
     }
*/
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 5.2 AddEdge(2): AddEdge(VID,VID, pair<WEIGHT,WEIGHT>) 
    MethodNames.push_back("AddEdge(2) - AddEdge(VID,VID,pair<WEIGHT,WEIGHT>&)" );
#ifdef VERBOSE    
    stapl_print("Testing AddEdge(VID,VID,pair<WEIGHT,WEIGHT>&) for ");
    stapl_print(deg); stapl_print(" edges/vertex for a graph with ");
    stapl_print(newsg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      i=0;
      sg=newsg;
      starttimer;
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          tmp = sg.AddEdge(vi->vid,j,wtpairs[i]);
          i++;
        }
      }
      newstoptimer;
    }
    if_want_corr {
      i=0;
      sg=newsg;
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          tmp = sg.AddEdge(vi->vid,j,wtpairs[i]);
          i++;
        }
      }
      if (sg.IsDirected()) {
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          if (!sg.IsEdge(vi->vid, j) ) {
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
      }     
     } else {
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (int j=maxj; j>=minj; j--) {
          if (!(sg.IsEdge(vi->vid, j)) || !(sg.IsEdge(j,vi->vid)) ){
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
      }
     }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
    stapl_print("\nTester_AddEdge tested.\n"); 
#endif

    * AddEdge(VERTEX&,VERTEX&, WEIGHT) is not tested b/c no corresponding method in pGraph
     * AddEdge(VERTEX&,VERTEX&, pair<WEIGHT,WEIGHT>) is not tested b/c no corresponding method in pGraph 
     */
  }

#ifdef VERBOSE    
    stapl_print("\nTester_AddEdge tested.\n"); 
#endif
};


/* (6) Tester_DeleteEdge : Tester for Graph's method DeleteEdge
 * must call AddEdge first
 */
template <class GRAPH>
class Tester_DeleteEdge : public GraphTester<GRAPH> {

public:
  Tester_DeleteEdge() {flag='b';}

  char* GetName() { return "Tester_DeleteEdge"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_DeleteEdge *t = new Tester_DeleteEdge; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    GraphTester<GRAPH>::SetTestNo();
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;
    int e_num = sg.GetEdgeCount();
   
    GRAPH newsg=sg;

    pair<VID,VID> vidp;
    vector<pair<VID,VID> > todos;

    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      for (typename GRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        todos.push_back(pair<VID,VID>(vi->vid,ei->vertex2id));
      }
    }

    counter = 0; 
    MethodNames.push_back("DeleteEdge(VID,VID)");
#ifdef VERBOSE    
    stapl_print("Testing DeleteEdge(VID,VID) for ");
    stapl_print(e_num); stapl_print(" edges in all...");
#endif
    if_want_perf {
      sg=newsg;
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
     //cout<<"#"<<stapl::get_thread_id()<<" trying to delete edge ("<<todos[i].first<<","<<todos[i].second<<")"<<endl;
        sg.DeleteEdge(todos[i].first, todos[i].second);
      }
      newstoptimer;
    }
    if_want_corr {
      sg=newsg;
      for (int i=0; i<todos.size(); i++)  {
        sg.DeleteEdge(todos[i].first, todos[i].second);
      }
      for (vector<pair<VID,VID> >::iterator it=todos.begin(); it!=todos.end(); ++it) {
        if (sg.IsEdge(it->first, it->second) ) {
          cout<<"ERROR while testing DeleteEdge/IsEdge"<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
    //finish
     stapl_print("\nTester_DeleteEdge tested.\n"); 
#endif
  }
};

/* (7) Tester_IsEdge : test pGraph's method IsEdge
 * must call AddEdge first
 */

template <class GRAPH>
class Tester_IsEdge : public GraphTester<GRAPH> {

public:
  Tester_IsEdge() {flag='b'; }

  char* GetName() { return "Tester_IsEdge"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_IsEdge *t = new Tester_IsEdge; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    typename GRAPH::VI vi, vi2;
    typename GRAPH::EI ei, ei2;

    counter = 0;
    /* 7.1  IsEdge(1): IsEdge(VID,VID) */
    MethodNames.push_back("IsEdge(1) - IsEdge(VID,VID)" );
#ifdef VERBOSE    
    stapl_print("Testing IsEdge(VID,VID) for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges in all..."); 
#endif
    if_want_perf {
      starttimer;
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
          tmp = sg.IsEdge(vi->vid, ei->vertex2id);
        }     
      }
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE    
      stapl_print("NO_CORRECT_CHECK\n");
#endif
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 7.2 IsEdge(2): IsEdge(VI,EI) */
    MethodNames.push_back("IsEdge(2) - IsEdge(VI,EI)" );
#ifdef VERBOSE    
    stapl_print("Testing IsEdge(vi,ei) for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges in all..."); 
#endif
    if_want_perf {
      starttimer;
      for (vi=sg.begin(); vi!=sg.end(); ++vi) {
        for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
          tmp = sg.IsEdge(vi->vid, ei->vertex2id, &vi2, &ei2);
        }     
      }
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK\n");
#endif
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_IsEdge tested.\n"); 
#endif
  }

};


/* (8) Tester_GetEdges : Tester for Graph's methods GetEdges and GetEdgesVData
 * depends on AddEdge 
 */
template <class GRAPH>
class Tester_GetEdges : public GraphTester<GRAPH> {

public:

  Tester_GetEdges() {flag='b';}

  char* GetName() { return "Tester_GetEdges";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_GetEdges *t = new Tester_GetEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    counter = 0;
    /* 8.1 GetEdges(1): GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >&) */
   MethodNames.push_back("GetEdges(3) - GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >&)" );
    vector<pair<pair<VID,VID>,Weight2> > edges_w;
#ifdef VERBOSE    
    stapl_print("Testing GetEdges_w for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = sg.GetEdges(edges_w);
      newstoptimer;
    }
    if_want_corr {
      tmp = sg.GetEdges(edges_w);
      if (tmp != sg.GetEdgeCount()) {
	cout<<"tmp="<<tmp<<" sg.GetCount="<<sg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges(1)"<<endl;
      }
      for (int i=0; i<edges_w.size(); i++) {
        if(!sg.IsEdge(edges_w[i].first.first, edges_w[i].first.second)) {
          cout<<"ERROR while testing GetEdges_w(2): "<<endl;
	  break;
	}
        if(edges_w[i].second.GetWeight() != edges_w[i].first.first * edges_w[i].first.second) {
          cout<<"ERROR while testing GetEdges_w(3): "<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 8.2 GetEdges(2): GetEdgs(vector<pair<pair<pair<Task,Task>,WEIGHT> >&) */
    vector<pair<Task,Task> > edges_data;
    MethodNames.push_back("GetEdges(2) - GetEdges(vector<pair<pair<Task,Task>, WEIGHT> >&)" );
    vector<pair<pair<Task,Task>,Weight2> > edges_vw;
#ifdef VERBOSE    
    stapl_print("Testing GetEdges_vw for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = sg.GetEdgesVData(edges_vw);
      newstoptimer;
    }
    if_want_corr {
      tmp = sg.GetEdgesVData(edges_vw);
      if (tmp != sg.GetEdgeCount()) {
	cout<<"tmp="<<tmp<<" sg.GetCount="<<sg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges(1)"<<endl;
      }
      for (int i=0; i<edges_data.size(); i++) {
        if(edges_data[i].first.GetWeight() != edges_vw[i].first.first.GetWeight()) {
          cout<<"ERROR while testing GetEdges_vw(2): "<<endl;
	  break;
	}
        if(edges_data[i].second.GetWeight() != edges_vw[i].first.second.GetWeight()) {
          cout<<"ERROR while testing GetEdges_vw(3): "<<endl;
	  break;
	}
        if(edges_vw[i].second.GetWeight() != edges_w[i].first.first * edges_w[i].first.second) {
          cout<<"ERROR while testing GetEdges_vw(4): "<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 8.3 GetEdges(3): GetEdges(VIDs) */
    MethodNames.push_back("GetEdges(3) - GetEdges(vector<pair<VID,VID> >)" );
    vector<pair<VID,VID> > edges;
#ifdef VERBOSE    
    stapl_print("Testing GetEdges(VIDs) for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges...");
#endif
    if_want_perf {
      starttimer;
      tmp = sg.GetEdges(edges);
      // cout<<"#"<<stapl::get_thread_id()<<" tmp="<<tmp<<endl;
      newstoptimer;
    }
    if_want_corr {
      tmp = sg.GetEdges(edges);
      if (tmp != sg.GetEdgeCount()) {
	cout<<"tmp="<<tmp<<" sg.GetCount="<<sg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges(1)"<<endl;
      }
      for (int i=0;i<edges.size();i++){
        if(!(sg.IsEdge(edges[i].first, edges[i].second))) {
          cout<<"ERROR while testing GetEdges(2): "<<edges[i].first<<":"<<edges[i].second<<endl;
	  return;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 8.4 GetEdges(4): GetEdges(vector<pair<Task,Task> >&) */
    MethodNames.push_back("GetEdges(4) - GetEdges(vector<pair<Task,Task> >&)" );
#ifdef VERBOSE    
    stapl_print("Testing GetEdges<Task,Task> for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = sg.GetEdgesVData(edges_data);
      newstoptimer;
    }
    if_want_corr {
      tmp = sg.GetEdgesVData(edges_data);
      if (tmp != sg.GetEdgeCount()) {
	cout<<"tmp="<<tmp<<" sg.GetCount="<<sg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges(1)"<<endl;
      }
      for (int i=0; i<edges_data.size(); i++) {
	//cout<<"i="<<i<<" edges[i]=("<<edges[i].first<<","<<edges[i].second<<") edges_data[i]="<<edges_data[i].first.GetWeight()<<","<<edges_data[i].second.GetWeight()<<")"<<endl;
        if(edges[i].first !=  edges_data[i].first.GetWeight()) {
          cout<<"ERROR while testing GetEdges<Task,Task>(2): "<<edges[i].first<<":"<<edges_data[i].first.GetWeight()<<endl;
	  break;
	}
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_GetEdges tested.\n"); 
#endif
  }
};

/* (9) Tester_SetGetVertexField : Tester corrsponding to pGraph's pSetVertexField and pGetVertexField. 
 */
template <class GRAPH>
class Tester_SetGetVertexField : public GraphTester<GRAPH> {

public:
  Tester_SetGetVertexField() {flag='b'; }

  char* GetName() { return "Tester_SetGetVertexField";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_SetGetVertexField *t = new Tester_SetGetVertexField; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    counter=0;
    /* 9.1 pSetVertexField(1 arg) */
    MethodNames.push_back("SetGetVertexField(1): SetVertexField(1 arg)");
#ifdef VERBOSE
    stapl_print("Testing SetVertexField(1 arg)	for ");
    stapl_print(sg.size()); stapl_print(" vertices...");
#endif
    vector<VID> todos;
    int tmp1;
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      todos.push_back(vi->vid);
    }
    if_want_perf {
      starttimer;
      for (int i=0; i<todos.size(); i++) {
	tmp1 = todos[i]+11;
	typename GRAPH::VI vi; Task* pvt;
	if (sg.IsVertex(todos[i], &vi) ) {
	  pvt = &(vi->data);
	  pvt->SetWeight1(tmp1);
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<todos.size(); i++) {
	tmp1 = todos[i]+11;
	typename GRAPH::VI vi; Task* pvt;
	if (sg.IsVertex(todos[i], &vi) ) {
	  pvt = &(vi->data);
	  pvt->SetWeight1(tmp1);
	  double dd = pvt->GetWeight1();
	  if ((int)dd != tmp1) {
	    cout<<"ERROR while testing SetVertexField(1 arg)/GetVertexField() "<<endl;
	    break;
	  }
	}
      }
    }
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 9.2 pGetVertexField() */
    MethodNames.push_back("SetGetVertexField(2): GetVertexField()");
#ifdef VERBOSE
    stapl_print("Testing GetVertexField()	for ");
    stapl_print(sg.size()); stapl_print(" vertices...");
#endif
    double dd;
    if_want_perf {
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        typename GRAPH::VI vi; Task* pvt;
        if (sg.IsVertex(todos[i], &vi) ) {
	  pvt = &(vi->data);
	  dd = pvt->GetWeight(); 
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<todos.size(); i++) {
        typename GRAPH::VI vi; Task* pvt;
        if (sg.IsVertex(todos[i], &vi) ) {
	  pvt = &(vi->data);
	  dd = pvt->GetWeight(); 
	  if ((int)dd != todos[i]+11) {
	    cout<<"ERROR while testing SetVertexField(1 arg)/GetVertexField() "<<endl;
	    break;
	  }
	}
      }
    }
#ifdef VERBOSE
  stapl_print("Done.\n");
  //finish
  stapl_print("\nTester_SetGetVertexField tested.\n");
#endif
  }
};

/* (10) Tester_SetGetWeightField : Tester corrsponding to pGraph's pSetWeightField and pGetWeightField. 
 */
template <class GRAPH>
class Tester_SetGetWeightField : public GraphTester<GRAPH> {

public:
  Tester_SetGetWeightField() {flag='b'; }

  char* GetName() { return "Tester_SetGetWeightField";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_SetGetWeightField *t = new Tester_SetGetWeightField; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    counter=0;
    /* 10.1 pSetWeightField(1 arg) */
    MethodNames.push_back("SetGetWeightField(1): SetWeightField(1 arg)");
    vector<pair<VID,VID> > edges;
#ifdef VERBOSE
    stapl_print("Testing SetWeightField(1 arg)	for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges...");
#endif
    int tmp1;
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      for (typename GRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        edges.push_back(pair<VID,VID>(vi->vid,ei->vertex2id) );
      }
    }
    if_want_perf {
      starttimer;
      for (int i=0; i<edges.size(); i++) {
	tmp1 = edges[i].first+22;
	typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
	if (sg.IsEdge(edges[i].first, edges[i].second, &vi, &ei) ) {
	  pwt = &(ei->weight);
	  pwt->SetWeight1(tmp1);
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<edges.size(); i++) {
	tmp1 = edges[i].first+22;
	typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
	if (sg.IsEdge(edges[i].first, edges[i].second, &vi, &ei) ) {
	  pwt = &(ei->weight);
	  pwt->SetWeight1(tmp1);
          double dd = pwt->GetWeight();
	  if ((int)dd != tmp1) {
	    cout<<"ERROR while testing SetWeightField(1 arg)/GetWeightField() dd="<<dd<<" tmp1="<<tmp1<<endl;
	    break;
	  }
	}
      }
    }
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 10.2 pGetWeightField() */
    MethodNames.push_back("SetGetWeightField(2): GetWeightField()");
#ifdef VERBOSE
    stapl_print("Testing GetWeightField()	for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<edges.size(); i++) {
	tmp1 = edges[i].first+22;
        typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
        if (sg.IsEdge(edges[i].first, edges[i].second, &vi, &ei) ) {
	  pwt = &(ei->weight);
	  double dd = pwt->GetWeight(); 
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<edges.size(); i++) {
        typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
        if (sg.IsEdge(edges[i].first, edges[i].second, &vi, &ei) ) {
	  pwt = &(ei->weight);
	  double dd = pwt->GetWeight(); 
	  if ((int)dd != edges[i].first+22) {
	    cout<<"ERROR while testing SetWeightField(1 arg)/GetWeightField() dd="<<dd<<" edges[i].first+22="<<edges[i].first+22<<endl;
	    break;
	  }
	}
      }
    }
#ifdef VERBOSE
  stapl_print("Done.\n");
  //finish
  stapl_print("\nTester_SetGetVertexField tested.\n");
#endif
  }
};

/* (11) Tester_SetGetWeightFieldEdgeId : Tester corrsponding to pGraph's pSetWeightFieldEdgeId and pGetWeightFieldEdgeId. 
 */
template <class GRAPH>
class Tester_SetGetWeightFieldEdgeId : public GraphTester<GRAPH> {

public:
  Tester_SetGetWeightFieldEdgeId() {flag='b'; }

  char* GetName() { return "Tester_SetGetWeightFieldEdgeId";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_SetGetWeightFieldEdgeId *t = new Tester_SetGetWeightFieldEdgeId; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    counter=0;
    /* 11.1 pSetWeightFieldEdgeId(1 arg) */
    MethodNames.push_back("SetGetWeightFieldEdgeId(1): SetWeightFieldEdgeId(1 arg)");
    vector<pair<VID,int> > edgeids;
#ifdef VERBOSE
    stapl_print("Testing SetWeightField(1 arg)	for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges...");
#endif
    int tmp1;
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      for (typename GRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        edgeids.push_back(pair<VID,int>(vi->vid,ei->edgeid) );
      }
    }
    if_want_perf {
      starttimer;
      for (int i=0; i<edgeids.size(); i++) {
	tmp1 = edgeids[i].first+33;
	typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
	if (sg.IsVertex(edgeids[i].first, &vi) ) {
	  ei = vi->find_edgeid_eq(edgeids[i].second);
	  if (ei != vi->edgelist.end()) {
	    pwt = &(ei->weight);
	    pwt->SetWeight1(tmp1);
	  }
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<edgeids.size(); i++) {
	tmp1 = edgeids[i].first+33;
	typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
	if (sg.IsVertex(edgeids[i].first, &vi) ) {
	  ei = vi->find_edgeid_eq(edgeids[i].second);
	  if (ei != vi->edgelist.end()) {
	    pwt = &(ei->weight);
	    pwt->SetWeight1(tmp1);
            double dd = pwt->GetWeight();
	    if ((int)dd != tmp1) {
  	      cout<<"ERROR while testing SetWeightField(1 arg)/GetWeightField() dd="<<dd<<" tmp1="<<tmp1<<endl;
	      break;
	    }
	  }
	}
      }
    }
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 11.2 pGetWeightFieldEdgeId() */
    MethodNames.push_back("SetGetWeightFieldEdgeId(2): GetWeightFieldEdgeId()");
#ifdef VERBOSE
    stapl_print("Testing GetWeightFieldEdgeId()	for ");
    stapl_print(sg.GetEdgeCount()); stapl_print(" edges...");
#endif
    double dd;
    if_want_perf {
      starttimer;
      for (int i=0; i<edgeids.size(); i++) {
	tmp1 = edgeids[i].first+33;
	typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
        if (sg.IsVertex(edgeids[i].first, &vi) ) {
	  ei = vi->find_edgeid_eq(edgeids[i].second);
	  if (ei != vi->edgelist.end()) {
	    pwt = &(ei->weight);
	    dd = pwt->GetWeight(); 
	  }
	}
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<edgeids.size(); i++) {
        typename GRAPH::VI vi; typename GRAPH::EI ei; Weight2* pwt;
        if (sg.IsVertex(edgeids[i].first, &vi) ) {
	  ei = vi->find_edgeid_eq(edgeids[i].second);
	  if (ei != vi->edgelist.end()) {
	    pwt = &(ei->weight);
	    dd = pwt->GetWeight(); 
	    if ((int)dd != edgeids[i].first+33) {
	      cout<<"ERROR while testing SetWeightField(1 arg)/GetWeightField() dd="<<dd<<" edgeids[i].first+33="<<edgeids[i].first+33<<endl;
	     // cout<<"ERROR while testing SetWeightFieldEdgeId(1 arg)/GetWeightFieldEdgeId() "<<endl;
	      break;
	    }
	  }
	}
      }
    }
#ifdef VERBOSE
  stapl_print("Done.\n");
  //finish
  stapl_print("\nTester_SetGetWeightFieldEdgeId tested.\n");
#endif
  }
};

/* (x) Tester_GetAdjacentVertices : Tester for Graph's method GetAdjacentVertices/DATA
 * must call AddVertex and AddEdge first
 */
template <class GRAPH>
class Tester_GetAdjacentVertices : public GraphTester<GRAPH> {

public:
  Tester_GetAdjacentVertices() {flag='b'; }

  char* GetName() { return "Tester_GetAdjacentVertices"; }
  
  GraphTester<GRAPH> *CreateCopy() { Tester_GetAdjacentVertices *t = new Tester_GetAdjacentVertices; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 3;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

    vector<VID> adj; vector<Task> adjdata;
    typename GRAPH::VI vi;
    vector<VID> todos;
    vector<Task> todovertices;
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      todos.push_back(vi->vid);
      todovertices.push_back(vi->data);
    }

    counter = 0;
    /* 9.1 GetAdjacentVertices(1): GetAdjacentVertices(VID,vector<VID>&) */ 
    MethodNames.push_back("GetAdjacentVertices(1): GetAdjacentVertices(VID,vector<VID>&...)");
#ifdef VERBOSE    
    stapl_print("Testing GetAdjacentVertices(VID,vector<VID>&...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adj.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        sg.GetAdjacentVertices(todos[i], adj);
      }
      newstoptimer;
    }
    if_want_corr {
      adj.clear();
      for (int i=0; i<todos.size(); i++)  {
        sg.GetAdjacentVertices(todos[i], adj);
        if (!sg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/IsVertex(VID) (1): wrong input vids"<<endl;
	  break;
	}
  	if (adj.size() != sg.GetVertexOutDegree(todos[i]) ) {
          cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/GetVertexOutDegree(VID) (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adj.size(); j++) {
 	  if (!sg.IsEdge(todos[i], adj[j]) )  {
            cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/IsEdge(VID,VID) (3): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 9.2 GetAdjacentVertices(2): GetAdjacentVerticesDATA(VID,vector<VERTEX>& ) */ 
    MethodNames.push_back("GetAdjacentVertices(2): GetAdjacentVerticesDATA(VID,vector<VERTEX&>...)");
#ifdef VERBOSE    
    stapl_print("Testing GetAdjacentVerticesDATA(VID, vector<VERTEX&>...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adjdata.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        sg.GetAdjacentVerticesDATA(todos[i], adjdata);
      }
      newstoptimer;
    }
    if_want_corr {
      adjdata.clear();
      for (int i=0; i<todos.size(); i++)  {
        sg.GetAdjacentVerticesDATA(todos[i], adjdata);
        if (!sg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/IsVertex(VID): wrong input vids"<<endl;
	  break;
	}
  	if (adjdata.size() != sg.GetVertexOutDegree(todos[i]) ) {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/GetVertexOutDegree(VID) (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adjdata.size(); j++) {
 	  if (!sg.IsEdge(todos[i], sg.GetVID(adjdata[j]) ) )  {
            cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/GetVID/IsEdge(VID,VID) (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 9.3 GetAdjacentVertices(3): GetAdjacentVertices(VERTEX&, vector<VID>& ) */ 
    MethodNames.push_back("GetAdjacentVertices(3): GetAdjacentVertices(VERTEX&,vector<VID>&...)");
#ifdef VERBOSE    
    stapl_print("Testing GetAdjacentVertices(VERTEX&, vector<VID>&...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adj.clear(); 
      starttimer;
      for (int i=0; i<todovertices.size(); i++)  {
        sg.GetAdjacentVertices(todovertices[i], adj);
      }
      newstoptimer;
    }
    if_want_corr {
      adj.clear(); 
      for (int i=0; i<todovertices.size(); i++)  {
	VID vid=sg.GetVID(todovertices[i]);
        sg.GetAdjacentVertices(vid, adj);
        if (!sg.IsVertex(vid)) {
          cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VID>&)/IsVertex: wrong input vids"<<endl;
	  break;
	}
  	if (adj.size() != sg.GetVertexOutDegree(vid) ) {
          cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VID>&)/GetVertexOutDegree (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adj.size(); j++) {
 	  if (!sg.IsEdge(vid, adj[j] ) )  {
            cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VERTEX>&)/GetVID/IsEdge (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

/* -only ug has, so comment
     9.4 GetAdjacentVertices(4): GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&) 
    if_not_none { 
      stapl_report("GetAdjacentVerticesDATA(4)");
      stapl_print("Testing GetAdjacentVerticesDATA(VERTEX&, vector<VERTEX>&...) for ");
      stapl_print(sg.size()); stapl_print(" vertices in all..");
    }
    if_want_perf {
      for(int index=0; index<REPEAT; index++) {
        adjdata.clear(); 
        starttimer;
        for (int i=0; i<todovertices.size(); i++)  {
          sg.GetAdjacentVerticesDATA(todovertices[i], adjdata);
        }
        stoptimer;
	prestat;
      }
      statistics;
    }
    if_want_corr {
      adjdata.clear(); 
      for (int i=0; i<todovertices.size(); i++)  {
        VID vid=sg.GetVID(todovertices[i]);
        sg.GetAdjacentVerticesDATA(vid, adjdata);
        if (!sg.IsVertex(vid) )  {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/IsVertex: wrong input vids"<<endl;
	  break;
	}
  	if (adjdata.size() != sg.GetVertexOutDegree(vid) ) {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/GetVertexOutDegree (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adjdata.size(); j++) {
 	  if (!sg.IsEdge(todovertices[i], adjdata[j] ) )  {
            cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/GetVID/IsEdge(VERTEX&,VERTEX&) (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    if_not_none { stapl_print("Done.\n"); stapl_report("Done.\n"); }
*/
#ifdef VERBOSE
    //finish
    stapl_print("\nTester_GetAdjacentVertices tested.\n");
#endif
  }
};

/*(10) Tester_GetPredecessors : Tester for DG's method GetPredecessors(DATA) */
template <class GRAPH>
class Tester_GetPredecessors : public GraphTester<GRAPH> {

public:
  Tester_GetPredecessors() {flag='b'; }

  char* GetName() { return "Tester_GetPredecessors";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_GetPredecessors *t = new Tester_GetPredecessors; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(GRAPH& sg) {
    int tmp;

    if (!sg.IsDirected()) { stapl_print("Only DG has method GetPredecessors(DATA)...return.\n"); return; }

	//used by all methods
    vector<VID> pred; vector<Task> preddata;
    //definition of local vars...
    vector<VID> todos; vector<typename GRAPH::VI> todovis; vector<Task> todovertices;
    //local computations...
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      todos.push_back(vi->vid);
      todovis.push_back(vi);
      todovertices.push_back(vi->data);
    }

    counter = 0;
    /* 10.1 GetPredecessors(1) - GetPredecessors(VID,vector<VID>&) */
   MethodNames.push_back("GetPredecessors(1): GetPredecessors(VID,vector<VID>&)");
#ifdef VERBOSE    
    stapl_print("Testing GetPredecessors(VID, vector<VID>&) for ");
    stapl_print(sg.size()); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {
      pred.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        sg.GetPredecessors(todos[i], pred);
      }
      newstoptimer;
    }
    //correctness check...
    if_want_corr {
      pred.clear();
      for (int i=0; i<todos.size(); i++) {
        sg.GetPredecessors(todos[i], pred);
        if (!sg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetPredecessors(VID,vector<VID>&)/IsVertex(VID) (1): wrong input vids"<<endl;
	  break;
	}
        for (int j=0; j<pred.size(); j++) {
 	  if (!sg.IsEdge(pred[j], todos[i]) )  {
            cout<<"ERROR while testing GetPredecessors(VID,vector<VID>&)/IsEdge(VID,VID) (2): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 10.2 GetPredecessors(2) - GetPredecessorsDATA(VID,vector<VERTEX>&)  */
    MethodNames.push_back("GetPredecessors(2): GetPredecessorsDATA(VID,vector<VERTEX>&)"); 
#ifdef VERBOSE    
    stapl_print("Testing GetPredecessorsDATA(VID, vector<VERTEX&>...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      preddata.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        sg.GetPredecessorsDATA(todos[i], preddata);
      }
      newstoptimer;
    }
    if_want_corr {
      preddata.clear();
      for (int i=0; i<todos.size(); i++) {
        sg.GetPredecessorsDATA(todos[i], preddata);
        if (!sg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetPredecessorsDATA(VID,vector<VERTEX>&)/IsVertex(VID): wrong input vids"<<endl;
	  break;
	}
        for (int j=0; j<preddata.size(); j++) {
 	  if (!sg.IsEdge(sg.GetVID(preddata[j]), todos[i] ) ) {
            cout<<"ERROR while testing GetPredecessorsDATA(VID,vector<VERTEX>&)/GetVID/IsEdge(VID,VID) (2): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 10.3 GetPredecessors(3) - GetPredecessors(VI,vector<VID>&)  */
    MethodNames.push_back("GetPredecessors(3): GetPredecessors(VI,vector<VID>&)");
#ifdef VERBOSE    
    stapl_print("Testing GetPredecessors(VI,vector<VID>&...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      pred.clear();
      starttimer;
      for (int i=0; i<todovis.size(); i++)  {
        sg.GetPredecessors(todovis[i], pred);
      }
      newstoptimer;
    }
    if_want_corr {
      pred.clear();
      for (int i=0; i<todovis.size(); i++) {
        sg.GetPredecessors(todovis[i], pred);
        if (!sg.IsVertex(todovis[i]->vid))  {
          cout<<"ERROR while testing GetPredecessors(VI,vector<VID>&)/IsVertex(VID) (1): wrong input vertices"<<endl;
	  break;
	}
        for (int j=0; j<pred.size(); j++) {
 	  if (!sg.IsEdge(todovis[i]->vid, pred[j]) )  {
            cout<<"ERROR while testing GetPredecessors(VI,vector<VID>&)/IsEdge(VID,VID) (2): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
#endif

    counter++;
    /* 10.4 - GetPredecessors(4) - GetPredecessorsDATA(VERTEX, vector<VERTEX>&)  */
#ifdef VERBOSE    
    stapl_print("Testing GetPredecessors(4) - GetPredecessorsDATA(VERTEX, vector<VERTEX&>...) for ");
    stapl_print(sg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      preddata.clear(); 
      starttimer;
      for (int i=0; i<todovertices.size(); i++)  {
        sg.GetPredecessorsDATA(todovertices[i], preddata);
      }
      newstoptimer;
    }
    if_want_corr {
      preddata.clear();
      for (int i=0; i<todovertices.size(); i++) {
        sg.GetPredecessorsDATA(todovertices[i], preddata);
        if (!sg.IsVertex(todovertices[i]))  {
          cout<<"ERROR while testing GetPredecessorsDATA(VERTEX,vector<VERTEX>&)/IsVertex(VERTEX&) (1): wrong input vis"<<endl;
	  break;
	}
        for (int j=0; j<preddata.size(); j++) {
 	  if (!sg.IsEdge(preddata[j], todovertices[i] ) )  {
            cout<<"ERROR while testing GetPredecessorsDATA(VERTEX,vector<VERTEX>&)/GetVID/IsEdge(VERTEX&,VERTEX&) (2): not edges"<<endl;
  	    break;
	  }
        } 
      }
    }
#ifdef VERBOSE    
    stapl_print("Done.\n");
    //finish
    stapl_print("Tester_GetPredecessors tested.\n");
#endif

  }
};

/* (11) Tester_GetOutgoingEdges : Tester for pGraph's methods GetOutgoingEdges/VData
 * must call AddVertex and AddEdge first
 * @v_num #of vertices to be worked on
template <class GRAPH>
class Tester_GetOutgoingEdges : public GraphTester<GRAPH> {

public:

  Tester_GetOutgoingEdges() {flag='b';}

  char* GetName() { return "Tester_GetOutgoingEdges";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_GetOutgoingEdges *t = new Tester_GetOutgoingEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void Test(GRAPH& sg) {

    int tmp;
    timer tv; double elapsed;
    double avgtime=0; double deviation=0; vector<double> elapses;

	//used by all methods
    vector<pair<VID,VID> > edges;
    vector<pair<pair<VID,VID>, Weight2> > edges_w;
    vector<pair<Task,Task> > edgedatas;
    vector<pair<pair<Task,Task>,Weight2> > edgedatas_w;
    //definition of local vars...
    vector<VID> todos;
    //local computations...
    for (typename GRAPH::VI vi=sg.begin(); vi!=sg.end(); ++vi) {
      todos.push_back(vi->vid);
    }

    if_not_none {
      stapl_print("Testing GetOutgoingEdges(VID,vector<pair<VID,VID> >&) for ");
      stapl_print(sg.size()); stapl_print(" vertices in all...");
    }
    //main testing part...
    if_want_perf {
      edges.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdges(todos[i],edges);
      }
      stoptimer;
    }
    //correctness check...
    if_want_corr {
      edges.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdges(todos[i],edges);
        if(tmp!=edges.size()) {
	  cout<<"ERROR while testing GetOutgoingEdges(1)"<<endl;
	  break;
	}
        if(tmp!=sg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdges/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
 	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!sg.IsEdge(edges[j].first, edges[j].second)) {
            cout<<"ERROR while testing GetOutgoingEdges(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }

    stapl::rmi_fence();
    if_not_none {
      stapl_print("OK\n");
      stapl_print("Testing GetOutgoingEdges(VID,vector<pair<pair<VID,VID>,WEIGHT> >&)  for ");
      stapl_print(sg.size()); stapl_print(" vertices in all...");
    }
    //main testing part...
    if_want_perf {
      edges_w.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdges(todos[i], edges_w);
      }
      stoptimer;
    }

    //correctness check...: must have same weights as other methods
    if_want_corr {
      edges_w.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdges(todos[i], edges_w);
        if(tmp==edges_w.size()) {
	  cout<<"ERROR while testing GetOutgoingEdges_w(1)"<<endl;
	  break;
	}
        if(tmp!=sg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdges_w/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!sg.IsEdge(edges_w[j].first.first, edges_w[j].first.second)) {
            cout<<"ERROR while testing GetOutgoingEdges(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }

    stapl::rmi_fence();
    if_not_none { 
      stapl_print("OK\n"); 
      stapl_print("Testing GetOutgoingEdgesVData(VID,vector<pair<VERTEX,VERTEX> >&) for ");
      stapl_print(sg.size()); stapl_print(" vertices in all...");
    }
    //main testing part...
    if_want_perf {
      edgedatas.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdgesVData(todos[i], edgedatas);
      }
      stoptimer;
    }
    //correctness check...
    if_want_corr {
      edgedatas.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdgesVData(todos[i], edgedatas);
        if(tmp!=edgedatas.size()) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData(1)"<<endl;
	  break;
	}
        if(tmp!=sg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!sg.IsEdge(edgedatas[j].first, edgedatas[j].second)) {
            cout<<"ERROR while testing GetOutgoingEdgesVData(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }

    stapl::rmi_fence();
    if_not_none { 
      stapl_print("OK\n"); 
      stapl_print("Testing GetOutgoingEdgesVData(VID,vector<pair<pair<VERTEX,VERTEX>,WEIGHT> >&) for ");
      stapl_print(sg.size()); stapl_print(" vertices in all...");
    }
    //main testing part...
    if_want_perf {
      edgedatas_w.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdgesVData(todos[i], edgedatas_w);
      }
      stoptimer;
    }
    //correctness check...: must have same weights as other methods
    if_want_corr {
      edgedatas_w.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = sg.GetOutgoingEdgesVData(todos[i], edgedatas_w);
        if(tmp!=edgedatas_w.size()) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData_w(1)"<<endl;
	  break;
	}
        if(tmp!=sg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData_w/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!sg.IsEdge(edgedatas_w[j].first.first, edgedatas_w[j].first.second)) {
            cout<<"ERROR while testing GetOutgoingEdgesVData(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }

    stapl::rmi_fence();
    if_not_none { stapl_print("OK\n"); }
    else {stapl_print("\nTester_GetOutgoingEdges  tested.\n");}
  }
};
 */


/* (12) Tester_SetPredecessors : Tester for pGraph's methods ClearPredecessors, pSetPredecessors and pAsyncSetPredecessors
 * must call AddVertex and AddEdge first  */
template <class GRAPH>
class Tester_SetPredecessors : public GraphTester<GRAPH> {

public:

  Tester_SetPredecessors() {flag='b';}

  char* GetName() { return "Tester_SetPredecessors";}
  
  GraphTester<GRAPH> *CreateCopy() { Tester_SetPredecessors *t = new Tester_SetPredecessors; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<GRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    GraphTester<GRAPH>::SetTestNo();
  }

  void Test(GRAPH& sg) {
    int tmp;

    counter = 0;
    MethodNames.push_back("pSetPredecessors()");
#ifdef VERBOSE
    stapl_print("Testing SetPredecessors for ");
    stapl_print(sg.size()); stapl_print(" vertices in all...");
#endif
    if(!sg.IsDirected()) {stapl_print("sg is not DG, no method of SetPredecessors, return.\n"); return; }
    starttimer;
    sg.SetPredecessors();
    newstoptimer;
    if_want_corr {
      for(typename GRAPH::VI vi = sg.begin(); vi != sg.end(); ++vi) {
        for(typename GRAPH::EI ei = vi->predecessors.begin(); ei != vi->predecessors.end(); ++ei) {
          if(!sg.IsEdge(ei->vertex2id,vi->vid)) {
            cout<<"ERROR while testing pSetPredecessors: edge added incorrect"<<endl; 
	    break;
	  }
        }
      }
    }
#ifdef VERBOSE
    stapl_print("Done.\n");
    stapl_print("\nTester_SetPredecessors tested.\n"); 
#endif
  }
};



