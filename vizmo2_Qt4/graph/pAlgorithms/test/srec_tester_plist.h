/* sequential version using pList . So we exclude the overhead caused by pList
 * to test:
 * mpirun -np 1 exefile -testall #elets
 * or 
 * mpirun -np 1 exefile Tester_Recurrence1(2) b/p/c #elets
 */
#include "common/BaseTester.h"
#include <vector>
#include "pContainers/plist/pList.h"
#include <cmath>

using namespace stapl;

#ifndef REPEAT
#define REPEAT 20
#endif

#ifndef Forward
#define Forward true
#endif

template <class VALTYPE>
class ASSOC_PLUS {
  public:
    VALTYPE identity() {return 0;}
    VALTYPE operator() (VALTYPE x1, VALTYPE x2) { return x1+x2; }
};

template <class VALTYPE>
class ASSOC_TIMES {
  public:
    VALTYPE identity() {return 1;}
    VALTYPE operator() (VALTYPE x1, VALTYPE x2) { return x1*x2; }
};

/* Tester_Recurrence1 : calling the same recurrence function as parallel tester for palgorithm Linear_Recurrence
 */
class Tester_Recurrence1 : public BaseTester {
private:
  int n_elet;

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
    
    pList<int> listx1;
    for (int j=0; j<n_elet; j++)
      listx1.AddElement2Part(1, 0);
    //pList<int>::iterator itx;

    //listx1.DisplayPContainer();
    //listx1.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();
    
    int tmpx;

    counter=0;

    /*  without a */
    MethodNames.push_back("Linear_Recurrence1(without a)");
    ASSOC_PLUS<int> plus;
    typedef pList<int>::PRange PRANGE;
    //use pRange for similar input as pAlgo.
    PRANGE pr(&listx1);
    listx1.get_prange(pr);
    stapl::rmi_fence();
    if_want_perf {starttimer;}
    typedef PRANGE::subrangeType subrangeType;
    subrangeType::iteratorType itx = (pr.get_subranges() )[0].get_boundary().start();
    tmpx = itx->GetUserData();
    ++itx;
    while (itx != (pr.get_subranges() )[0].get_boundary().finish() ) {
      tmpx = plus(tmpx, itx->GetUserData() );
      itx->SetUserData(tmpx);
      ++itx;
    }
    if_want_perf {stoptimer;} 
    
    if_want_corr {
      //results should be : 123...
      int i = 1;
      for (pList<int>::iterator itx=listx1.begin(); itx!=listx1.end(); ++itx) {
	if (itx->GetUserData() != i) {
            cout<<"ERROR while testing Linear_Recurrence1 i="<<i<<" so should be "<<i<<" but *itx="<<itx->GetUserData()<<endl;
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
  template <class X, class Y>
  class pairxy {
    public:  
      X first;
      Y second;
      pairxy() {}
      pairxy(X x, Y y) : first(x), second(y) {}
      ~pairxy() {}
      void define_type(typer& t) { t.local(first); t.local(second); }
  };

template <class X, class Y>
ostream& operator<<(ostream& os, const pairxy<X,Y>& p) {
  os<<"("<<p.first<<","<<p.second<<") ";
  return os;
}

class Tester_Recurrence2 : public BaseTester {
private:
  int n_elet;

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
    
    pList<pairxy<int,int> > listx2;
    for (int j=0; j<n_elet; j++) {
      listx2.AddElement2Part(pairxy<int,int>(1,1), 0);
    }
    //pList<pairxy<int,int> >::iterator itx;

    //listx2.DisplayPContainer();
    //listx2.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();

    int tmpx;

    counter=0;
    /*  with a */
    MethodNames.push_back("Linear_Recurrence2(with a)");
    ASSOC_PLUS<int> plus;
    ASSOC_TIMES<int> times;
    typedef pList<pairxy<int,int> >::PRange PRANGE;
    PRANGE pr(&listx2);
    listx2.get_prange(pr);
    stapl::rmi_fence();
    if_want_perf {starttimer;}
    typedef PRANGE::subrangeType subrangeType;
    subrangeType::iteratorType itx = (pr.get_subranges() )[0].get_boundary().start();
    tmpx = itx->GetUserData().first;
    ++itx;
    while (itx != (pr.get_subranges() )[0].get_boundary().finish() ) {
      tmpx = plus( times(itx->GetUserData().second, tmpx), itx->GetUserData().first );
      itx->SetUserData( pairxy<int,int>(tmpx, itx->GetUserData().second) );
      ++itx;
    }
    if_want_perf {stoptimer;} 
    
    if_want_corr {
      int i = 1;
      for (pList<pairxy<int,int> >::iterator itx=listx2.begin(); itx!=listx2.end(); ++itx) {
	if (itx->GetUserData().first != i) {
            cout<<"ERROR while testing Linear_Recurrence2 i="<<i<<" so should be "<<i<<" but (*itx).first="<<itx->GetUserData().first<<endl;
	    break;
	}
	i++;
      }
    }
    cout<<"Passed"<<endl;
  }
};


