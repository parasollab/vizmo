/* Tester class for parallel  SimplePointerJumping technique.
 * to test:
 * mpirun -np num_processors exefilename 10(aggregation) -testall #elets
 * or:
 * mpirun -np num_processors exefilename 10(aggregation) Tester_SimplePointerJumping [b|p|c] #elets
 * 
 * the testing case is a pList as:
 *      #0   part0(0, ...), part1(3, ...), part2(6, ...)
 *      #1   part0(1, ...), part1(4, ...), part2(7, ...)
 *      #2   part0(2, ...), part1(5, ...), part2(8, ...)
 */

#include "common/BaseTester.h"
#include "pContainers/plist/pList.h"
#include "pAlgorithms/SimplePointerJumping.h"
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

/* Tester_SimplePointerJumping : Tester for Simple Pointer Jumping technique
 * the testing case is computing prefix sums for a pList<int>
 * whose each subPContainer has only 1 integer.
 */
class Tester_SimplePointerJumping : public BaseTester {
private:
  int n_elet; 
  int each, rem;
    
  template<class VALTYPE>
  class TODOFunc {
    public:
      ~TODOFunc(){}
      VALTYPE identity() { return 0;}
      VALTYPE operator() (VALTYPE x1, VALTYPE x2) {return x1+x2;}
  };


public:
  Tester_SimplePointerJumping() {flag='p'; n_elet=0;}

  char* GetName() { return "Tester_SimplePointerJumping"; }
  
  BaseTester *CreateCopy() { Tester_SimplePointerJumping *t = new Tester_SimplePointerJumping; t->flag=this->flag;  t->n_elet=this->n_elet;  return t; }
 
  void SetTestNo () { 
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

  void ShowUsage() {
    stapl_print("\n"); stapl_print(GetName()); stapl_print(" p|c|b\n"); stapl_print(" #elets % #thread is assumed to be 0.\n"); 
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
    counter = 0;

    MethodNames.push_back("SimplePointerJumping");
    int myid = stapl::get_thread_id();
    int nprocs = stapl::get_num_threads();
    each = n_elet/nprocs;
    rem = n_elet%nprocs;
    int j;
    int partid=0;

    //first, build a pList<int>
    pList<int> x;

    //build parts
    for(j=0; j<each; j++) {
      x.AddElement2Part(myid+nprocs*j, partid);
      if (j<n_elet/nprocs-1)  
      partid=x.AddPart();
    }
    if (myid<rem) {
      partid=x.AddPart();
      x.AddElement2Part(myid+nprocs*j, partid);
    }
    stapl::rmi_fence();
    //x.DisplayPContainer();
    //x.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();

    //build links
    for (partid=0; partid<x.get_num_parts(); partid++) {

      if (nprocs==1) { //case 1) nprocs==1 then rem==0
	if (each==1) //case 1.1 each==1 only 1 part at #(0,0) both the first and the last
          x.InitBoundaryInfo(partid, Location(INVALID_PID,INVALID_PART), Location(INVALID_PID,INVALID_PART) );
        else {
          if (partid==0)  //case 1.2.1: the first part at #(0,0)
            x.InitBoundaryInfo(partid, Location(INVALID_PID,INVALID_PART), Location(myid, partid+1) );
          else if (partid==each-1) //case 1.2.2: the last part at #(0,each-1)
            x.InitBoundaryInfo(partid, Location(myid, partid-1), Location(INVALID_PID,INVALID_PART) );
          else //case 1.2.3: the other parts at #(0,i)  i!=0&&i!=each-1
            x.InitBoundaryInfo(partid, Location(myid, partid-1), Location(myid, partid+1) );
        }
      } 

      else if (nprocs>1 && rem == 0) { //case 2) more than 1 threads, but rem==0
        if (myid==0 && partid==0)  //case 2.1: the first part at #(0,0)
          x.InitBoundaryInfo(partid, Location(INVALID_PID,INVALID_PART), Location(myid+1, partid) );
	else if (myid==0 && partid!=0) //case 2.2: other parts on thread0 #(0,i) i!=0
          x.InitBoundaryInfo(partid, Location(nprocs-1,partid-1), Location(myid+1, partid) );
        else if (myid==nprocs-1 && partid==each-1)  //case 2.3: the last part at #(nprocs-1, each-1)
          x.InitBoundaryInfo(partid, Location(myid-1,partid), Location(INVALID_PID, INVALID_PART) );
        else if (myid==nprocs-1 && partid!=each-1)  //case 2.4: the other parts at #(nprocs-1, i)  i!=each-1
          x.InitBoundaryInfo(partid, Location(myid-1,partid), Location(0, partid+1) );
        else //case 2.5: all other (internal) parts
          x.InitBoundaryInfo(partid, Location(myid-1,partid), Location(myid+1,partid) );
      } 

      else if (rem==1) { //case 3) rem==1
        if (myid==0 && partid==0) //case 3.1: the first part at #(0,0)
          x.InitBoundaryInfo(partid, Location(INVALID_PID, INVALID_PART), Location(myid+1,partid) );
        else if (myid==0 && partid==each) //case 3.2 the last part at #(0,each)
          x.InitBoundaryInfo(partid, Location(nprocs-1, partid-1), Location(INVALID_PID, INVALID_PART) );
        else if (myid==0 && partid!=0 && partid!=each)  //case 3.3: the other parts on #(0,i)
          x.InitBoundaryInfo(partid, Location(nprocs-1, partid-1), Location(myid+1, partid) );
        else if (myid==nprocs-1) //case 3.4: parts on #(nprocs-1,i)
          x.InitBoundaryInfo(partid, Location(myid-1, partid), Location(0, partid+1) );
        else  //case 3.5: other internal parts
          x.InitBoundaryInfo(partid, Location(myid-1, partid), Location(myid+1, partid) );
      }
 
      else { //case 4) rem>1
	if (myid==0 && partid==0) 
          x.InitBoundaryInfo(partid, Location(INVALID_PID,INVALID_PART), Location(myid+1, partid) );
	else if (myid==0 && partid!=0) 
          x.InitBoundaryInfo(partid, Location(nprocs-1, partid-1), Location(myid+1, partid) );
 	else if (myid==nprocs-1)	
          x.InitBoundaryInfo(partid, Location(myid-1, partid), Location(0, partid+1) );
	else if (myid==rem-1 && partid==each) 
          x.InitBoundaryInfo(partid, Location(myid-1, partid), Location(INVALID_PID,INVALID_PART) );
	else 
          x.InitBoundaryInfo(partid, Location(myid-1, partid), Location(myid+1, partid) );
      } //end if
    } //end for

    stapl::rmi_fence();
    //x.DisplayPContainer();
    //x.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();

    /* Simple Pointer jumping is only a technique, not a complete algo.,
     * and there's no corresponding sequential technique 
     * but we do allow reporting absolute running time for each
     * calling of the tech. 
     */
    if_want_perf { starttimer; }

    TODOFunc<int> todofunc;
    //pList<int>::PRange prx(&x);
    pList<int>::reverse_PRange prx(&x);
    x.get_prange(prx);
    stapl::rmi_fence();

    bool pure = true;

    //SimplePointerJumping<pList<int>::PRange, TODOFunc<int> >(prx, todofunc, Forward, pure);
    SimplePointerJumping<pList<int>::reverse_PRange, TODOFunc<int> >(prx, todofunc, Backward, pure);
    stapl::rmi_fence();

    if_want_perf { stoptimer; }
    stapl::rmi_fence();
    //x.DisplayPContainer();
    //x.GetDistribution().DisplayBoundaryInfo();
    //stapl::rmi_fence();

    if_want_corr { //only check the last sum, if it's not the total sum, wrong
      int npart = x.get_num_parts();
      if ( (rem>0 && myid==rem-1 && partid==npart-1) || (rem==0 && myid==nprocs-1 && partid==npart-1)) { //last part, should have totalsum
        int tmp = (*( (*(x.GetPart(partid)) ).part_data.begin() ) ).GetUserData();
        double corr;
	if (pure)
 	  corr = (0+n_elet-1)*n_elet/2 - 1;
	else 
          corr = (0+n_elet-1)*n_elet/2;
        if (tmp!=corr) {
          cout<<"Wrong Result: "<<tmp<<" should be "<<corr<<endl;
        } else {
          cout<<"Correct!"<<endl;
        }
      }
    }
    stapl::rmi_fence();
  }

};
