#include "pArray.h"
#include "pRange.h"
#include <algorithm>
#include "vector"

#include "mtl/mtl.h"
#include "mtl/utils.h"
#include "mtl/matrix.h"
#include "mtl/matrix_implementation.h"

#include "MatrixDistribution.h"
#include "pMatrix.h"

#define NUMTYPE double
#define SHAPE mtl::rectangle<>
#define STORAGE mtl::dense<>
#define ORIEN mtl::row_major

using namespace stapl;

void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}


void stapl_main(int argc, char** argv) {

  int i,j,myid=stapl::get_thread_id();
  int nThreads=stapl::get_num_threads();
  NUMTYPE tmp;

  if(argc < 2) {
    cout<< "Usage: exe nElements" <<endl;
    exit(1);
  }
  int nElements = atoi(argv[1]);

  typedef pMatrix<double,stapl_block_row> PMATRIX;
  typedef pMatrix<double,stapl_block_column> PMATRIXCOLUMN;
  PMATRIX::iterator mend,mi;
  PMATRIX::element_iterator eit;
  PMATRIXCOLUMN::iterator pmend,pmi;
  PMATRIXCOLUMN::element_iterator peit;

  stapl_print("testing constructor (M,N)...");
  PMATRIX P(nElements,nElements);
  PMATRIX Q(nElements,nElements);
  PMATRIXCOLUMN PB(nElements,nElements);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  mi = Q.local_begin();
  mend = Q.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      *eit = 0;
    }
  }

  mi = P.local_begin();
  mend = P.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      *eit = drand48();
    }
  }
  pmi = PB.local_begin();
  pmend = PB.local_end();
  for(;pmi != pmend;++pmi){
    for(peit= (*pmi).begin();peit != (*pmi).end(); ++peit){
      *peit = drand48();
    }
  }

  stapl_print("testing matrix matrix multiplication 2D...");
  stapl::timer t;

  t = stapl::start_timer();

   P.Multiply2D(PB,Q);

  double t2 = stapl::stop_timer(t);    
  cout<<"\nExecution time:"<<t2<<endl;
  stapl::rmi_fence();
  return;
}
