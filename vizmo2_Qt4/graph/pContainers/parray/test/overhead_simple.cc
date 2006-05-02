// This example implements prefix_sums().  It basically calls
// std::accumulate and std::partial_sum() for each thread's portion 
// of the calculation, and uses stapl::async_rmi() to combine the results. 

#include <runtime.h>
#include <pArray.h>
#include <pRange.h>
#include <numeric>

using namespace stapl;

void stapl_main(int argc, char *argv[]) {
  
  //PARRAY VERSION----------------------------------------------
  typedef stapl::pArray<int> parrayType;
  typedef stapl::linear_boundary<parrayType::iterator> linearBoundaryType;
  typedef stapl::pRange<linearBoundaryType, parrayType> prange;


  //PVECTOR VERSION----------------------------------------------
  typedef stapl::pvector<int> pVectorType;

/*
  typedef stapl::linear_boundary<pvectorType::iterator> linearBoundaryType;
  typedef stapl::pRange<linearBoundaryType, pVectorType> prange;
*/ 

  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if( id == 0 )
    if( argc <= 2 ) {
      cout << "Usage: " << argv[0] << " [numints] [numiterations]\n";
      return;
    }
  int nElems = atoi( argv[1] );
  int nIters = atoi( argv[2] );

  //timer stuff
  double etime=0;
  stapl::timer stime;

 
  //PARRAY VERSION----------------------------------------------

  //instantiating a pArray
  parrayType input(nElems);
  parrayType output(nElems);
  stapl::rmi_fence();

  //filling a pArray
  int start = 0;

  stime = stapl::start_timer();
  for (parrayType::iterator i = input.local_begin();
       i != input.local_end();
       ++i) {
    *i = ++start;
  }
  etime = stop_timer(stime);
  cout<<"time stapl parray: "<<etime<<endl;


  pVectorType vinput(nElems);
  pVectorType voutput(nElems);
  stapl::rmi_fence();
  start=0;
  stime = stapl::start_timer();
  for (pVectorType::iterator vi = vinput.local_begin();
       vi != vinput.local_end();
       ++vi) {
    *vi = ++start;
  }
  etime = stop_timer(stime);
  cout<<"time stapl pvector: "<<etime<<endl;


  /////////////////start sequential 
  //VALARRAY(SEQUENTIAL) VERSION  
  //Define and fill a valarray
  valarray<int> val1(nElems), val2(nElems);
  start = 0;
  stime = stapl::start_timer(); 
  for (int* j = &val1[0];
       j != &val1[val1.size()];
       j++) {
    *j = ++start;
  }
  etime = stop_timer(stime);
  cout<<"time stl: "<<etime<<endl;

  //END OF SEQUENTIAL VERSION
  //COMPARE THE RESULTS OF THE SEQUENTIAL WITH PARALLEL
  for (int i=0; i<val1.size();i++){
    //cout<<"val2[i]="<<val2[i]<<"; output[i]="<<output[i]<<endl;
    if (val1[i]!=input[i])
      cout<<"ERROR in prefix sum comparison"<<endl; 	
    
  }
  cout<<"done testing prefix sum"<<endl; 
  /////////////////stop sequential 
  stapl::rmi_fence();
}
