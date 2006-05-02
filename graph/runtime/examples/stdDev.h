#ifndef _STD_DEV_H
#define _STD_DEV_H


#include <stdio.h>
#include <math.h>
#include <vector>
#include <runtime.h>


struct stdDevTimer {
  std::vector<double> runTime;
  double totTime;
  double avgTime;
  int iteration;
  stapl::timer start;
  double curTime;
  int maxIteration;

  stdDevTimer(int maxIter) : runTime(), totTime(0), avgTime(0), 
    iteration(0), maxIteration(maxIter) { 
    runTime.reserve(maxIter); 
  }

  inline void startIteration() {
    start = stapl::start_timer();
  }

  inline void stopIteration() {
    curTime = stapl::stop_timer( start );
    ++iteration;
    runTime.push_back( curTime );
    totTime += curTime;
    avgTime = totTime / iteration;
  }

  void print(int threadID) {
    double stdDev = -1;    
    bool confident = false;
    if( iteration >= 3 ) {
      double sumSquares = 0;
      for( int i=0; i<iteration; i++ )
	sumSquares += ( runTime[i] - avgTime ) * ( runTime[i] - avgTime );
      stdDev = sqrt( sumSquares / (iteration - 1) );

      if( iteration >= 30 ) {
	double confInterval = 1.96 * stdDev / sqrt( static_cast<double>(iteration) );
	if( confInterval < avgTime*0.025 )
	  confident = true;
      }
    }

    if( confident )
      printf( "thread %d: avgTime %6.6f, stdDev %6.6f, 95%% CONFIDENT\n", threadID, avgTime, stdDev );
    else if( stdDev != -1 )
      printf( "thread %d: avgTime %6.6f, stdDev %6.6f\n", threadID, avgTime, stdDev );
    else
      printf( "thread %d: avgTime %6.6f\n", threadID, avgTime );
  }
};




template<class Rtn, class Arg1, class Arg2, class Arg3, class Arg4>
void confidenceExperiment(const int threadID, const int maxIteration, 
			  Rtn (*experiment)(Arg1, Arg2, Arg3, Arg4), 
			  Arg1 a1, Arg2 a2, Arg3 a3, Arg4 a4) {
  std::vector<double> runTime; 
  runTime.reserve( maxIteration );
  double totTime = 0;
  double avgTime = 0;
  double stdDev = -1;
  bool confident = false;

  int iteration = 0;
  while( iteration<maxIteration ) {
    const stapl::timer start = stapl::start_timer();
    experiment( a1, a2, a3, a4 );
    const double curTime = stapl::stop_timer( start );
    ++iteration;
    runTime.push_back( curTime );
    totTime += curTime;
    avgTime = totTime / iteration;

//    // Remove any drastic outliers before finishing the experiment.
//    if( iteration == maxIteration ) {
//	const double spuriousTime = avgTime*5;
//	for( int i=0; i<maxIteration; i++ ) {
//	  if( runTime[i] > spuriousTime ) {
//	    std::cout << "WARNING: removed spurious result of " << runTime[i]
//		      << " where current average is " << avgTime << std::endl;
//	    --iteration;
//	    runTime.erase( &runTime[i] );
//	  }
//	}
//    }

    // Execute enough iterations (samples) to ensure the statistical
    // significance of standard deviation and confidence interval.
//    else
    if( iteration >= 3 ) {
      double sumSquares = 0;
      for( int i=0; i<iteration; i++ )
	sumSquares += ( runTime[i] - avgTime ) * ( runTime[i] - avgTime );
      stdDev = sqrt( sumSquares / (iteration - 1) );

//	// Stop the experiment once we've executed the maximum number of
//	// iterations or we've reached the 95% confidence interval.  The 95%
//	// confidence interval means 95% of all runs should have a mean time
//	// within the interval (2.5% will be less, 2.5% will be greater).  This
//	// assumes the results of the experiment form a normal distribution!
//	const double confInterval = 1.96 * stdDev / sqrt( static_cast<double>(iteration) );
//	if( confInterval < avgTime*0.025 ) {
//	  iteration = maxIteration;
//	  confident = true;
//	}
    }
  }

  if( confident )
    printf( "thread %d: avgTime %6.6f, stdDev %6.6f, 95%% CONFIDENT\n", threadID, avgTime, stdDev );
  else if( stdDev != -1 )
    printf( "thread %d: avgTime %6.6f, stdDev %6.6f\n", threadID, avgTime, stdDev );
  else
    printf( "thread %d: avgTime %6.6f\n", threadID, avgTime );
}

template<class Rtn, class Arg1, class Arg2, class Arg3>
void confidenceExperiment(const int threadID, const int maxIteration, 
			  Rtn (*experiment)(Arg1, Arg2, Arg3), Arg1 a1, Arg2 a2, Arg3 a3) {
  confidenceExperiment( threadID, maxIteration, 
			(Rtn (*)(Arg1, Arg2, Arg3, int))experiment, 
			a1, a2, a3, 0 );
}

template<class Rtn, class Arg1, class Arg2>
void confidenceExperiment(const int threadID, const int maxIteration, 
			  Rtn (*experiment)(Arg1, Arg2), Arg1 a1, Arg2 a2) {
  confidenceExperiment( threadID, maxIteration, 
			(Rtn (*)(Arg1, Arg2, int, int))experiment, 
			a1, a2, 0, 0 );
}

template<class Rtn, class Arg1>
void confidenceExperiment(const int threadID, const int maxIteration, 
			  Rtn (*experiment)(Arg1), Arg1 a1) {
  confidenceExperiment( threadID, maxIteration, 
			(Rtn (*)(Arg1, int, int, int))experiment, 
			a1, 0, 0, 0 );
}

template<class Rtn>
void confidenceExperiment(const int threadID, const int maxIteration, 
			  Rtn (*experiment)()) {
  confidenceExperiment( threadID, maxIteration, 
			(Rtn (*)(int, int, int, int))experiment, 
			0, 0, 0, 0 );
}


#endif //_STD_DEV_H
