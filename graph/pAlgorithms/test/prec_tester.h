/* Tester class for parallel First_order Linear Recurrence algo. 
 * to test:
 * mpirun -np #-threads exefile 10(aggregation) -testall #elets
 * or:
 * mpirun -np #-threads exefile 10(aggregation) Tester_Recurrence1 b/p/c #elets [ Tester_Recurrence2 b/p/c #elets] 
 */
#include "common/BaseTester.h"
#include "pContainers/plist/pList.h"
#include "pAlgorithms/p_Recurrence.h"
#include <cmath>

using namespace stapl;

#ifndef REPEAT
#define REPEAT 20
#endif

#ifndef Forward
#define Forward 1
#endif

#ifndef Backward
#define Backward 0
#endif

template <class X, class Y>
class pairxy {
 public:
  X first;
  Y second;
  pairxy() {}
  pairxy(X x, Y y) : first(x), second(y){}
  ~pairxy() {}
  void define_type(typer& t) { t.local(first); t.local(second); }
};

template <class X, class Y>
ostream& operator<<(ostream& os, const pairxy<X,Y>& p) {
  os<<"("<<p.first<<","<<p.second<<") ";
  return os;
}
  
template<class VALTYPE>
class ASSOC_PLUS {
  public:
    VALTYPE identity() { return 0;}
    VALTYPE operator() (VALTYPE x1, VALTYPE x2) {return x1 + x2; }
};

template<class VALTYPE>
class ASSOC_TIMES { 
  public:
    VALTYPE identity() { return 1;}
    VALTYPE operator() (VALTYPE x1, VALTYPE x2) {return x1 * x2; }
};


/* (1) Tester_Recurrence1 : Tester for Recurrence1
 */
class Tester_Recurrence1 : public BaseTester {
private:
  int n_elet; 
    

public:
  Tester_Recurrence1() {flag='p'; n_elet=0;}

  char* GetName() { return "Tester_Recurrence1"; }
  
  BaseTester *CreateCopy() { Tester_Recurrence1 *t = new Tester_Recurrence1; t->flag=this->flag;  t->n_elet=this->n_elet;  return t; }
 
  void SetTestNo () { 
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n"); stapl_print(" #elets % #thread is assumed to be 0 \n");
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
    int nprocs = stapl::get_num_threads();
    if (n_elet < nprocs) {cout<<"Too few elements"<<endl;exit(-1);}
    if (n_elet % nprocs !=0) {
      n_elet = nprocs * (n_elet / nprocs);
      if (stapl::get_thread_id()==0)
      cout<<"For simplicity, assume n_elet % nprocs == 0. So change n_elet as "<<n_elet<<endl;
    }
  
    return;
  }  

  void Test() {
    int myid = stapl::get_thread_id();
    int nprocs = stapl::get_num_threads();

    int j;

    //first, build a pList<int>
    pList<int> x(n_elet,1);

    //build links
    if (nprocs==1) {
      x.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(INVALID_PID,INVALID_PART) );
    } else {
      if (myid==0) {
        x.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(1,0) );
      } else if (myid==nprocs-1) {
        x.InitBoundaryInfo(0, Location(myid-1, 0), Location(INVALID_PID,INVALID_PART) );
      } else {
        x.InitBoundaryInfo(0, Location(myid-1, 0), Location(myid+1, 0) );
      }
    }

    stapl::rmi_fence();

    counter = 0;
    /* Recurrence1 : Linear_Recurrence (without a) */
    MethodNames.push_back("Linear_Recurrence1 (without a)");

    pList<int>::PRange prx1(&x);
    x.get_prange(prx1);
    stapl::rmi_fence();

    if_want_perf {starttimer;}
    ASSOC_PLUS<int> plus;
    Recurrence1<pList<int>::PRange, ASSOC_PLUS<int> > (prx1, plus);
    if_want_perf { stoptimer;}
    stapl::rmi_fence();

    if_want_corr {
      int value = (x.GetPart(0)->part_data.begin() )->GetUserData();
      if (value != n_elet/nprocs * myid+1) {
        cout<<"ERROR while testing Linear_Recurrence (1): should be "<<n_elet/nprocs*myid+1<<" but actually is "<<value<<endl;
      }
    }
    stapl::rmi_fence();
    if(stapl::get_thread_id() == 0)
      cout<<"Passed"<<endl;
    stapl::rmi_fence();
  }

};

/* (2) Tester_Recurrence2 : Tester for palgorithm parallel Linear_Recurrence2.
 */
class Tester_Recurrence2 : public BaseTester {
private:
  int n_elet; 

public:
  Tester_Recurrence2() {flag='p'; n_elet=0;}

  char* GetName() { return "Tester_Recurrence2"; }
  
  BaseTester *CreateCopy() { Tester_Recurrence2 *t = new Tester_Recurrence2; t->flag=this->flag;  t->n_elet=this->n_elet;  return t; }
 
  void SetTestNo () { 
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n"); stapl_print(" #elets % #thread is assumed to be 0 \n");
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
    int nprocs = stapl::get_num_threads();
    if (n_elet < nprocs) {cout<<"Too few elements"<<endl;exit(-1);}
    if (n_elet % nprocs !=0) {
      n_elet = nprocs * (n_elet / nprocs);
      if (stapl::get_thread_id()==0)
      cout<<"For simplicity, assume n_elet % nprocs == 0. So change n_elet as "<<n_elet<<endl;
    }
  
    return;
  }  

  void Test() {
    int tmp;
    int myid = stapl::get_thread_id();
    int nprocs = stapl::get_num_threads();

    int j;
    pList<pairxy<int,int> > x(n_elet, pairxy<int,int>(1,1) );

    //build links
    if (nprocs==1) {
      x.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(INVALID_PID,INVALID_PART) );
    } else {
      if (myid==0) {
        x.InitBoundaryInfo(0, Location(INVALID_PID,INVALID_PART), Location(1,0) );
      } else if (myid==nprocs-1) {
        x.InitBoundaryInfo(0, Location(myid-1, 0), Location(INVALID_PID,INVALID_PART) );
      } else {
        x.InitBoundaryInfo(0, Location(myid-1, 0), Location(myid+1, 0) );
      }
    }

    stapl::rmi_fence();

    pList<pairxy<int,int> >::PRange prx2(&x);
    x.get_prange(prx2);
    stapl::rmi_fence();

    counter = 0;
    /* Recurrence2 : Linear_Recurrence2 (with a) */
    MethodNames.push_back("Linear_Recurrence2 (with a)");

    if_want_perf { starttimer;}
    ASSOC_PLUS<int> plus; ASSOC_TIMES<int> times;
    Recurrence2<pList<pairxy<int,int> >::PRange,ASSOC_PLUS<int>, ASSOC_TIMES<int> > (prx2, plus, times);
    if_want_perf { stoptimer; }
    stapl::rmi_fence();

    //x.DisplayPContainer();
    //x.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();

    if_want_corr {
      int value = ((x.GetPart(0)->part_data.begin() )->GetUserData() ).first;
      if (value != n_elet/nprocs * myid+1) {
        cout<<"ERROR while testing Linear_Recurrence (2): should be "<<n_elet/nprocs*myid+1<<" but actually is "<<value<<endl;
      }
    }
    if(stapl::get_thread_id() == 0)
      cout<<"Passed"<<endl;

    stapl::rmi_fence();
  }

};
