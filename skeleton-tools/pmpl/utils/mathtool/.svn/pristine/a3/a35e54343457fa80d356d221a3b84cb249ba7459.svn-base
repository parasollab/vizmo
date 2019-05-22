#ifndef GAUSS_H_
#define GAUSS_H_

#include "Basic.h"

namespace mathtool {

  struct GaussianRand {
    //generate a random number from N(_mean, _std).
    //generates 2 random numbers at a time, so it saves it for the next call
    double operator()(double _mean = 0, double _std = 1) {
      static bool hasNext = false;//flag that tells whether the next gaussian has been computed
      static double gset; //saved random for consequtive call

      double gr, v1, v2, rsq;// the result, two independent random variables, and r^2

      //If the next value to return is not already computed, compute it and the one after that
      if(!hasNext) {
        do {
          v1 = 2*drand48() - 1.0;
          v2 = 2*drand48() - 1.0;
          rsq = v1*v1 + v2*v2;
        } while (rsq >= 1.0 || rsq == 0.0);

        double fac = sqrt(-2.0*log(rsq)/rsq);

        //Generates two gaussians and returns one and stores the other for another call
        hasNext = true;
        gset = v1*fac;
        gr = v2*fac;
      }
      //Else return gset that was computed in the previous call
      else {
        hasNext = false;
        gr = gset;
      }

      return gr * _std + _mean;
    }
  };

}

#endif
