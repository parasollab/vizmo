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
  typedef stapl::pRange<linearBoundaryType, pvectorType> prange;
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
  for (parrayType::iterator i = input.local_begin();
       i != input.local_end();
       ++i) {
    *i = ++start;
  }
  //END OF PARRAY VERSION--------------------------------------

  pVectorType vinput(nElems);
  pVectorType voutput(nElems);
  stapl::rmi_fence();

  //filling a pVector
  start = 0;
  for (pVectorType::iterator vi = vinput.local_begin();
       vi != vinput.local_end();
       ++vi) {
    *vi = ++start;
  }
  //END OF PARRAY VERSION--------------------------------------

  stime = stapl::start_timer(); 
  partial_sum (input.local_begin(), input.local_end(), output.local_begin());  
  etime = stop_timer(stime);
  cout<<"time stapl parray: "<<etime<<endl;

  stime = stapl::start_timer(); 
  partial_sum (vinput.local_begin(), vinput.local_end(), voutput.local_begin());  
  etime = stop_timer(stime);
  cout<<"time stapl pvector: "<<etime<<endl;


  /////////////////start sequential 
  //VALARRAY(SEQUENTIAL) VERSION  
  //Define and fill a valarray
  valarray<int> val1(nElems), val2(nElems);
  
  for (int i=0; i < nElems; i++) {
    val1[i] = i + 1;
  }
  
  stime = stapl::start_timer(); 
   partial_sum (&val1[0], &val1[val1.size()], &val2[0]);
  etime = stop_timer(stime);
  cout<<"time stl: "<<etime<<endl;

  //END OF SEQUENTIAL VERSION
  //COMPARE THE RESULTS OF THE SEQUENTIAL WITH PARALLEL
  for (int i=0; i<val1.size();i++){
    //cout<<"val2[i]="<<val2[i]<<"; output[i]="<<output[i]<<endl;
    if (val2[i]!=output[i])
      cout<<"ERROR in prefix sum comparison"<<endl; 	
    
  }
  cout<<"done testing prefix sum"<<endl; 
  /////////////////stop sequential 
  stapl::rmi_fence();
}
