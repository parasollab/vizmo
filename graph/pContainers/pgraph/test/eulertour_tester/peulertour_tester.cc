/* tester main file for testing parallel Euler Trou algo
 * to use it to test 
 * mpirun -np ? pET_Tester 10(aggregation) Tester_pET {b|p|c} v_num [startvid]
 * or simply use
 * mpirun -np ? pET_Testers 10(aggregation) -testall v_num [startvid]
 */
#include "peulertour_tester.h"
#include <assert.h>

using namespace stapl;

template <class PTREE>
class TestManager {
  vector<ETTester<PTREE>*> AllTesters;
  vector<ETTester<PTREE>*> SelectedTesters;
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
    Tester_pET<PTREE>* t1 = new Tester_pET<PTREE>();
    AllTesters.push_back(t1);
  }

  void ReadSelectedTesters(int argc, char** argv) {
#ifdef VERBOSE
    stapl_print("TestManager ReadSelectedTesters called...\n");
#endif
    int i=2;
    while (i<argc) {
      if (strstr(argv[i], "Tester_")!=0 ) {	//meet a new tester
        ETTester<PTREE>* currenttesterptr;
        for(int j=0; j<AllTesters.size(); j++) {
          currenttesterptr = AllTesters[j];
          if(strcmp(argv[i], currenttesterptr->GetName())==0) break;  //find the tester's name in AllTesters vector
        } 
        ETTester<PTREE>* testercopy = currenttesterptr->CreateCopy(); //create a copy for the tester
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
  
  void ExecuteSelectedTesters(PTREE& ptree) {
    typename vector<ETTester<PTREE>*>::iterator it;
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...\n");
#endif
    PTREE newptree;
    for (it=SelectedTesters.begin(); it!=SelectedTesters.end(); ++it) {
      (*it)->SetTestNo();
      newptree = ptree;
      stapl::rmi_fence();
      for(int i=0; i<REPEAT; i++) {
 	ptree = newptree;	
	(*it)->Test(ptree);     
        stapl::rmi_fence();
      }
      newptree = ptree;
      stapl::rmi_fence();
      (*it)->Print_Statistics(REPEAT);
      stapl::rmi_fence();
    }
#ifdef VERBOSE
    stapl_print("TestManager ExecuteSelectedTesters called...End\n");
#endif
  }

  void ExecuteAllTesters(PTREE& ptree, int v_num, VID startvid) {
    vector<ETTester<PTREE>* > ts;
    ts.push_back(new Tester_pET<PTREE>(v_num, startvid));
    
    PTREE newptree;
    typename vector<ETTester<PTREE>* >::iterator it;
    for (it=ts.begin(); it!=ts.end(); ++it) {
      (*it)->SetQuiet(false);
      (*it)->SetTestNo();
      stapl::rmi_fence();
      newptree = ptree;
      for(int i=0; i<REPEAT; i++) {
        ptree = newptree;
        stapl::rmi_fence();
        (*it)->Test(ptree);
        stapl::rmi_fence();
      }
      newptree = ptree;
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
  stapl_print("testing default pET  constructor...");
#endif
  PTREE ptree;
  stapl::rmi_fence();
#ifdef VERBOSE
  stapl_print("Passed\n");
#endif

  bool flag = false;
  int v_num, startvid;
  //read in argvs if "test_funcs -testall 50 2" 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
  int agg = atoi(p);
  //cout<<"agg="<<agg<<endl;
  stapl::set_aggregation(agg);
  p=strtok(argv[2], space);
  if (strcmp(p,"-testall")==0) {
    if (argc < 4 ) { cout<<"To test all methods, run : test_funcs agg -testall #vertex [startvid]"<<endl; return; }
    p=strtok(argv[3], space);
    if(!p) { cout<<"To test all methods, run : test_funcs -testall #vertex startvid"<<endl; return; }
    v_num = atoi(p);
    if (v_num%stapl::get_num_threads()!=0) { cout<<" Notice: we assume v_num%nprocs is 0 "<<endl; return; }
    if (p=strtok(argv[4], space) ) {
      startvid = atoi(p);
      if (startvid<0 || startvid>=v_num) { cout<<"startvid is invalid"<<endl; return; }
    } else startvid = -1;
    flag = true;
  }

  
  TestManager<PTREE> tm;
  if (flag) {
    tm.ExecuteAllTesters(ptree,v_num, startvid);
  } else  {
    tm.ReadSelectedTesters(argc, argv);
    tm.ExecuteSelectedTesters(ptree);
  }
}
