/* Tester class for parallel inner product algo. 
 */

#include "unistd.h"
#include "stdlib.h"
#include "common/BaseTester.h"
#include "pContainers/parray/pArray.h"
#include "pRange/pRange.h"
#include "vector"
#include "pAlgorithms/p_inner_product.h"
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


/// Tester_Inner_Product : Tester for Inner Product
class Tester_Inner_Product : public BaseTester {

private:
  int nElements, local_size, start_gid;
  int type;       //0=int, 1=double, 2=Point
  int presort;    //0=inorder, 1=all 2's, 2=random, 3=reverse
    
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
  Tester_Inner_Product() {flag='p'; nElements=0;}
  char* GetName() { return "Tester_Inner_Product"; }
  BaseTester *CreateCopy() { Tester_Inner_Product *t = new Tester_Inner_Product; t->flag=this->flag;  t->nElements=this->nElements;  return t; }
 
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
    return;
  }  

  void Test() {
    counter = 0;

    MethodNames.push_back("Inner_Product");
    int threadID = stapl::get_thread_id();
    int nThreads = stapl::get_num_threads();
    local_size = nElements/nThreads;
    start_gid = threadID*local_size;
    if (threadID == nThreads-1){
      local_size = local_size+nElements%nThreads;
    }
    srand48(getpid());
    srand(getpid());
    
    switch(type){
      
      //INTEGERS
    case 0:{//build pArray<int>
      pArray<int> parray1(nElements);
      pArray<int> parray2(nElements);
      pArray<int> parray3(nElements);
      pArray<int> parray4(nElements);//equal to parray1
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,start_gid+i);
	  parray2.SetElement(start_gid+i,start_gid+i);
	  parray3.SetElement(start_gid+i,start_gid+i);
	  parray4.SetElement(start_gid+i,start_gid+i);
	}
	break;
      }

      case 1:{//2's
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,2);
	  parray2.SetElement(start_gid+i,2);
	  parray3.SetElement(start_gid+i,2);
	  parray4.SetElement(start_gid+i,2);
	}
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,rand()%10000);
	  parray2.SetElement(start_gid+i,rand()%10000);
	  parray3.SetElement(start_gid+i,rand()%10000);
	  parray4.SetElement(start_gid+i,parray1[start_gid+i]);
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,nElements-start_gid-i-1);
	  parray2.SetElement(start_gid+i,nElements-start_gid-i-1);
	  parray3.SetElement(start_gid+i,nElements-start_gid-i-1);
	  parray4.SetElement(start_gid+i,nElements-start_gid-i-1);
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort)
      
      //get the corresponding pRange
      pArray<int>::PRange prange1(&parray1);
      parray1.get_prange(prange1);

      pArray<int>::PRange prange2(&parray2);
      parray2.get_prange(prange2);

      pArray<int>::PRange prange3(&parray3);
      parray3.get_prange(prange3);
   
      vector<pArray<int>::PRange> v;
      v.push_back(prange1);
      v.push_back(prange2);
      v.push_back(prange3);

      vector<pArray<int>::PRange*> v2;
      v2.push_back(&prange1);
      v2.push_back(&prange2);
      v2.push_back(&prange3);

      //start timer
      if_want_perf { starttimer; }
      
      //inner_product
      int result = p_inner_product(prange1,prange2,2);
      int result1 = p_inner_product(v,2);
      int result2 = p_inner_product(v2,2);

      //end timer
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      if (threadID ==0){
	cout<<"RESULT="<<result<<endl;
	cout<<"RESULT1="<<result1<<endl;
	cout<<"RESULT2="<<result2<<endl;
      }
      //check correctness
      if_want_corr {
	bool correct = true;
	if (threadID == 0){
	  vector<int> vec1, vec2, vec3;
	  for (int i=0; i<nElements; i++){
	    vec1.push_back(parray4[i]);
	    vec2.push_back(parray2[i]);
	    vec3.push_back(parray3[i]);
	  }
	  int correct_result = std::inner_product(vec1.begin(),
						  vec1.end(),
						  vec2.begin(),2);
	  for (int i=0; i<nElements; i++){
	    vec1[i] = vec1[i]*vec2[i]*vec3[i];
	  }

	  int correct_result1 = std::accumulate(vec1.begin(),
						vec1.end(),2);

	  if (result != correct_result)
	    correct = false;

	  if (result1 != correct_result1)
	    correct = false;

	  if (result2 != correct_result1)
	    correct = false;

	  if (correct == false){
	    cout<<"wrong result, thread "<<threadID<<endl;
	  }
	}//end for thread 0
	stapl::rmi_fence();
      }//end for checking correctness
      break;//end for int
    }
      
      //DOUBLES
    case 1:{//build pArray<double>
      pArray<double> parray1(nElements);
      pArray<double> parray2(nElements);
      pArray<double> parray3(nElements);
      pArray<double> parray4(nElements);//equal to parray1
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,(double)(start_gid+i)+drand48());
	  parray2.SetElement(start_gid+i,(double)(start_gid+i)+drand48());
	  parray3.SetElement(start_gid+i,(double)(start_gid+i)+drand48());
	  parray4.SetElement(start_gid+i,parray1[start_gid+i]);
	}
	break;
      }
	
      case 1:{//2.0
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,2.0);
	  parray2.SetElement(start_gid+i,2.0);
	  parray3.SetElement(start_gid+i,2.0);
	  parray4.SetElement(start_gid+i,2.0);
	}	
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,(double)(rand()%10000)+drand48());
	  parray2.SetElement(start_gid+i,(double)(rand()%10000)+drand48());
	  parray3.SetElement(start_gid+i,(double)(rand()%10000)+drand48());
	  parray4.SetElement(start_gid+i,parray1[start_gid+i]);
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  parray1.SetElement(start_gid+i,(double)(nElements-start_gid-i-1)+drand48());
	  parray2.SetElement(start_gid+i,(double)(nElements-start_gid-i-1)+drand48());
	  parray3.SetElement(start_gid+i,(double)(nElements-start_gid-i-1)+drand48());
	  parray4.SetElement(start_gid+i,parray1[start_gid+i]);
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort) for doubles
      
      //get the corresponding pRange
      pArray<double>::PRange prange1(&parray1);
      parray1.get_prange(prange1);

      pArray<double>::PRange prange2(&parray2);
      parray2.get_prange(prange2);

      pArray<double>::PRange prange3(&parray3);
      parray3.get_prange(prange3);

      vector<pArray<double>::PRange> v;
      v.push_back(prange1);
      v.push_back(prange2);
      v.push_back(prange3);

      vector<pArray<double>::PRange*> v2;
      v2.push_back(&prange1);
      v2.push_back(&prange2);
      v2.push_back(&prange3);
            
      //start timer
      if_want_perf { starttimer; }
      
      //inner_product
      double result = p_inner_product(prange1,prange2,2.0);
      double result1 = p_inner_product(v,2.0);
      double result2 = p_inner_product(v2,2.0);

      //stop_timer
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      if (threadID ==0){
	cout<<"RESULT="<<result<<endl;
	cout<<"RESULT1="<<result1<<endl;
	cout<<"RESULT2="<<result2<<endl;
      }

      //check correctness
      if_want_corr {
	bool correct = true;
	if (threadID == 0){
	  vector<double> vec1, vec2, vec3;
	  for (int i=0; i<nElements; i++){
	    vec1.push_back(parray4[i]);
	    vec2.push_back(parray2[i]);
	    vec3.push_back(parray3[i]);
	  }
	  double correct_result = std::inner_product(vec1.begin(),
						     vec1.end(),
						     vec2.begin(),2.0);
	  for (int i=0; i<nElements; i++){
	    vec1[i] = vec1[i]*vec2[i]*vec3[i];
	  }
	  double correct_result2 = std::accumulate(vec1.begin(),
						vec1.end(),2.0);
	  if (!((result-correct_result)<0.1 && (result-correct_result)>-0.1))
	    correct = false;
	  if (!((result1-correct_result2)<0.1 && (result1-correct_result2)>-0.1))
	    correct = false;
	  if (!((result2-correct_result2)<0.1 && (result2-correct_result2)>-0.1))
	    correct = false;
	  if (correct == false){
	    cout<<"wrong result, thread "<<threadID<<endl;
	  }
	}//end for thread 0
	stapl::rmi_fence();
      }//end for checking correctness
      break;//end for doubles
    }
      
      //POINTS
    case 2:{//build pArray<Point>
      pArray<Point> parray1(nElements);
      pArray<Point> parray2(nElements);
      pArray<Point> parray3(nElements);
      stapl::rmi_fence();
      switch(presort){
	
      case 0:{//inorder
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(start_gid+i)+drand48();
	  temp.y = 0.0;
	  parray1.SetElement(start_gid+i,temp);
	  parray2.SetElement(start_gid+i,temp);
	  parray3.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      case 1:{//2.0,0.0
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = 2.0;
	  temp.y = 0.0;
	  parray1.SetElement(start_gid+i,temp);
	  parray2.SetElement(start_gid+i,temp);
	  parray3.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      case 2:{//random
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(rand()%10000)+drand48();
	  temp.y = 0.0;
	  parray1.SetElement(start_gid+i,temp);
	  parray2.SetElement(start_gid+i,temp);
	  parray3.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      case 3:{//reverse
	for (int i = 0; i<local_size; i++){
	  Point temp;
	  temp.x = (double)(nElements-start_gid-i-1)+drand48();
	  temp.y = 0.0;
	  parray1.SetElement(start_gid+i,temp);
	  parray2.SetElement(start_gid+i,temp);
	  parray3.SetElement(start_gid+i,temp);
	}
	break;
      }
	
      default:
	cout<<"default in case statement"<<endl;
	break;
      }//end of switch(presort) for Points
      
      //get the corresponding pRange
      pArray<Point>::PRange prange1(&parray1);
      parray1.get_prange(prange1);

      pArray<Point>::PRange prange2(&parray2);
      parray2.get_prange(prange2);
      
      pArray<Point>::PRange prange3(&parray3);
      parray3.get_prange(prange3);

      vector<pArray<Point>::PRange> v;
      v.push_back(prange1);
      v.push_back(prange2);
      v.push_back(prange3);

      vector<pArray<Point>::PRange*> v2;
      v2.push_back(&prange1);
      v2.push_back(&prange2);
      v2.push_back(&prange3);

      //start timer
      if_want_perf { starttimer; }

      //inner_product
      Point pt(0);
      Point result = p_inner_product(prange1,prange2,pt,point_add,point_mult);
      Point result1 = p_inner_product(v,pt,point_add,point_mult);
      Point result2 = p_inner_product(v2,pt,point_add,point_mult);

      //stop_timer     
      if_want_perf { stoptimer; }
      stapl::rmi_fence();
      //print point
      if (threadID ==0){
	cout<<"RESULT="<<result<<endl;
	cout<<"RESULT1="<<result1<<endl;
	cout<<"RESULT2="<<result2<<endl;
      }
      break;
    }
      
    default:
      cout<<"default in case statement - types"<<endl;
      break;
    }//end of switch(types)
    
  }
};
