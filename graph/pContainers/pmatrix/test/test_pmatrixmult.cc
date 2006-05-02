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

class assignx{
  unsigned int M,N;
public:
  assignx(unsigned int _m, unsigned int _n){
    M=_m;
    N=_n;
  }
  int operator()(double& data, unsigned int _i, unsigned int _j){
    data = _i*N+_j;
    return OK;
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


  typedef mtl::matrix<NUMTYPE, SHAPE, STORAGE, ORIEN>::type Matrix;
  typedef mtl::matrix<NUMTYPE, SHAPE, STORAGE, mtl::column_major>::type MatrixC;
  Matrix A(nElements,nElements);
  MatrixC B(nElements,nElements);
  Matrix C(nElements,nElements);


  //pMatrix definitions; 
  //row major
  typedef pMatrix<double,stapl_row_major> PMATRIXROW;
  //column major
  typedef pMatrix<double,stapl_column_column_major> PMATRIXCOLUMNCOLUMN;

  typedef pMatrix<double,stapl_block_row> PMATRIX;
  typedef pMatrix<double,stapl_block_column> PMATRIXCOLUMN;
  PMATRIX::iterator mend,mi;
  PMATRIX::element_iterator eit;


  stapl_print("testing constructor (M,N)...");
  PMATRIX P(nElements,nElements);
  PMATRIX Q(nElements,nElements);
  PMATRIXCOLUMN PB(nElements,nElements);
  PMATRIXROW PR(nElements,nElements);
  PMATRIXROW QR(nElements,nElements);
  PMATRIXCOLUMNCOLUMN PBCC(nElements,nElements);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  if(stapl::get_thread_id() == 0)
    PB.DisplayDistributionInfo();

  stapl_print("testing SetElement(i,j,value)...");
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	A(i,j) = i*nElements+j;
	B(i,j) = i*nElements+j;
	C(i,j) = 0;
	P.SetElement(i,j,i*nElements+j);
	PB.SetElement(i,j,i*nElements+j);
	Q.SetElement(i,j,0);
	PR.SetElement(i,j,i*nElements+j);
	PBCC.SetElement(i,j,i*nElements+j);
	QR.SetElement(i,j,0);
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing multiply sequential...");
  if(myid == 0)
    mtl::mult(A,B,C);
  stapl_print("Passed\n");

  stapl_print("testing multiply 1D...");
  PR.Multiply(PBCC,QR);
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
        if (QR.GetElement(i,j) != C(i,j)){
	  cout<<"ERROR while multiplying 1D["<<i<<":"<<j<<"]";
	  cout<<QR.GetElement(i,j)<<"<>"<<C(i,j)<<endl;
	}
      }
      //cout<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing multiply 2D...");
  P.Multiply2D(PB,Q);
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
        if (Q.GetElement(i,j) != C(i,j)) {
	  cout<<"ERROR while multiplying 2D["<<i<<":"<<j<<"]";
	  cout<<Q.GetElement(i,j)<<"<>"<<C(i,j)<<endl;
	}
      }
      //cout<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  //reset Q to zero
  mi = Q.local_begin();
  mend = Q.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      *eit = 0;
    }
  }
  stapl::rmi_fence();
  //cout<<myid<<" ";
  //mtl::print_all_matrix(PB.GetPart(0)->part_data);

  stapl_print("testing multiply BMR...");
  P.Multiply2DBMR(PB,Q);
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
        if (Q.GetElement(i,j) != C(i,j)) {
	  cout<<"ERROR while multiplying BMR["<<i<<":"<<j<<"]";
	  cout<<Q.GetElement(i,j)<<"<>"<<C(i,j)<<endl;
	}
      }
      //cout<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("FINISHED\n");
  stapl::rmi_fence();
  return;
}
