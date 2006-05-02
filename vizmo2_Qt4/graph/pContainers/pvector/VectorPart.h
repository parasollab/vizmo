
//*************************************************************************
/*!
	\file VectorPart.h
	\author  Alin Jula	
	\date  01/13/03
	\ingroup stapl
	\brief - A Wrapper around the STL container. This is the basic cell the pvector container operates on.

	STAPL - project
	Texas A&M University,College Station, TX, USA
*/
//*************************************************************************

#ifndef VECTORPART_H
#define VECTORPART_H

#ifdef _TIMER_EXPERIMENT
extern stapl::timer timer_part_get_element;
extern double time_part_get_element;
#endif

#include <BasePart.h>
#include <BaseDistribution.h>
namespace stapl {

/**
 * @addtogroup pvector
 * @{
 **/
/**
 * pointer_to_integral uses templates to find an integral type that can safely
 * store a pointer type.  The C++ standard states that
 *
 * sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
 *
 * and that reinterpret_cast can safely cast pointers to integrals and back
 * again.  However, it does not specify which integrals, and hence the
 * appropriate integral type varies across platforms and makes using the wrong
 * integral is dangerous (e.g., casting a pointer that only fits in a long to
 * an int will truncate some of the pointer information).  This template is
 * not valid with function pointers and member function pointers.
 *
 * Example:
 * \code
 * typedef pointer_to_integral<int*>::integral integral;
 * int* a;
 * integral b = reinterpret_cast<integral>( a );
 * int* c = reinterpret_cast<int*>( b );
 * \endcode
 **/


  /**
     This is the part class that stores elements in vector.  It is a
     wrapper class for the vector data structure, and it has 2
     additional data members: 
     
     - offset - which represents the index in the pvector from where
     this part begins
     
     - map_index_gid - which stores the corresponding gids for the
     elements in the VectorPart class in a 1-to-1 correspondance
     (note: if the pvector uses vector distribution, this structure is
     empty at all times - it is not used. It is used only when the
     pvector instantiates with base_distribution

     The template parameters represent the element type, the range
     type and the distribution that is used for this class (they are
     all set by the pvector class upon instantiation).

  */
   


template<class T, class Element_Set_Type, class distribution_tag>
  class VectorPart : public BasePart< 
                                     T , 
                                     typename vector<T>::iterator , 
                                     typename vector<T>::const_iterator , 
                                     Element_Set_Type >
  {
  public:
    typedef VectorPart<T,Element_Set_Type,distribution_tag> this_type;

    typedef BasePart< T ,typename vector<T>::iterator , typename vector<T>::const_iterator, Element_Set_Type > Base_Part_type;

    typedef vector<T>   Container_type;

    typedef T element_type;

    typedef T BaseElement_type;// This definition is provided for the BasePContainer class which assumes this type is defined. 
   //Maybe it could be replaced with element_type : Alin

    typedef T value_type;

    typedef typename vector<T>::iterator iterator;

    typedef typename vector<T>::const_iterator const_iterator;

    typedef vector<T>   Sequential_Container_type;

    typedef hash_map<GID,size_t,hash<GID> > map_gid_index_type;
    
    typedef hash_map<size_t,GID,hash<size_t> > map_index_gid_type;

  private:    
    // Data
    /**
       The elements stored in a vector
    */
    Container_type  __data;
    /**
       The index in the pvector from which this part starts
    */
    long int __offset;
     vector<GID> vector_index_gid;

    /**
       This stores the gids for each element for the case when pvector
       is instatiated with base distribution.  It is not used at all
       if the pvector instantiates with vector distribution.
    */

     map_index_gid_type map_index_gid;

     map_gid_index_type map_gid_index;

    /**
////////////////// NOTE ///////////////////////////////////////////////////////////////////
      We have practically 3 functions for each method:
      PartMethod(arguments)
      _PartMethod(arguments,vector_distribution & tag)
      _PartMethod(arguments,base_distribution & tag)
      The vector and base distribution tags allow for different implementations for the same method. The real method calls
      the _PartMethod(args, tag) with the appropiate tag for the specific distribution the part has been instatiated with.


      Methods that do not make the distinction between the two types of distributions have only one implementation.
     */



 public:
  void define_type(typer &t)  {
    t.local(partid);
    t.local(__data);
    t.local(__offset);
    t.local(vector_index_gid);
  }

    // Constructors

  /**
     Default constructor
  */
   VectorPart() {__offset=-1;}

    ////////////////////////////////////////Constructor///////////////////////////////////////////////////////    
  protected:
    /**
       Base distribution : initializes the n gids to -1. 

       Called by VectorPart(n) and VectorPart(n,val)
    */
    void   _VectorPart(int __n, base_distribution tag)
      {
	//	new (&map_index_gid) vector<GID>(__n,-1);
	//	new(&map_index_gid) map_index_gid_type(__n,-1);
	//	new(&map_gid_index) map_gid_index_type(__n,-1);

	printf("\n This part has been instatiated under the base distribution, but the elements do not have corresponding GIDs necessary to work with base distribution");
	
      }
    /**
       Vector distribution : nothing
       Called by VectorPart(n) and VectorPart(n,val)

     */
   void  _VectorPart(int __n, vector_distribution tag)
      {}

  public:
   /**
      Constructor : instatiates the part with n elements with default value and the offset with -1.
   */

   VectorPart(int __n):__data(__n),__offset(-1)
      {
	_VectorPart(__n, distribution_tag());

      }
    ////////////////////////////////////////Constructor///////////////////////////////////////////////////////    
    /**
      Constructor : instatiates the part with n elements with value of passed as parameter and the offset with -1.
     */
    VectorPart(int __n, value_type __val):__data(__n,val),__offset(-1)
      {
	_VectorPart(__n, distribution_tag());
      }

    ////////////////////////////////////////Copy Constructor///////////////////////////////////////////////////////    
  protected:
    /**
       Base distribution :Copies the index to gid map.

       Called by VectorPart( VectorPart&) and VectorPart(VectorPart&, partid)
     */

    void _VectorPart_copy(const this_type& __vpart, base_distribution  tag)
      {
	//	map_index_gid=__vpart.map_index_gid;
	//	map_gid_index=__vpart.map_gid_index;
	vector_index_gid=__vpart.vector_index_gid;
      }
    /**
       Vector distribution : nothing
       Called by VectorPart( VectorPart&) and VectorPart(VectorPart&, partid)
     */
    void _VectorPart_copy(const this_type & __vpart, vector_distribution  tag)
      { }

  public:
    /**
       Copy constructor : copies the data and the offset
     */
    VectorPart(const this_type & __vpart) : __offset(__vpart.__offset),__data(__vpart.__data)      {
	_VectorPart_copy(__vpart, distribution_tag());
      }
    ////////////////////////////////////////Constructor///////////////////////////////////////////////////////
    VectorPart(const this_type & __vpart, PARTID part_id): Base_Part_type(part_id) ,__data(__vpart.__data),__offset(__vpart.__offset)
	{
	  _VectorPart_copy(__vpart, distribution_tag());
	}

    ////////////////////////////////////////Constructor///////////////////////////////////////////////////////
  protected:
      /**
	 Vector distribution : nothing

	 Called by VectorPart(n,gid_vector,val,partid)
       */
      void _VectorPart_gids( const vector<GID>& _gidvec, vector_distribution  tag)
	{  }
      /**
	 Copies the vector of gids passed as parameter to the index gid map.

      	 Called by VectorPart(n,gid_vector,val,partid)
       */
      void _VectorPart_gids( const vector<GID>& _gidvec, base_distribution  tag)
	{
	  for(int i=0;i<_gidvec.size();++i)
	    {
	      map_index_gid.insert(pair<size_t,GID>(i,_gidvec[i]));
	      map_gid_index.insert(pair<GID,size_t>(_gidvec[i],i));
	    }
	  //	  map_index_gid=_gidvec;
	}
  public:
      /**
	 Constructor : it creates n elements with the value passed as
	 parameters and assigns the GIDs passed as parameters as the
	 corresponding GIDs for the elements, and it assigns the
	 partid to the value passed as parameter.
       */
      VectorPart(int __n, const vector<GID>& _gidvec, const T& __value, PARTID __id) : Base_Part_type(__id),__data(__n,element_type(__value)),__offset(-1)
	{
	  _VectorPart_gids(_gidvec, distribution_tag());
	}
    
      ////////////////////////////////////////Constructor///////////////////////////////////////////////////////

      /**
	 Constructor : it creates n elements with the default value
	 and assigns the GIDs passed as parameters as the
	 corresponding GIDs for the elements, and it assigns the
	 partid to the value passed as parameter.
       */
      VectorPart(int __n, const vector<GID>& _gidvec, PARTID __id):Base_Part_type(__id),__data(__n), __offset(-1)
      {
	_VectorPart_gids(_gidvec, distribution_tag());

      }
      ////////////////////////////////////////Constructor///////////////////////////////////////////////////////
      /**
	 Creates an empty part with partid set to the value passed as parameter. It also initializes the offset to -1.
       */
      //VectorPart(PARTID _id) : Base_Part_type(_id),__offset(-1) {}
    
    
      ////////////////////////////////////////Constructor///////////////////////////////////////////////////////

      /**
	 It copies the contents of X into the data and assigns the offset to -1 and the partid to _id
       */
      VectorPart(const Container_type& __x, PARTID __id) : Base_Part_type(__id) ,__data(__x),__offset(-1)
      {
	_VectorPart(__x.size(),distribution_tag());
      }
 

      /**
	 Returns the part offset
       */
    long int GetOffset()
      {return __offset;}

    /**
       Sets the part offset
     */
    void SetOffset(long int _x)
      {
	__offset=_x;
      }

    /**
       Returns the range of indices this part takes in a pvector object. Range = (__offset, __offset + size())
     */
    pair<int,int> GetRange() const 
      {return pair<int,int>(__offset,__offset + __data.size());}

    /**
       Returns a vector of the gids corresponding to the elements.
     */
    void GetPartGids(vector<GID>& _gidvec)const 
      { 
	new (&_gidvec) vector<GID>(map_index_gid.size());
	
	for(map_index_gid_type::const_iterator it=map_index_gid.begin();it!=map_index_gid.end();++it)
	  _gidvec[(*it).first]=(*it).second;
	//	_gidvec= map_index_gid;
      }
   
    ///////////////////////////////////////////////////////////////////////////////////////////    
    // Modifing methods
    ///////////////////////////////////////////////////////////////////////////////////////////    

      protected:
    /**
       vector distribution : nothing
     */

      void _AddElement(const T& _t, GID _gid, vector_distribution tag )
	{ }
      /**
	 base distribution: add the gid to the index gid map
       */
      void _AddElement(const T& _t, GID _gid, base_distribution tag)
	{
	  map_index_gid.insert(pair<size_t,GID> (__data.size(),_gid));
	  map_gid_index.insert(pair<GID,size_t> (_gid,__data.size()));

	}

      /**
	 Adds an element to the part ( as the last element in the
	 part). It also adds a corresponding gid to the index gid map
	 if the part has been instantiated with base distribution .
      */
  public:
      void AddElement(const T& _t,  GID _gid )
	{
	  _AddElement(_t,_gid,distribution_tag());
	  __data.push_back( _t);

	}
      
      ///////////////////////////////////////////////////////////////////////////////////////////    
  protected:
      /**
	 Inserts and value into the part at position passed as an iterator
       */
      iterator _insert(iterator __position, const value_type& _t, const GID& _gid, vector_distribution tag)
	{
	  return __data.insert(__position,_t);
	}
      /**
	 Inserts and value into the part at position passed as an
	 iterator and it also inserts a gid passed as parameter to
	 exactly the same position ( indexwise) in the index gid map
 
       */
      iterator _insert(iterator __position, const value_type& _t, const GID& _gid, base_distribution tag)
	{
	  //	  map_index_gid.insert( map_index_gid.begin()+ ( __position-__data.begin()), _gid);
	  map_index_gid.insert(pair<size_t,GID> (( __position-__data.begin()),_gid));
	  map_gid_index.insert(pair<GID,size_t> (_gid, ( __position-__data.begin())));

	  return __data.insert(__position,_t);
	}
  public:
      /**
	 Inserts a value into the part at the position specified by
	 the iterator __position. It also inserts a corresponding gid
	 into the index gid map if the part has been instantiated with
	 base distribution .  
	 
	 It returns and iterator that points to
	 the newly inserted element.
       */
    iterator insert(iterator __position, const value_type& _t, const GID& _gid ) 
      {
	
	return _insert(__position,_t,_gid,distribution_tag());
      }

    ///////////////////////////////////////////////////////////////////////////////////////////    
      protected:
    /**
       Vector distribution : It inserts a range of elements (b,e) into the part starting
	 at the position indicated by the __position.
     */
      void _insert(iterator __position, iterator _b,iterator _e, vector<GID>::iterator gid_iterator, vector_distribution tag)
	{	  
	  __data.insert(__position   , _b, _e);
	}
      /**
	 Base distribution : It inserts a range of elements (b,e) into the part starting
	 at the position indicated by the __position. It also inserts a range of gids into the index gid
	 map. It assumes that the range of gids starts at the position
	 gid_iterator and it has al leat (_b-_e) elements
       */
      void _insert(iterator __position, iterator _b,iterator _e, vector<GID>::iterator gid_iterator, base_distribution tag)
	{
	  size_t i=0;
	  size_t sup=(_e-_b);
	  size_t __pos=(__position - __data.begin());
	  for(vector<GID>::iterator it=gid_iterator;i<sup;++i,++it)
	    {
	      map_index_gid.insert(pair<size_t,GID> (__pos+i,*it));
	      map_gid_index.insert(pair<GID,size_t> (*it,__pos+i));
	    }
	  //	  map_index_gid.insert(map_index_gid.begin()+(__position - __data.begin()), gid_iterator, gid_iterator + (_e-_b));
	  __data.insert(__position   , _b, _e);
	  
	}
  public:
      /**
	 It inserts a range of elements (b,e) into the part starting
	 at the position indicated by the __position. It also inserts
	 a range of gids (gid_iterator, gid_iterator + (e-b)) into the
	 index gid map if the part has been instantiated with base
	 distribution .
       */
      void  insert(iterator __position, iterator _b,iterator _e, vector<GID>::iterator gid_iterator)
	{

	  _insert(__position,_b,_e,gid_iterator, distribution_tag());
	}
    ///////////////////////////////////////////////////////////////////////////////////////////    
	protected:
      /**
	 It erases an element from the part, element that is pointed to by the
	 iterator __position. It returns an iterator to the newly inserted element.
       */
	iterator _erase(iterator __position,vector_distribution tag)
	  {
	    return __data.erase(__position);
	  }
	/**
	   It erases an element from the part, element that is pointed
	   to by the iterator __position. It also erases its
	   corresponding gid from the index gid map.It returns an iterator to the newly inserted element.
	*/
	iterator _erase(iterator __position,base_distribution tag)
	  {
	    //	   map_index_gid.erase(map_index_gid.begin() + (__position - __data.begin()));
	    size_t _pos=__position - __data.begin();
	    GID _gid= (*(map_index_gid.find(_pos))).second;
	    map_index_gid.erase(_pos);
	    map_gid_index.erase(_gid);
	    return __data.erase(__position);

	  }
  public:
      /**
	 It erases an element from the part, element that is pointed
	 to by the iterator __position.It also erases its
	 corresponding gid from the index gid map if the part has been
	 instantiated with base distribution .
	 It returns an iterator to the newly inserted element.
       */
       iterator erase(iterator __position)
      {

	_erase(__position,distribution_tag());
      }
       /**
	  Returns the size of the index gid map
	*/

       size_t size_map()
	 {
	   return map_index_gid.size();
	 }
    ///////////////////////////////////////////////////////////////////////////////////////////    

	 void push_back(const value_type& t)
	   {
	     __data.push_back(t);
	   }








 protected:
       /**
	  Returns the value of the element at the index gid
	*/
 const T _GetElement_gid(const GID _gid, vector_distribution tag) const 
   {

     return *(__data.begin()+_gid);
   }
 /**
    Locates the gid with the value passed as parameter in the index
    gid map. If found, it returns the corresponding element form the
    part. 
  */
 const T _GetElement_gid(const GID _gid, base_distribution tag) const
   {

     /*     for( vector<GID>::const_iterator it = map_index_gid.begin();it != map_index_gid.end();it++)
       if(*it == _gid)
	 {
	   time_part_get_element+=stop_timer(timer_part_get_element);
	   return *(__data.begin()+ (it - map_index_gid.begin()) );
	 }
     */
     map_gid_index_type::const_iterator it=map_gid_index.find(_gid);
     if (it!=map_gid_index.end())
       {
	 return *(__data.begin() + (*it).second);
       }
     pair<int,int> __p=GetRange();
     printf("\n VectorPart::GetElement(GID) on thread %d did not find the element %d. This part id %d and the range (%d,%d)",get_thread_id(),_gid,this->partid,__p.first,__p.second);
     exit(0);
   }
  public:
 /**
    Returns the value of an element. If the part has been instatiated
    with vector distribution, the parameter passed is considered as
    index, else it is considered as gid.
  */
  const T GetElement(const GID _gid) const 
    {
      return _GetElement_gid(_gid,distribution_tag());
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////    
    protected:
  /**
     Vector distribution :Sets the value passed as parameter to the element located at index gid.
   */
      void _SetElement(GID _gid,const T& _t, vector_distribution tag)
	{
	  __data[_gid]=_t;
	}
      /**
	 Base Distribution: sets the value passed as parameter to the element whose gid is _gid
       */      
      void _SetElement(GID _gid,const T& _t, base_distribution tag)
	{
	  /*
	  for(vector<GID>::iterator it = map_index_gid.begin();it != map_index_gid.end();it++)
	    if(*it == _gid)
	    *(__data.begin()+ (it - map_index_gid.begin()) )= _t;
	    */
	  map_gid_index_type::iterator it=map_gid_index.find(_gid);
	  if (it!=map_gid_index.end())
	    {
	      *(__data.begin() + (*it).second)=_t;
	      return;
	    }
	  
	  printf("\n VectorPart::SetElement(GID) did not find the element ");
	  exit(0);  
	}
      /**
	 Sets the value t to the element whose gid is _gid (if the
	 part has been instantiated with base distribution). If it has
	 been instantiated with vector distribution, it sets the value
	 t to the element located at index gid.
       */
  public:
    void SetElement(GID _gid,const T& _t)
      {
	_SetElement(_gid,_t,distribution_tag());
      }


    ///////////////////////////////////////////////////////////////////////////////////////////    
      protected:
      /**
	 Vector distribution :Deletes the element located at local index gid from the data
      */
      void _DeleteElement(GID _gid, vector_distribution tag)
	{
	  __data.erase(__data.begin()+ _gid);
	}
      /**
	 Base distribution :Deletes the element whose gid is _gid from
	 the data. It looks for gid in the index gid map, and if found
	 , deletes the element from the data.
      */
      void _DeleteElement(GID _gid, base_distribution tag)
	{
	  map_gid_index_type::iterator it=map_gid_index.find(_gid);
	  if (it!=map_gid_index.end())
	    {
	      __data.erase(__data.begin()+ (*it).second);
	      map_gid_index.erase(it);
	      return;
	    }
	  printf("\n VectorPart::DeleteElement(GID) did not find the element ");
	  exit(0);  
	  /*
	  for(vector<GID>::iterator it = map_index_gid.begin();it != map_index_gid.end();it++)
	    if(*it == _gid)
	      __data.erase(__data.begin()+ (it - map_index_gid.begin()));
	  */
	}
      /**
	 It deletes the elements from the data. If the part has been
	 instantiated with base distribution, the parameter passed is
	 considered to be the corresponding gid of the element that
	 needs to be deleted. If the part has been instantiated with
	 the vector distribution, the parameter passed is considered
	 to be the local index of that element.
       */
  public:
    void DeleteElement(GID _gid)
      {
	_DeleteElement(_gid,distribution_tag());
      }
    ///////////////////////////////////////////////////////////////////////////////////////////    
      protected:
    /**
       It considers the parameter passed as index, so it checks if the
       index is between bounds. If so, returns true, false otherwise.
     */
      bool _ContainElement(const GID _gid, vector_distribution tag)
	{
	  if  ((_gid>=__offset)&&(_gid<(__data.size()+__offset)))
	    //((_gid>=0)&&(_gid<__data.size()))
	    return true;
	  else 
	    return false;
	}
      /**
	 It looks in the index gid map for gid. If found returns true,
	 false otherwise.
       */
      bool _ContainElement(const GID _gid, base_distribution tag)
	{
	  map_index_gid_type::iterator it=map_index_gid.find(_gid);
	  if (it!=map_index_gid.end())
	    return true;
	  else return false;
	  /*
	  for( vector<GID>::iterator it = map_index_gid.begin();it != map_index_gid.end();it++)
	    if(*it == _gid) return true;
	  return false;
	  */
	}

  public:
      /**
	 It looks up an element. If the part has been instantiated
	 with vector distribution , it considers the argument passed
	 as index, if part has been instantiated with base
	 distribution it considers the argument passed as GID.
       */
    bool ContainElement(const GID _gid)
      {
	return _ContainElement(_gid,distribution_tag());
      }
    
    ///////////////////////////////////////////////////////////////////////////////////////////    
      protected:
    /**
       Vector distribution : It checks wether the element with the pvector index gid belongs
       to this part.If so, it sets the iterator to point to that element.
     */

      bool _ContainElement(const GID _gid,iterator *__pos, vector_distribution tag)
	{
	  if ((_gid>=__offset)&&(_gid<(__data.size()+__offset)))
	    {
	      *__pos=__data.begin()+ _gid;
	      return true;
	    }
	  else return false;
	}

      /**
	 Base distribution : it checks if the element whose gid is
	 passed as argument belongs to this part. If so, it sets the iterator to point to that element.
       */

      bool _ContainElement(const GID _gid,iterator *__pos, base_distribution tag)
	{

	  map_gid_index_type::iterator it=map_gid_index.find(_gid);
	  if (it!=map_gid_index.end())
	    {
	      *(__pos)=__data.begin()+ (*it).second;
	      return true;
	    }
	  else 
	    return false;
	  /*
	  for(vector<GID>::iterator it = map_index_gid.begin();it != map_index_gid.end();++it)
	    if(*it == _gid) 
	      {
		*__pos=__data.begin()+ (it - map_index_gid.begin());
		return true;
	      }
	  return false;
	  */
	}

  public:
      /**
	 It checks if the element whose gid (if base distribution) or
	 pvector index ( if vector distribution) belongs to this
	 part.If so, it sets the iterator to point to that element.
       */
    bool ContainElement(GID _gid,iterator *__pos )
      {
	return _ContainElement(_gid,__pos,distribution_tag());
      }
    ///////////////////////////////////////////////////////////////////////////////////////////    
      protected:
    /**
       Vector distribution : checks is the pvector index is between
       pvector index bounds. If so, it returns the iterator that
       points to that element and true in a pair. (false, default
       iterator value) otherwise.
     */
      pair<bool,T> _ContainIndex(size_t index, iterator & __pos, vector_distribution tag)
	{
	  if ((index>=__offset)&&(index< (__offset+__data.size())))
	    {
	      __pos=__data.begin()+index;
	      return pair<bool,T>(true,*__pos);
	    }
	  else 
	    return pair<bool,T>(false,T());
	}
      /**
	 Base distribution : checks if the gid passed as argument
	 belongs to the part ( index gid map). If so, it returns the
	 iterator that points to that element and true in a
	 pair. (false, default iterator value) otherwise.
       */
      pair<bool,T> _ContainIndex(size_t index, iterator & __pos, base_distribution tag)
	{
	  map_index_gid_type::iterator it=map_index_gid.find(index);
	  if (it!=map_index_gid.end())
	    {
	      __pos=__data.begin()+ (*it).second;
	      return pair<bool,T>(true,*__pos);
	    }
	  else 
	    return pair<bool,T>(false,T());
	  /*
	  for( vector<GID>::iterator it = map_index_gid.begin();it != map_index_gid.end();it++)
	    if(*it == index)
	      {
		__pos=__data.begin() + (it- map_index_gid.begin());
		return pair<bool,T>(true,*__pos);
	      }
	  return pair<bool,T>(false,T());
	  */
	}
  public:
      /**
	 Checks if the index (if vector distribution is used) or gid ( if base distribution is used) belongs to this part. 
	 If not, return (false,T()), else return (true,data[index])
      */
    pair<bool,T>  ContainIndex( size_t index, iterator& __pos)
      {
	return _ContainIndex(index,__pos,distribution_tag());
      }

    ///////////////////////////////////////////////////////////////////////////////////////////    
      /**
	 Returns the range type default value.
       */
    Element_Set_Type GetElementSet() const 
      {return Element_Set_Type();}

    /************************Map Index Gid methods*****************************************/
    /**
       It returns pvector index of the element whose gid is passed
       as argument. It returns -1 otherwise.
     */

    size_t get_index(GID _gid)
      {
	  map_gid_index_type::iterator it=map_gid_index.find(_gid);
	  if (it!=map_gid_index.end())
	    {
	      return (*it).second;
	    }
	  else 
	    return -1;
	/*
	// find the location of the _gid in the map index-gid
	vector<GID>::iterator it=find(map_index_gid.begin(),map_index_gid.end(),_gid);
	// if it's not found, return -1
	if (it==map_index_gid.end())
	  return -1;
	// else return the index=offset + position of the _gid
	else
	  return __offset + (it-map_index_fid.begin());
	*/
      }
    /**
       It returns the gid of the element located at pvector index "index". If
       index is outside bounds it returns -1.
     */
    GID get_gid(size_t index)
      {
	// if the index is not located within the range of this part, return -1
	map_index_gid_type::iterator it=map_index_gid.find(index-__offset);
	if (it!=map_index_gid.end())
	    {
	      return (*it).second;
	    }
	  else 
	    {
	      printf("\n VectorPart::get_gid Looked up for %d, index %d and offset %d",index-__offset,index,__offset);
	      for(map_index_gid_type::iterator it=map_index_gid.begin();it!=map_index_gid.end();++it)
		printf("\n Map index %d to gid %d",(*it).first,(*it).second);
	    return -1;
	    }
      }


    /***************************************************************************/
    // Size of the data
    /**
       Returns the size of the data
     */
    size_t size() const
      {
	return __data.size();
      }
    // Whether or not  __data has any elements 
    /**
       Returns trus is the data is empty, false otherwise
     */
    bool empty() const
      {
	return __data.empty();
      }
    /**
       Reserves n memory locations for the data and n memory locations for the index gid map
     */
    void reserve(size_t n)
      {
	__data.reserve(n);
	//	map_index_gid.reserve(n);
      }
    

    /***************************************************************************/
    /**
       Returns an iterator to the beginnig of the data
     */ 
   iterator begin()
     { return __data.begin();}

   /**
       Returns a const  iterator to the beginnig of the data
     */ 
   const_iterator begin() const 
     { return __data.begin();} 
   /**
      Returns an iterator to the end of the data
   */ 
    iterator end()
      { return __data.end();}
    /**
      Returns a const iterator to the end of the data
   */    
    const_iterator end() const
      { return  __data.end();} 
   /**
      Returns an iterator to the beginning of the index gid map
   */     

    vector<GID>::iterator get_gids_begin()
      {
	return vector_index_gid.begin();
      }
   /**
      Returns an iterator to the end of the index gid map
   */     

    vector<GID>::iterator get_gids_end()
      {
	return vector_index_gid.end();
      }

    /**
       Prints the part
     */
    void DisplayPart() const
      {
	printf("\n Part id %d, size %d, range (%d,%d) ---------Thread %d",GetPartId(), size(), __offset,__offset + size(),get_thread_id());
	  printf("\n Vector index_gid size %d, index_gid size %d and gid_index size %d",vector_index_gid.size(),map_index_gid.size(),map_gid_index.size());

	  if (vector_index_gid.size()==__data.size())
	    for(int i=0;i<__data.size();++i)
	      printf("\n Part[%d] and GID=%d",i,i,vector_index_gid[i]);
	  printf("\n Map Index Gid ");
	  for(map_index_gid_type::const_iterator it=map_index_gid.begin();it!=map_index_gid.end();++it)
	    printf("\n Index =%d and GID %d",(*it).first,(*it).second);

	  printf("\n Map Gid Index ");

	  for(map_gid_index_type::const_iterator it=map_gid_index.begin();it!=map_gid_index.end();++it)
	    printf("\n GID %d and Index %d",(*it).first,(*it).second);
      }
    


    /**

     */
 void make_gid_index_maps_from_vector()
      {
	if (vector_index_gid.size()==0)  return;
	for(int i=0;i<vector_index_gid.size();++i)
	  {
	    map_index_gid[i]=vector_index_gid[i];
	    map_gid_index[vector_index_gid[i]]=i;
	  }
	vector_index_gid.clear();
      }
    void make_vector_from_gid_index_maps()
      {
	//	printf("\n START MAKING  A VECTOR  %d---vector size %d index_gid %d gid_index %d",get_thread_id(),vector_index_gid.size(),map_index_gid.size(),map_gid_index.size());
	if (map_index_gid.size()==0) return;
	new (&vector_index_gid) vector<GID>(map_index_gid.size());
	//	printf("\n Size %d and reserve %d",vector_index_gid.size(),map_index_gid.size());
	for(map_index_gid_type::iterator it=map_index_gid.begin();it!=map_index_gid.end();++it)
	  {
	    //	    printf("\n Vector on position %d GID %d",(*it).first,(*it).second);
	    vector_index_gid[(*it).first]=(*it).second;
	    //    vector_index_gid.push_back((*it).second);
	  }
	map_index_gid.clear();
	map_gid_index.clear();
	//	printf("\n END MAKING  A VECTOR  %d",get_thread_id());

      }
    //===============================

    /**
       Empties both the data and the index gid map
     */
    void clear()
      {
	__data.clear();
	map_index_gid.clear();
	map_gid_index.clear();
      }
    /**
       Returns the capacity of the part
     */
    size_t capacity()
      {return __data.capacity();}



    //************ Operators ****************
      /**
	 If the offsets and the sizes are equal returns true, false otherwise
       */
      bool operator == (const this_type & __x) const
	{ return ((__offset==__x.offset) && (size()==__x.size()));}

      /**
	 Offset based comparison
      */     
      bool operator < (const this_type & __x) const
	{return (__offset < __x.__offset);}
            /**
	 Offset based comparison
      */   
      bool operator <= (const this_type & __x) const 
	{ return (__offset <= __x.__offset);}
      /**
	 Offset based comparison
      */   
      bool operator > (const this_type & __x) const 
	{return (__offset > __x.__offset);}
      /**
	 Offset based comparison
      */   
      bool operator >= (const this_type & __x) const 
	{return (__offset >= __x.__offset);}
      /**
	 Returns the value of the element at the position n
      */   
      value_type& operator[] (size_t __n)
	{return __data[__n];}

      /**
	 Prints the part
       */
      void print()
	{
	  printf("\n Vector index_gid %d, index_gid %d gid_index %d",vector_index_gid.size(),map_index_gid.size(),map_gid_index.size());

	  if (vector_index_gid.size()==__data.size())
	    for(int i=0;i<__data.size();++i)
	      printf("\n Part[%d] and GID=%d",i,vector_index_gid[i]);
	  printf("\n Map Index Gid ");
	  for(map_index_gid_type::iterator it=map_index_gid.begin();it!=map_index_gid.end();++it)
	    printf("\n Index =%d and GID %d",(*it).first,(*it).second);

	  printf("\n Map Gid Index ");

	  for(map_gid_index_type::iterator it=map_gid_index.begin();it!=map_gid_index.end();++it)
	    printf("\n GID %d and Index %d",(*it).first,(*it).second);
	}
	
  }; 

 template <class T> 
 bool operator<(pair<T,T>& __pair, const size_t& value)
   {
     	printf("\n CHeck this %d in this range (%d, %d)",value,__pair.first,__pair.second);
	if (__pair.second <= value) return true;
	else return false;
   }  


 // struct within_range:public binary_function<size_t,pair<int,int>,bool>{
 struct within_range {
   bool operator()(pair<int,int>& __pair,const size_t& value)
      {
	if (__pair.second <= value) return true;
	else return false;
	
      }
  };


}// end namespace stapl

//@}
#endif
