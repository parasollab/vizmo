/* Tester classes for basic methods of pGraph
 * inheritted from GraphTester class
 */
#include "commontester.h"
#include "pArray.h"

using namespace stapl;


/* (0) Tester_SimpleTest some simple methods of pgraph: size, local_size, empty, local_empty, 
 * GetEdgeCount, GetLocalEdgeCount, IsVertex, IsEdge, ErasePGraph 
 */
template <class PGRAPH>
class Tester_Simple : public GraphTester<PGRAPH> {

public:

  Tester_Simple() {flag='p';}

  char* GetName() { return "Tester_Simple"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_Simple *t = new Tester_Simple; t->flag=this->flag; return t; }

  void ShowUsage() {
    GraphTester<PGRAPH>::ShowUsage();
  }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 8; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
	//this tester is simple, there's no main difference between performance test and correctness test
	//so just write them together in Test()
    int NP=stapl::get_num_threads();
    int tmp;

    typename PGRAPH::VI vi;
    typename PGRAPH::EI ei;

    PGRAPH newpg = pg;
    stapl::rmi_fence();
    newpg.ErasePGraph();

    counter = 0;
    /* 0.1 local_empty */
    MethodNames.push_back("local_empty");
    bool emptyflag=false;
#ifdef VERBOSE 
    stapl_print("Testing local_empty... ");
#endif
    if_want_perf {	
      starttimer;
      emptyflag=newpg.local_empty();
      newstoptimer;
    }
    if_want_corr {
      if (!newpg.local_empty()) {
	cout<<"ERROR while testing ErasePGraph/local_empty"<<endl;
      }  
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n");
#endif

    counter++;
    /* 0.2 empty */
    MethodNames.push_back("empty");
#ifdef VERBOSE 
      stapl_print("Testing empty... ");
#endif
    if_want_perf {	
      starttimer;
      emptyflag=newpg.empty();
      newstoptimer;
    }
    if_want_corr {
      if (!(newpg.empty() )) {
	cout<<"ERROR while testing ErasePGraph/empty"<<endl;
      }  
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 0.3 local_size */
    MethodNames.push_back("local_size()");
    int localsize;
#ifdef VERBOSE 
    stapl_print("Testing local_size() for "); 
    stapl_print(pg.local_size()); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {	
      starttimer;
      localsize = pg.local_size();
      newstoptimer;
    }	
    if_want_corr { 
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 0.4 size */
    MethodNames.push_back("size()");
    int size;
#ifdef VERBOSE 
    stapl_print("Testing size() for "); 
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {	
      starttimer;
      size=pg.size();
      newstoptimer;
    }
    if_want_corr { 
      if(size%NP!=0) {
        cout<<"ERROR while testing size()"<<endl;
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n");
#endif

    counter++;
    /* 0.5 GetLocalEdgeCount */
    MethodNames.push_back("GetLocalEdgeCount()");
#ifdef VERBOSE 
    stapl_print("Testing GetLocalEdgeCount() for "); 
    stapl_print(pg.GetLocalEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      int elocalcount;
      starttimer;
      elocalcount=pg.GetLocalEdgeCount();
      newstoptimer;
    }
    if_want_corr { 
#ifdef  VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 0.6 GetEdgeCount */
    MethodNames.push_back("GetEdgeCount()");
    int ecount;
#ifdef VERBOSE 
    stapl_print("Testing GetEdgeCount() for "); 
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      ecount=pg.GetEdgeCount();
      newstoptimer;
    }
    if_want_corr { 
      if(ecount%NP!=0) {
	cout<<"ERROR while testing GetEdgeCount()"<<endl;
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 0.7 iterator */
    MethodNames.push_back("iterator");
#ifdef VERBOSE 
    stapl_print("Testing iterator (VI) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      vi=pg.local_begin();
      while (vi != pg.local_end() ) vi++; 
      newstoptimer;
    }
    if_want_corr { 
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 0.8 Part's iterator */
    MethodNames.push_back("Part's iterator");
    typename PGRAPH::pContainerPart_type* pp;
    typename PGRAPH::pContainerPart_type::iterator pvi;
#ifdef VERBOSE 
    stapl_print("Testing Part's iterator for ");
    stapl_print(pg.GetPartsCount()); stapl_print(" parts and ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int p=0; p<pg.GetPartsCount(); ++p) {
	pp=pg.GetPart(p);
        pvi=pp->begin();
        while (pvi != pp->end() ) pvi++; 
      }
      newstoptimer;
    }
    if_want_corr { 
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE 
    stapl_print("Done.\n"); 
    //finished 
    stapl_print("\nTester_Simple tested.\n");
#endif
  }

};

/* (1) Tester_AddVertexTester for pGraph's method AddVertex
 * this method can be called directly, not depending on any other methods
 * @v_num given number of vertices to be added into graph on each thread
 * In our testing, we add v_num vertices to each thread, each vertex having index(0~v_num) as its weight.
 */
template <class PGRAPH>
class Tester_AddVertex : public GraphTester<PGRAPH> {
  int v_num;

public:
  Tester_AddVertex() {flag='p'; v_num=0;}
  Tester_AddVertex(int _v_num) {flag='p'; v_num=_v_num;}

  char* GetName() { return "Tester_AddVertex"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_AddVertex *t = new Tester_AddVertex; t->flag=this->flag; t->v_num=this->v_num; return t; }
 
  void SetTestNo () { 
    testno = 3;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void ParseParams(char* s) {
	//the incoming string is such as: "n 20": neither performance nor correctness  & add 20 vertices
	//OR"p 20": report performance & work on current graph & add 20 vertices
    char *space = " "; char *p;

    if(!(p=strtok(s,space))) { 
      ShowUsage(); 
      exit(1); 
    } else flag = p[0];
    if (!(flag=='p'||flag=='c'||flag=='b'))  { 
      ShowUsage(); 
      exit(1); 
    }
    if (p=strtok(NULL,space)) v_num=atoi(p);
    return;
  }  


  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int myid = stapl::get_thread_id();
    int tmp;

	//used by all methods
    PGRAPH newpg = pg;
    Task datum; vector<Task> data;
    typename PGRAPH::VI vi;
    for (int i=0; i<v_num*NP; i++) {
      datum.SetWeight(i);
      data.push_back(datum);
    }

    counter = 0;
    /* 1.1 AddVertex(1): AddVertex(VERTEX&) */
    MethodNames.push_back("AddVertex(1) - AddVertex(VERTEX&)" );
#ifdef VERBOSE
    stapl_print("Testing AddVertex(VERTEX&) for ");
    stapl_print(v_num); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {
      pg=newpg;
      starttimer;
      for (int i=0; i<v_num*NP; i++)  {
        if (myid==i%NP) tmp = pg.AddVertex(data[i]);
      }
      newstoptimer;
    }
    if_want_corr {
      pg=newpg;
      for (int i=0; i<v_num*NP; i++) {
	if (myid==i%NP) tmp=pg.AddVertex(data[i]);
      }
      stapl::rmi_fence();
      for (int i=0; i<v_num*NP; i++) {
        if (stapl::get_thread_id()==i%NP) {
	  if (!pg.IsVertex(i,&vi)) {
            cout<<"ERROR while testing AddVertex(VERTEX&)/IsVertex (1)"<<endl;
     	    break;
 	  } 
  	  if (vi->data.GetWeight() != data[i].GetWeight()) {
            cout<<"ERROR while testing AddVertex(VERTEX&)/IsVertex (2)"<<vi->vid<<" "<<vi->data.GetWeight()<<"  "<<i<<endl;
     	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 1.2 AddVertex(2): AddVertex(VERTEX&, VID, int) */
    MethodNames.push_back("AddVertex(2) - AddVertex(VERTEX&, VID, int)" );
#ifdef VERBOSE
    stapl_print("Testing AddVertex(VERTEX&, VID, int) for ");
    stapl_print(v_num); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {	
      pg=newpg;
      starttimer;
      for (int i=0; i<v_num*NP; i++) {
        if (myid == i%NP)     tmp = pg.AddVertex(data[i], i, myid);
      }
      newstoptimer;
    }
    if_want_corr {	
      pg=newpg;
      for (int i=0; i<v_num*NP; i++) {
	if (myid==i%NP) tmp=pg.AddVertex(data[i],i,myid);
      }
      stapl::rmi_fence();
      for (int i=0; i<v_num*NP; i++) {
        if (stapl::get_thread_id()==i%NP) {
	  if (!pg.IsVertex(i,&vi)) {
            cout<<"ERROR while testing AddVertex(VERTEX&,VID,int)/IsVertex (1)"<<endl;
     	    break;
 	  } 
  	  if (vi->data.GetWeight() != data[i].GetWeight()) {
            cout<<"ERROR while testing AddVertex(VERTEX&,VID,int)/IsVertex (2)"<<vi->vid<<" "<<vi->data.GetWeight()<<"  "<<i<<endl;
     	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
     stapl_print("Done.\n"); 
#endif

    counter++;
    /* 1.3 AddVertex(3): AddVertex(VERTEX&, VID) */
    MethodNames.push_back("AddVertex(3) - AddVertex(VERTEX&, VID)" );
#ifdef VERBOSE
    stapl_print("Testing AddVertex(VERTEX&, VID) for ");
    stapl_print(v_num); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {
      pg=newpg;
      starttimer;
      for (int i=0; i<v_num*NP; i++) {
        if (myid==i%NP) tmp = pg.AddVertex(data[i], i);
      }
      newstoptimer;
    }
    if_want_corr {
      pg=newpg;
      for (int i=0; i<v_num*NP; i++) {
	if (myid==i%NP) tmp=pg.AddVertex(data[i],i);
      }
      stapl::rmi_fence();
      for (int i=0; i<v_num*NP; i++) {
        if (stapl::get_thread_id()==i%NP) {
	  if (!pg.IsVertex(i,&vi)) {
            cout<<"ERROR while testing AddVertex(VERTEX&,VID)/IsVertex (1)"<<endl;
     	    break;
 	  } 
  	  if (vi->data.GetWeight() != data[i].GetWeight()) {
            cout<<"ERROR while testing AddVertex(VERTEX&,VID)/IsVertex (2)"<<vi->vid<<" "<<vi->data.GetWeight()<<"  "<<i<<endl;
     	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
   stapl_print("\nTester_AddVertex tested.\n");
#endif

  }

};

/* (2) Tester_DeleteVertex: Tester for pGraph's method DeleteVertex(VID) and pDeleteVertices(vector<VID>&);
 * must call AddVertex first
 * @v_num number of vertices to be deleted from graph on each thread
 */
template <class PGRAPH>
class Tester_DeleteVertex : public GraphTester<PGRAPH> {

public:
  Tester_DeleteVertex() {flag='p'; }

  char* GetName() { return "Tester_DeleteVertex"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_DeleteVertex *t = new Tester_DeleteVertex; t->flag=this->flag;  return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  
 
  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;
    double avgtime=0; double deviation=0; vector<double> elapses;

    PGRAPH newpg = pg;
    vector<VID> myvids; 
    for(typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      myvids.push_back(vi->vid);
    }

    counter = 0;
    /* 2.1 DeleteVertex: DeleteVertex(VID) */
    MethodNames.push_back("DeleteVertex(1) - DeleteVertex(VID)" );
#ifdef VERBOSE
    stapl_print("Testing DeleteVertex for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg=newpg;
      starttimer;
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
        pg.DeleteVertex(*it);
      }
//cout<<"#"<<stapl::get_thread_id()<<" delete "<<myvids.size()<<" vertices"<<endl;
      newstoptimer;
    } 
    if_want_corr {
      pg=newpg;
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
        pg.DeleteVertex(*it);
      }
      stapl::rmi_fence();
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
        if (pg.IsVertex(*it)) {
          cout<<"ERROR while testing DeleteVertex - "<<*it<<" is still vertex!"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 2.2 pDeleteVertices: pDeleteVertics(vector<VID>) */
    MethodNames.push_back("DeleteVertex(2) - pDeleteVertices(vector<VID>)" );
#ifdef VERBOSE
    stapl_print("Testing pDeleteVertices for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg=newpg;
      starttimer;
      pg.pDeleteVertices(myvids);
      newstoptimer; 
    }
    if_want_corr {
      pg=newpg;
      pg.pDeleteVertices(myvids);
      stapl::rmi_fence();
      for (vector<VID>::iterator it=myvids.begin(); it!=myvids.end(); ++it) {
        if (pg.IsVertex(*it)) {
          cout<<"ERROR while testing pDeleteVertices "<<*it<<" is still vertex!"<<endl;
	  break;
  	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
    stapl_print("\nTester_DeleteVertex tested.\n");
#endif
  }

};

/* (3) Tester_IsVertex: Tester for pGraph's method IsVertex 
 * must call AddVertex first
 */
template <class PGRAPH>
class Tester_IsVertex : public GraphTester<PGRAPH> {

public:
  Tester_IsVertex() {flag='p';}

  char* GetName() { return "Tester_IsVertex"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_IsVertex *t = new Tester_IsVertex; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    vector<VID> tovids;
    vector<Task> tovertices;

    Task datum; vector<Task> data;
    typename PGRAPH::VI vi;
    for (vi=pg.local_begin(); vi!=pg.local_end(); vi++) {
      tovids.push_back(vi->vid);
      tovertices.push_back(*vi);
    }

    counter = 0;
    /* 3.1 IsVertex(1): IsVertex(VID) */
    MethodNames.push_back("IsVertex(1) - IsVertex(VID)" );
#ifdef VERBOSE
    stapl_print("Testing IsVertex(VID) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<tovids.size(); i++)  {
        tmp = pg.IsVertex(tovids[i]);
      }
      newstoptimer;
    }
    if_want_corr { 
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK"); 
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 3.2 IsVertex(2): IsVertex(VID,VI*) */
    MethodNames.push_back("IsVertex(2) - IsVertex(VID,VI*)");
#ifdef VERBOSE
    stapl_print("Testing IsVertex(VID,VI*) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<tovids.size(); i++)  {
        tmp = pg.IsVertex(tovids[i],&vi);
      }
      newstoptimer;
    }
    if_want_corr { 
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK"); 
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 3.3 IsVertex(3): IsVertex(VERTEX&) */
    MethodNames.push_back("IsVertex(3) - IsVertex(VERTEX&)" );
#ifdef VERBOSE
    stapl_print("Testing IsVertex(VERTEX&) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<tovertices.size(); i++)  {
        tmp = pg.IsVertex(tovertices[i]);
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<tovertices.size(); i++) {
        if (!pg.IsVertex(i))  {
          cout<<"ERROR while testing IsVertex"<<endl;
	  break;
	}
	if (!pg.IsVertex(tovertices[i])) {
          cout<<"ERROR while testing IsVertex(VERTEX&)"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 3.4 IsVertex(4): IsVertex(VERTEX&, VI*) */
    MethodNames.push_back("IsVertex(4) - IsVertex(VERTEX&, VI*)" );
//#ifdef VERBOSE
    stapl_print("Testing IsVertex(VERTEX&,VI*) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
//#endif
    if_want_perf {
      starttimer;
      for (int i=0; i<tovertices.size(); i++)  {
        tmp = pg.IsVertex(tovertices[i], &vi);
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<tovertices.size(); i++) {
        if (!pg.IsVertex(i))  {
          cout<<"ERROR while testing IsVertex"<<endl;
	  break;
	}
	if (!pg.IsVertex(tovertices[i],&vi))  {
          cout<<"ERROR while testing IsVertex(VERTEX&,VI*) i="<<i<<", data[i]="<<data[i].GetWeight()<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    stapl_print("\nTester_IsVertex tested.\n");
#endif
  }

};

/* (4) Tester_GetVertices: Tester for pGraph's methods GetVertices(vector<VID>&), GetVertices(vector<VI>&)  and GetVertices(vector<VERTEX>&)
 * these methods depend on AddVertex - assume user will guarantee that
 */
template <class PGRAPH>
class Tester_GetVertices : public GraphTester<PGRAPH> {

public:

  Tester_GetVertices() {flag='p'; }

  char* GetName() { return "Tester_GetVertices";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetVertices *t = new Tester_GetVertices; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    pArray<VID> verts;
    pArray<Task> vertsD;

    counter = 0;
    /* 4.1 GetVertices(1): GetVertices(VIDs) */
    MethodNames.push_back("GetVertices(1) - GetVertices(pArray<VID>)" );
#ifdef VERBOSE
    stapl_print("Testing GetVertices(pArray<VID>) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetVertices(verts);
      newstoptimer;
    }
    if_want_corr {
      verts.clear();
      tmp = pg.GetVertices(verts);
      for(int i=0; i<tmp; i++) {
	if(!pg.IsVertex(verts[i])) {
	  cout<<"ERROR while testing GetVertices(pArray<VID>) / IsVertex(VID) :"<<verts[i]<<" is returned, but not vertex"<<endl;
	  break;
	}
      }	
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 4.2 GetVertices(2): GetVertices(VERTEXs) */
    MethodNames.push_back("GetVertices(2) - GetVertices(pArray<VERTEX>)" );
#ifdef VERBOSE
    stapl_print("Testing GetVertices(pArray<VERTEX>) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetVertices(vertsD);
      newstoptimer;
    }
    if_want_corr {
      for (int i=0; i<tmp; i++) {
        if (verts[i] != vertsD[i].GetWeight()) {
          cout<<"ERROR while testing GetVertices(pArray<VERTEX>)"<<endl;
          //cout<<stapl::get_thread_id()<<":index:"<<i<<":val"<<vertsD[i].GetWeight()<<endl;
	  break;
        }
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish 
    stapl_print("\nTester_GetVertices passed testing.\n");
#endif
  }

};

/* (5) Tester_AddEdge: Tester for pGraph's method AddEdge(VID, VID, Weight2&);
 * this method depends on addVertex - only when graph has vertices, AddEdge can be called - We assume user will take this into account
 * @deg the number of edges of each vertex in the graph, so the graph has pg.size()*deg edges in all while pg.size()=NP*v_num (in AddVertex)
 * each vertex having index(0~v_num) as its weight. Then add deg edges for each vertex, each edge having its 2 vertices' weights
 * multiplied as the edge's weight.
 *
 */
template <class PGRAPH>
class Tester_AddEdge : public GraphTester<PGRAPH> {

  int deg;

public:

  Tester_AddEdge() {flag='p'; deg=1;}
  Tester_AddEdge(int _deg) {flag='p'; deg=_deg;}

  char* GetName() { return "Tester_AddEdge"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_AddEdge *t = new Tester_AddEdge; t->flag=this->flag; t->deg=this->deg; return t; }

  void ParseParams(char* s) {
	//the incoming string is such as: "n 20": neither performance nor correctness  & add 20 edges
	//OR"p 20": report performance & work on current graph & add 20 edges
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

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;
    PGRAPH newpg=pg;
    int gsize= pg.size();

    typename PGRAPH::VI vi;
    Weight2 wt,wt1,wt2; vector<Weight2> wts; vector<pair<Weight2,Weight2> > wtpairs;
    int maxj,minj;

    vector<_StaplTriple<VID,VID,Weight2> > edgesvec;
    GenEdge(edgesvec, pg, deg);

    counter = 0;
    /* 5.1 AddEdge(1): AddEdge(VID,VID,WEIGHT&) */
    MethodNames.push_back("AddEdge(1) - AddEdge(VID,VID,WEIGHT)" );
#ifdef VERBOSE
    stapl_print("Testing AddEdge(VID,VID,WEIGHT&) for ");
    stapl_print(deg); stapl_print(" edges/vertex for a graph with ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg=newpg; //i=0; 
      starttimer;
      for (int i=0; i<edgesvec.size(); i++) {
	tmp = pg.AddEdge(edgesvec[i].first, edgesvec[i].second, edgesvec[i].third);
      }
/*
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-5) {
        for (int j=vi->vid+5;  j>vi->vid; j--) {
          tmp = pg.AddEdge(vi->vid,j,wts[i]);
          i++;
        }
	}
      }
*/
      newstoptimer;
    }
    if_want_corr {
      pg=newpg; //i=0;
      for (int i=0; i<edgesvec.size(); i++) {
	tmp = pg.AddEdge(edgesvec[i].first, edgesvec[i].second, edgesvec[i].third);
      }
/*
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-5) {
        for (int j=vi->vid+5;  j>vi->vid; j--) {
       // for (int j=maxj; j>=minj; j--) {
          tmp = pg.AddEdge(vi->vid,j,wts[i]); i++;
	}
	}
      }
*/
      stapl::rmi_fence();
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-deg) {
        for (int j=vi->vid+deg;  j>vi->vid; j--) {
        //for (int j=maxj; j>=minj; j--) {
          if (!pg.IsEdge(vi->vid, j) ) {
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
	}
      }     
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
#endif

    //counter++;
    /* 5.2 AddEdge(2): AddEdge(VID,VID,pair<WEIGHT,WEIGHT>&) 
    MethodNames.push_back("AddEdge(2) - AddEdge(VID,VID,pair<WEIGHT,WEIGHT>&)" );
#ifdef VERBOSE
    stapl_print("Testing AddEdge(VID,VID,pair<WEIGHT,WEIGHT>&) for ");
    stapl_print(deg); stapl_print(" edges/vertex for a grpah with ");
    stapl_print(newpg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg=newpg; i=0;
      starttimer;
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-5) {
        for (int j=vi->vid+5;  j>vi->vid; j--) {
        //for (int j=maxj; j>=minj; j--) {
          tmp = pg.AddEdge(vi->vid,j,wtpairs[i]);
          i++;
        }
	}
      }
      newstoptimer;
    }
    if_want_corr {
      pg=newpg; i=0;
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-5) {
        for (int j=vi->vid+5;  j>vi->vid; j--) {
        //for (int j=maxj; j>=minj; j--) {
          tmp = pg.AddEdge(vi->vid,j,wtpairs[i]); i++;
	}
	}
      }
      stapl::rmi_fence();
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
       if (vi->vid < gsize-5) {
        for (int j=vi->vid+5;  j>vi->vid; j--) {
       // for (int j=maxj; j>=minj; j--) {
          if (!pg.IsEdge(vi->vid, j) ) {
    	    cout<<"ERROR while testing AddEdge/IsEdge"<<endl;
	    break;
	  }
        }
	}
      }     
    }
    stapl::rmi_fence();
    stapl_print("Done.\n"); */
    //finish
#ifdef VERBOSE
    stapl_print("\nTester_AddEdge passed testing.\n");
#endif
  }

};


/* (6) Tester_DeleteEdge: Tester for pGraph's method DeleteEdge(VID,VID)
 * must call AddEdge first
 */
template <class PGRAPH>
class Tester_DeleteEdge : public GraphTester<PGRAPH> {

public:
  Tester_DeleteEdge() {flag='p';}

  char* GetName() { return "Tester_DeleteEdge"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_DeleteEdge *t = new Tester_DeleteEdge; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    GraphTester<PGRAPH>::SetTestNo();
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;
    int e_num = pg.GetEdgeCount();
    PGRAPH newpg = pg;
   
    counter = 0;
    MethodNames.push_back("DeleteEdge(VID,VID)");
#ifdef VERBOSE
    stapl_print("Testing DeleteEdge(VID,VID) for ");
    stapl_print(e_num); stapl_print(" edges in all...");
#endif

    pair<VID,VID> vidp;
    vector<pair<VID,VID> > todos;
    int counter=0;
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      for (typename PGRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
        todos.push_back(pair<VID,VID>(vi->vid,ei->vertex2id));
      }
    }
    if_want_perf {
      pg=newpg;
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        //cout<<"#"<<stapl::get_thread_id()<<" trying to delete edge ("<<todos[i].first<<","<<todos[i].second<<")"<<endl;
        pg.DeleteEdge(todos[i].first, todos[i].second);
      }
      newstoptimer;
    }
    if_want_corr {
      pg=newpg;
      for (vector<pair<VID,VID> >::iterator it=todos.begin(); it!=todos.end(); ++it) {
        pg.DeleteEdge(it->first, it->second);
      }
      stapl::rmi_fence();
      for (vector<pair<VID,VID> >::iterator it=todos.begin(); it!=todos.end(); ++it) {
        if (pg.IsEdge(it->first, it->second) ) {
          cout<<"ERROR while testing DeleteEdge/IsEdge"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_DeleteEdge tested.\n"); 
#endif

  }
};

/* (7) Tester_IsEdge: Tester for pGraph's method IsEdge
 * must call AddEdge first
 */

template <class PGRAPH>
class Tester_IsEdge : public GraphTester<PGRAPH> {

public:
  Tester_IsEdge() {flag='p'; }

  char* GetName() { return "Tester_IsEdge"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_IsEdge *t = new Tester_IsEdge; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    typename PGRAPH::VI vi, vi2, vi3;
    typename PGRAPH::EI ei, ei3;

    counter = 0;
    /* 7.1 IsEdge(1): IsEdge(VID,VID) */
    MethodNames.push_back("IsEdge(1) - IsEdge(VID,VID)" );
#ifdef VERBOSE
    stapl_print("Testing IsEdge(VID,VID) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all..."); 
#endif
    if_want_perf {
      starttimer;
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
        for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
          tmp = pg.IsEdge(vi->vid, ei->vertex2id);
        }     
      }
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK\n");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 7.2 IsEdge(2): IsEdge(VI,EI) */
    MethodNames.push_back("IsEdge(2) - IsEdge(VI,EI)" );
#ifdef VERBOSE
    stapl_print("Testing IsEdge(vi,ei) for ");
    stapl_print(pg.size()*pg.size()); stapl_print(" vertex pairs in all..."); 
#endif
    if_want_perf {
      starttimer;
      for (vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
        for (ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
          tmp = pg.IsEdge(vi->vid, ei->vertex2id, &vi3, &ei3);
        }     
      }
      newstoptimer;
    }
    if_want_corr {
#ifdef VERBOSE
      stapl_print("NO_CORRECT_CHECK\n");
#endif
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_IsEdge tested.\n"); 
#endif
  }

};

/* (8) Tester_GetAllEdges: Tester for pGraph's methods GetGetAllEdges
 * must call AddEdge first - we assume user will guarantee that
 */
template <class PGRAPH>
class Tester_GetAllEdges : public GraphTester<PGRAPH> {

public:

  Tester_GetAllEdges() {flag='p';}

  char* GetName() { return "Tester_GetAllEdges";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetAllEdges *t = new Tester_GetAllEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    pArray<pair<pair<VID,VID>, Weight2> > pedges;
    pArray<pair<pair<Task,Task>, Weight2> > edgesD;

    counter = 0;
    /* 8.1 GetAllEdges(1): GetAllEdges(VIDs) */
    MethodNames.push_back("GetAllEdges(1) - GetAllEdges(pArray<pair<pair<VID,VID>,WEIGHT> >&)" );
#ifdef VERBOSE
    stapl_print("Testing GetAllEdges(VIDs...) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetAllEdges(pedges);
      newstoptimer;
    }
    if_want_corr {
      tmp = pg.GetAllEdges(pedges);
      for(int i=0; i<tmp; i++) {
	if (pedges[i].first.first * pedges[i].first.second != pedges[i].second.GetWeight() ) {
	  cout<<"ERROR while testing GetAllEdges(VIDs...)"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 8.2 GetAllEdges(2): GetAllEdges(VERTEX) */
    MethodNames.push_back("GetAllEdges(2) - GetAllEdges(pArray<pair<pair<VERTEX,VERTEX>,WEIGHT> >&)" );
#ifdef VERBOSE
    stapl_print("Testing GetAllEdges(VERTEX) [i.e., GetAllEdgesVData] for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetAllEdgesVData(edgesD);
      newstoptimer;
    }
    if_want_corr {
      tmp = pg.GetAllEdgesVData(edgesD);
      for (int i=0; i<tmp; i++) {
        if (edgesD[i].first.first.GetWeight() * edgesD[i].first.second.GetWeight() != edgesD[i].second.GetWeight())  {
          cout<<"ERROR while testing GetAllEdgesVData"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_GetAllEdges tested.\n"); 
#endif
  }
};


/* (9) Tester_GetEdges: Tester for pGraph's methods GetEdges (which return local edges instead of all edges in GetAllEdges)
 * depends on AddEdge 
 */
template <class PGRAPH>
class Tester_GetEdges : public GraphTester<PGRAPH>, BasePObject {

private:
  void sum(int* in, int* inout) {*inout += *in; }
  
public:

  Tester_GetEdges() {flag='p'; this->register_this(this); }
  ~Tester_GetEdges() {rmiHandle handle = this->getHandle(); if (handle>-1) stapl::unregister_rmi_object(handle); }

  char* GetName() { return "Tester_GetEdges";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetEdges *t = new Tester_GetEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    counter = 0;
    /* 9.1 GetEdges(1): GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >&) */
    MethodNames.push_back("GetEdges(1) - GetEdges(vector<pair<pair<VID,VID>,WEIGHT> >&)" );
    vector<pair<pair<VID,VID>,Weight2> > edges_w;
#ifdef VERBOSE
    stapl_print("Testing GetEdges_w for ");
    stapl_print(pg.GetLocalEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetEdges(edges_w);
      newstoptimer;
    }
    if_want_corr {
 	//sum of all 'tmp's should equal to GetEdgeCount()
      int sum = 0;
      tmp = pg.GetEdges(edges_w);
      stapl::reduce_rmi(&tmp, &sum, this->getHandle(), &Tester_GetEdges<PGRAPH>::sum, true);
      //if(!(pg.IsDirected()) ) sum=sum*2;
      if(sum!=pg.GetEdgeCount() ) {
	cout<<"sum="<<sum<<" pg.GetCount="<<pg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges()"<<endl;
      }
      for (int i=0; i<edges_w.size(); i++) {
        if(!pg.IsEdge(edges_w[i].first.first, edges_w[i].first.second)) {
          cout<<"ERROR while testing GetEdges_w(2): "<<endl;
	  break;
	}
        if(edges_w[i].second.GetWeight() != edges_w[i].first.first * edges_w[i].first.second) {
          cout<<"ERROR while testing GetEdges_w(3): "<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 9.2 GetEdges(2): GetEdges(vector<pair<pair<Task,Task>,WEIGHT> >&) */
    vector<pair<Task,Task> > edges_data;
    MethodNames.push_back("GetEdges(2) - GetEdges(vector<pair<pair<Task,Task>, WEIGHT> >&)" );
    vector<pair<pair<Task,Task>,Weight2> > edges_vw;
#ifdef VERBOSE
    stapl_print("Testing GetEdges_vw for ");
    stapl_print(pg.GetLocalEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetEdgesVData(edges_vw);
      newstoptimer;
    }
    if_want_corr {
	//sum of all 'tmp's should equal to GetEdgeCount()
      int sum = 0;
      tmp = pg.GetEdgesVData(edges_vw);
      stapl::reduce_rmi(&tmp, &sum, this->getHandle(), &Tester_GetEdges<PGRAPH>::sum, true);
      //if(!(pg.IsDirected()) ) sum=sum*2;
      if(sum!=pg.GetEdgeCount() ) {
	cout<<"sum="<<sum<<" pg.GetCount="<<pg.GetEdgeCount()<<endl;
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
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 9.3 GetEdges(3): GetEdges(VIDs) */
    MethodNames.push_back("GetEdges(3) - GetEdges(vector<pair<VID,VID> >&)" );
    vector<pair<VID,VID> > edges;
#ifdef VERBOSE
    stapl_print("Testing GetEdges(VIDs) for ");
    stapl_print(pg.GetLocalEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetEdges(edges);
      newstoptimer;
    }
    if_want_corr {
 	//sum of all 'tmp's should equal to GetEdgeCount()
      int sum = 0;
      tmp = pg.GetEdges(edges);
      stapl::reduce_rmi(&tmp, &sum, this->getHandle(), &Tester_GetEdges<PGRAPH>::sum, true);
      //if(!(pg.IsDirected()) ) sum=sum*2;
      if(sum!=pg.GetEdgeCount() ) {
	cout<<"sum="<<sum<<" pg.GetCount="<<pg.GetEdgeCount()<<endl;
	cout<<"ERROR while testing GetEdges(1)"<<endl;
      }
      for (int i=0;i<edges.size();i++){
        if(!(pg.IsEdge(edges[i].first, edges[i].second))) {
          cout<<"ERROR while testing GetEdges(2): "<<edges[i].first<<":"<<edges[i].second<<endl;
	  return;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 9.4 GetEdges(4): GetEdges(vector<pair<Task,Task> >&) */
    MethodNames.push_back("GetEdges(4) - GetEdges(vector<pair<Task,Task> >&)" );
#ifdef VERBOSE
    stapl_print("Testing GetEdges(vector<pair<Task,Task> >) for ");
    stapl_print(pg.GetLocalEdgeCount()); stapl_print(" edges on each thread...");
#endif
    if_want_perf {
      starttimer;
      tmp = pg.GetEdgesVData(edges_data);
      newstoptimer;
    }
    if_want_corr {
	//sum of all 'tmp's should equal to GetEdgeCount()
      int sum = 0;
      tmp = pg.GetEdgesVData(edges_data);
      stapl::reduce_rmi(&tmp, &sum, this->getHandle(), &Tester_GetEdges<PGRAPH>::sum, true);
      //if(!(pg.IsDirected()) ) sum=sum*2;
      if(sum!=pg.GetEdgeCount() ) {
	cout<<"sum="<<sum<<" pg.GetCount="<<pg.GetEdgeCount()<<endl;
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
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_GetEdges tested.\n"); 
#endif
  }
};

/* (10) Tester_SetGetVertexField: Tester for pGraph's methods pSetVertexField and pGetVertexField
 * must call AddVertex first
 */
template <class PGRAPH>
class Tester_SetGetVertexField : public GraphTester<PGRAPH> {

public:
  Tester_SetGetVertexField() {flag='p';}

  char* GetName() { return "Tester_SetGetVertexField";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_SetGetVertexField *t = new Tester_SetGetVertexField; t->flag=this->flag;  return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  //void Test(PGRAPH& pg) const {
  void Test(PGRAPH& pg)  {
    int NP=stapl::get_num_threads();
    int tmp;

    double dd;

    counter = 0;
    /* 10.1 SetGetVertexField(1): pSetVertexField(1 arg) */
    MethodNames.push_back("SetGetVertexField(1): pSetVertexField(1 arg)");
    vector<VID> todos; int tmp1;
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      todos.push_back(vi->vid);
    }
#ifdef VERBOSE
    stapl_print("Testing pSetVertexField(1 arg) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf { 
      starttimer;
      for(int i=0; i<todos.size(); i++) {
        tmp1=todos[i]+11;
        pg.pSetVertexField(todos[i], &Task::SetWeight1, tmp1);
      }
      newstoptimer;
    }
    if_want_corr {
      for(int i=0; i<todos.size(); i++) {
        tmp1=todos[i]+11;
        pg.pSetVertexField(todos[i], &Task::SetWeight1, tmp1);
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("...\n");
#endif

    counter++;
    /* 10.2 SetGetVertexField(2): pGetVertexField() */
    MethodNames.push_back("SetGetVertexField(2): pGetVertexField()");
#ifdef VERBOSE
    stapl_print("Testing pGetVertexField() for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      starttimer;
      for(int i=0; i<todos.size(); i++) {
        dd = pg.pGetVertexField(todos[i], &Task::GetWeight);
      }
      newstoptimer;
    }
    if_want_corr {
      for(int i=0; i<todos.size(); i++) {
        dd = pg.pGetVertexField(todos[i], &Task::GetWeight);
        if(dd!=todos[i]+11) {
	  cout<<"ERROR while testing pSet/GetVertexField(1 arg)" <<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_SetGetVertexField tested.\n"); 
#endif
  }
};

/* (11) Tester_SetGetWeightField: Tester for pGraph's methods pSetWeightField(Id) and pGetWeightField(Id)
 * must call AddVertex and AddEdge first
 */
template <class PGRAPH>
class Tester_SetGetWeightField : public GraphTester<PGRAPH> {

public:
  Tester_SetGetWeightField() {flag='p'; }

  char* GetName() { return "Tester_SetGetWeightField";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_SetGetWeightField *t = new Tester_SetGetWeightField; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  //void Test(PGRAPH& pg) const {
  void Test(PGRAPH& pg)  {
    int NP=stapl::get_num_threads();
    int tmp;

    double dd;
   
    counter = 0;
    /* 11.1 pSetGetWeightField(1): pSetWeightField(1 arg) */
    MethodNames.push_back("pSetGetWeightField(1): pSetWeightField(1 arg)");
    vector<pair<VID,VID> > edges;
    int wt;
//    pg.pSetPredecessors();
 //   stapl::rmi_fence();
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      for (typename PGRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
	edges.push_back(pair<VID,VID>(vi->vid,ei->vertex2id));
      }
    }
#ifdef VERBOSE
    stapl_print("Testing pSetWeightField(1 arg) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      for (int i=0;i<edges.size();i++){
        wt = edges[i].first * edges[i].second;
        tmp = pg.pSetWeightField(edges[i].first,edges[i].second,&Weight2::SetWeight1,wt);  
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0;i<edges.size();i++){
        wt = edges[i].first * edges[i].second;
        tmp = pg.pSetWeightField(edges[i].first,edges[i].second,&Weight2::SetWeight1,wt);  
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("...\n");
#endif

    counter++;
    /* 11.2 pSetGetWeightField(2): pGetWeightField() */
    MethodNames.push_back("pSetGetWeightField(2): pGetWeightField()");
#ifdef VERBOSE
    stapl_print("Testing pGetWeightField(1 arg) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;  
      for(int i=0;i<edges.size();i++){
        dd = pg.pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1) ;
      }
      newstoptimer;
    } 
    if_want_corr {
      for(int i=0; i<edges.size(); i++) {
        dd = pg.pGetWeightField(edges[i].first,edges[i].second,&Weight2::GetWeight1) ;
        if(dd!=edges[i].first*edges[i].second) {
	  cout<<"ERROR while testing pSet/GetWeightField(1 arg) dd="<<dd<<" product="<<edges[i].first*edges[i].second<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_SetGetWeightField tested.\n"); 
#endif

  }
};

/* (12) Tester_SetGetWeightFieldEdgeId: Tester for pGraph's methods pSetWeightFieldEdgeId and pGetWeightFieldEdgeId
 * must call AddVertex and AddEdge first
 */
template <class PGRAPH>
class Tester_SetGetWeightFieldEdgeId : public GraphTester<PGRAPH> {

public:
  Tester_SetGetWeightFieldEdgeId() {flag='p'; }

  char* GetName() { return "Tester_SetGetWeightFieldEdgeId";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_SetGetWeightFieldEdgeId *t = new Tester_SetGetWeightFieldEdgeId; t->flag=this->flag;  return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 2; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  //void Test(PGRAPH& pg) const {
  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    double dd;
    vector<pair<VID,VID> > edges;
    pg.pSetPredecessors();
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      for (typename PGRAPH::EI ei=vi->edgelist.begin(); ei!=vi->edgelist.end(); ++ei) {
	edges.push_back(pair<VID,VID>(vi->vid, ei->edgeid));
      }
    }
    stapl::rmi_fence();

    counter = 0;
    /* 12.1 pSetGetWeightFieldEdgeId(1):  pSetWeightFieldEdgeId(1 arg) */
    MethodNames.push_back("pSetGetWeightFieldEdgeId(1):  pSetWeightFieldEdgeId(1 arg)");
#ifdef VERBOSE
    stapl_print("Testing pSetWeightFieldEdgeId(1 arg) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {	
      starttimer;
      for (int i=0;i<edges.size();i++){
        tmp = edges[i].first * edges[i].second;
        pg.pSetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::SetWeight1,tmp);  
      }
      newstoptimer;
    }
    if_want_corr {
      for (int i=0;i<edges.size();i++){
        tmp = edges[i].first * edges[i].second;
        pg.pSetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::SetWeight1,tmp);  
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("...\n");
#endif

    counter++;
    /* 12.2 pSetGetWeightFieldEdgeId(2):  pGetWeightFieldEdgeId(1 arg) */
    MethodNames.push_back("pSetGetWeightFieldEdgeId(2):  pGetWeightFieldEdgeId(1 arg)");
#ifdef VERBOSE
    stapl_print("Testing pGetWeightFieldEdgeId(1 arg) for ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif
    if_want_perf {
      starttimer;
      for(int i=0; i<edges.size();i++){
        dd=pg.pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
      }
      newstoptimer;
    }
    if_want_corr {
      for(int i=0; i<edges.size(); i++) {
        dd=pg.pGetWeightFieldEdgeId(edges[i].first,edges[i].second,&Weight2::GetWeight1);
        if(dd!=edges[i].first*edges[i].second) {
	  cout<<"ERROR while testing pSet/GetWeightFieldEdgeId(1 arg)"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_SetGetWeightFieldEdgeId tested.\n"); 
#endif
  }
};

/* (x) Tester_GetAdjacentVertices: Tester for pGraph's method GetAdjacentVertices/DATA
 * must call AddVertex and AddEdge first
 */
template <class PGRAPH>
class Tester_GetAdjacentVertices : public GraphTester<PGRAPH> {

public:
  Tester_GetAdjacentVertices() {flag='p'; }

  char* GetName() { return "Tester_GetAdjacentVertices"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetAdjacentVertices *t = new Tester_GetAdjacentVertices; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 3;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    vector<VID> adj; vector<Task> adjdata;
    typename PGRAPH::VI vi;
    vector<VID> todos; 
    vector<typename PGRAPH::VI> todovis;
    vector<Task> todovertices;
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      todovis.push_back(vi);
      todos.push_back(vi->vid);
      todovertices.push_back(*vi);
    }

    counter = 0;
    /* 10.1 GetAdjacentVertices(1): GetAdjacentVertices(VID,vector<VID>&...) */
    MethodNames.push_back("GetAdjacentVertices(1): GetAdjacentVertices(VID,vector<VID>&)");
#ifdef VERBOSE
    stapl_print("Testing GetAdjacentVertices(VID,vector<VID>&) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adj.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        pg.GetAdjacentVertices(todos[i], adj);
      }
      newstoptimer;
    }
    if_want_corr {
      adj.clear();
      for (int i=0; i<todos.size(); i++)  {
        pg.GetAdjacentVertices(todos[i], adj);
        if (!pg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/IsVertex(VID) (1): wrong input vids"<<endl;
	  break;
	}
  	if (adj.size() != pg.GetVertexOutDegree(todos[i]) ) {
          cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/GetVertexOutDegree(VID) (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adj.size(); j++) {
 	  if (!pg.IsEdge(todos[i], adj[j]) )  {
            cout<<"ERROR while testing GetAdjacentVertices(VID,vector<VID>&)/IsEdge(VID,VID) (3): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 10.2 GetAdjacentVertices(2): GetAdjacentVerticesDATA(VID,vector<VERTEX&>...) */
    MethodNames.push_back("GetAdjacentVertices(2): GetAdjacentVerticesDATA(VID,vector<VERTEX&>...)");
#ifdef VERBOSE
    stapl_print("Testing GetAdjacentVerticesDATA(VID, vector<VERTEX&>...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adjdata.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        pg.GetAdjacentVerticesDATA(todos[i], adjdata);
      }
      newstoptimer;
    }
    if_want_corr {
      adjdata.clear();
      for (int i=0; i<todos.size(); i++)  {
        pg.GetAdjacentVerticesDATA(todos[i], adjdata);
        if (!pg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/IsVertex(VID): wrong input vids"<<endl;
	  break;
	}
  	if (adjdata.size() != pg.GetVertexOutDegree(todos[i]) ) {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/GetVertexOutDegree(VID) (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adjdata.size(); j++) {
 	  if (!pg.IsEdge(todos[i], pg.GetVID(adjdata[j]) ) )  {
            cout<<"ERROR while testing GetAdjacentVerticesDATA(VID,vector<VERTEX>&)/GetVID/IsEdge(VID,VID) (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
#endif

    counter++;
    /* 10.3 GetAdjacentVertices(3): GetAdjacentVertices(VI,vector<VID>...) */
    MethodNames.push_back("GetAdjacentVertices(3): GetAdjacentVertices(VI,vector<VID>...)");
#ifdef VERBOSE
    stapl_print("Testing GetAdjacentVertices(VI,vector<VID>&...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adj.clear(); 
      starttimer;
      for (int i=0; i<todovis.size(); i++)  {
        pg.GetAdjacentVertices(todovis[i], adj);
      }
      newstoptimer;
    }
    if_want_corr {
      adj.clear(); 
      for (int i=0; i<todovis.size(); i++)  {
        pg.GetAdjacentVertices(todovis[i], adj);
        if (!pg.IsVertex(todovis[i]->vid))  {
          cout<<"ERROR while testing GetAdjacentVertices(VI,vector<VID>&)/IsVertex(VID) (1): wrong input vis"<<endl;
	  break;
	}
	if (adj.size() != pg.GetVertexOutDegree(todovis[i]->vid) ) {
          cout<<"ERROR while testing GetAdjacentVertices(VI,vector<VID>&)/GetVertexOutDegree(VID) (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adj.size(); j++) {
 	  if (!pg.IsEdge(todovis[i]->vid, adj[j]) )  {
            cout<<"ERROR while testing GetAdjacentVertices(VI,vector<VID>&)/IsEdge(VID,VID) (3): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

/* - only pug has , so comment
    counter++;
     10.4 GetAdjacentVertices(4): GetAdjacentVertices(VERTEX&,vector<VID>&...) 
    MethodNames.push_back("GetAdjacentVertices(4): GetAdjacentVertices(VERTEX&,vector<VID>&...)");
#ifdef VERBOSE
    stapl_print("Testing GetAdjacentVertices(VERTEX&, vector<VID>&...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adj.clear(); 
      starttimer;
      for (int i=0; i<todovertices.size(); i++)  {
        pg.GetAdjacentVertices(todovertices[i], adj);
      }
      newstoptimer;
    }
    if_want_corr {
      adj.clear(); 
      for (int i=0; i<todovertices.size(); i++)  {
	VID vid=pg.GetVID(todovertices[i]);
        pg.GetAdjacentVertices(vid, adj);
        if (!pg.IsVertex(vid)) {
          cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VID>&)/IsVertex: wrong input vids"<<endl;
	  break;
	}
  	if (adj.size() != pg.GetVertexOutDegree(vid) ) {
          cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VID>&)/GetVertexOutDegree (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adj.size(); j++) {
 	  if (!pg.IsEdge(vid, adj[j] ) )  {
            cout<<"ERROR while testing GetAdjacentVertices(VERTEX&,vector<VERTEX>&)/GetVID/IsEdge (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
     10.5 GetAdjacentVertices(5): GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&...) 
    MethodNames.push_back("GetAdjacentVertices(5): GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&...)");
#ifdef VERBOSE
    stapl_print("Testing GetAdjacentVerticesDATA(VERTEX&, vector<VERTEX>&...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      adjdata.clear(); 
      starttimer;
      for (int i=0; i<todovertices.size(); i++)  {
        pg.GetAdjacentVerticesDATA(todovertices[i], adjdata);
      }
      newstoptimer;
    }
    if_want_corr {
      adjdata.clear(); 
      for (int i=0; i<todovertices.size(); i++)  {
        VID vid=pg.GetVID(todovertices[i]);
        pg.GetAdjacentVerticesDATA(vid, adjdata);
        if (!pg.IsVertex(vid) )  {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/IsVertex: wrong input vids"<<endl;
	  break;
	}
  	if (adjdata.size() != pg.GetVertexOutDegree(vid) ) {
          cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/GetVertexOutDegree (2): wrong # of adj. vertices"<<endl;
	  break;
	}
        for (int j=0; j<adjdata.size(); j++) {
 	  if (!pg.IsEdge(todovertices[i], adjdata[j] ) )  {
            cout<<"ERROR while testing GetAdjacentVerticesDATA(VERTEX&,vector<VERTEX>&)/GetVID/IsEdge(VERTEX&,VERTEX&) (3): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_GetAdjacentVertices tested.\n");
#endif
*/
  }
};

/* (x) Tester_GetPredVertices: Tester for pGraph's method GetPredVertices and GetPredVerticesDATA
 * must call AddVertex, AddEdge and pSetPredecessors first
 */
template <class PGRAPH>
class Tester_GetPredVertices : public GraphTester<PGRAPH> {

public:
  Tester_GetPredVertices() {flag='p'; }

  char* GetName() { return "Tester_GetPredVertices"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetPredVertices *t = new Tester_GetPredVertices; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {

    int NP=stapl::get_num_threads();
    int tmp;

    vector<VID> pred; vector<Task> preddata;
    vector<VID> todos; vector<typename PGRAPH::VI> todovis; 
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      todos.push_back(vi->vid);
      todovis.push_back(vi);
    }

    counter = 0; 
    /* 11.1 GetPredVertices(1): GetPredVertices(VID,vector<VID>&) */
    MethodNames.push_back("GetPredVertices(1): GetPredVertices(VID,vector<VID>&)");
#ifdef VERBOSE
    stapl_print("Testing GetPredVertices(VID, vector<VID>&) for ");
    stapl_print(pg.size()); stapl_print(" vertices on each thread...");
#endif
    if_want_perf {
      pred.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        pg.GetPredVertices(todos[i], pred);
      }
      newstoptimer;
    }
    if_want_corr {
      pred.clear();
      for (int i=0; i<todos.size(); i++) {
        pg.GetPredVertices(todos[i], pred);
        if (!pg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetPredVertices(VID,vector<VID>&)/IsVertex(VID) (1): wrong input vids"<<endl;
	  break;
	}
        for (int j=0; j<pred.size(); j++) {
 	  if (!pg.IsEdge(pred[j], todos[i]) )  {
            cout<<"ERROR while testing GetPredVertices(VID,vector<VID>&)/IsEdge(VID,VID) (2): not edges"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 11.2 GetPredVertices(2): GetPredVerticesDATA(VID,vector<VERTEX>&) */
    MethodNames.push_back("GetPredVertices(2): GetPredVerticesDATA(VID,vector<VERTEX>&)");
#ifdef VERBOSE
    stapl_print("Testing GetPredVerticesDATA(VID, vector<VERTEX&>...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      preddata.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++)  {
        pg.GetPredVerticesDATA(todos[i], preddata);
      }
      newstoptimer;
    }
    if_want_corr {
      preddata.clear();
      for (int i=0; i<todos.size(); i++) {
        pg.GetPredVerticesDATA(todos[i], preddata);
        if (!pg.IsVertex(todos[i]))  {
          cout<<"ERROR while testing GetPredVerticesDATA(VID,vector<VERTEX>&)/IsVertex(VID): wrong input vids"<<endl;
	  break;
	}
        for (int j=0; j<preddata.size(); j++) {
 	  if (!pg.IsEdge(pg.GetVID(preddata[j]), todos[i] ) ) {
            cout<<"ERROR while testing GetPredVerticesDATA(VID,vector<VERTEX>&)/GetVID/IsEdge(VID,VID) (2): not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 11.3 GetPredVertices(3): GetPredVertices(VI,vector<VID>&) */
    MethodNames.push_back("GetPredVertices(3): GetPredVertices(VI,vector<VID>&)");
#ifdef VERBOSE
    stapl_print("Testing GetPredVertices(VI,vector<VID>&...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      pred.clear();
      starttimer;
      for (int i=0; i<todovis.size(); i++)  {
        pg.GetPredVertices(todovis[i], pred);
      }
      newstoptimer;
    }
    if_want_corr {
      pred.clear();
      for (int i=0; i<todovis.size(); i++) {
        pg.GetPredVertices(todovis[i], pred);
        if (!pg.IsVertex(*todovis[i]))  {
          cout<<"ERROR while testing GetPredVertices(VI,vector<VID>&)/IsVertex(VERTEX) (1): wrong input vis"<<endl;
	  break;
	}
        for (int j=0; j<pred.size(); j++) {
 	  if (!pg.IsEdge(pred[j],todovis[i]->vid) )  {
            cout<<"#"<<stapl::get_thread_id()<<" ERROR while testing GetPredVertices(VI,vector<VID>&)/IsEdge(VID,VID) (2): ("<<pred[j]<<","<<todovis[i]->vid<<") is not edge"<<endl;
	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 11.4 GetPredVertices(4): GetPredVerticesDATA(VI,vector<VERTEX>&) */
    MethodNames.push_back("GetPredVertices(4): GetPredVerticesDATA(VI,vector<VERTEX>&)");
#ifdef VERBOSE
    stapl_print("Testing GetPredVerticesDATA(VI, vector<VERTEX&>...) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all..");
#endif
    if_want_perf {
      preddata.clear(); 
      starttimer;
      for (int i=0; i<todovis.size(); i++)  {
        pg.GetPredVerticesDATA(todovis[i], preddata);
      }
      newstoptimer;
    }
    if_want_corr {
      preddata.clear(); 
      for (int i=0; i<todovis.size(); i++) {
        pg.GetPredVerticesDATA(todovis[i], preddata);
        if (!pg.IsVertex(*todovis[i]))  {
          cout<<"ERROR while testing GetPredVerticesDATA(VI,vector<VERTEX>&)/IsVertex(VERTEX&) (1): wrong input vis"<<endl;
	  break;
	}
        for (int j=0; j<preddata.size(); j++) {
 	  if (!pg.IsEdge(preddata[j], *todovis[i] ) )  {
            //cout<<"#"<<stapl::get_thread_id()<<" ERROR while testing GetPredVerticesDATA(VI,vector<VERTEX>&)/GetVID/IsEdge(VERTEX&,VERTEX&) (2): ("<<pg.GetVID(preddata[j])<<","<<todovis[i]->vid<<") is not edge"<<endl;
            cout<<" ERROR while testing GetPredVerticesDATA(VI,vector<VERTEX>&)/GetVID/IsEdge(VERTEX&,VERTEX&) (2): not edge"<<endl;
  	    break;
	  }
        } 
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
    //finish
    stapl_print("Tester_GetPredVerteices tested.\n");
#endif
  }
};

/* (x) Tester_GetOutgoingEdges: Tester for pGraph's methods GetOutgoingEdges/VData
 * must call AddVertex and AddEdge first
 * @v_num #of vertices to be worked on
 */
template <class PGRAPH>
class Tester_GetOutgoingEdges : public GraphTester<PGRAPH> {

public:

  Tester_GetOutgoingEdges() {flag='p';}

  char* GetName() { return "Tester_GetOutgoingEdges";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_GetOutgoingEdges *t = new Tester_GetOutgoingEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 4;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    vector<pair<VID,VID> > edges;
    vector<pair<pair<VID,VID>, Weight2> > edges_w;
    vector<pair<Task,Task> > edgedatas;
    vector<pair<pair<Task,Task>,Weight2> > edgedatas_w;
    vector<VID> todos;
    for (typename PGRAPH::VI vi=pg.local_begin(); vi!=pg.local_end(); ++vi) {
      todos.push_back(vi->vid);
    }

    counter = 0;
    /* 12.1 GetOutgoingEdges(1): GetOutgoingEdges(VID,vector<pair<VID,VID> > & ) */
    MethodNames.push_back("GetOutgoingEdges(1): GetOutgoingEdges(VID,vector<pair<VID,VID> > & )");
#ifdef VERBOSE
    stapl_print("Testing GetOutgoingEdges(VID,vector<pair<VID,VID> >&) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      edges.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdges(todos[i],edges);
      }
      newstoptimer;
    }
    if_want_corr {
      edges.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdges(todos[i],edges);
        if(tmp!=edges.size()) {
	  cout<<"ERROR while testing GetOutgoingEdges(1)"<<endl;
	  break;
	}
        if(tmp!=pg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdges/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
 	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!pg.IsEdge(edges[j].first, edges[j].second)) {
            cout<<"ERROR while testing GetOutgoingEdges(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif
 
    counter++;
    /* 12.2 GetOutgoingEdges(2): GetOutgoingEdges(VID,vector<pair<pair<VID,VID>,WEIGHT> > & ) */
    MethodNames.push_back("GetOutgoingEdges(2): GetOutgoingEdges(VID,vector<pair<pair<VID,VID>,WEIGHT> > & )");
#ifdef VERBOSE
    stapl_print("Testing GetOutgoingEdges(VID,vector<pair<pair<VID,VID>,WEIGHT> >&)  for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      edges_w.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdges(todos[i], edges_w);
      }
      newstoptimer;
    }
    if_want_corr {
      edges_w.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdges(todos[i], edges_w);
        if(tmp!=edges_w.size()) {
	  cout<<"ERROR while testing GetOutgoingEdges_w(1) tmp="<<tmp<<" edges_w.size()="<<edges_w.size()<<endl;
	  break;
	}
        if(tmp!=pg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdges_w/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!pg.IsEdge(edges_w[j].first.first, edges_w[j].first.second)) {
            cout<<"ERROR while testing GetOutgoingEdges(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 12.3 GetOutgoingEdges(3): GetOutgoingEdgesVData(VID,vector<pair<VERTEX,VERTEX> > & ) */
    MethodNames.push_back("GetOutgoingEdges(3): GetOutgoingEdgesVData(VID,vector<pair<VERTEX,VERTEX> > & )");
#ifdef VERBOSE
    stapl_print("Testing GetOutgoingEdgesVData(VID,vector<pair<VERTEX,VERTEX> >&) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      edgedatas.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdgesVData(todos[i], edgedatas);
      }
      newstoptimer;
    }
    if_want_corr {
      edgedatas.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdgesVData(todos[i], edgedatas);
        if(tmp!=edgedatas.size()) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData(1)"<<endl;
	  break;
	}
        if(tmp!=pg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!pg.IsEdge(edgedatas[j].first, edgedatas[j].second)) {
            cout<<"ERROR while testing GetOutgoingEdgesVData(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 12.4 GetOutgoingEdges(4): GetOutgoingEdgesVData(VID,vector<pair<pair<VERTEX,VERTEX>,WEIGHT> > & ) */
    MethodNames.push_back("GetOutgoingEdges(4): GetOutgoingEdgesVData(VID,vector<pair<pair<VERTEX,VERTEX>,WEIGHT> > & )");
#ifdef VERBOSE
    stapl_print("Testing GetOutgoingEdgesVData(VID,vector<pair<pair<VERTEX,VERTEX>,WEIGHT> >&) for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      edgedatas_w.clear();
      starttimer;
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdgesVData(todos[i], edgedatas_w);
      }
      newstoptimer;
    }
    if_want_corr {
      edgedatas_w.clear();
      for (int i=0; i<todos.size(); i++) {
        tmp = pg.GetOutgoingEdgesVData(todos[i], edgedatas_w);
        if(tmp!=edgedatas_w.size()) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData_w(1)"<<endl;
	  break;
	}
        if(tmp!=pg.GetVertexOutDegree(todos[i])) {
	  cout<<"ERROR while testing GetOutgoingEdgesVData_w/GetVertexOutDegree (2): wrong # of outgoing edges"<<endl;
	  break;
	}
        for (int j=0; j<tmp; j++) {
          if(!pg.IsEdge(edgedatas_w[j].first.first, edgedatas_w[j].first.second)) {
            cout<<"ERROR while testing GetOutgoingEdgesVData(3): not edge"<<endl;
	    break;
	  }
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_GetOutgoingEdges  tested.\n");
#endif
  }
};

/* (x) Tester_pGetCutEdges:Tester for pGraph's methods pGetCutEdges
 * must call AddVertex and AddEdge first
 */
template <class PGRAPH>
class Tester_pGetCutEdges : public GraphTester<PGRAPH> {

public:

  Tester_pGetCutEdges() {flag='p';}

  char* GetName() { return "Tester_pGetCutEdges";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_pGetCutEdges *t = new Tester_pGetCutEdges; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    GraphTester<PGRAPH>::SetTestNo();
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    set<pair<VID,VID> > cut;

    counter =0; 
    MethodNames.push_back("pGetCutEdges()");
 
#ifdef VERBOSE
    stapl_print("Testing pGetCutEdges() for ");
    stapl_print(pg.size()); stapl_print(" vertices and ");
    stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
#endif

    if_want_perf {
      cut.clear();
      starttimer;
      pg.pGetCutEdges(cut);
      newstoptimer;
    }
    if_want_corr {
      cut.clear();
      pg.pGetCutEdges(cut);
      for(set<pair<VID,VID> >::iterator sit = cut.begin();sit != cut.end();++sit){
	//cout<<"#"<<stapl::get_thread_id()<<" cut edges:("<<sit->first<<","<<sit->second<<")"<<endl;
        if(!pg.IsEdge(sit->first,sit->second)) {
          cout<<sit->first<<","<<sit->second<<" is not edge; ERROR while testing pGetCutEdges"<<endl;
	  break;
	}
/*
	if(!pg.IsDirected()) {
          if(!pg.IsEdge(sit->first,sit->second))
        //  if(!pg.IsEdge(sit->first,sit->second) && !pg.IsEdge(sit->second, sit->first) )
            cout<<sit->first<<","<<sit->second<<" is not edge; ERROR while testing pGetCutEdges"<<endl;
  	} else {
          if(!pg.IsEdge(sit->first,sit->second))
            cout<<sit->first<<","<<sit->second<<" is not edge; ERROR while testing pGetCutEdges"<<endl;
	}
*/
        if(pg.IsLocal(sit->second)){
          cout<<"ERROR while testing pGetCutEdges(2)"<<endl;
	  break; 
        }
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_pGetCutEdges tested.\n"); 
#endif
  }
};

/* (x) Tester_SetPredecessors:Tester for pGraph's methods ClearPredecessors, pSetPredecessors and pAsyncSetPredecessors
 * must call AddVertex and AddEdge first 
 */
template <class PGRAPH>
class Tester_SetPredecessors : public GraphTester<PGRAPH> {

public:

  Tester_SetPredecessors() {flag='p';}

  char* GetName() { return "Tester_SetPredecessors";}
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_SetPredecessors *t = new Tester_SetPredecessors; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
    GraphTester<PGRAPH>::ParseParams(s);
  }  

  void SetTestNo() {
    testno = 3; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

    counter = 0;
    /* 14.1 SetPredecessors(1): pSetPredecessors() */
    MethodNames.push_back("SetPredecessors(1): pSetPredecessors()");
#ifdef VERBOSE
    stapl_print("Testing pSetPredecessors for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg.ClearPredecessors();
      starttimer;
      pg.pSetPredecessors();
      newstoptimer;
    }
    if_want_corr {
      pg.ClearPredecessors();
      stapl::rmi_fence();
      pg.pSetPredecessors();
      stapl::rmi_fence();
      for(typename PGRAPH::VI vi = pg.local_begin(); vi != pg.local_end(); ++vi) {
        for(typename PGRAPH::EI ei = vi->predecessors.begin(); ei != vi->predecessors.end(); ++ei) {
          if(!pg.IsEdge(ei->vertex2id,vi->vid)) {
            cout<<"ERROR while testing pSetPredecessors: edge added incorrect"<<endl; 
	    break;
	  }
        }
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 14.2 SetPredecessors(2): ClearPredecessors() */
    MethodNames.push_back("SetPredecessors(2): ClearPredecessors()");
#ifdef VERBOSE
    stapl_print("Testing ClearPredecessors for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg.pSetPredecessors();
      starttimer;
      pg.ClearPredecessors();
      newstoptimer;
    }
    if_want_corr {
      pg.pSetPredecessors();
      stapl::rmi_fence();
      pg.ClearPredecessors();
      stapl::rmi_fence();
      for(typename PGRAPH::VI vi = pg.local_begin(); vi != pg.local_end(); ++vi) {
        if (vi->predecessors.size() != 0) {
	  cout<<"ERROR while testing ClearPredecessors: predecessors still exist"<<endl;
	  break;
	}
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
#endif

    counter++;
    /* 14.3 SetPredecessors(3): pAsyncSetPredecessors() */
    MethodNames.push_back("SetPredecessors(3): pAsyncSetPredecessors()");
#ifdef VERBOSE
    stapl_print("Testing pAsyncSetPredecessors for ");
    stapl_print(pg.size()); stapl_print(" vertices in all...");
#endif
    if_want_perf {
      pg.ClearPredecessors();
      starttimer;
      pg.pAsyncSetPredecessors();
      newstoptimer;
    }
    if_want_corr {
      pg.ClearPredecessors();
      stapl::rmi_fence();
      pg.pAsyncSetPredecessors();
      stapl::rmi_fence();
      stapl::rmi_fence();
      for(typename PGRAPH::VI vi = pg.local_begin(); vi != pg.local_end(); ++vi) {
        for(typename PGRAPH::EI ei = vi->predecessors.begin(); ei != vi->predecessors.end(); ++ei) {
          if(!pg.IsEdge(ei->vertex2id,vi->vid)) {
            cout<<"ERROR while testing pAsyncSetPredecessors: edge added incorrect"<<endl; 
	    break;
	  }
        }
      }
    }
    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n");
    //finish
    stapl_print("\nTester_SetPredecessors tested.\n"); 
#endif
  }
};


/* (18) Tester_SetPGraphwGraphTester for pGraph's method SetPGraphwGraph 
 * this method can be called directly, not depending on any other methods
 * which copy a sequential graph's structure into a pgraph, on each thread...
 * @v_num @deg  - the # of vertices and #edges/vertex of the sequential graph
 */
template <class PGRAPH>
class Tester_SetPGraphwGraph : public GraphTester<PGRAPH> {
  int v_num;
  int deg;

public:

  Tester_SetPGraphwGraph() {flag='p'; deg=4; v_num=10;}

  char* GetName() { return "Tester_SetPGraphwGraph"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_SetPGraphwGraph *t = new Tester_SetPGraphwGraph; t->flag=this->flag; t->deg=this->deg; t->v_num=this->v_num; return t; }

  void ParseParams(char* s) {
	//even if there's no "newgraph" here, pg is still cleared, since it should be changed to be like sg
    char *space = " ";
    char *p;
    flag = strtok(s,space)[0];
    if(p=strtok(NULL,space)) v_num=atoi(p);
    if(p=strtok(NULL,space)) deg = atoi(p);
    return;
  }  

  void Test(PGRAPH& pg) {
    int NP=stapl::get_num_threads();
    int tmp;

#ifdef VERBOSE
      stapl_print("Testing SetPGraphwGraph from a sequential grpah with  ");
      stapl_print(v_num); stapl_print(" vertices and ");
      stapl_print(deg); stapl_print(" edges/vertex... ");
#endif

  if(pg.IsDirected()) {
    //definition of local vars...
    DGRAPH sg;
    vector<VID> vs;
    map<PID,vector<VID> > parts;
    VID vd;
    Task datum;
    Weight2 wt;

    //local computations...
    for (int j=0; j<NP; j++) {
      for (int i=0; i<v_num; i++) {
        datum.SetWeight(i);
        vd = sg.AddVertex(datum);
	vs.push_back(vd);
      }
      parts[j] = vs;
    }
    for (int i=0; i<NP*v_num; i++) {
      for (int j=NP*v_num-1; j>=NP*v_num-deg; j--) {
	wt.SetWeight(i*j);
	sg.AddEdge(i,j,wt);
      } 
    }

    //main testing part...
    starttimer;
    pg.SetPGraphwGraph(sg, parts);
    stoptimer;

    //correctness check...
    if_want_corr {
      pg.CheckPGraphwGraph(sg);
    }
  } else {
    //definition of local vars...
    UGRAPH sg;
    vector<VID> vs;
    map<PID,vector<VID> > parts;
    VID vd;
    Task datum;
    Weight2 wt;

    //local computations...
    for (int j=0; j<NP; j++) {
      for (int i=0; i<v_num; i++) {
        datum.SetWeight(i);
        vd = sg.AddVertex(datum);
	vs.push_back(vd);
      }
      parts[j] = vs;
    }
    for (int i=0; i<NP*v_num; i++) {
      for (int j=NP*v_num-1; j>=NP*v_num-deg; j--) {
	wt.SetWeight(i*j);
	sg.AddEdge(i,j,wt);
      } 
    }

    //main testing part...
    starttimer;
    pg.SetPGraphwGraph(sg, parts);
    stoptimer;

    //correctness check...
    if_want_corr {
      pg.CheckPGraphwGraph(sg);
    }
  }

    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
    //finish
    stapl_print("Tester_SetGraphwGraph tested.\n");
#endif
  }

};

/* (unused) Tester_pFileRWTester for pGraph's methods pFileWrite and pFileRead
 * this testing depends on AddEdge and AddVertex - assume user will guarantee that
 * @v_num, @deg optional parameter - if the graph has no vertex currently, add v_num vertices and deg edges/vertex to the graph on each thread, first.
 */
template <class PGRAPH>
class Tester_pFileRW : public GraphTester<PGRAPH> {

public:

  Tester_pFileRW() {flag='p';}
  Tester_pFileRW(char _f) {flag=_f; }

  char* GetName() { return "Tester_pFileRW"; }
  
  GraphTester<PGRAPH> *CreateCopy() { Tester_pFileRW *t = new Tester_pFileRW; t->flag=this->flag; return t; }

  void ParseParams(char* s) {
	//the incoming string is such as: "n newgraph": neither performance nor correctness & work on newgraph(so no vertex at all...) 
	//OR"p": report performance & work on current graph
    char *space = " ";
    char *p;
    flag = strtok(s,space)[0];
    newgraph=false;
    if(!(p=strtok(NULL,space))) return;
    if (strcmp(p, "newgraph")==0) newgraph=true; 
  }  

  void Test(PGRAPH& pg) {
    if(newgraph) {PGRAPH* newpg = new PGRAPH(); pg=*newpg; }
    stapl::rmi_fence();

    int NP=stapl::get_num_threads();
    int tmp;
    timer tv; double elapsed;

    if_not_none {
      stapl_print("Testing pFileWrite for a pgraph with ");
      stapl_print(pg.size()); stapl_print(" vertices and ");
      stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
      if(pg.size()==0) {stapl_print(" no vertex...OK\n"); return;}
    }

    //no definition of local vars
    //no local computations

    starttimer;
    pg.pFileWrite("test_pgraph.out", 15);
    stoptimer;

    if_not_none {
      stapl_print("OK\n");
      stapl_print("Testing pFileRead for a pgraph with ");
      stapl_print(pg.size()); stapl_print(" vertices and ...");
      stapl_print(pg.GetEdgeCount()); stapl_print(" edges in all...");
    }
  
    //no definition of local vars
    //no local computations

    //main testing part...
    PGRAPH pg1;
    starttimer;
    pg1.pFileRead("test_pgraph.out", 15);
    stoptimer;

    //correctness check...: the 2 methods should return same vertices
    if_want_corr {
      //compare pg1 with pg;
      typename PGRAPH::VI vi1 = pg.begin();
      typename PGRAPH::VI vi2 = pg1.begin();
      for(;vi1 != pg.end();++vi1,++vi2){
        if(vi1->vid != vi2->vid)  {
	  cout<<"ERROR while testing read from file(vid)"<<endl;
	  break;
	}
        if(vi1->data.GetWeight() != vi2->data.GetWeight()) {
          cout<<"ERROR while testing read from file(data)"<<endl;
	  break;
	}
        //testing edges
        typename PGRAPH::EI ei1 = vi1->edgelist.begin();
        typename PGRAPH::EI ei2 = vi2->edgelist.begin();
        for(;ei1 != vi1->edgelist.end();++ei1,++ei2){
          if(ei1->vertex2id != ei2->vertex2id) {
            cout<<"ERROR while testing read from file(edges vids)"<<endl;
	    break;
	  }
          if(ei1->weight.GetWeight() != ei2->weight.GetWeight()) {
            cout<<"ERROR while testing read from file(edges data)"<<endl;
	    break;
	  }
        }
      }
    }

    stapl::rmi_fence();
#ifdef VERBOSE
    stapl_print("Done.\n"); 
    //finish
    stapl_print("Tester_pFileRW tested.\n");
#endif
  }

};
