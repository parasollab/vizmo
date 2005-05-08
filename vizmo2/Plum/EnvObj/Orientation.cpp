/////////////////////////////////////////////////////////////////////
//  Orientation.cpp
//
//  Based on ver. 3/ 1/98 by Aaron Michalk
/////////////////////////////////////////////////////////////////////

#include "Orientation.h"

namespace plum{

#define EPSILON  1.0e-6
  
  
  //===================================================================
  //  Static Data Initialization
  //===================================================================
  //const Orientation Orientation::Identity = Orientation(IdentityMatrix);
  
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
  Orientation::Orientation(OrientationType _type) {
    type = _type;
    
    if (type == Matrix) {
      matrix.identity();
    } 
    else {
      alpha = 0.0;
      beta  = 0.0;
      gamma = 0.0;
    }
  }
  
  Orientation::Orientation(const Matrix3x3 _matrix) {
    type = Matrix;
    matrix = _matrix;

    //ToEulerXYZ(matrix);
    
  }
  
  Orientation::Orientation(OrientationType _type, double _alpha, double _beta, double _gamma){
    type  = _type;
    
    alpha = _alpha;
    beta  = _beta;
    gamma = _gamma;

    ToMatrix(alpha, beta, gamma);
    
  }
  
  //-------------------------------------------------------------------
  // If no argument is assumed, it is considered as having Euler angles
  //-------------------------------------------------------------------
  Orientation::Orientation() {
    type  = EulerXYZ;
    
    alpha = 0;
    beta  = 0;
    gamma = 0;
  }
  
  Orientation::Orientation(const Orientation & _o) {
    *this = _o;
  }
  
  Orientation::~Orientation() {
  }
    
  Orientation & Orientation::operator=(const Orientation & _o) {
    type = _o.type;
    
    for (int i=0; i < 3; i++)
      for (int j=0; j < 3; j++)
	matrix[i][j] = _o.matrix[i][j];
    
    alpha = _o.alpha;
    beta  = _o.beta;
    gamma = _o.gamma;
    
    return *this;
  }
  

  //===================================================================
  //  ConvertType
  //===================================================================
  void Orientation::ConvertType(OrientationType _newType) {
    // Don't do the conversion, if there is no need
  }
  
  void Orientation::ToMatrix(double alpha, double beta, double gamma){

    double sa = sin(gamma);
    double ca = cos(gamma);
    double sb = sin(beta);
    double cb = cos(beta);
    double sg = sin(alpha);
    double cg = cos(alpha);

    //EULER ZYX    Craig (2.64)
    
    matrix.set(ca*cb, ca*sb*sg - sa*cg, ca*sb*cg + sa*sg,
	       sa*cb, sa*sb*sg + ca*cg, sa*sb*cg - ca*sg,
	       -sb, cb*sg, cb*cg);    
    
  }
  
  //===================================================================
  //  Read
  //===================================================================
  void Orientation::Read(istream & _is) {
    type = EulerXYZ;
    _is >> alpha;
    _is >> beta;
    _is >> gamma;
    
    ConvertType(Matrix);
  }

  
  //===================================================================
  //  Write
  //===================================================================
  void Orientation::Write(ostream & _os) {
    ConvertType(EulerZYX);
    
    _os << gamma << " ";
    _os << beta << " ";
    _os << alpha << " ";
    
#if 0
    //ConvertType(EulerXYZ); //comment out by Guang
    _os << alpha << " ";
    _os << beta << " ";
    _os << gamma << " ";
#endif
  }

  ostream & operator<<(ostream & out, const Orientation & o){

    out<<"-\t Alpha Beta Gamma (deg.) = ("<<o.alpha*57.2957795<<", "<<o.beta*57.2957795<<", "<<
      o.gamma*57.2957795<<")"<<endl;

      for (int i=0; i < 3; i++)
	for (int j=0; j < 3; j++){
	  out<<"-\t matrix["<<i<<"]["<<j<<"]= "<<o.matrix[i][j]<<endl;
	}

    return out;

  }
  
}//namespace plum


