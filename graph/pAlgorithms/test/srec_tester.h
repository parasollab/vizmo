/* this only compares the "sequential linear recurrence ALGO" to parallel algo 
 * (pRec). So we exclude the overhead caused by pContainer
 * that's why we use pList here instead of list
 * to test:
 * mpirun -np 1 sRec_Testers -testall #elets
 */
#include "common/BaseTester.h"
#include <vector>
#include "pContainers/plist/pList.h"
#include <cmath>

using namespace stapl;

#ifndef REPEAT
#define REPEAT 200
#endif

#ifndef Forward
#define Forward true
#endif


/* (1) Tester_Recurrence : calling the same recurrence function as parallel tester for palgorithm Linear_Recurrence
 */
class Tester_Recurrence : public BaseTester {
private:
  int n_elet;

  template<class VALTYPE>
  class RecFunc1 {
      bool direction;
    public:
      RecFunc1(bool _dir):direction(_dir) {}
      ~RecFunc1(){}
      VALTYPE identity() { return 1;}
      VALTYPE zero() {return 0; }
      VALTYPE assoc_plus(VALTYPE x1, VALTYPE x2) { return x1+x2; }
      VALTYPE operator() (VALTYPE x1, VALTYPE x2) {return assoc_plus(x1, x2); }
      bool GetDirection() { return direction;}
  };

  template<class VALTYPEX, class VALTYPEA> 
  class RecFunc2 {
      bool direction;
    public:
      RecFunc2(bool _dir):direction(_dir) {}
      ~RecFunc2(){}
      VALTYPEA identity() { return 1;}
      VALTYPEX zero() {return 0; }
      VALTYPEX assoc_plus(VALTYPEX x1, VALTYPEX x2) { return x1+x2; }
      VALTYPEX assoc_times(VALTYPEA a, VALTYPEX x) { return a*x; } 
      VALTYPEX operator() (VALTYPEA a, VALTYPEX x1, VALTYPEX x2){ return assoc_plus(assoc_times(a, x1), x2); }
      bool GetDirection() { return direction;}
  };

public:
  Tester_Recurrence() {flag='b'; n_elet=0;}

  char* GetName() { return "Tester_Recurrence"; }
  
  BaseTester *CreateCopy() { Tester_Recurrence *t = new Tester_Recurrence; t->flag=this->flag; t->n_elet=this->n_elet;return t; }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n"); stapl_print(" #elets(should be <=53)");
    return;
  }

  void ParseParams(char* s) {
    char *space = " "; char *p;
    if(!(p=strtok(s,space))) { 
      ShowUsage(); 
      exit(1); 
    } else flag = p[0];
    if (!(flag=='p'||flag=='c'||flag=='b'))  { 
      ShowUsage(); 
      exit(1); 
    }
    if (p=strtok(NULL,space)) n_elet=atoi(p);
/*
    if (n_elet > 53) {
      stapl_print(" Please do not input too big n_elet value b/c it may cause too big number for computer.\n"); 
      exit(1);
    }
*/
    return;
  }  

  void SetTestNo() {
    testno = 2;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test() {
    int tmp;
    
    pList<int> listx1(n_elet,1);
    pList<int> listx2(n_elet,1);
    pList<int> lista(n_elet,1);
    stapl::rmi_fence();
    pList<int>::iterator itx, ita;
    int tmpx;

    counter=0;
    /* 1.1 call RecFunc1: without a */
    MethodNames.push_back("Linear_Recurrence(without a)");
    RecFunc1<int> recfunc1(Forward);
    if_want_perf {starttimer;}
    itx = listx1.begin();
    tmpx = *itx;
    itx++; 
    while (itx != listx1.end() ) {
      *itx = recfunc1(tmpx, *itx);
      tmpx = *itx;
      itx++;
    }
    if_want_perf {newstoptimer;} 
    
    if_want_corr {
      //results should be : 123 456 789... if n_elet is 3
      int i = 1;
      for (pList<int>::iterator itx=(listx1).begin(); itx!=(listx1).end(); ++itx) {
	if (*itx != i) {
            cout<<"ERROR while testing Linear_Recurrence (1) i="<<i<<" so should be "<<i<<" but *itx="<<*itx<<endl;
	    break;
	}
	i++;
      }
    }

    counter++;
    /* 1.2 call RecFunc1: with a */
    MethodNames.push_back("Linear_Recurrence(with a)");
    RecFunc2<int,int> recfunc2(Forward);
    if_want_perf {starttimer;}
    itx = listx2.begin(); ita = lista.begin();
    tmpx = *itx;
    itx++; ita++;  
    while (itx != listx2.end() ) {
      *itx = recfunc2(*ita, tmpx, *itx);
      tmpx = *itx;
      itx++; ita++;
    }
    if_want_perf {newstoptimer;} 
    cout<<"Passed"<<endl;
    
    if_want_corr {
      int i = 1;
      for (pList<int>::iterator itx=(listx2).begin(); itx!=(listx2).end(); ++itx) {
	if (*itx != i) {
            cout<<"ERROR while testing Linear_Recurrence (2) i="<<i<<" so should be "<<i<<" but *itx="<<*itx<<endl;
	    break;
	}
	i++;
      }
    }
    cout<<"Passed"<<endl;
  }
};




