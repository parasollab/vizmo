
/////////////////////////////////////////////////////////////////////
//  Transformation.cpp
//
//  Based on code by Aaron Michalk
//
//  Aimee Vargas
//  01/30/04
/////////////////////////////////////////////////////////////////////

#include "Transformation.h"
#include <stdlib.h>
namespace plum{

#define DEGTORAD 3.1415926535/180.0
  
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
  Transformation::Transformation() 
  {
    
    m_position.set(0.0, 0.0, 0.0);
    m_orientation.matrix.identity();
    
  }
  
  Transformation::Transformation(Orientation _orientation, Vector3d _position) 
  {
    m_position.set(_position);
    m_orientation.matrix = _orientation.matrix;
  }
  
  //==============================================================================
  // Function: Create a transformation corresponding to the given DH parameters
  //           receives CMultiBody to access them
  // Refer Craig Eq 3.6
  //==============================================================================
  Transformation::Transformation(Vector4d dh){
    //The vector comes as: alpha, a, d, theta

    double a, d, alpha, theta;
    alpha = dh[0]*DEGTORAD;
    a = dh[1];
    d = dh[2];
    theta = dh[3];
    
    m_position.set(a, -sin(alpha)* d, cos(alpha)* d);
 
    m_orientation.matrix.set(cos(theta),
			     -sin(theta), 
			     0.0,
			     sin(theta)*cos(alpha), 
			     cos(theta)*cos(alpha),
			     -sin(alpha),
			     sin(theta)*sin(alpha),
			     cos(theta)*sin(alpha),
			     cos(alpha) );
  }
  
  void Transformation::ResetTransformation(){

    m_position.set(0.0, 0.0, 0.0);
    m_orientation.matrix.identity();
    m_orientation.alpha = m_orientation.beta = m_orientation.gamma = 0;

  }
  
  Transformation::Transformation(const Transformation & _t) :
    
    m_orientation(_t.m_orientation)
  {
    m_position.set(_t.m_position);
  }
  
  Transformation::~Transformation() {
    
  }
  
  //===================================================================
  //  Operators
  //===================================================================

  ostream & operator<<(ostream & out, const Transformation & t){

    out<<"-\t Position = ("<<t.m_position[0]<<", "<<t.m_position[1]<<", "<<
      t.m_position[2]<<")"<<endl;

      for (int i=0; i < 3; i++)
	for (int j=0; j < 3; j++){
	  out<<"-\t Orientation["<<i<<"]["<<j<<"]= "<<t.m_orientation.matrix[i][j]<<endl;
	}

    return out;

  }

  Transformation & Transformation::operator+(const Transformation & _transformation) {
    
    m_orientation.matrix = m_orientation.matrix + _transformation.m_orientation.matrix;
    m_position = m_position + _transformation.m_position;
    
    return *this;
  }
  
  Transformation Transformation::operator-(const Transformation & _transformation) {
    //orientation = orientation - _transformation.orientation;
    //position = position - _transformation.position;
    
    return *this;
  }
  
  
  Transformation & Transformation::operator=(const Transformation & _t) {
    m_position.set(_t.m_position);
    m_orientation = _t.m_orientation;
    
    return *this;
  }
  
  //===================================================================
  //  Refer to Craig Eq 2.45 
  //===================================================================
  Transformation Transformation::operator*(const Transformation & _t) {

    return Transformation(m_orientation.matrix * _t.m_orientation.matrix, 
			  m_orientation.matrix * _t.m_position + m_position);
  }
  
  //===================================================================
  //  Inverse
  //
  //  Function: Creates the reverse transformation of "this"
  //
  //  Output: The reverse transformation
  //
  //  Leaves "this" transformation unchanged.
  //  Refer to Craig Eq 2.45 
  //===================================================================
  Transformation Transformation::Inverse() {
    
    return Transformation(m_orientation.matrix.transpose(), 
			  -(m_orientation.matrix.transpose() * m_position));
    
  }

}//end namespace plum


