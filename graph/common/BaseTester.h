#include <iostream>
#include <math.h>
#include <iomanip>
#include "Defines.h"
#include "common/rmitools.h"
#include "runtime.h"

using namespace stapl;

#define REPEAT 2

#define starttimer {stapl::rmi_fence(); if (flag=='p' || flag=='b') tv=start_timer(); }
#define stoptimer { stapl::rmi_fence(); if (flag=='p' || flag=='b') { \
	elapsed=stop_timer(tv); ElapsedTimes[counter].push_back(elapsed); \
         }}
#define if_not_none if(quiet==false)
#define if_want_perf if(flag=='p' || flag=='b')
#define if_want_corr if(flag=='c' || flag=='b')

void stapl_print(const char* s) {
  if (stapl::get_thread_id() == 0)
    cout<<s<<flush;
}

void stapl_print(double e) {
  if (stapl::get_thread_id() == 0)
    cout<<e<<"seconds"<<flush;
}

void stapl_report(const char* s) {
  if (stapl::get_thread_id()==0) {
    if (s[0]=='+') // +/-...
      cout<<s;
    else if (s[0]=='f') //for 95%...
      cout<<" "<<s<<" ";
    else
      cout<<s<<" ";
  }
}

void stapl_report(double e) {
  if (stapl::get_thread_id()==0) 
    cout<<e<<"\t";
}


class BaseTester {
public:
  char flag; //what are you interested in? performance? correctness? neither? or both?
  bool quiet;

  int counter, testno;
  vector<char*> MethodNames;
  vector<vector<double> > ElapsedTimes;

  timer tv; double elapsed;

  virtual char* GetName() {}

  virtual BaseTester* CreateCopy() {}
  
  virtual void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n");
    return;
  }

  virtual void SetQuiet( bool _f) {
    quiet = _f; 
  }

  virtual void ParseParams(char * s) {
    char *space = " "; char *p;

    if(!(p=strtok(s,space))) { 
      ShowUsage(); 
      exit(1); 
    } else flag = p[0];
    if (!(flag=='p'||flag=='c'||flag=='b'))  { 
      ShowUsage(); 
      exit(1); 
    }
    return;
  }

  virtual void SetTestNo() {
    testno = 1; 
    ElapsedTimes = *(new vector<vector<double> >(testno));
  } //use counter to index each tested method


  virtual void Test() { } 
  virtual void Test() const { }

  virtual void Print_Statistics(int repno) { //repno: the max. value of repeating, set by outer loop
    double avg=0, deviation=0;
    for (int i=0; i<testno; i++) {
      for (int j=0; j<repno; j++) {
        avg += ElapsedTimes[i][j];
      }
      avg /= repno;
      for (int j=0; j<repno; j++)  {
        deviation += (ElapsedTimes[i][j]-avg)*(ElapsedTimes[i][j]-avg);
      }
      if (repno==1) {
        deviation=0;
      } else {
        deviation /= (repno-1)*repno;
        deviation = sqrt(deviation);
      }
      stapl_report(MethodNames[i]);
      stapl_report("\n\tavgtime=");
      stapl_report(avg);
      stapl_report("\n\t1.96*deviation=");
      stapl_report(1.96*deviation);
      stapl_report("\n");
    }
  }
};


