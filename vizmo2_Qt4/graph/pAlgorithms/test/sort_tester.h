#ifndef _SORT_SAMPLING_H_
#define _SORT_SAMPLING_H_


/* Tester class for parallel Sort algo. 
 * to test:
 * mpirun -np ? Sort_Testers 10(aggregation) -testall nElements
 * the testing case is a pList as:
 *      #0   part0(0, ...), part1(3, ...), part2(6, ...)
 *      #1   part0(1, ...), part1(4, ...), part2(7, ...)
 *      #2   part0(2, ...), part1(5, ...), part2(8, ...)
 */

#include "unistd.h"
#include "stdlib.h"
#include "common/BaseTester.h"
#include "pContainers/parray/pArray.h"
#include "pRange/pRange.h"
#include "vector"
#include "pAlgorithms/p_sort.h"
#include "pContainers/base/BasePContainer.h"
#include "Point.h"

using namespace stapl;

#ifndef REPEAT
#define REPEAT 200
#endif

#ifndef Forward
#define Forward 1
#endif

#ifndef Backward
#define Backward 0 
#endif

/// Tester_Sort : Tester for Sorting techniques
class Tester_Sort : public BaseTester {

private:
  int nElements, local_size, start_gid;
  int type;       //0=int, 1=double, 2=point
  int presort;    //0=inorder, 1=2percent, 2=random, 3=reverse
  int sort;       //0=adaptive, 1=sample, 2=column, 3=bitonic, 4=radix
    
  template<class VALTYPE>
  class TODOFunc {
      bool direction;
    public:
      TODOFunc(bool _dir):direction(_dir) {}
      ~TODOFunc(){}
      VALTYPE identity() { return 1;}
      VALTYPE zero() {return 0; }
      VALTYPE assoc_plus(VALTYPE x1, VALTYPE x2) { return x1+x2; }
      VALTYPE operator() (VALTYPE x1, VALTYPE x2) {return assoc_plus(x1, x2); }
      bool GetDirection() { return direction;}
  };//end for TODOFunc

public:
  Tester_Sort() {flag='p'; nElements=0;}
  char* GetName() { return "Tester_Sort"; }
  BaseTester *CreateCopy() { Tester_Sort *t = new Tester_Sort; t->flag=this->flag;  t->nElements=this->nElements;  return t; }
 
  void SetTestNo () { 
    testno = 1;
    ElapsedTimes = *(new vector<vector<double> >(testno));
  }

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
    if (p=strtok(NULL,space)) nElements=atoi(p);
    if (p=strtok(NULL,space)) type=atoi(p);
    if (p=strtok(NULL,space)) presort=atoi(p);
    if (p=strtok(NULL,space)) sort=atoi(p);
    return;
  }  

  void Test() {
    counter = 0;
 
    MethodNames.push_back("Sort");
    int threadID = stapl::get_thread_id();
    int nThreads = stapl::get_num_threads();
 
    if (threadID==0){
      cout<<"nThreads="<<nThreads<<" n="<<nElements<<" type_of_element="<<type<<" presortedness="<<presort<<" sort_used="<<sort<<endl;
    }

    local_size = nElements/nThreads;
    start_gid = threadID*local_size;
    if (threadID == nThreads-1){
      local_size = local_size+nElements%nThreads;
    }
    srand48(getpid());
    srand(getpid());
 
    switch(type){
      /////////////////////////////////////////////////      
      //INTEGERS
    case 0:{//build pArray<int>
      pArray<int> parray(nElements);
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,start_gid+i);
	}
	break;
      }
	
      case 1:{//2percent
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,start_gid+i);
	}
	if (threadID%2 ==0){  //swap with the odd
	  for (int i = 5; i<local_size-1; i=i+50){
	    int my_elem = start_gid+i;
	    int swap_elem = my_elem+local_size;
	    int temp = parray.GetElement(swap_elem);
	    parray.SetElement(swap_elem,parray.GetElement(my_elem));
	    parray.SetElement(my_elem,temp);
	  }
	}
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,rand()%10000);
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,nElements-start_gid-i-1);
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort)
      
      //get the corresponding pRange
      pArray<int>::PRange prange(&parray);
      parray.get_prange(prange);
   
      //start timer and sort
      if_want_perf { starttimer; }
      int used_sort = p_sort<pArray<int>::PRange>(prange,sort);
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      if (threadID == 0 ){
	cout<<"used_sort="<<used_sort<<endl;
      }
      
      //check correctness
      if_want_corr {
	bool correct = true;
	//in parallel, check local order
	for (int i = start_gid; i<start_gid+local_size-1; i++){
	  if (parray[i+1] < parray[i]){
	    correct = false;
	  }
	}
	stapl::rmi_fence();
	
	//thread zero checks the boundaries
	if (threadID == 0){
	  for (int i=0; i<nThreads; i++){
	    if (parray[i*local_size+1] < parray[i*local_size]){
	      correct = false;
	    }
	  }
	}
	if (correct == false){
	  cout<<"Failed, thread "<<threadID<<endl;
	}
	else{
	  cout<<"Passed"<<endl;
	}
	stapl::rmi_fence();
      }//end for checking correctness
      break;
    }//end for int

      ////////////////////////////////////////////////////////      
      //DOUBLES
    case 1:{//build pArray<double>
      pArray<double> parray(nElements);
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,(double)(start_gid+i)+drand48());
	}
	break;
      }
	
      case 1:{//2percent
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,(double)(start_gid+i)+drand48());
	}
	if (threadID%2 ==0){  //swap with the odd
	  for (int i = 5; i<local_size-1; i+=50){
	    int my_elem = start_gid+i;
	    int swap_elem = my_elem+local_size;
	    double temp = parray.GetElement(swap_elem);
	    parray.SetElement(swap_elem,parray.GetElement(my_elem));
	    parray.SetElement(my_elem,temp);
	  }
	}
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,(double)(rand()%10000)+drand48());
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  parray.SetElement(start_gid+i,(double)(nElements-start_gid-i-1)+drand48());
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort) for doubles
      
      //get the corresponding pRange
      pArray<double>::PRange prange(&parray);
      parray.get_prange(prange);
            
      //start timer and sort
      if_want_perf { starttimer; }
      int used_sort=p_sort<pArray<double>::PRange,double>(prange,sort);      
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      if (threadID == 0){
	cout<<"used_sort="<<used_sort<<endl;
      }

      //check correctness
      if_want_corr {
	bool correct = true;
	//in parallel, check local order
	for (int i = start_gid; i<start_gid+local_size-1; i++){
	  if (parray[i+1] < parray[i]){
	    correct = false;
	  }
	}
	stapl::rmi_fence();
	
	//thread zero checks the boundaries
	if (threadID == 0){
	  for (int i=0; i<nThreads; i++){
	    if (parray[i*local_size+1] < parray[i*local_size]){
	      correct = false;
	    }
	  }
	}
	if (correct == false){
	  cout<<"wrong result, thread "<<threadID<<endl;
	}
	stapl::rmi_fence();
      }//end for checking correctness
      
      break;
    }//end for doubles    

      ///////////////////////////////////////////////////////
      //POINTS
    case 2:{//build pArray<Point>
      pArray<Point> parray(nElements);
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(start_gid+i)+drand48();
	  temp.y = 0.0;
	  parray.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      case 1:{//2percent
	for (int i = 0; i<local_size; i++){
	  Point temp((double)(start_gid+i)+drand48(),0.0);
	  parray.SetElement(start_gid+i,temp);
	}
	if (threadID%2 ==0){  //swap with the odd
	  for (int i = 5; i<local_size-1; i+=50){
	    int my_elem = start_gid+i;
	    int swap_elem = my_elem+local_size;
	    Point temp = parray.GetElement(swap_elem);
	    parray.SetElement(swap_elem,parray.GetElement(my_elem));
	    parray.SetElement(my_elem,temp);
	  }
	}
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(rand()%10000)+drand48();
	  temp.y = 0.0;
	  parray.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(nElements-start_gid-i-1)+drand48();
	  temp.y = 0.0;
	  parray.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort) for Points
      
      //get the corresponding pRange
      pArray<Point>::PRange prange(&parray);
      parray.get_prange(prange);

      //start timer and sort
      if_want_perf { starttimer; }
      int used_sort = p_sort(prange,sort,point_less);
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      if (threadID == 0){
	cout<<"used_sort="<<used_sort<<endl;
      }
      
      //check correctness
      if_want_corr {
	bool correct = true;
	//in parallel, check local order
	for (int i = start_gid; i<start_gid+local_size-1; i++){
	  if (point_less(parray[i+1],parray[i])){
	    correct = false;
	  }
	}
	stapl::rmi_fence();
	
	//thread zero checks the boundaries
	if (threadID == 0){
	  for (int i=0; i<nThreads; i++){
	    if (point_less(parray[i*local_size+1],parray[i*local_size])){
	      correct = false;
	    }
	  }
	}
	if (correct == false){
	  cout<<"wrong result, thread "<<threadID<<endl;
	}
	stapl::rmi_fence();
      }//end for checking correctness

      break;
    }//end for Points
        
    default:
      cout<<"default in case statement - types"<<endl;
      break;
    }//end of switch(types)

  }
};


#endif
