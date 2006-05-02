#ifndef ARRAYPART_H
#define ARRAYPART_H

#include "valarray"
#include "BasePart.h"
/**
 * @ingroup parray
 * @{
 */
namespace stapl {
  /**
   *ArrayPart class is a wrapper arround valarray class. It implements
   *the interface specified by the BasePart class.
   */
  template<class T>
    class ArrayPart : public BasePart<T, T*,const T*, pair<int,GID> > {
    
    public:
    
    //================================================================
    //required typedefs for each part class to instantiate 
    //BasePart and BasePContainer
    //================================================================

    ///Element type
    typedef T value_type;
    typedef T BaseElement_type;
    ///Sequential vallaray container
    typedef valarray<T> Sequential_Container_type;
    typedef valarray<T> Container_type;

    ///Part iterator
    typedef value_type* iterator;
    ///Part const iterator
    typedef const value_type* const_iterator;

    ///Reference to the type T
    typedef value_type& reference;
    ///Const reference to the type T
    typedef const value_type& const_reference;
    
    typedef pair<int,GID> Element_Set_type;
    typedef Element_Set_type Array_Range_type;

    ///Base class for ArrayPart
    typedef BasePart<T,iterator,const_iterator, Array_Range_type> Base_Part_type;
    
    //=======================================
    //data
    //=======================================
    
    ///Sequential valarray
    Container_type part_data;

    ///Start index for current part.
    GID start_index;
    
    public:
    //=======================================
    //constructors 
    //=======================================
    
    /**@brief Default constructor
     */
    ArrayPart() {
    }
    
    /**@brief Constructor with part identifier specified.
     *@param PARTID the identifier for the newly created part.
     */
    ArrayPart(PARTID _id) : 
      Base_Part_type(_id) { 
    }
    
    /**@brief Constructor.
     *@param int part size
     *@param GID the first GID in this part.
     *@param PARTID part identifier.
     */
    ArrayPart(int _part_size, GID _start, PARTID _id){
      part_data.resize(_part_size);
      start_index = _start;
      this->partid = _id;
    }
    
    /**@brief Set part information
     *@param int part size.
     *@param GID the first GID stored in this part.
     */
    void SetPart(int _part_size, GID _start){
      // cout<<"Set part size start"<<stapl::get_thread_id()<<" "<<_part_size<<" "<<_start<<endl;
      part_data.resize(_part_size);
      start_index = _start;
    }
    
    /**@brief Copy constructor
     */
    ArrayPart(const ArrayPart<T>& _valarray) {
      this->partid = _valarray.partid;
      part_data.resize(_valarray.size());  
      part_data = _valarray.part_data;
      start_index = _valarray.start_index;
    }
    
    /**@brief Copy constructor. The part id for current part will be set to _id
     *@param const ArrayPart<T>& reference to the source part.
     *@param PARTID identifier for the part.
     */
    ArrayPart(const ArrayPart<T>& _valarray, PARTID _id) {
      this->partid = _id;
      part_data.resize(_valarray.size());  
      part_data = _valarray.part_data;
      start_index = _valarray.start_index;
    }
    
    /**@brief 
     *Define type for ArrayPart
     */
    void define_type(typer &t)  {
      t.local(partid);
      t.local(part_data);
      t.local(start_index);
    }
    
    
    /**@brief Destructor
     */
    ~ArrayPart() {
    }
    
    
    //=======================================
    //operator
    //=======================================
    
    /**@brief Assignement operator
     */
    ArrayPart<T>& operator=(const ArrayPart<T>& _valarray) {
      //if(empty()) 
      if(this->size() != _valarray.size()) part_data.resize(_valarray.size()); 
      part_data = _valarray.part_data;
      this->partid = _valarray.partid;
      start_index = _valarray.start_index;
      return *this;
    }
    
    //=======================================
    //methods required by base part
    //=======================================
    
    /**@brief ArrayPArt is of a fixed size. This method is not implemented.
     */
    void AddElement(const T& _t, GID _gid) {                 
      cout << "AddElement method is not implemented in ArrayPart";
    }
    
    /**@brief Operator []; Wrapper arround sequentiall valarray.
     */
    T& operator[](int _local_index) {
      return part_data[_local_index];
    }
    
    /*@brief Operator []; Wrapper arround sequentiall valarray.
     */
    //const T& operator[](int _local_index) const {
    //return part_data[_local_index];
    //}

    virtual T& GetElement(GID _gid) {   
#ifdef STAPL_DEBUG  
      cout<<"_gid: "<<_gid<<"  start_index: "<<start_index<<"  part size: "<<size()<<endl;
#endif
      if (_gid >= start_index && _gid < (start_index + size()))
	return part_data[_gid - start_index];
      else
	throw ElementNotFound(_gid);
    }
   

    /**@brief  Return a reference to the data corresponding to GID.
     *@param GID identifier of the element to be returned.
     */
    virtual T GetElement(GID _gid) const {             
      if (_gid >= start_index && _gid < (start_index + size()))
	return part_data[_gid - start_index];
      else
	throw ElementNotFound(_gid);
    }
    
    /**@brief  Set the value of an existent element.
     *@param T the element to be added;
     *@param GID associated with the element. The GID is generated by the distribution. 
     */
    void SetElement(GID _gid, const T& _t) {                  
#ifdef STAPL_DEBUG  
      cout<<"SET"<<_gid<<" "<<start_index<<" "<<size()<<endl;
#endif
      if (_gid >= start_index && _gid < (start_index + size()))
	part_data[_gid - start_index] = _t;
      else
	throw ElementNotFound(_gid);
    }
    
    /**@brief  DeleteElement is not implemented. pArray is of a fixed size.
     */
    void DeleteElement(GID _gid) {                             
      cout << "DeleteElement method is not implemented in ArrayPart";
    }
    
    /**@brief Check if the given gid is local to the part.
     *@param GID identifier of the element to be searched.
     */
    bool ContainElement(GID _gid) {                           
      if (_gid >= start_index && _gid < (start_index + size()))
	return true;
      else
	return false;
    }
   
    /**@brief Check if the given gid is local to the part.
     *@param iterator will be initialized with the location of the element.
     *@param GID identifier of the element to be searched.
     *@Note. this method should be called after we check if the element is local or not. 
     */
    bool ContainElement(const GID _gid, iterator* it) {
      if(ContainElement(_gid)){
	*it = &part_data[0] + _gid - start_index ;
	return true;
      }
      return false;
    }
    
    virtual Array_Range_type GetElementSet() const {          
      cout << "GetElementSet method is not implemented in ArrayPart";
      return Array_Range_type();
    }
    
    /**@brief 
     *Return a part iterator pointing to the begining of the part.
     */
    iterator begin() {   
      return &part_data[0];
    }
    
    /**@brief 
     *Return a part iterator pointing to the end of the part.
     */
    iterator end() {
      return &part_data[part_data.size()];                                      
    }
    
    /**@brief
     *Return a part const iterator pointing to the begining of the part.
     */
    const_iterator begin() const {                            
    //cout << "const_iterator begin method is not implemented in ArrayPart";
      const T& t = part_data[0];
      return &t;
    }
    
    /**@brief
     *Return a part const iterator pointing to the end of the part.
     */
    const_iterator end() const {                              
    //cout << "const_iterator end  method is not implemented in ArrayPart";
      const T& t = part_data[part_data.size()];
      return &t;
    }
    
    /**@brief 
     *Return the size of the part.
     */
    size_t size() const { return part_data.size(); }          

    /**@brief
     *Return if the part is empty or not.
     */
    bool empty() const { return (part_data.size()==0); }      
    
    /**@brief Clear the data stored in this part.
     */
    void clear() {                                            
      part_data.resize(0);
    }                       
    
    /**@brief  Method to display the content of the part
     */
    void DisplayPart() const {                                
      cout << "Valarray Part with ID: "<<this->partid<<" Size: "<<size()
	   <<" Data:"<<endl;
      //for(GID it = 0; it < part_data.size(); ++it) {  
      //cout<<"("<<(it + start_index) <<", "<<part_data[it]<<"),  ";
      //}
      //cout<<endl;
    }
    
    //=======================================
    //facility functions
    //=======================================
    
    /**@brief  Method to return the data of the part.
     *@return const valarray<T> the data of the current part.
     */
    const valarray<T>& GetArrayPart() const { 
      return part_data; 
    }
    
    /**@brief Set the content of the current part.
     *@param const valarray<T>& data to be stored in the part.
     *@param PARTID identifier for the part.
     *@param GID start index for current part.
     */
    void SetArrayPart(const valarray<T>& _valarray, PARTID _id, GID start ) {
      if(empty()) part_data.resize(_valarray.size());
      part_data = _valarray;
      this->SetPartId(_id);
      start_index = start;
    }
    

    /**@brief Get the GIDs stored in the current part.
     *@param vector<GID>& reference to the vector that will be filled with the GIDs
     *stored inside this part.
     */
    virtual void GetPartGids(vector<GID>& _gidvec) const{
      for(int i=0; i<part_data.size(); ++i){
	_gidvec.push_back(start_index + i );
      }  
    }
    
  }; //end ArrayPart
  
} //end namespace stapl
//*}
#endif
