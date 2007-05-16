/* testing basic methods of sequential graph
 * to test methodA and methodB,
 * mpirun -np 1 sGraph_Testers Tester_A {b|p|c} [args for A] Tester_B {b|p|c} [args for B]
 * or to test all:
 * mpirun -np 1 sGraph_Testers -testall [args for all if needed]
 */
#include "sgraph_tester.h"
#include <assert.h>

using namespace stapl;

template <class GRAPH>
class TestManager {
  vector<GraphTester<GRAPH>*> AllTesters;
  vector<GraphTester<GRAPH>*> SelectedTesters;
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
    Tester_Simple<GRAPH>* t0 = new Tester_Simple<GRAPH>();
    AllTesters.push_back(t0);
    Tester_AddVertex<GRAPH>* t1 = new Tester_AddVertex<GRAPH>();
    AllTesters.push_back(t1);
    Tester_DeleteVertex<GRAPH>* t2 = new Tester_DeleteVertex<GRAPH>();
    AllTesters.push_back(t2);
    Tester_IsVertex<GRAPH>* t3 = new Tester_IsVertex<GRAPH>();
    AllTesters.push_back(t3);
    Tester_GetVertices<GRAPH>* t4 = new Tester_GetVertices<GRAPH>();
    AllTesters.push_back(t4);
    Tester_AddEdge<GRAPH>* t5 = new Tester_AddEdge<GRAPH>();
    AllTesters.push_back(t5);
    Tester_DeleteEdge<GRAPH>* t6 = new Tester_DeleteEdge<GRAPH>();
    AllTesters.push_back(t6);
    Tester_IsEdge<GRAPH>* t7 = new Tester_IsEdge<GRAPH>();
    AllTesters.push_back(t7);
    Tester_GetEdges<GRAPH>* t8 = new Tester_GetEdges<GRAPH>();
    AllTesters.push_back(t8);
    Tester_SetGetVertexField<GRAPH>* t9 = new Tester_SetGetVertexField<GRAPH>();
    AllTesters.push_back(t9);
    Tester_SetGetWeightField<GRAPH>* t10 = new Tester_SetGetWeightField<GRAPH>();
    AllTesters.push_back(t10);
    Tester_SetGetWeightFieldEdgeId<GRAPH>* t11 = new Tester_SetGetWeightFieldEdgeId<GRAPH>();
    AllTesters.push_back(t11);
    Tester_SetPredecessors<GRAPH>* t14 = new Tester_SetPredecessors<GRAPH>();
    AllTesters.push_back(t14);
/*
    Tester_GetAdjacentVertices<GRAPH>* t10 = new Tester_GetAdjacentVertices<GRAPH>();
    AllTesters.push_back(t10);
    Tester_GetPredecessors<GRAPH>* t11 = new Tester_GetPredecessors<GRAPH>();
    AllTesters.push_back(t11);
    Tester_GetOutgoingEdges<GRAPH>* t12 = new Tester_GetOutgoingEdges<GRAPH>();
    AllTesters.push_back(t12);
*/
  }

  void ReadSelectedTesters(int argc, char** argv) {
#ifdef VERBOSE
    stapl_print("TestManager ReadSelectedTesters called...\n");
#endif
    int i=1;
    while (i<argc) {
      if (strstr(argv[i], "Tester_")!=0 ) {	//meet a new tester
        GraphTester<GRAPH>* currenttesterptr;
        for(int j=0; j<AllTesters.size(); j++) {
          currenttesterptr = AllTesters[j];
          if(strcmp(argv[i], currenttesterptr->GetName())==0) break;  //find the tester's name in AllTesters vector
        } 
        GraphTester<GRAPH>* testercopy = currenttesterptr->CreateCopy(); //create a copy for the tester
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
  
  void ExecuteSelectedTesters(GRAPH& sg) {
    typename vector<GraphTester<GRAPH>*>::iterator it;
    GRAPH newsg;
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...\n");
#endif
    for (it=SelectedTesters.begin(); it!=SelectedTesters.end(); ++it) {
        (*it)->SetTestNo();
	newsg = sg;
        for(int i=0; i<REPEAT; i++) {
	  sg = newsg;
	  (*it)->Test(sg);     
	}
	newsg = sg;
 	(*it)->Print_Statistics(REPEAT);
    }
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...End\n");
#endif
  }

  void ExecuteAllTesters(GRAPH& sg, int v_num, int deg) {
    vector<GraphTester<GRAPH>* > ts;
    ts.push_back(new Tester_AddVertex<GRAPH>(v_num));
    ts.push_back(new Tester_AddEdge<GRAPH>(deg));
    ts.push_back(new Tester_Simple<GRAPH>());
    ts.push_back(new Tester_GetVertices<GRAPH>());
    ts.push_back(new Tester_IsVertex<GRAPH>());
    ts.push_back(new Tester_IsEdge<GRAPH>());
    ts.push_back(new Tester_GetEdges<GRAPH>());
    ts.push_back(new Tester_SetGetVertexField<GRAPH>());
    ts.push_back(new Tester_SetGetWeightField<GRAPH>());
    ts.push_back(new Tester_SetGetWeightFieldEdgeId<GRAPH>());
//ug has:    ts.push_back(new Tester_GetAdjacentVertices<GRAPH>());
//dg has
    ts.push_back(new Tester_GetPredecessors<GRAPH>());
//dg has
    ts.push_back(new Tester_SetPredecessors<GRAPH>());
/*
if(sg.IsDirected())
    ts.push_back(new Tester_GetOutgoingEdges<GRAPH>());
*/
    ts.push_back(new Tester_DeleteEdge<GRAPH>());
    ts.push_back(new Tester_DeleteVertex<GRAPH>());
    
    typename vector<GraphTester<GRAPH>* >::iterator it;
    GRAPH newsg;
    for (it=ts.begin(); it!=ts.end(); ++it) {
      (*it)->SetTestNo();
      newsg = sg;
      for(int i=0; i<REPEAT; i++) {
  	sg = newsg;
        (*it)->Test(sg);
      }
      newsg = sg;
      (*it)->Print_Statistics(REPEAT);
    }
  }
};


//int SZ;
void stapl_main(int argc, char** argv) {

#ifdef VERBOSE
  stapl_print("testing default pGraph  constructor...");
#endif
  DGRAPH sg;
  DGRAPH::VI vi;
#ifdef VERBOSE
  stapl_print("Passed\n");

  stapl_print("testing pDGraph properties...");
#endif
//  stapl_assert(!sg.IsDirected(),"is directed failed");
  stapl_assert(sg.IsMulti(),"is multi failed");
  stapl_assert(sg.IsWeighted(),"is weighted failed");
  stapl_print("Passed\n");

  stapl_print("testing empty...");
  //if (!sg.empty()) cout<<"ERROR while testing empty"<<endl;

  bool flag = false;
  int v_num, deg;
  //read in argvs if "test_funcs -testall 50 2" 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
//  cout<<"p="<<p<<endl;
  if (strcmp(p,"-testall")==0) {
    if (argc < 4 ) { cout<<"To test all methods, run : test_funcs -testall #vertex #deg"<<endl; }//return -1; }
    p=strtok(argv[2], space);
    if(!p) { cout<<"To test all methods, run : test_funcs -testall #vertex #deg"<<endl; }//return -1; }
//    cout<<"p="<<p<<endl;
    v_num = atoi(p);
    p=strtok(argv[3], space);
    if(!p) { cout<<"To test all methods, run : test_funcs -testall #vertex #deg"<<endl;}// return -1; }
//    cout<<"p="<<p<<endl;
    deg = atoi(p);
    flag = true;
 //   cout<<"v_num="<<v_num<<" deg="<<deg<<endl;
  }

  TestManager<DGRAPH> tm;
  if (flag) {
    tm.ExecuteAllTesters(sg,v_num, deg);
  } else  {
    tm.ReadSelectedTesters(argc, argv);
    tm.ExecuteSelectedTesters(sg);
  }
}
