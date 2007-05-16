/*!
	\file MatrixPart.h
	\author Gabriel Tanase	
	\date Jan. 25, 04
	\ingroup stapl
	\brief Matrix Part class : Wrapper arround sequential matrix container

 	Internal to stapl developer. 
*/
#ifndef MATRIXPART_H
#define MATRIXPART_H
#include "mtl/utils.h"
#include "mtl/matrix.h"
#include "BasePart.h"

/**
 * @ingroup pmatrix
 * @{
 **/

namespace stapl {

/**
 *Matrix Part is a wrapper arround sequential MTL matrix.
*/
  template<class T,class Shape = mtl::rectangle<>, class Storage = mtl::dense<>, class Orientation = mtl::row_major>
    class MatrixPart : public BasePart<T, 
                                       typename mtl::matrix<T,Shape,Storage,Orientation>::type::iterator,
                                       typename mtl::matrix<T,Shape,Storage,Orientation>::type::const_iterator, 
                                       pair<int,GID> > {
    
    public:
    
    //================================================================
    //required typedefs for each part class to instantiate 
    //BasePart and BasePContainer
    //================================================================
    ///Element type
    typedef T value_type;
    typedef T BaseElement_type;

    ///Sequential matrix container
    typedef typename mtl::matrix<T,Shape,Storage,Orientation>::type Sequential_Container_type;
    typedef typename mtl::matrix<T,Shape,Storage,Orientation>::type Container_type;
    
    ///Iterator over the rows/columns of the matrix(depending on the orientation of the matrix)
    typedef typename Container_type::iterator                iterator;
    ///Const terator over the rows of the matrix
    typedef typename Container_type::const_iterator          const_iterator;

    ///Reference to the row/line of the matrix
    typedef typename Container_type::OneD          reference;

    ///Const reference to the row/line of the matrix
    typedef const typename Container_type::OneD    const_reference;

    ///Iterator over the elements of row/column
    typedef typename Container_type::OneD::iterator  element_iterator;

    ///Const terator over the elements of row/column
    typedef typename Container_type::OneD::const_iterator const_element_iterator;

    typedef pair<int,GID>     Element_Set_type;
    typedef Element_Set_type  Array_Range_type;
    typedef BasePart<T,iterator,const_iterator, Array_Range_type> Base_Part_type;

    ///Matrix part type
    typedef MatrixPart<T,Shape,Storage,Orientation>  Part_type;
    //=======================================
    //data
    //=======================================   

    Container_type part_data;

    // for each part we store some extra information 
    // that will help us place this part in the global matrix
    // we store the indices of the upper, left corner together with 
    // the number of rows and columns; these values identifies a block from the 
    // global matrix
    unsigned int I, J, nRows, nColumns;

    //maybe we don't need the global number of columns;
    //only the distribution should be aware of this
    unsigned int GN; 

    public:
    //=======================================
    //constructors 
    //=======================================
    
    /**@brief Default constructor
     */
    MatrixPart() {
    }
    
    /**@brief Constructor with a specific part id.
     *@param PARTID the id of the part to be created.
     */
    MatrixPart(PARTID _id) : 
      Base_Part_type(_id) { 
    }
    
    /**@brief Constructor
     *@param _m rows number.
     *@param _n columns number.
     *@param _i Upper left corner row in the global matrix.
     *@param _j Upper left corner column in the global matrix.
     *@param PARTID the id of the part to be created.
     */
    MatrixPart(unsigned int _m,unsigned int _n,  unsigned int _i, unsigned int _j, PARTID _id) : 
    part_data(_m,_n), I(_i),J(_j),nRows(_m),nColumns(_n)
    {
      this->partid = _id;
    }
    
    /**@brief Copy constructor
     */
    MatrixPart(const Part_type& _other) : part_data(_other.nRows,_other.nColumns) {
      I     = _other.I;
      J     = _other.J;
      nRows = _other.nRows;
      nColumns = _other.nColumns;
      partid = _other.partid;
      mtl::copy(_other.part_data , part_data);
    }
    
    /**@brief Copy constructor; copy everything except the partid which is specified as 
     *one of the arguments of the constructor.
     *@param Part_type part that will be clonned.
     *@param PARTID the partid associated with the part.
     */
    MatrixPart(const Part_type& _other, PARTID _partid) : part_data(_other.nRows,_other.nColumns){
      I     = _other.I;
      J     = _other.J;
      nRows = _other.nRows;
      nColumns = _other.nColumns;
      partid = _partid;
      mtl::copy(_other.part_data , part_data);
    }
    
    /**@brief 
     *Defyne type for matrix part.
     */
    void define_type(typer &t)  {
      cout<<"define_type method is not yet implemented in MatrixPart"<<endl;
    }
    
    
    /**@brief Destructor
     */
    ~MatrixPart() {
    }
    
    /**@brief Set part information
     *@param _m rows number.
     *@param _n columns number.
     *@param _i Upper left corner row in the global matrix.
     *@param _j Upper left corner column in the global matrix.
     */
    void SetPart(unsigned int _m,unsigned int _n,  unsigned int _i, unsigned int _j){
      // cout<<"Set part size start"<<stapl::get_thread_id()<<" "<<_part_size<<" "<<_start<<endl;
      I=_i; 
      J=_j;
      nRows    = _m;
      nColumns = _n;
    }

    /**@brief Set part information and data.
     *@param _m rows number.
     *@param _n columns number.
     *@param _i Upper left corner row in the global matrix.
     *@param _j Upper left corner column in the global matrix.
     *@param vector<T> data that will overwrite the current storage of the part.
     */
    void SetPart(unsigned int _m,unsigned int _n,  unsigned int _i, unsigned int _j,vector<T>& _data){
      // cout<<"Set part size start"<<stapl::get_thread_id()<<" "<<_part_size<<" "<<_start<<endl;
      I=_i; 
      J=_j;
      nRows    = _m;
      nColumns = _n;
      part_data.resize(_m,_n);
      *(this->GetVectorData()) = _data;
    }

    void SetPart(vector<T>& _data){
      *(this->GetVectorData()) = _data;
    }
    
    //=======================================
    //operator
    //=======================================
    
    /**@brief Assignment operator
     */
    MatrixPart<T>& operator=(const MatrixPart<T>& _other) {
      //Base_Part_type::operator=(_other);
      mtl::copy(_other.part_data , part_data);
      I            = _other.I;
      J            = _other.J;
      nRows        = _other.nRows;
      nColumns     = _other.nColumns;
      this->partid = _other.partid;
      part_data    = _other.part_data;
      return *this;
    }
    /**@brief 
     *Initialize all data with the specified value_type.
     *@param value_type value that will be assigned to all elements in the part.
     */
    void fill(value_type _t){
      mtl::set(part_data, _t);
    }
    
    //=======================================
    //methods required by base part
    //=======================================
    
    /*used in BasePContainer as the default add element action */
    void AddElement(const T& _t, GID _gid) {                 
      cout << "AddElement method is not implemented in MatrixPart\n";
      cout << "Matrix is of a fixed size; you can only use set/get methods\n";
    }
  
    /**@brief Access operator.
     *@param unsigned int row index for the element to be accessed.
     *@param unsigned int column index.
     */
    T& operator()(unsigned int _i,unsigned int _j){
      return part_data(_i,_j);
    }
    
    /**@brief 
     * GetElement corresponding to the specified GID.
     * @param GID associated with the element for which we want to read the value.
     * @return T the value of the element.
     */
    T& GetElement(GID _localgid) {   
      int i = _localgid / nColumns;
      int j = _localgid % nColumns;
#ifdef STAPL_DEBUG  
      //cout<<"size:"<<nRows<<"x"<<nColumns<<":_localgid: "<<_localgid<<":"<<i<<" "<<j<<endl;
#endif
      if (i < nRows && j < nColumns)
	return part_data(i,j);
      else{
	throw ElementNotFound(_localgid);
	return part_data(i,j);
	}
    }
    
    /**@brief Const GetElement(). See above.
     */
    const T& GetElement(GID _localgid) const {
      //there is a compile error because operator(i,j)const is not defined inside
      //matrix class; there is a definition but returns const reference
      //which is not quite correct; It requires to search more trough MTL
      //to add the const operator that returns non const reference
      
      int i = _localgid / nColumns;
      int j = _localgid % nColumns;
      if (i < nRows && j < nColumns)
	return part_data(i,j);
      else{
	throw ElementNotFound(_localgid);
	return part_data(i,j);
	}          
    }

    /**@brief SetElement
     */
    void SetElement(GID _localgid, const T& _t) {                  
#ifdef STAPL_DEBUG  
      cout<<"SET"<<_localgid<<" "<<endl;
#endif
      int i = _localgid / nRows;
      int j = _localgid % nColumns;

      if (_localgid < nRows && _localgid < nColumns)
	part_data(i,j) = _t;
      else
	throw ElementNotFound(_localgid);
    }
    
    /**@brief  DeleteElement is not supported by MatrixPart.
     */
    void DeleteElement(GID _gid) {                             
      cout << "DeleteElement method is not implemented in MatrixPart";
      cout << "Matrix is of a fixed size; you can only use set/get methods\n";
    }
    
    /**@brief Returns true if the specified GID is in the part.
     */
    bool ContainElement(GID _localgid) {
      //int i = _localgid / nRows;
      //int j = _localgid % nColumns;
      if (_localgid < nRows && _localgid < nColumns)
	return true;
      else
	return false;
    }
   
    /**@brief Check if the given gid is local to the part.
     *@param iterator will be initialized with the row containing the GID.
     *@param GID identifier of the element to be searched.
     *@Note. this method should be called after we check if the element is local or not.
     */
    bool ContainElement(const GID _localgid, iterator* it) {
      int i = _localgid / nColumns;
      int j = _localgid % nColumns;
      if(i < nRows && j < nColumns){
	*it = part_data.begin() + i;
	return true;
      }
      return false;
    }

    /**@brief Check if the given gid is local to the part.
     *@param element_iterator will be initialized with the location of the element.
     *@param GID identifier of the element to be searched.
     *@Note. this method should be called after we check if the element is local or not.
     */
    bool ContainElement(const GID _localgid, element_iterator* it) {
      int i = _localgid / nColumns;
      int j = _localgid % nColumns;
      if(i < nRows && j < nColumns){
	*it = (*(part_data.begin() + i)).begin() + j;
	return true;
      }
      return false;
    }
    
    /*required by the base, not implemented here
     */
    virtual Array_Range_type GetElementSet() const {          
      cout << "GetElementSet method is not implemented in ArrayPart";
      return Array_Range_type();
    }
    
    /**@brief Part begin. The first row/column of the part depending on the orientation.
     *@return a part iterator pointing to the begining of the part.
     */
    iterator begin() {   
      return part_data.begin();
    }
    
    /**@brief Part end. Last row/column of the part depending on the orientation.
     *@return a part iterator pointing after the end of the part.
     */
    iterator end() {
      return part_data.end();                                      
    }

    /**@brief Part begin. The first row/column of the part depending on the orientation.
     *@return a part const_iterator pointing to the begining of the part.
     */
    const_iterator begin() const {                            
      return part_data.begin();
    }
    
    /**@brief Part end. Last row/column of the part depending on the orientation.
     *@return a part const_iterator pointing after the end of the part.
     */
    const_iterator end() const {                              
      return part_data.end();                                      
    }

    /**@brief
     *Return the size of the part.
     */
    size_t size() const { return nRows; }          
   
    /**@brief
     *@return true if the part is not empty and false otherwise.
     */
    bool empty() const { return (nRows == 0); }      
    
    /**@brief Erase all  elements.
     */
    void clear() {                                            
      cout << "Clear method is not implemented in MatrixPart";
      cout << "Matrix is of a fixed size; you can only use set/get methods\n";
    }                       
    
    
    //=======================================
    //facility functions
    //=======================================
    
    /**@brief 
     *@return Container_type the data of the part.
     */
    const Container_type& GetArrayPart() const { 
      return part_data;
    }
    
    /**@brief Set data for the part.
     *@param Container_type& reference to a sequential matrix.
     *@param PARTID for the part.
     *@param unsigned int 
     */
    void SetArrayPart(const Container_type& _matrix, 
		      PARTID _id, 
		      unsigned int _m,
		      unsigned int _n) {
      
      this->SetPartId(_id);
      M = _m;
      N = _n;
      mtl::copy(_matrix,_part_data);
    }
    
    /**@brief Return all GIDs.
     *@param vector<GID>& output argument that will be initialized with the values of all GIDs
     *stored in the current part.
     */
    virtual void GetPartGids(vector<GID>& _gidvec) const{
      for(int i=I; i < I + nRows; ++i){
	for(int j=J; j < J+nColumns; ++j){	
	  _gidvec.push_back(i*GN+j);
	}  
      }
    }

    /**@brief 
     *Get the row corresponding to the upper left corner of the corresponding part.
     *@return unisgned int row identifier.
     */
    unsigned int GetI() const {
      return I;
    }

    /**@brief 
     *Get the column corresponding to the upper left corner of the corresponding part.
     *@return unsigned int column identifier.
     */
    unsigned int GetJ() const {
      return J;
    }

    /**@brief 
     *Get the number of rows.
     *@return unsigned int the number of rows.
     */
    unsigned int GetnRows() const {
      return nRows;
    }

    /**@brief 
     *Get the number of columns.
     *@return unsigned int the number of columns.
     */
    unsigned int GetnColumns() const {
      return nColumns;
    }

    //at the bottom the sequentaill matrix is stored in a vector
    //this method returns a pointer to that vector;
    //it is used to ship the data of the part;
    /**@brief Get a vector pointer pointing to the data of the part.
     *@return vector<T>* pointer to the data of the part.
     */
    vector<T>* GetVectorData(){
      return part_data.twod.data_.object;
    }

    /**@brief Method to display information about the part.
     */
    virtual void DisplayPart() const {                               
      cout<<"PART ID:"<<partid<<" I="<<I<<" :J="<<J<<" :nRows="<<nRows<<" :nColumns="<<nColumns<<endl;
      mtl::print_all_matrix(part_data);
    }

    void DisplayGID() const {                               
      cout<<"PART ID:"<<partid<<endl;
      for(int i=I; i < I + nRows; ++i){
	for(int j=J; j < J+nColumns; ++j){	
	  cout<<i*nColumns+j<<" ";
	}  
	cout<<endl;
      }
    }
    
  }; //end ArrayPart
  
} //end namespace stapl
//@}
#endif
