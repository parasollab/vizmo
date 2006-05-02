#ifndef STAPLITERATOR
#define STAPLITERATOR
//*************************************************************************
/*!
        \file splLocaIterator.h
        \author  Alin Jula
        \date  07/07/03
        \ingroup stapl
        \brief The class implements an iterator over all the elements in a pContainer representative.S
	It iterates over all the elements of a pContainer part and moves to the next (storage wise)
	part of the pContainer.
                                                                                                                                                             
        STAPL - project
        Texas A&M University,College Station, TX, USA
*/
//*************************************************************************
                                                                                                                                                             

//#include "BasePContainer.h"
#include <iterator>
#include "stapl_pc_tracer.h"

namespace stapl {

struct stapl_base_element_tag {};
struct stapl_element_tag {};


/////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////
template<class pContainer,class trace_class, typename  element_tag>
class spl_Local_Bidirectional_Iterator;

template<class pContainer_type,class trace_class,typename element_tag>
class spl_Local_Random_Iterator ;

/////////////////////////////////////////////////////////////

template<class pContainer_type, class trace_class, typename  element_tag, typename  iterator_tag>
class stapl_iterator_type{
typedef spl_Local_Bidirectional_Iterator<pContainer_type,trace_class,element_tag>  iterator;
};

template<class pContainer_type, class trace_class, typename  element_tag>
class stapl_iterator_type<pContainer_type, trace_class, element_tag, random_access_iterator_tag>{
 public:
  typedef spl_Local_Random_Iterator<pContainer_type,trace_class, element_tag>  iterator;
};


template<class pContainer_type, class trace_class, typename element_tag>
class stapl_iterator_type<pContainer_type,trace_class, element_tag,bidirectional_iterator_tag>{
 public:
  typedef spl_Local_Bidirectional_Iterator<pContainer_type,trace_class, element_tag>  iterator;
};

///////////////////////////////////////////////////////////////////////////////////////

/*
template<class Iterator,class Reference,int N>
class stapl_dereference
{
 public:
  Reference operator()(Iterator it)
   {return *it; }
};



template<class Iterator,class Reference>
class stapl_dereference<Iterator,Reference,STAPL_BASE_ELEMENT_ITERATOR>
{
 public:
  Reference operator()(Iterator it)
    {
      return  it->GetUserData();
    }

};

template<class Iterator,class Reference>
class stapl_dereference<Iterator,Reference, STAPL_ELEMENT_ITERATOR>
{
 public:
  Reference operator()(Iterator it)
    {
      return  *it;
    }

};

*/

template<class Iterator,class Reference>
inline Reference stapl_dereference(Iterator it,stapl_base_element_tag x)
{ return  it->GetUserData(); }


template<class Iterator,class Reference>
inline Reference stapl_dereference(Iterator it, stapl_element_tag x)
{ return  *it; }







template<class pContainer,class trace_class , typename  element_tag>
class spl_Local_Bidirectional_Iterator
{
 public:
  // ############ Types #########################################
  /** The STL iterator on the part elements*/
  typedef typename pContainer::pContainerPart_type::iterator  _base_iterator;

  /** Part Iterator type */
  typedef typename pContainer::parts_internal_iterator  part_iterator;

  /** Part Iterator type */
  typedef typename pContainer::const_parts_internal_iterator  const_part_iterator;

  /** Value type : the element type in the part*/
  typedef typename pContainer::pContainerPart_type::value_type value_type;


 /** reference type */
  typedef typename pContainer::pContainerPart_type::reference reference;

  /** Difference type*/
  typedef typename iterator_traits<_base_iterator>::difference_type difference_type;

  /** Pointer type */
  typedef typename iterator_traits<_base_iterator>::pointer pointer;

  /** Self type*/
  typedef spl_Local_Bidirectional_Iterator<pContainer, trace_class, element_tag> this_type;

 /** Type of the iterator*/
  typedef bidirectional_iterator_tag iterator_category;

 private:
  //############ Data members ####################################
  /** Iterator on a pContainer part */
  _base_iterator current;

  /** The end iterator of the current part in which the "current" iterator is active */
  _base_iterator end_part;

  /** The begin iterator of the current part in which the "current" iterator is active */
  _base_iterator begin_part;

  /** Iterator to the current part in the pContainer */
  part_iterator current_part;

  /** pointer to the pContainer respresentative which owns that parts.
      Necessary when the iterator moves to the next part.
  */
  pContainer * _base_pc;
  
 public:
 // ############ Constructors & Destructor #########################
  /** Constructor*/
  spl_Local_Bidirectional_Iterator()
    {
    }

  spl_Local_Bidirectional_Iterator(pContainer * _p)
    {
      _base_pc=_p;
      current_part=_base_pc->parts_begin();
      current=begin_part=(*current_part)->begin();
      end_part=(*current_part)->end();
    }

  /** Constructor*/
  spl_Local_Bidirectional_Iterator(pContainer * _p,  part_iterator __pit,  _base_iterator it)
    {
      _base_pc=_p;
      current=it;
      begin_part=(*__pit)->begin();
      end_part=(*__pit)->end();
      current_part=__pit;
    }

  /** Copy constructor */
  spl_Local_Bidirectional_Iterator(const spl_Local_Bidirectional_Iterator & _x)
    {
      _base_pc=_x._base_pc;
      current=_x.current;
      end_part=_x.end_part;
      begin_part=_x.begin_part;
      current_part=_x.current_part;
    }

  /** Destructor*/
  ~spl_Local_Bidirectional_Iterator()
    {}
  //######### Operators defined ##########################

  /** Equal operator */
  bool operator==(const this_type & _x)
    {
      return (current==_x.current);
    }

  /** Not Equal operator */
  inline bool operator !=(const this_type& _x)
    {
      return (current!=_x.current);
    }

  /** Dereference operator */
  
  inline reference operator*()
    {
     return stapl_dereference<_base_iterator,reference>(current, element_tag());
    }
  
  _base_iterator operator->() const
    { return current;}
  

  /** ++ operator()*/
  inline this_type& operator++()
    {
      if (current!=end_part)
	++current;
      
      // if it reached the end of the part, get the next part if available and set the iterator to the beginning of that part AND
      // if it's the last part
      if ((current==end_part)&&(current_part!=(_base_pc->parts_end()-1)))
	{
	  ++current_part;
	  begin_part=(*current_part)->begin();
	  end_part=(*current_part)->end();
	  current=begin_part;
	}
      return *this; 
    } 

  /** ++ operator(int)*/
  this_type operator++(int)
    {
      this_type _tmp=*this;
      ++(*this);
      return _tmp;
    }

  /** -- operator()*/
 this_type& operator--()
    {
     if (current!=begin_part) // if not at the end of the part.
	--current;
      else // if it reached the end of the part, get the next part if available and set the iterator to the beginning of that part
	if(current_part!=_base_pc->parts_begin())
	  { 
	    --current_part;
	    begin_part=(*current_part)->begin();
	    end_part=(*current_part)->end();
	    current=end_part;
	    --current;

	  }
      return *this;
    } 
 
 

 part_iterator get_part()
   {return current_part;}


 _base_iterator get_iterator()
   {return current;}

 pContainer* get_pcontainer()
   {return _base_pc;}

 reference get_element()
  {return *current;}
};





//***********************************************************************************************




template<class pContainer,class trace_class, typename element_tag>
class spl_Local_Random_Iterator 
{
public:
  // ############ Types #########################################
  /** The STL iterator on the part elements*/
  typedef typename pContainer::pContainerPart_type::iterator  _base_iterator;

  /** Part Iterator type */
  typedef typename pContainer::parts_internal_iterator  part_iterator;

  /** Value type : the element type in the part*/
  typedef typename pContainer::pContainerPart_type::value_type value_type;


 /** reference type */
  typedef typename pContainer::pContainerPart_type::reference reference;


 /** Difference type*/
  typedef typename iterator_traits<_base_iterator>::difference_type difference_type;
 
  /** Pointer type */
  typedef typename iterator_traits<_base_iterator>::pointer pointer;

 
  /** Self type*/
  typedef spl_Local_Random_Iterator<pContainer,trace_class, element_tag> this_type;


  /** Difference type*/
  //  typedef typename _base_iterator::difference_type difference_type;

  /** Type of the iterator*/
  typedef random_access_iterator_tag iterator_category;
  

 // ############ Constructors & Destructor #########################
  /** Constructor*/
  spl_Local_Random_Iterator()
    {
    }

  spl_Local_Random_Iterator(pContainer * _p)
    {
      _base_pc=_p;
      current_part=_base_pc->parts_begin();
      current=begin_part=(*current_part)->begin();
      end_part=(*current_part)->end();

    }

  /** Constructor*/
  spl_Local_Random_Iterator(pContainer * _p, part_iterator __pit, _base_iterator it)
    {
      _base_pc=_p;
      current=it;
      begin_part=(*__pit)->begin();
      end_part=(*__pit)->end();
      current_part=__pit;
    }

  /** Copy constructor */
  spl_Local_Random_Iterator(const spl_Local_Random_Iterator & _x)
    {
      _base_pc=_x._base_pc;
      current=_x.current;
      end_part=_x.end_part;
      begin_part=_x.begin_part;
      current_part=_x.current_part;
    }

  /** Destructor*/
  ~spl_Local_Random_Iterator()
    {}

 private:
  //############ Data members ####################################
  /** Iterator on a pContainer part */
  _base_iterator current;

  /** The end iterator of the current part in which the "current" iterator is active */
  _base_iterator end_part;

  /** The begin iterator of the current part in which the "current" iterator is active */
  _base_iterator begin_part;

  /** Iterator to the current part in the pContainer */
  part_iterator current_part;

  /** pointer to the pContainer respresentative which owns that parts.
      Necessary when the iterator moves to the next part.
  */
  pContainer * _base_pc;

  //################ Operators##################################
 public:
  /** operator +n */
  this_type operator+(ptrdiff_t  n) const {

    _plus_ptrdiff(trace_class());

    if (n<0) return operator-(0 -n);
    this_type temp(*this);

    size_t d=distance(temp.current,temp.end_part);
    if (n<d) // if the +n iterator is within the current part
      {
	temp.current=temp.current+n;
	return temp;
      }
      else // if it's not
	{

	  n=n-d; 
	  d=0;
	  do
	    {
	      // move to the next part;
	      ++(temp.current_part);
	      //	      printf("\n Currently in PART id=%d",(*(temp.current_part))->GetPartId());

	      //	      (*(temp.current_part))->print();
	      // if it's the last part
	      if ( temp.current_part==temp._base_pc->parts_end()) 
		{
		  temp.current_part = temp._base_pc->parts_end() - 1 ;
		  temp.end_part=temp.current = (*(temp.current_part))->end();
		  temp.begin_part=(*(temp.current_part))->begin();
		  return temp;
		}
	      d=(*(temp.current_part))->size();
	      n=n-d;
	    } while(n>=0);
	  // here n is negative
	  n=n+d;
	  
	  // now n points to the position in the current part which represents the current iterator;
	  temp.current= (*(temp.current_part))->begin() + n;
	  temp.begin_part=(*(temp.current_part))->begin();
	  temp.end_part=(*(temp.current_part))->end();
	}
    return temp;
  }

  ptrdiff_t operator-(const this_type& it) const {

    _minus_this(trace_class());

    if (_base_pc!=it._base_pc) return -1;
    ptrdiff_t result=0;
    int __current_iterator_smaller=0;
    part_iterator __first_part=current_part;
    part_iterator __second_part=it.current_part;
    while ((*__first_part)->GetPartId()< (*(__second_part))->GetPartId())
      {
	result+=(*__first_part)->size();
	++__first_part;
	__current_iterator_smaller=-1;
      }
    while ((*__first_part)->GetPartId()> (*(__second_part))->GetPartId())
      {
	result+=(*__second_part)->size();
	++__second_part;
	__current_iterator_smaller=1;

      }
    if (__current_iterator_smaller==-1)
      {
	result-=current-begin_part;
	result+=it.current-it.begin_part;
	return -result;
      }
    if(__current_iterator_smaller==1)
      {
	result-=it.current- it.begin_part;
	result+=current-begin_part;
	return result;
      }
    if (__current_iterator_smaller==0)
      {
	return current-it.current;
      }
    






    return current - it.current;

  }

  bool operator<(const this_type& it) const{
    //if current part id < it. current part it, return true
    if ((*current_part)->GetPartId()< (*(it.current_part))->GetPartId())
      return true;
    else
      // if parts are the same
      if ((*current_part)->GetPartId()== (*(it.current_part))->GetPartId())
	return current < it.current;
    //if current part id > it.current part id
      else
	return false;
  }

  this_type operator-(ptrdiff_t n) const {
    
    _minus_ptrdiff(trace_class());
    
    if (n<0) return operator+(0-n);
    this_type temp(*this);
    size_t d = distance(temp.begin_part,temp.current);
    if (n<=d) // if the -n iterator is within the current part
      {
	temp.current=temp.current-n;
	return temp;
      }
    else // if it's not
      {
	n=n-d; 
	d=0;
	  do
	    {
	      if(temp.current_part == temp._base_pc->parts_begin()){
		temp.current = (*(temp.current_part))->begin();
		return temp;
	      }
	      --(temp.current_part); // move to the next part;
	      d=(*(temp.current_part))->size();      
	      n=n-d;
	    } while(n>0);
	  // here n is negative
	  n=n+d;
	  // now n points to the position in the current part which represents the current iterator;
	  temp.current= (*(temp.current_part))->end() - n;
	  temp.begin_part=(*(temp.current_part))->begin();
	  temp.end_part=(*(temp.current_part))->end();
	}
    return temp;
  }

  /** Equal operator */
  bool operator==(const this_type & _x) const
    {
      return (current==_x.current);
    }

  /** Not Equal operator */
  bool operator !=(const this_type& _x) const
    {
      return (current!=_x.current);
    }

  /** Dereference operator */
  
  reference operator*()
    {
      _dereference(trace_class());
      return stapl_dereference<_base_iterator,reference>(current, element_tag());
    }
  
  _base_iterator operator->()
    {
      _arrow(trace_class());
      return current;
    }
  

  /** ++ operator()*/
  this_type& operator++()
    {
      _plus_plus(trace_class());

      if (current!=end_part)
	++current;
      
      // if it reached the end of the part, get the next part if available and set the iterator to the beginning of that part AND
      // if it's the last part
      if ((current==end_part)&&(current_part!=(_base_pc->parts_end()-1)))
	{
	  ++current_part;
	  begin_part=(*current_part)->begin();
	  end_part=(*current_part)->end();
	  current=begin_part;
	}
      return *this;
    } 

  /** ++ operator(int)*/
  this_type operator++(int)
    {
      this_type _tmp=*this;
      ++(*this);
      return _tmp;
    }

  /** -- operator()*/
 this_type& operator--()
    {
      _minus_minus(trace_class());

      if (current!=begin_part) // if not at the end of the part.
	--current;
      else // if it reached the end of the part, get the next part if available and set the iterator to the beginning of that part
	if(current_part!=_base_pc->parts_begin())
	  { 
	    --current_part;
	    begin_part=(*current_part)->begin();
	    end_part=(*current_part)->end();
	    current=(end_part-1);

	  }
      return *this;
    } 
 
 this_type operator--(int)
    {
      this_type _tmp=*this;
      --(*this);
      return _tmp;
    } 
 this_type& operator+=(const ptrdiff_t n)
   {
     _plus_equal(trace_class());

     this_type temp=operator+(n);
     current=temp.current;
     current_part=temp.current_part;
     begin_part=temp.begin_part;
     end_part=temp.end_part;
     
     return *this;
   }

 this_type& operator-=(const ptrdiff_t n)
   {
     _minus_equal(trace_class());

     this_type temp=operator-(n);
     current=temp.current;
     current_part=temp.current_part;
     begin_part=temp.begin_part;
     end_part=temp.end_part;
     
     return *this;
   }


 part_iterator get_part()
   {return current_part;}


 _base_iterator get_iterator()
   {return current;}

 pContainer* get_pcontainer()
   {return _base_pc;}

 reference get_element()
  { return *current;}


 protected:
 void _plus_plus(trace x)
   {
     _base_pc->iterator_plus_plus_trace();
   }
 void _plus_plus(no_trace x) const
   {
     return;
   }
 void _minus_minus(trace x)
   {
     _base_pc->iterator_minus_minus_trace();
   }
 void _minus_minus(no_trace x) const
   { 
     return;
   }


  void _plus_ptrdiff(trace x)
   {
     _base_pc->iterator_plus_ptrdiff_trace();
   }
 void _plus_ptrdiff(no_trace x) const 
   {
     return;
   }
  void _minus_ptrdiff(trace x) 
   {
     _base_pc->iterator_minus_ptrdiff_trace();
   }
  void _minus_ptrdiff(no_trace x)const
    {
      return;
    }
  void _minus_this(trace x)
   {
     _base_pc->iterator_minus_this_trace();
   }
  void _minus_this(no_trace x)const
    {
      return;
    }
  void _plus_equal(trace x)
   {
     _base_pc->iterator_plus_equal_trace();
   }
 void _plus_equal(no_trace x)const
   {
     return;
   }
  void _minus_equal(trace x)
   {
     _base_pc->iterator_minus_equal_trace();
   }
  void _minus_equal(no_trace x)const
    {
      return;
    }
  void _arrow(trace x)
    {
      _base_pc->iterator_arrow_trace();
    }
  void _arrow(no_trace x)const
    { 
      return;
    }

  void _dereference(trace x)
    {
      _base_pc->iterator_dereference_trace();
    }
  void _dereference(no_trace x)const
    {
      return;
    }

};




}//end namespace stapl
#endif
