/* tester main file for testing basic functions of pGraph
 * to use it to test methodA, methodB:
 * mpirun -np ? pGraph_Testers 10(aggregation) Tester_A [b|p|c] {args for A} Tester_B [b|p|c] {args for B}
 * to use it to call all implemented testers:
 * mpirun -np ? pGraph_Testers 10(aggregation) -testall Vnum deg
 */
#include "pgraph_tester.h"
#include <assert.h>

using namespace stapl;

template <class PGRAPH>
class TestManager {
  vector<GraphTester<PGRAPH>*> AllTesters;
  vector<GraphTester<PGRAPH>*> SelectedTesters;
public:
  TestManager() {
#ifdef VERBOSE
    stapl_print("TestManager constructor called...\n");
#endif
    AllTesters.clear();
    SelectedTesters.clear();
    SetAllTesters();
  }

  ~TestManager() {
#ifdef VERBOSE
    stapl_print("TestManager deconstructor called...\n");
#endif
    for(int i=0; i<AllTesters.size(); i++) delete(AllTesters[i]);
    for(int i=0; i<SelectedTesters.size(); i++) delete(SelectedTesters[i]);
  }

  void SetAllTesters() {
    Tester_Simple<PGRAPH>* t0 = new Tester_Simple<PGRAPH>();
    AllTesters.push_back(t0);
    Tester_AddVertex<PGRAPH>* t1 = new Tester_AddVertex<PGRAPH>();
    AllTesters.push_back(t1);
    Tester_DeleteVertex<PGRAPH>* t2 = new Tester_DeleteVertex<PGRAPH>();
    AllTesters.push_back(t2);
    Tester_IsVertex<PGRAPH>* t3 = new Tester_IsVertex<PGRAPH>();
    AllTesters.push_back(t3);
    Tester_GetVertices<PGRAPH>* t4 = new Tester_GetVertices<PGRAPH>();
    AllTesters.push_back(t4);
    Tester_AddEdge<PGRAPH>* t5 = new Tester_AddEdge<PGRAPH>();
    AllTesters.push_back(t5);
    Tester_DeleteEdge<PGRAPH>* t6 = new Tester_DeleteEdge<PGRAPH>();
    AllTesters.push_back(t6);
    Tester_IsEdge<PGRAPH>* t7 = new Tester_IsEdge<PGRAPH>();
    AllTesters.push_back(t7);
    Tester_GetAllEdges<PGRAPH>* t8 = new Tester_GetAllEdges<PGRAPH>();
    AllTesters.push_back(t8);
    Tester_GetEdges<PGRAPH>* t9 = new Tester_GetEdges<PGRAPH>();
    AllTesters.push_back(t9);
    Tester_GetAdjacentVertices<PGRAPH>* t10 = new Tester_GetAdjacentVertices<PGRAPH>();
    AllTesters.push_back(t10);
    Tester_GetPredVertices<PGRAPH>* t11 = new Tester_GetPredVertices<PGRAPH>();
    AllTesters.push_back(t11);
    Tester_GetOutgoingEdges<PGRAPH>* t12 = new Tester_GetOutgoingEdges<PGRAPH>();
    AllTesters.push_back(t12);
    Tester_pGetCutEdges<PGRAPH>* t13 = new Tester_pGetCutEdges<PGRAPH>();
    AllTesters.push_back(t13);
    Tester_SetPredecessors<PGRAPH>* t14 = new Tester_SetPredecessors<PGRAPH>();
    AllTesters.push_back(t14);
    Tester_SetGetVertexField<PGRAPH>* const t15 = new Tester_SetGetVertexField<PGRAPH>();
    AllTesters.push_back(t15);
    Tester_SetGetWeightField<PGRAPH>* const t16 = new Tester_SetGetWeightField<PGRAPH>();
    AllTesters.push_back(t16);
    Tester_SetGetWeightFieldEdgeId<PGRAPH>* const t17 = new Tester_SetGetWeightFieldEdgeId<PGRAPH>();
    AllTesters.push_back(t17);
/*
    Tester_SetPGraphwGraph<PGRAPH>* t18 = new Tester_SetPGraphwGraph<PGRAPH>();
    AllTesters.push_back(t18);
*/
  }

  void ReadSelectedTesters(int argc, char** argv) {
#ifdef VERBOSE
    stapl_print("TestManager ReadSelectedTesters called...\n");
#endif
    int i=2;
    while (i<argc) {
      if (strstr(argv[i], "Tester_")!=0 ) {	//meet a new tester
        GraphTester<PGRAPH>* currenttesterptr;
        for(int j=0; j<AllTesters.size(); j++) {
          currenttesterptr = AllTesters[j];
          if(strcmp(argv[i], currenttesterptr->GetName())==0) break;  //find the tester's name in AllTesters vector
        } 
        GraphTester<PGRAPH>* testercopy = currenttesterptr->CreateCopy(); //create a copy for the tester
        i++;
  	char empty[30]="";
        char *paramstring;
        while ( (i<argc) && (strstr(argv[i], "Tester_")==0 ) ){
          paramstring = strcat(empty, argv[i]);
          paramstring = strcat(empty, " ");
  	  i++; 
        }
        testercopy->ParseParams(paramstring);
        SelectedTesters.push_back(testercopy);  
      }
    }
#ifdef VERBOSE
    stapl_print("TestManager ReadSelectedTesters called...End\n");
#endif
  }
  
  void ExecuteSelectedTesters(PGRAPH& pg) {
    typename vector<GraphTester<PGRAPH>*>::iterator it;
    PDGRAPH newpg;
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...\n");
#endif
    for (it=SelectedTesters.begin(); it!=SelectedTesters.end(); ++it) {
	(*it)->SetTestNo();
        stapl::rmi_fence();
	newpg = pg;
      for(int i=0; i<REPEAT; i++) {
	pg = newpg;
	(*it)->Test(pg);     
        stapl::rmi_fence();
      }
	newpg = pg;
        stapl::rmi_fence();
        (*it)->Print_Statistics(REPEAT);
        stapl::rmi_fence();
    }
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...End\n");
#endif
  }

  void ExecuteAllTesters(PGRAPH& pg, int v_num, int deg) {
    vector<GraphTester<PGRAPH>* > ts;
    ts.push_back(new Tester_AddVertex<PGRAPH>(v_num));
    ts.push_back(new Tester_AddEdge<PGRAPH>(deg));
    ts.push_back(new Tester_Simple<PGRAPH>());
    ts.push_back(new Tester_GetVertices<PGRAPH>());
//    ts.push_back(new Tester_IsVertex<PGRAPH>());
//    ts.push_back(new Tester_IsEdge<PGRAPH>());
    ts.push_back(new Tester_GetEdges<PGRAPH>());
    ts.push_back(new Tester_GetAllEdges<PGRAPH>());
//    ts.push_back(new Tester_GetAdjacentVertices<PGRAPH>());
//    ts.push_back(new Tester_SetPredecessors<PGRAPH>());
//    ts.push_back(new Tester_GetPredVertices<PGRAPH>());
//    ts.push_back(new Tester_GetOutgoingEdges<PGRAPH>());
//    ts.push_back(new Tester_pGetCutEdges<PGRAPH>());
    ts.push_back(new Tester_SetGetVertexField<PGRAPH>());
    ts.push_back(new Tester_SetGetWeightField<PGRAPH>());
    ts.push_back(new Tester_SetGetWeightFieldEdgeId<PGRAPH>());
    ts.push_back(new Tester_DeleteEdge<PGRAPH>());
    ts.push_back(new Tester_DeleteVertex<PGRAPH>());
    
    PDGRAPH newpg;
    typename vector<GraphTester<PGRAPH>* >::iterator it;
    for (it=ts.begin(); it!=ts.end(); ++it) {
      (*it)->SetQuiet(false);
      (*it)->SetTestNo();
      stapl::rmi_fence();
      newpg = pg;
      for(int i=0; i<REPEAT; i++) {
        pg=newpg;
        stapl::rmi_fence();
        (*it)->Test(pg);
        stapl::rmi_fence();
     }
      newpg = pg;
      stapl::rmi_fence();
      (*it)->Print_Statistics(REPEAT);
      stapl::rmi_fence();
stapl_print("==============\n");
    }
    for(int i=0; i<ts.size(); i++) delete(ts[i]);
  }
};


//int SZ;
void stapl_main(int argc, char** argv) {
#ifdef VERBOSE
  stapl_print("testing default pGraph  constructor...");
#endif
  PDGRAPH pg;
  PDGRAPH::VI vi;
  stapl::rmi_fence();
#ifdef VERBOSE
  stapl_print("Passed\n");

  stapl_print("testing pDGraph properties...");
#endif
//  stapl_assert(!pg.IsDirected(),"is directed failed");
  stapl_assert(pg.IsMulti(),"is multi failed");
  stapl_assert(pg.IsWeighted(),"is weighted failed");
#ifdef VERBOSE
  stapl_print("Passed\n");

  stapl_print("testing empty, local_empty...");
#endif
  if (!pg.empty()) cout<<"ERROR while testing empty"<<endl;
  if (!pg.local_empty()) cout<<"ERROR while testing local_empty"<<endl;

  bool flag = false;
  int v_num, deg;
  //read in argvs if "test_funcs -testall 50 2" 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
  int agg = atoi(p);
cout<<"agg="<<agg<<endl;
  stapl::set_aggregation(agg);
  p=strtok(argv[2], space);
  if (strcmp(p,"-testall")==0) {
    if (argc < 5 ) { cout<<"To test all methods, run : test_funcs agg -testall #vertex #deg"<<endl; return; }
    p=strtok(argv[3], space);
    if(!p) { cout<<"To test all methods, run : test_funcs -testall #vertex #deg"<<endl; return; }
    v_num = atoi(p);
    p=strtok(argv[4], space);
    if(!p) { cout<<"To test all methods, run : test_funcs -testall #vertex #deg"<<endl; return; }
    deg = atoi(p);
    flag = true;
cout<<"flag="<<flag<<endl;
  }

  TestManager<PDGRAPH> tm;
  if (flag) {
    tm.ExecuteAllTesters(pg,v_num, deg);
  } else  {
    tm.ReadSelectedTesters(argc, argv);
    tm.ExecuteSelectedTesters(pg);
  }

}
