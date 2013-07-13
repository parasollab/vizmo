
/**@file Transformation.h
  *@based on code by Aaron Michalk
  *@Aimee Vargas 01/30/04
  */

///////////////////////////////////////////////////////////////////////////////////////////

#ifndef Transformation_h
#define Transformation_h

///////////////////////////////////////////////////////////////////////////////////////////
//Include files
#include "Orientation.h"

namespace plum{

///////////////////////////////////////////////////////////////////////////////////////////
/** This class contains transformational information and operations.
  * Orientation and position of object are stored in the instance of this class.
  * Position of object is stored as instance of Vector3d.
  */

  class Transformation {
    
  public:
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Constructors and Destructor
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    //-----------------------------------------------------------
    /**@name  Constructors and Destructor*/
    //-----------------------------------------------------------
    //@{
    ///Create a Transformation with position (0,0,0) and a matrix represented orientation.
    Transformation();
    
    ///Create a Transformation with given position and orientation.
    Transformation( Orientation  _orientation,  Vector3d  _position);
        
    ///@Creates a transformation with DH parameters:
    Transformation(Vector4d dh);
    
    ///Copy constructor
    Transformation(const Transformation & _t);
    ///Destructor. Do nothing.
    ~Transformation();
    //@}

    //-----------------------------------------------------------
    /**@name  Functions*/
    //-----------------------------------------------------------
    //@{
    ///Reset positiona and orientation
    void ResetTransformation();
    //@}  
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Operator Overloadings
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    //-----------------------------------------------------------
    /**@name Operator Overload with other Transformation or Vector*/
    //-----------------------------------------------------------
    //@{
    
    /*This acts like +=.
     *The orientation of this instance is changed to (orientation+_t.orientation)
     *The position of this instance is changed to (position+_t.position)
     *this instance is returned.
     */
    Transformation & operator+(const Transformation & _transformation);
    /*This acts like -=.
     *The orientation of this instance is changed to (orientation-_t.orientation)
     *The position of this instance is changed to (position-_t.position)
     *this instance is returned.
     */
    Transformation operator-(const Transformation & _transformation);
    /**Return a new Transformation which is 
     *The new orientation will be (orientation * _t.orientation)
     *The new position will be (orientation * _t.position + position)
     */
    Transformation operator*(const Transformation & _transformation);
    ///Copy position and orientation from _t
    Transformation & operator=(const Transformation & _transformation);
    //@}
    
    ///Create a new Transformation which is inverse of this Transformation.
    Transformation Inverse();

    friend ostream & operator<<( ostream & out, const Transformation & t );

   
    //-----------------------------------------------------------
    //  Data Member
    //-----------------------------------------------------------
    Vector3d m_position;          ///<Translation
    Orientation m_orientation;    ///<Rotation
    Vector3d originalPosition;
    
  };
  
}//namespace plum
#endif
