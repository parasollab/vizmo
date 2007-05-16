// This example implements prefix_sums().  It basically calls
// std::accumulate and std::partial_sum() for each thread's portion 
// of the calculation, and uses stapl::async_rmi() to combine the results. 

#include <runtime.h>
#include <pArray.h>
#include <pRange.h>
#include <numeric>

using namespace stapl;

template<class pRange1, class pRange2, class T>
struct prefix_sums : public stapl::parallel_task {
  pRange1 input;
  pRange2 output;
  T initial_value;
  T result;
  vector<T> indexarray;
  double t1,t2,t3;
  stapl::timer stime;

  prefix_sums(pRange1 _input, pRange2 _output, T _sum)
     : input(_input), output(_output), initial_value(_sum), result(_sum), indexarray(stapl::get_num_threads()){} 
   
  struct combo {
    int thread_id;
    T partsum;
    T startindex;

    combo() : thread_id(stapl::get_thread_id()), partsum(0), startindex(0) {}
    
    void define_type(stapl::typer& t) { 
      t.local(thread_id);
      t.local(partsum); 
      t.local(startindex); 
    }
  };

  void set_value(combo* in) {
    indexarray[in->thread_id] = in->partsum;
  }

  void set_initial_value(T* val) {
    initial_value = *val;
  }

  void printTimers(){
    cout<<"Phase 1:"<<t1<<"\nPhase2:"<<t2<<"\nPhase3:"<<t3<<endl;
  }

  void execute() {
    combo c, rtn;
   
    //if the pRange is not empty
    if ( input.get_boundary().start() != input.get_boundary().finish() ) {
      //stime = stapl::start_timer();       
      //1.all processors sum up array parts in parallel using stl's accumulate
      c.partsum = accumulate(input.get_boundary().start(), 
			     input.get_boundary().finish(), 
			     0);

      //t1 = stop_timer(stime);

      //stime = stapl::start_timer();       

      //2.data exchange
      //2.1.each processor sends processor 0 its partsum  
      c.thread_id = stapl::get_thread_id();
      if (stapl::get_thread_id() != 0) {
        async_rmi(0, rmiHandle, &prefix_sums<pRange1, pRange2, T>::set_value, &c);
      } 
      else {
        indexarray[0] = c.partsum;
      }
      stapl::rmi_fence();

      //2.2.processor 0 calculates the starting sums for all processors
      //using stl's partial_sum
      if (stapl::get_thread_id()==0){
	partial_sum(indexarray.begin(), indexarray.end(), indexarray.begin());
      //2.3.processor 0 sends out to all processors their starting sums
	for (int i = 1; i < stapl::get_num_threads(); i++){
	  stapl::async_rmi(i, rmiHandle, &prefix_sums<pRange1, pRange2, T>::set_initial_value, &indexarray[i-1]);
	}
      }
      stapl::rmi_fence();
      //t2 = stop_timer(stime);

      //stime = stapl::start_timer();       
      //3.all processors get prefix sums using stl's partial_sum
      
      //VERSION 1: THIS WILL BE USED ONCE THE ITERATOR WORKS RIGHT
      // partial_sum(input.get_boundary().start(), 
      //	  input.get_boundary().finish(),
      //       	  output.get_boundary().start()); 
      //VERSION 1: THE END

      //VERSION 2: THIS SHOULD BE USED UNTIL THE ITERATOR RETURNS COPIES
      typename pRange2::iteratorType __result = output.get_boundary().start();
      typename pRange1::iteratorType __first = input.get_boundary().start();
      typename pRange1::iteratorType __last = input.get_boundary().finish();

      if (__first == __last) return;
      *__result = *__first + initial_value;
      T __val   = *__first + initial_value;
      ++__first;++__result;
      while (__first != __last) {
	T __tmp = *__first;
	*__result = __tmp + __val;
	__val += __tmp;
	++__first;++__result;
      }
      /*
	//olga's version
	*o = *i + initial_value;
	o++; i++;
	for (; i != input.get_boundary().finish(); i++, o++, prev_o++) {
	*o = *i + *prev_o;
	}
      */
      //VERSION 2: THE END
      //t3 = stop_timer(stime);
    }//end of if not empty pRange
  }//end of execute()
};


void stapl_main(int argc, char *argv[]) {
  
  //PVECTOR VERSION----------------------------------------------
  typedef stapl::pvector<int> pvectorType;
  typedef stapl::linear_boundary<pvectorType::iterator> linearBoundaryType;
  typedef stapl::pRange<linearBoundaryType, pvectorType> prange;

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


  //PVECTOR VERSION--------------------------------------------

  //instantiating a pVector
  pvectorType input(nElems),output(nElems);
  stapl::rmi_fence();

  //filling a pVector
  int start = id * (nElems/nThreads);
  for (pvectorType::iterator i = input.local_begin();
       i != input.local_end();
       ++i) {
    *i = ++start;
  }
  //END OF PVECTOR VERSION-------------------------------------

  //instantiating a pRange
  prange pri(&input, linearBoundaryType(input.local_begin(), input.local_end()));
  prange pro(&input, linearBoundaryType(output.local_begin(), output.local_end()));
  stapl::rmi_fence();

  //displaying the input pContainer
  //input.DisplayPContainer();

  //instantiating a prefix sums class
  prefix_sums<prange,prange,int> psums(pri, pro, 0);
  stapl::rmi_fence();

  //calculating the prefix sums
  stime = stapl::start_timer(); 
  stapl::execute_parallel_task(&psums);
  etime = stop_timer(stime);
  cout<<"p time: "<<etime<<endl;
  //psums.printTimers();
  
  //displaying the output pContainer
  //input.DisplayPContainer();
  /*
  stapl::rmi_fence();
  /////////////////start sequential 
  if(stapl::get_thread_id() == 0){
    //VALARRAY(SEQUENTIAL) VERSION  
    //Define and fill a valarray
    valarray<int> val1(nElems), val2(nElems);

    for (int i=0; i < nElems; i++) {
      val1[i] = i + 1;
    }

    stime = stapl::start_timer(); 
    partial_sum (&val1[0], &val1[val1.size()], &val2[0]);
    etime = stop_timer(stime);
    cout<<"time seq: "<<etime<<endl;
    //END OF SEQUENTIAL VERSION
    //COMPARE THE RESULTS OF THE SEQUENTIAL WITH PARALLEL
    for (int i=0; i<val1.size();i++){
      //cout<<"val2[i]="<<val2[i]<<"; output[i]="<<output[i]<<endl;
      if (val2[i]!=output.GetElement(i))
	cout<<"ERROR in prefix sum comparison"<<endl; 	

    }
    cout<<"done testing prefix sum"<<endl; 
  }
  /////////////////stop sequential 
  */
  stapl::rmi_fence();
}
