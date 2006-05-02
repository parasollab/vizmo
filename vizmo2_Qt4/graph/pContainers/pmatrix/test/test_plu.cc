#include "pArray.h"
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

class assignx{
  unsigned int M,N;
public:
  assignx(unsigned int _m, unsigned int _n){
    M=_m;
    N=_n;
  }
  int operator()(double& data, unsigned int _i, unsigned int _j){
    data = _i*N+_j;
  }
  void define_type(stapl::typer &t){}
};


class MatrixOp{
public:
  MatrixOp(){}
  double operator()(double& d1, double& d2){
    return d1 + d2;
  }
  void define_type(stapl::typer &t){}
};


void stapl_main(int argc, char** argv) {

  int i,j,myid=stapl::get_thread_id();
  int nThreads=stapl::get_num_threads();

  if(argc < 4) {
    cout<< "Usage: exe nElements block_size aggfactor" <<endl;
    exit(1);
  }

  int nElements = atoi(argv[1]);
  int block = atoi(argv[2]);
  int aggf = atoi(argv[3]);

  typedef pMatrixBCD<double,stapl_block_row> PMATRIX;
  typedef pMatrixBCD<double,stapl_column_column_major> PMATRIXCOLUMN;
  typedef mtl::matrix<double, 
    mtl::rectangle<>, 
    //mtl::dense<mtl::external>, 
    mtl::dense<>, 
    mtl::row_major>::type MatrixLU;
  
  PMATRIX P(nElements,nElements,block,block);
  stapl::rmi_fence();
  stapl::timer t;

  cout<<"Elements="<<nElements;
  cout<<" Block size:"<<block;
  cout<<" AggF:"<< aggf<<endl;
  stapl::set_aggregation(1000);

  //MatrixLU ALU(nElements,nElements);
  t = stapl::start_timer();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      //int tt = nElements*nElements - i;
      for (j = 0; j < nElements; ++j) {
	double tt = drand48();
	P.SetElement(i,j,tt);
	//ALU(i,j) = tt;
      }
    }
  }  
  stapl::rmi_fence();
  double t2 = stapl::stop_timer(t);    
  cout<<"Init time:"<<t2<<endl;

  stapl::set_aggregation(aggf);
  stapl::rmi_fence();

  t = stapl::start_timer();
   P.LU();
   stapl::rmi_fence();
  t2 = stapl::stop_timer(t);    
  cout<<"Parallel Execution time:"<<t2<<endl;

  /*
  stapl::rmi_fence();
  if(myid == 0){
    cout<<"Accessing the elements of the  pMatrix"<<endl;
    for (i = 0; i < N1; ++i) {
      for (j = 0; j < N1; ++j) {
        if (P.GetElement(i,j) != ALU(i,j)) cout<<"ERROR while LU decomp["<<i<<":"<<j<<"]"<<endl;
	//cout<<P.GetElement(i,j)<<" ";
      }
      //cout<<endl;
    }
  }
  */
  stapl::rmi_fence();
  return;
}
