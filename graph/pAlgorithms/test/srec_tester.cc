#include "srec_tester.h"
#include <assert.h>
#include "runtime.h"

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
    for(int i=0; i<AllTesters.size(); i++) delete(AllTesters[i]);
    for(int i=0; i<SelectedTesters.size(); i++) delete(SelectedTesters[i]);
  }

  void SetAllTesters() {
    Tester_Recurrence* t0 = new Tester_Recurrence();
    AllTesters.push_back(t0);
  }

  void ReadSelectedTesters(int argc, char** argv) {
    int i=1;
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
        for(int i=0; i<REPEAT; i++) {
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
  if (stapl::get_num_threads() != 1) {
    cout<<" This is sequential Tester, must run on only 1 thread!"<<endl;
    exit(1);
  }

  //read in argvs if "seqtest -testall 50 " 
  char *p; char *space=" ";
  p=strtok(argv[1], space);
  if (strcmp(p,"-testall")==0) 
    flag=true;

  TestManager tm;
  if (flag) {
    char* n_elet = argv[2]; argc=4; argv[1]="Tester_Recurrence"; argv[2]="b"; argv[3]=n_elet;
  };
  tm.ReadSelectedTesters(argc, argv);
  tm.ExecuteSelectedTesters();
}
