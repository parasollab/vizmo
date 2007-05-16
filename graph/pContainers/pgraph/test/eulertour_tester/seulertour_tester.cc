/* tester main file for testing seq Euler Trou algo
 * to use it to test 
 * mpirun -np 1 sET_Tester  Tester_sET {b|p|c} v_num [startvid]
 * or simply use
 * mpirun -np 1 sET_Testers -testall v_num [startvid]
 */
#include "seulertour_tester.h"
#include <assert.h>

using namespace stapl;

template <class TREE>
class TestManager {
  vector<ETTester<TREE>*> AllTesters;
  vector<ETTester<TREE>*> SelectedTesters;
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
    Tester_sET<TREE>* t1 = new Tester_sET<TREE>();
    AllTesters.push_back(t1);
  }

  void ReadSelectedTesters(int argc, char** argv) {
#ifdef VERBOSE
    stapl_print("TestManager ReadSelectedTesters called...\n");
#endif
    int i=1;
    while (i<argc) {
      if (strstr(argv[i], "Tester_")!=0 ) {	//meet a new tester
        ETTester<TREE>* currenttesterptr;
        for(int j=0; j<AllTesters.size(); j++) {
          currenttesterptr = AllTesters[j];
          if(strcmp(argv[i], currenttesterptr->GetName())==0) break;  //find the tester's name in AllTesters vector
        } 
        ETTester<TREE>* testercopy = currenttesterptr->CreateCopy(); //create a copy for the tester
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
  
  void ExecuteSelectedTesters(TREE& tree) {
    typename vector<ETTester<TREE>*>::iterator it;
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...\n");
#endif
    TREE newtree;
    for (it=SelectedTesters.begin(); it!=SelectedTesters.end(); ++it) {
      (*it)->SetTestNo();
      newtree = tree;
      for(int i=0; i<REPEAT; i++) {
 	tree = newtree;	
	(*it)->Test(tree);     
      }
      newtree = tree;
      (*it)->Print_Statistics(REPEAT);
    }
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...End\n");
#endif
  }

  void ExecuteAllTesters(TREE& tree, int v_num, int startvid) {
    vector<ETTester<TREE>* > ts;
    ts.push_back(new Tester_sET<TREE>(v_num, startvid));
    
    TREE newtree;
    typename vector<ETTester<TREE>* >::iterator it;
    for (it=ts.begin(); it!=ts.end(); ++it) {
      (*it)->SetQuiet(false);
      (*it)->SetTestNo();
      stapl::rmi_fence();
      newtree = tree;
      for(int i=0; i<REPEAT; i++) {
        tree = newtree;
        (*it)->Test(tree);
      }
      newtree = tree;
      stapl::rmi_fence();
      (*it)->Print_Statistics(REPEAT);
      stapl::rmi_fence();
      stapl_print("==============\n");
    }
    for(int i=0; i<ts.size(); i++) delete(ts[i]);
  }
};


void stapl_main(int argc, char** argv) {
#ifdef VERBOSE
  stapl_print("testing default sET  constructor...");
#endif
  TREE tree;
  stapl::rmi_fence();
#ifdef VERBOSE
  stapl_print("Passed\n");
#endif

  bool flag = false;
  int v_num, startvid;
  //read in argvs if "test_funcs -testall 50 2" 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
  if (strcmp(p,"-testall")==0) {
    if (argc < 4 ) { cout<<"To test all methods, run : exe -testall #vertex startvid"<<endl; return; }
    p=strtok(argv[2], space);
    if(!p) { cout<<"To test all methods, run : exe -testall #vertex startvid"<<endl; return; }
    v_num = atoi(p);
    if (p = strtok(argv[3], space) ) {
      startvid = atoi(p);
      if (startvid<0 || startvid>=v_num) { cout<<"startvid is invalid"<<endl; return; }
    } else startvid = -1;
    flag = true;
  }

  
  TestManager<TREE> tm;
  if (flag) {
    tm.ExecuteAllTesters(tree,v_num, startvid);
  } else  {
    tm.ReadSelectedTesters(argc, argv);
    tm.ExecuteSelectedTesters(tree);
  }
}
