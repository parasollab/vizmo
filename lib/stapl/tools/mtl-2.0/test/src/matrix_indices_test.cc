// -*- c++ -*-
//
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.
//
// All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
//
// Copyright 1997, 1998, 1999 University of Notre Dame.
// Authors: Andrew Lumsdaine, Jeremy G. Siek, Lie-Quan Lee
//
// This file is part of the Matrix Template Library
//
// You should have received a copy of the License Agreement for the
// Matrix Template Library along with the software;  see the
// file LICENSE.  If not, contact Office of Research, University of Notre
// Dame, Notre Dame, IN  46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//
//===========================================================================

#include "mtl/utils.h"
#include "mtl/matrix.h"
#include "matrix_test.h"

// matrix_attr.h is generated by make_and_test.pl and defines
// NUMTYPE, SHAPE, STORAGE, ORIEN, and TESTNAME
// you can create your own for testing purposes
#include "matrix_attr.h"

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   rectangle_tag, row_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int ii, jj;
  ii = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++ii) {
    jj = 0;
    for (typename Matrix::Row::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++jj)
      if (Int(j.row()) != ii ||  Int(j.column()) != jj) {
	std::cerr << "**** FAILED: (iterator indices rect row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(j.row(),j.column()) = (" << j.row() << "," << j.column()
	     << ")" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;	
	return false;
      }
  }
  std::cout << test_name.c_str() << " passed iterator indices" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   rectangle_tag, column_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int ii, jj;
  ii = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++ii) {
    jj = 0;
    for (typename Matrix::Column::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++jj)
      if (Int(j.row()) != jj ||  Int(j.column()) != ii) {
	std::cerr << "**** FAILED: (iterator indices rect column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(j.row(),j.column()) = (" << j.row() << "," << j.column()
	     << ")" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;	
	return false;
      }
  }
  std::cout << test_name.c_str() << " passed iterator indices (rect col)" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   banded_tag, row_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int ii, jj;
  int sub = A.sub();
  ii = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++ii) {
    jj = 0;
    for (typename Matrix::Row::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++jj) {
      Int row = ii;
      Int col = (jj + MTL_MAX(ii - sub, 0));

      if (Int(j.row()) != Int(row) ||  Int(j.column()) != Int(col)) {
	std::cerr << "**** FAILED: (iterator indices banded row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   banded_tag, column_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int ii, jj;
  int super = A.super();
  jj = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++jj) {
    ii = 0;
    for (typename Matrix::Column::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++ii) {
      Int row = (ii + MTL_MAX(jj - super, 0));
      Int col = jj;
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices banded column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices banded column" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   symmetric_tag, row_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int ii, jj;
  int sub = A.is_lower() ? A.sub() : 0;
  ii = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++ii) {
    jj = 0;
    for (typename Matrix::Row::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++jj) {
      Int row = ii;
      Int col = (jj + MTL_MAX(ii - sub, 0));
      if (Int(j.row()) != row ||  Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices banded row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   symmetric_tag, column_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int ii, jj;
  int super = A.is_upper() ? A.super() : 0;
  jj = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++jj) {
    ii = 0;
    for (typename Matrix::Column::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++ii) {
      Int row = (ii + MTL_MAX(jj - super, 0));
      Int col = jj;
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices banded column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(ii,jj) = (" << ii << "," << jj << ")" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices banded column" << std::endl;
  return true;
}


template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   diagonal_tag, row_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int d = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++d) {
    Int row = MTL_MAX(d - int(A.super()), 0);
    Int col = MTL_MAX(int(A.super()) - d, 0);
    for (typename Matrix::Diagonal::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++row, ++col) {
      if (Int(j.row()) != Int(row) || Int(j.column()) != Int(col)) {
	std::cerr << "**** FAILED: (iterator indices diag row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices diag row" << std::endl;
  return true;
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, 
			   diagonal_tag, column_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  int d = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++d) {
    Int col = MTL_MAX(d - int(A.sub()), 0);
    Int row = MTL_MAX(int(A.sub()) - d, 0);
    for (typename Matrix::Diagonal::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++row, ++col) {
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices diag column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  std::cout << test_name.c_str() << " passed iterator indices diag column" << std::endl;
  return true;
}


template <class Matrix, class Shape>
bool iterator_indices_test_sparse(const Matrix& A, std::string test_name, 
				  row_tag, Shape)
{
  typedef typename mtl::matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int row, col;
  row = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++row) {
    col = 0;
    for (typename Matrix::Row::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++col) {
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices sparse row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  return true;
}

template <class Matrix, class Shape>
bool iterator_indices_test_sparse(const Matrix& A, std::string test_name, 
				  column_tag, Shape)
{
  typedef typename mtl::matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int row, col;
  col = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++col) {
    row = 0;
    for (typename Matrix::Column::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++row) {
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices sparse column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  return true;
}

template <class Matrix>
bool iterator_indices_test_sparse(const Matrix& A, std::string test_name, 
				  row_tag, symmetric_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int row, col;
  row = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++row) {
    col = 0;
    for (typename Matrix::Row::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++col) {
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices sparse row) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  return true;
}

template <class Matrix>
bool iterator_indices_test_sparse(const Matrix& A, std::string test_name, 
				  column_tag, symmetric_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;
  Int row, col;
  col = 0;
  for (typename Matrix::const_iterator i = A.begin();
       i != A.end(); ++i, ++col) {
    row = 0;
    for (typename Matrix::Column::const_iterator j = (*i).begin();
	 j != (*i).end(); ++j, ++row) {
      if (Int(j.row()) != row || Int(j.column()) != col) {
	std::cerr << "**** FAILED: (iterator indices sparse column) "
	     << test_name.c_str() << " ****" << std::endl;
	std::cerr << "(row,col) = (" << row << "," << col << ")" << std::endl;
	std::cerr << "(j.row(),j.col) = (" << j.row() << "," 
	     << j.column() << ")" << std::endl;
	return false;
      }
    }
  }
  return true;
}



template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, dense_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::shape Shape;
  typedef typename mtl::matrix_traits<Matrix>::orientation Orien;
  return iterator_indices_test(A, test_name, Shape(), Orien());
}

template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name, sparse_tag)
{
  typedef typename mtl::matrix_traits<Matrix>::orientation Orien;
  typedef typename mtl::matrix_traits<Matrix>::shape Shape;
  return iterator_indices_test_sparse(A, test_name, Orien(), Shape());
}


template <class Matrix>
bool iterator_indices_test(const Matrix& A, std::string test_name)
{
  typedef typename mtl::matrix_traits<Matrix>::sparsity Sparsity;
  return iterator_indices_test(A, test_name, Sparsity());
}


template <class Matrix>
void
do_test(Matrix& A, std::string test_name)
{
  using namespace mtl;

  typedef typename mtl::matrix_traits<Matrix>::value_type T;
  typedef typename mtl::matrix_traits<Matrix>::size_type Int;

  iterator_fill(A);

  iterator_indices_test(A, test_name);
}


int
main(int argc, char* argv[])
{
  if (argc < 5) {
    std::cerr << "matrix_test <M> <N> <SUB> <SUPER>" << std::endl;
    return -1;
  }

  using namespace mtl;
  using std::string;

  const int M = atoi(argv[1]);
  const int N = atoi(argv[2]);
  const int SUB = atoi(argv[3]);
  const int SUP = atoi(argv[4]);

  std::cout << "M: " << M << " N: " << N 
       << " SUB: " << SUB << " SUPER: " << SUP << std::endl;

  typedef matrix<NUMTYPE, SHAPE, STORAGE, ORIEN>::type Matrix;

  string test_name = TESTNAME;
  Matrix* a = 0;

  create_and_run(M, N, SUB, SUP, test_name, a, Matrix::shape());

  return 0;
}