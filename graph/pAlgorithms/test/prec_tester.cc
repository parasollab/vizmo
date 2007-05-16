#include "prec_tester.h"
#include <assert.h>
#include <runtime.h>

using namespace stapl;

class TestManager {
  vector<BaseTester*> AllTesters;
  vector<BaseTester*> SelectedTesters;
public:
  TestManager() {
    AllTesters.clear();
    SelectedTesters.clear();
    SetAllTesters();
  }

  ~TestManager() {
    for(int i=0; i<SelectedTesters.size(); i++) delete(SelectedTesters[i]);
    for(int i=0; i<AllTesters.size(); i++) delete(AllTesters[i]);
  }


  void SetAllTesters() {
    Tester_Recurrence1* t1 = new Tester_Recurrence1();
    Tester_Recurrence2* t2 = new Tester_Recurrence2();
    AllTesters.push_back(t1);
    AllTesters.push_back(t2);
  }

  void ReadSelectedTesters(int argc, char** argv) {
    int i=2;
    while (i<argc) {
      if (strstr(argv[i], "Tester_")!=0 ) {	//meet a new tester
        BaseTester* currenttesterptr;
        for(int j=0; j<AllTesters.size(); j++) {
          currenttesterptr = AllTesters[j];
          if(strcmp(argv[i], currenttesterptr->GetName())==0) break;  //find the tester's name in AllTesters vector
        } 
        BaseTester* testercopy = currenttesterptr->CreateCopy(); //create a copy for the tester
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
  }
  
  void ExecuteSelectedTesters() {
    vector<BaseTester*>::iterator it;
    for (it=SelectedTesters.begin(); it!=SelectedTesters.end(); ++it) {
      (*it)->SetTestNo();
      stapl::rmi_fence();
      for (int i=0; i<REPEAT; i++) {
	(*it)->Test();     
        stapl::rmi_fence();
      }
      (*it)->Print_Statistics(REPEAT);
      stapl::rmi_fence();
    }
  }
};


void stapl_main(int argc, char** argv) {

  bool flag = false;

  //read in argvs if "test_funcs 10 -testall 50 " 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
  int agg = atoi(p);
  cout<<"agg="<<agg<<endl;
  stapl::set_aggregation(agg);
  p=strtok(argv[2], space);
  if (strcmp(p,"-testall")==0) 
    flag = true;

  TestManager tm;
  if (flag) {
    char* n_elet = argv[3]; argc = 8; argv[2]="Tester_Recurrence1";  argv[3]="p"; argv[4]=n_elet; argv[5]="Tester_Recurrence2"; argv[6]="p"; argv[7]=n_elet;
    //for(int i=0; i<argc; i++) cout<<"argv["<<i<<"]="<<argv[i]<<endl;
  } 
  tm.ReadSelectedTesters(argc, argv);
  tm.ExecuteSelectedTesters();
}
