/**@file Orientation.h
  *
  *@based on the file by Aaron Michalk
  *@Aimee Vargas 01/30/04
  */
  
#ifndef Orientation_h
#define Orientation_h

/////////////////////////////////////////////////////////////////////////////////////////
//Include headers

#include<Vector.h>
#include<Matrix.h>

namespace plum{

////////////////////////////////////////////////////////////////////////////////////////
const Matrix3x3 IdentityMatrix;

/////////////////////////////////////////////////////////////////////////////////////////

/**This class stores inforation about object orientation in 3D space.
  *Representation implemented here:
  * - Matrix
  * - EulerXYZ
  */
  
  class Orientation {
  public:
    //-----------------------------------------------------------
    //  Enumerations
    //-----------------------------------------------------------
    
    /**The type of Orientation instance. 
     *Including Matix, Euler, Quaternion, and Rodriques types.
     */
    enum OrientationType {
      Matrix   = 0,
      Quaternion = 1,
      EulerXYZ = 2,
      EulerZYX = 3,
      FixedXYZ = 4
    };
    //-----------------------------------------------------------
    //  Static Data
    //-----------------------------------------------------------
    ///Identity matrix
    
    static const Orientation Identity;
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Constructors and Destructor
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    //-----------------------------------------------------------
    /**@name  Constructors and Destructor.*/
    //-----------------------------------------------------------
    //@{
    /**Default constructor.
     *If no argument is assumed, it is considered as having Euler angles.
     *The type is set to EulerXYZ.
     *alpha ,beta ,gamma is set to 0.
     */
    Orientation();
    
    /**Construct itself according to specified orientation type.
     *@param _type What type this Orientation instance should be.
     *@todo Quaternion and  Rodriques is not implemented.
     *@notice if _type is Matrix, then elements in matrix with coordinate
     *i=j are set to 1 and other elements are set to 0.
     *@see OrientationType for more type information.
     */
    Orientation(OrientationType _type);
    
    /**Construct a matrix type Orientation instance.
     *@param _matrix[3][3] A transformation matrix which will be copied in to this->matrix.
     *@notice this->type is set to Matrix.
     *@see OrientationType for more type information.
     */
    Orientation(Matrix3x3 _matrix );
    
    /**Construct a matrix with given Euler angles.
     *@param _type What type this Orientation instance should be.
     *@param _alphs .
     *@param _beta .
     *@param _gamma .
     *@see OrientationType for more type information.
     */ 
    Orientation(OrientationType _type, double _alpha, double _beta, double _gamma);
    
    
    /**A copy constructor.
     *This construtor calls "operator=" function.
     *@param _o Copy _o to this instance.
     *@see operator=
     */
    Orientation(const Orientation & _o);
    
    ///Destructor
    ~Orientation();
    //@}
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Operator Overloadings
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    
     /**@name Operator overloading.*/
    //@{
    
    /** Copy values of a given Orientation instance to this instance according to OrientaionType.
     * According to type in given Orientation instance, values of data memebers are copied to 
     * data member in this instance.
     * @param _orientation An Orientation with any OrientationType
     * @return This instance is returned.
     * @todo Rodriques and Quaternion are not implemented.
     */
    Orientation & operator=(const Orientation & _o);
    
    /** Access matix element by given coordinate.
     * @param _row Whis row the element is in.
     * @param _col Whis coloum the element is in.
     * @return the value in matrix[_row-1][_col-1]
     */
    
    double & operator()(int _row, int _col);

    friend ostream & operator<<( ostream & out, const Orientation & o );
    //@}
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Help Methods
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    //-----------------------------------------------------------
    /**@name Help Methods.*/
    //-----------------------------------------------------------
    //@{
    
    /** Convert the Orientation instance from current type to specified type.
     * @param _newType The type this instance will be converted to.
     * @see OrientationType for more type information.
     * @todo There are a lot of to do marks in implemetation (.cpp) of this function.
     */
    void ConvertType(OrientationType _newType);
    
    /** Read data from input stream.
     * Read Euler angles and convert to Matrix type of Orientation instance.
     * @param _is The source of input data, alpha, beta,and gamma.
     */
    void Read(istream & _is);
    
    /** Write alpha, beta,and gamma to output stream.
     * @param _os Onput data, alpha, beta,and gamma to _os.
     */
    void Write(ostream & _os);
    
    /** Convert from EulerXYZ to matrix
     * @params alpha, beta, gamma are the Euler angles
     */
    void ToMatrix(double alpha, double beta, double gamma);
        
    //@}
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Data
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
    //-----------------------------------------------------------
    //  Data
    //-----------------------------------------------------------
    
    ///Type of the Orientation instance.
    OrientationType type;
    
    /**@name  Matrix type Data members*/
    //@{
    
    Matrix3x3 matrix;
    //@}
    
    /**@name Euler angle type Data members*/
    //@{
    double alpha;
    double beta;
    double gamma;

    double rx, ry, rz;
    //@}
    
    //@}
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Protected data member and member methods
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
  protected:
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //    Private data member and member methods
    //
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    
  private:
  };
  
  //===============================================================
  // Inline functions
  //===============================================================
  inline double & Orientation::operator()(int _row, int _col) {
    return matrix[_row-1][_col-1];
  }
  
#endif

}//namespace plum
