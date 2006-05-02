// This example solves Poisson's equation, dV2/dx2 + dV2/dy2 = b using a
// parallel jacobi iterative solver.  Each thread operates on n x n matrices,
// where there is a spike in the middle of each thread's b matrix, and the
// initial guess for V is 0.  Data is manually distributed via 1-D vertical
// block partitioning.  A fixed number of iterations are performed, instead of
// testing for convergence.

#include "stdDev.h"
#include "testContainers.h"

#include <runtime.h>
#include <stdlib.h>


// Implements a 2-D n x n matrix as a 1-D array.
template<class T> struct Matrix {
  struct Row {
    T* _row;
    int _n;
    Row(T* const row, const int n) : _row(row), _n(n) {}
    void define_type(stapl::typer& t) { t.dynamic( _row, _n ); t.local( _n ); }
          T& operator[](const int column)       { return _row[column]; }
    const T& operator[](const int column) const { return _row[column]; }
  };

  T* const _matrix;
  const int _n;

  Matrix(const int n) : _matrix(new T[n*n]), _n(n) {}
  ~Matrix() { delete[] _matrix; }
        Row operator[](const int row)       { return Row( &_matrix[row*_n], _n ); }
  const Row operator[](const int row) const { return Row( &_matrix[row*_n], _n ); }
};


struct p_jacobi : public stapl::parallel_task {
  const int _iters;
  const int _n;
  Matrix<double>& _V;
  const Matrix<double>& _b;
  Matrix<double> _tmp; // _tmp includes a perimeter of ghost nodes

  p_jacobi(const int iters, Matrix<double>& V, Matrix<double>& b) :
    _iters(iters), _n(V._n), _V(V), _b(b), _tmp(_n+2) {}

  void updateTopGhost(const Matrix<double>::Row& row) {
    memcpy( &_tmp[0][1], &row[0], _n );
  }

  void updateBottomGhost(const Matrix<double>::Row& row) {
    memcpy( &_tmp[_n+1][1], &row[0], _n );
  }

  void execute() {
    stapl::set_aggregation( 0 );
    const int id = stapl::get_thread_id();
    const int nThreads = stapl::get_num_threads();
    for( int iter=0; iter<_iters; ++iter ) {

      // update north & south neighbors' ghost nodes with current estimate of V
      if( id != nThreads-1 )
	stapl::async_rmi( id+1, rmiHandle, &p_jacobi::updateTopGhost, _V[_n-1] );
      if( id != 0 )
	stapl::async_rmi( id-1, rmiHandle, &p_jacobi::updateBottomGhost, _V[0] );

      // update _tmp with current estimate of V
      for( int i=0; i<_n; ++i )
	memcpy( &_tmp[i+1][1], &_V[i][0], _n );

      // wait for incoming ghost nodes' updates
      if( id != 0 )
	stapl::rmi_wait();
      if( id != nThreads-1 )
	stapl::rmi_wait();

      // perform jacobi 5-point stencil to find next estimate of V
      for( int i=1; i<_n+1; ++i )
	for( int j=1; j<_n+1; ++j )
	  _V[i-1][j-1] = 0.25 * ( _tmp[i-1][j] + _tmp[i+1][j] +
				  _tmp[i][j-1] + _tmp[i][j+1] - _b[i-1][j-1] );
    }
    stapl::rmi_fence();
  }
};


void stapl_main(int argc, char *argv[]) {
  int id = stapl::get_thread_id();
  if( id == 0 )
    if( argc != 4 ) {
      printf( "Usage: %s [n] [jacobi iterations] [numiterations]\n", argv[0] );
      return;
    }
  int n      = atoi( argv[1] );
  int jIters = atoi( argv[2] );
  int nIters = atoi( argv[3] );

  Matrix<double> V( n );
  Matrix<double> b( n );
  b[n/2][n/2] = -1.0; // b is a spike

  p_jacobi pj( jIters, V, b );
  confidenceExperiment( id, nIters, stapl::execute_parallel_task, (stapl::parallel_task*)&pj );
}
