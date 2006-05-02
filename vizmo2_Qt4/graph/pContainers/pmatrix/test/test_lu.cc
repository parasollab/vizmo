#include "pArray.h"
#include "pRange.h"
#include <algorithm>
#include "vector"
#include "valarray"
#include "mtl/mtl.h"
#include "mtl/utils.h"
#include "mtl/matrix.h"
#include "mtl/lu.h"
#include "mtl/matrix_implementation.h"

#include "pContainers/pmatrix/pMatrixBCD.h"

#define NUMTYPE double
#define SHAPE mtl::rectangle<>
#define STORAGE mtl::dense<>
#define ORIEN mtl::row_major

using namespace stapl;


void stapl_main(int argc, char** argv) {

  int i,j,myid=stapl::get_thread_id();
  int nThreads=stapl::get_num_threads();

  if(argc < 3) {
    cout<< "Usage: exe nElements block size" <<endl;
    exit(1);
  }

  int nElements = atoi(argv[1]);
  int block = atoi(argv[2]);
  int start,stop;


  typedef mtl::matrix<NUMTYPE, SHAPE, STORAGE, ORIEN>::type Matrix;
  typedef mtl::matrix<NUMTYPE, SHAPE, STORAGE, mtl::column_major>::type MatrixC;
  Matrix A(nElements,nElements);

  for (i = 0; i < nElements; ++i) {
    //int tt = nElements*nElements - i;
    for (j = 0; j < nElements; ++j) {
      double tt = drand48();
      A(i,j) = tt;
    }
  }

  valarray<int> pivots(nElements);
  stapl::timer t = stapl::start_timer();

  mtl::lu_factor(A, pivots);

  double tt2 = stapl::stop_timer(t);    
  cout<<"Sequentiall Execution time:"<<tt2<<endl;
}
