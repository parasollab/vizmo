// This example implements prefix_sums().  It basically calls
// std::accumulate and std::partial_sum() for each thread's portion 
// of the calculation, and uses stapl::async_rmi() to combine the results. 

#include <runtime.h>
#include <pArray.h>
#include <pRange.h>
#include <numeric>

using namespace stapl;

void stapl_main(int argc, char *argv[]) {
  
 
 //VALARRAY(SEQUENTIAL) VERSION  

  //timer stuff
  double etime=0;
  stapl::timer stime;

  if( argc <= 2 ) {
    cout << "Usage: " << argv[0] << " [numints] [numiterations]\n";
    return;
  }
  int nElems = atoi( argv[1] );
  int nIters = atoi( argv[2] );
  
  //Define and fill a valarray
  valarray<int> val1(nElems), val2(nElems);
  
  for (int i=0; i < nElems; i++) {
    val1[i] = i + 1;
  }
  
  stime = stapl::start_timer(); 
  partial_sum (&val1[0], &val1[val1.size()], &val2[0]);
  etime = stop_timer(stime);
  cout<<"sequential time: "<<etime<<endl;
}
