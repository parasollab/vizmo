/* pure sequential version, so the overhead includes both : from pcontainer and from palgo
 * to test:
 * mpirun -np 1 exefile -testall #elets
 * or:
 * mpirun -np 1 exefile Tester_Recurrence1(2) b/p/c #elets
 */
#include "common/BaseTester.h"
#include <vector>
#include <cmath>

using namespace stapl;

#ifndef REPEAT
#define REPEAT 20
#endif

#ifndef Forward
#define Forward true
#endif


/* (1) Tester_Recurrence1 : calling the same recurrence function as parallel tester for palgorithm Linear_Recurrence
 */
class Tester_Recurrence1 : public BaseTester {
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

public:
  Tester_Recurrence1() {flag='b'; n_elet=0;}

  char* GetName() { return "Tester_Recurrence1"; }
  
  BaseTester *CreateCopy() { Tester_Recurrence1 *t = new Tester_Recurrence1; t->flag=this->flag; t->n_elet=this->n_elet;return t; }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n"); 
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
    return;
  }  

  void SetTestNo() {
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test() {
    int tmp;
    
    list<int> listx1(n_elet,1);
    list<int>::iterator itx;
    int tmpx;

    counter=0;
    /*  call RecFunc1: without a */
    MethodNames.push_back("Linear_Recurrence1(without a)");
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
    if_want_perf {stoptimer;} 
    
    if_want_corr {
      //results should be : 123 456 789... if n_elet is 3
      int i = 1;
      for (list<int>::iterator itx=(listx1).begin(); itx!=(listx1).end(); ++itx) {
	if (*itx != i) {
            cout<<"ERROR while testing Linear_Recurrence1 i="<<i<<" so should be "<<i<<" but *itx="<<*itx<<endl;
	    break;
	}
	i++;
      }
    }

    cout<<"Passed"<<endl;
  }

};

/* Tester_Recurrence2 : calling the same recurrence function as parallel tester for palgorithm Linear_Recurrence
 */
class Tester_Recurrence2 : public BaseTester {
private:
  int n_elet;

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
  Tester_Recurrence2() {flag='b'; n_elet=0;}

  char* GetName() { return "Tester_Recurrence2"; }
  
  BaseTester *CreateCopy() { Tester_Recurrence2 *t = new Tester_Recurrence2; t->flag=this->flag; t->n_elet=this->n_elet;return t; }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n");
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
    return;
  }  

  void SetTestNo() {
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void Test() {
    int tmp;

    list<pair<int,int> > listx2(n_elet, pair<int,int>(1,1) );
    list<pair<int,int> >::iterator itx;
    int tmpx, tmpa;
    counter = 0;
    MethodNames.push_back("Linear_Recurrence2(with a)");
    RecFunc2<int, int> recfunc2(Forward);
    if_want_perf {starttimer;}
    itx = listx2.begin();
    tmpx = (*itx).first; 
    itx++;
    while (itx != listx2.end() ) {
      (*itx).first = recfunc2( (*itx).second, tmpx, (*itx).first);
      tmpx = (*itx).first;
      itx++;  
    }
    if_want_perf {stoptimer;} 
    
    if_want_corr {
      int i=1;
      for(list<pair<int,int> >::iterator itx=listx2.begin(); itx!=listx2.end(); ++itx) {
        if ((*itx).first != i)  {
            cout<<"ERROR while testing Linear_Recurrence2 i="<<i<<" so should be "<<i<<" but (*itx).first="<<(*itx).first<<endl;
	    break;
 	}
	i++;
      }
    }
    cout<<"Passed"<<endl;
  }
};




