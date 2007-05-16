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

  //Matrix definition
  typedef mtl::matrix<NUMTYPE, SHAPE, STORAGE, ORIEN>::type Matrix;
  Matrix A(nElements,nElements);
  Matrix B(nElements,nElements);


  //pMatrix definitions; 
  //row major
  typedef pMatrix<double,stapl_row_major> PMATRIX;

  //column major
  typedef pMatrix<double,stapl_column_column_major> PMATRIXCOLUMN;



  stapl_print("testing constructor (M,N)...");
  PMATRIX P(nElements,nElements);
  PMATRIX Q(nElements,nElements);
  PMATRIXCOLUMN PB(nElements,nElements);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  if(stapl::get_thread_id() == 0)
    PB.DisplayDistributionInfo();

  stapl_print("testing SetElement(i,j,value)...");
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	P.SetElement(i,j,i*nElements+j);
	Q.SetElement(i,j,11);
	PB.SetElement(i,j,9);
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing GetElement(i,j,value)...");
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	tmp = P.GetElement(i,j);
	if(tmp != i*nElements+j)
	  cout<<"ERROR while testing GetElement"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing copy constructor and iterators...");
  PMATRIX P1(P);
  stapl::rmi_fence();
  PMATRIX::iterator mend,mi = P.local_begin();
  PMATRIX::element_iterator eit;
  PMATRIX::iterator mend1,mi1 = P1.local_begin();
  PMATRIX::element_iterator eit1;
  mend = P.local_end();
  mend1 = P1.local_end();
  for(;mi != mend;++mi,++mi1){
    eit1 = (*mi1).begin();
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit,++eit1){
      if(*eit != *eit1) cout<<"ERROR while testing the copy constructor"<<endl;
    }
  }


  stapl_print("Passed\n");

  stapl_print("testing constructor with layout...");
  pair< unsigned int, unsigned int > _layout(1,nThreads);
  PMATRIX P2(nElements,nElements,_layout);
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	P2.SetElement(i,j,i*nElements+j);
      }
    }
  }
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	tmp = P2.GetElement(i,j);
	if(tmp != i*nElements+j)
	  cout<<"ERROR while testing constructor with layout"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing constructor with user specified distribution...");
  if(nThreads == 4){
    vector<MatrixDistributionInfo> di;
    MatrixDistributionInfo dentry;
    Location l;
    
    dentry.SetI(0);dentry.SetJ(0); 
    dentry.SetnRows(2);dentry.SetnColumns(4); 
    l.SetPid(0);l.SetPartId(0);
    dentry.SetLocation(l);
    di.push_back(dentry);
    
    dentry.SetI(0);dentry.SetJ(4); 
    dentry.SetnRows(2);dentry.SetnColumns(4); 
    l.SetPid(1);l.SetPartId(0);
    dentry.SetLocation(l);
    di.push_back(dentry);
    
    dentry.SetI(2);dentry.SetJ(0); 
    dentry.SetnRows(2);dentry.SetnColumns(4); 
    l.SetPid(2);l.SetPartId(0);
    dentry.SetLocation(l);
    di.push_back(dentry);
    
    dentry.SetI(2);dentry.SetJ(4); 
    dentry.SetnRows(2);dentry.SetnColumns(4); 
    l.SetPid(3);l.SetPartId(0);
    dentry.SetLocation(l);
    di.push_back(dentry);
    PMATRIX P3(4,8,di);
    if(myid == 0){
      for (i = 0; i < 4; ++i) {
	for (j = 0; j < 8; ++j) {
	  P2.SetElement(i,j,i*4+j);
	}
      }
    }
    stapl::rmi_fence();
    if(myid == 0){
      for (i = 0; i < 4; ++i) {
	for (j = 0; j < 8; ++j) {
	  tmp = P2.GetElement(i,j);
	  if(tmp != i*4+j)
	    cout<<"ERROR while testing constructor with layout"<<endl;
	}
      }
    }
    stapl::rmi_fence();
    stapl_print("Passed\n");
  }
  else{
    stapl_print("\n pMatrix constructor with specified distribution skept\n");
    stapl_print("because it works only when nThreads=4\n");
  }

  stapl_print("testing initialize...");
  assignx ax(nElements,nElements);
  P.initialize(ax);
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	tmp = P.GetElement(i,j);
	if(tmp != i*nElements+j)
	  cout<<"ERROR while testing initialize"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing assignment operator and (i,j) operator...");
  P1 = P;
  stapl::rmi_fence();
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	tmp = P1(i,j);
	if(tmp != i*nElements+j)
	  cout<<"ERROR while testing initialize"<<endl;
      }
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing Add...");
  P.Add(P1);
  mi = P.local_begin();
  mi1 = P1.local_begin();
  mend = P.local_end();
  mend1 = P1.local_end();
  for(;mi != mend;++mi,++mi1){
    eit1 = (*mi1).begin();
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit,++eit1){
      if(*eit != ((*eit1)+(*eit1))) 
	cout<<"ERROR while testing the Add method:"<<*eit<<":"<<*eit1+*eit1<<endl;
    }
  }  
  stapl_print("Passed\n");

  stapl_print("testing Subtract...");
  P.Subtract(P1);
  mi = P.local_begin();
  mi1 = P1.local_begin();
  mend = P.local_end();
  mend1 = P1.local_end();
  for(;mi != mend;++mi,++mi1){
    eit1 = (*mi1).begin();
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit,++eit1){
      if(*eit != *eit1) cout<<"ERROR while testing the Subtract method"<<endl;
    }
  }  
  stapl_print("Passed\n");

  stapl_print("testing Scale...");
  mi = P.local_begin();
  mend = P.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      *eit = 1;
    }
  }  
  tmp  = -1;
  P.Scale(tmp);
  mi = P.local_begin();
  mend = P.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      if(*eit != -1)
	cout<<"ERROR while scaling the matrix"<<endl ;
    }
  }
  stapl::rmi_fence();
  P.Scale(tmp);
  stapl_print("Passed\n");

  stapl_print("testing matrix array multiplication...");
  typedef pArray<double> PARRAY; 
  PARRAY a(nElements);
  PARRAY b(nElements);
  //P contains 1 on all positions
  for (PARRAY::iterator it=a.local_begin(); it != a.local_end(); ++it){
    *it = 2;
  }
  stapl::rmi_fence();

  P.Multiply(a,b);
  stapl::rmi_fence();
  double expected = nElements*2;
  for (PARRAY::iterator it=b.local_begin(); it != b.local_end(); ++it){
    if(*it != expected)
      cout<<myid<<"error in matrix array multiplication"<<endl;
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing rows operation...");
  MatrixOp func;
  P.RowsOperation(1,7,func);
  stapl::rmi_fence();
  if(myid == 0){
    i = 1;
    for (j = 0; j < nElements; ++j) {
      tmp = P(i,j);
      if(tmp != 2)
	cout<<"ERROR while testing operation on rows"<<endl;
    }
  }  
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing swap rows...");
  P.SwapRows(1,7);
  stapl::rmi_fence();
  if(myid == 0){
    i = 7;
    for (j = 0; j < nElements; ++j) {
      tmp = P(i,j);
      if(tmp != 2)
	cout<<"ERROR while testing operation on rows"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing matrix matrix multiplication...");
  mi = P.local_begin();
  mend = P.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit){
      *eit = 1;
    }
  }
  stapl::rmi_fence();

  P.Multiply(PB,Q);

  stapl::rmi_fence();
  /*
  cout<<endl;
  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	cout<<Q(i,j)<<" ";
      }
      cout<<endl;
    }
  }
  stapl::rmi_fence();

  if(myid == 0){
    for (i = 0; i < nElements; ++i) {
      for (j = 0; j < nElements; ++j) {
	cout<<P(i,j)<<" ";
      }
      cout<<endl;
    }
  }
  stapl::rmi_fence();
  */

  mi = Q.local_begin();
  mend = Q.local_end();
  for(;mi != mend;++mi){
    for(eit= (*mi).begin();eit != (*mi).end(); ++eit,++eit1){
      if(*eit != nElements*9+11) 
	cout<<"ERROR while testing the matrix matrix multiply method:"<<*eit<<endl;
    }
  }  
  stapl_print("Passed\n");

  stapl_print("FINISHED\n");
  stapl::rmi_fence();
  return;
}
