#include "pArray.h"
#include "pRange.h"
#include <algorithm>
#include "vector"
#include <values.h>
#include <../../../pAlgorithms/sample_sort.h>
#include <../../../pAlgorithms/radix_sort.h>

using namespace stapl;

struct assignindex : public unary_function<int, int> {
  int state;
  assignindex(int newstate){
    state = newstate;
  }
  int operator()(vector<int>& x,int val) { 
    x.push_back(val);
    return 1; 
  }
};

void stapl_main(int argc, char** argv) {

  int i,threadID=stapl::get_thread_id();
  int nThreads=stapl::get_num_threads();

  if(argc < 2) {
    cout<< "Usage: exe nElements" <<endl;
    exit(1);
  }

  printf ("%d %d %d \n",MAXSHORT,sizeof(long int), sizeof(long long int));
  int nElements = atoi(argv[1]);
  int start,stop;

  //timer stuff
  double etime=0;
  timer stime;
  
  typedef pArray<int>  PARRAY;
  PARRAY x(nElements);
  stapl::rmi_fence();
  PARRAY::PRange pr(&x);
  typedef stapl::defaultScheduler scheduler;


  //=======================================
  // filling a pArray
  //=======================================

  int local_size = nElements/nThreads;
  int start_gid = local_size*threadID; 
  if (threadID == (nThreads - 1)){
    local_size = nElements - threadID*local_size;
  }
  
  for (int i = 0; i<local_size; i++){
    x.SetElement(start_gid+i,i);
  }
  stapl::rmi_fence();
  
  x.get_prange(pr);
  
  if (threadID == 0){
    for (int i=0; i<nElements; i++){
      cout<<"i="<<i<<", data="<<pr.get(i)<<endl;
    } 
  }

  stime = stapl::start_timer(); 

//////////////////////// SAMPLE SORT ////////////////////////
  // p_sample_sort(pr,x);

//////////////////////// RADIX SORT ////////////////////////
  p_radix_sort(pr,x);

///////////////////////// FOR ALL SORTS /////////////////////
  etime = stop_timer(stime);
  cout<<"Parallel time: "<<etime<<endl;
  stapl::rmi_fence();
  
  //displaying the sorted pContainer/pRange
 
  if (threadID == 0){
    for (int i=0; i<nElements; i++){
      cout<<"i="<<i<<", data="<<pr.get(i)<<endl;
    }
  }
  stapl::rmi_fence(); 

}
