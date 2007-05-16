/*!
	\file GlobalIterator.h
	\date Jan. 17, 03
	\ingroup stapl
	\brief General bidirectional Iterator class. To access data globally
	random access iterator can be derived from this / or 
	define a global pointer????

	Base on LocalIterator written by Gabi.
*/

#ifndef PCONTAINERBASEITERATOR_H
#define PCONTAINERBASEITERATOR_H

#include "AbstractBasePContainer.h"

namespace stapl {

template <class PCONTAINER>
class pContainerBaseIterator : 
  public 
   iterator<typename iterator_traits<typename PCONTAINER::pContainer_Part_type::iterator>::iterator_category,
   typename iterator_traits<typename PCONTAINER::pContainer_Part_type::iterator>::value_type,
   typename iterator_traits<typename PCONTAINER::pContainer_Part_type::iterator>::difference_type,
   typename iterator_traits<typename PCONTAINER::pContainer_Part_type::iterator>::pointer,
   typename iterator_traits<typename PCONTAINER::pContainer_Part_type::iterator>::reference>
{
 public:
  typedef typename PCONTAINER::pContainer_Part_type Part_type;
  typedef typename PCONTAINER::pContainer_Part_type::iterator _Iterator;
  typedef typename PCONTAINER::value_type _value;

  typedef typename iterator_traits<_Iterator>::iterator_category  iterator_category;
  typedef typename iterator_traits<_Iterator>::value_type value_type;
  typedef typename iterator_traits<_Iterator>::difference_type difference_type;
  typedef typename iterator_traits<_Iterator>::pointer pointer;
  typedef typename iterator_traits<_Iterator>::reference reference;
  typedef _Iterator iterator_type;

 protected:
  /**
   * current pid, partid, and elemet pointed by the iterator
   */
  IterLocation<iterator_type> current;

  bool uselocaliterator;

  _value clone;

  /**
   * The pcontainer on which the iterator works; 
   * It is necessary for accesing the parts; 
   * a const ptr to local pcontainer object
   */
  PCONTAINER* pcontainer;
  
  typedef pContainerBaseIterator<PCONTAINER> pContainerBaseIterator_type;

 public:
  void define_type(stapl::typer &t) {
    stapl_assert(1,"pContainerBaseIterator define_type used.\n");
    t.local(current);
    t.local(uselocaliterator);
    t.local(clone);
    long tmp= (long) pcontainer;
    t.local(tmp);
  }

  //constructors
  pContainerBaseIterator():uselocaliterator(0) {}

  pContainerBaseIterator(PCONTAINER* __pc, iterator_type __x, PARTID _partid, PID _pid) : 
    pcontainer(__pc),uselocaliterator(0) 
    {
      current.Set(_pid,_partid, __x);
    }

  pContainerBaseIterator(PCONTAINER* __pc, iterator_type __x, 
		 PARTID _partid, bool _localit) : 
    pcontainer(__pc) ,uselocaliterator(_localit)
    {

      current.Set(get_thread_id(),_partid, __x);
    }

  pContainerBaseIterator(PCONTAINER* __pc) : 
    pcontainer(__pc) ,uselocaliterator(0)
    { 
    }

  pContainerBaseIterator(PCONTAINER* __pc, const Location& _loc, const iterator_type& _it) : 
    pcontainer(__pc), current(_loc,_it),uselocaliterator(0)
    { 
    }

  pContainerBaseIterator(const pContainerBaseIterator_type& __x) : 
    current(__x.current), 
    pcontainer(__x.base_pcontainer()),uselocaliterator(__x.uselocaliterator)
     {}

  ~pContainerBaseIterator() {}

/*   pContainerBaseIterator& operator=(const pContainerBaseIterator_type& __x) {  */
/*     current=__x.current;  */
/*     pcontainer = __x.pcontainer; */
/*     uselocaliterator=__x.uselocaliterator; */
/*     return *this; */
/*   } */

  const iterator_type base() const { return current.local_iter(); }

  void set_base(const iterator_type _c) {
    current.Set_Local_Iter(_c); 
  }

  void set_uselocaliterator(bool _b) {
    uselocaliterator = _b;
  }

  void set_all(PCONTAINER* _pc, PID _pid, PARTID  _partid, iterator_type _it) { 
    pcontainer = _pc;
    current.Set(_pid,_partid,_it);
  }


  PARTID base_partid() const { 
    return current.location().partid(); 
  }

  PCONTAINER* base_pcontainer() const { return pcontainer;}

  PID base_pid() const {return current.location().locpid(); }

  //operators
  /**LOCAL
   */

  void _set_current(const pContainerBaseIterator_type& __x) { 
    current.Set(__x.current);
  }
    
  /**NOTE: return a copy. Cannot be modifed (like  
     " *it=data; "  won't work)
     GLOBAL
  */
  /*   _value& operator*() const { */

  // Alin : the returning type was "_value&" , changed to "_value" because it is not correct to return a reference to an element in a different address space
  // ( the returning value of pcontainer ->Dereference(..)). 
  _value operator*() {

    if(uselocaliterator)
      {
	//       printf("\n return the local value");
       return pcontainer->Dereference(current.location().locpid(),
				     current.local_iter());
     }
    else {
      //       printf("\n return the remote value");

      clone = pcontainer->Dereference(current.location().locpid(),
				     current.local_iter());
      return clone;
    }
  }

  _value GetReference() {
    return pcontainer->Dereference(current.location().locpid(),
				   current.local_iter());
  }

  /**
   *non standard method required because of the fact that we 
   *iterate trough base element  and not user data
   */
  GID GetGid() {
    return pcontainer->DereferenceGID(current.location().locpid(),
				      current.local_iter());
  }

  /**LOCAL
   */
  bool operator==(const pContainerBaseIterator_type& _x) const {
    return (current == _x.current );
  }
  
  /**LOCAL
   */
  bool operator!=(const pContainerBaseIterator_type& _x) const {
    return (current != _x.current );
  }

  /**GLOBAL
   */
  pContainerBaseIterator_type& operator++() 
    {
      IterLocation<iterator_type> next = 
	pcontainer->GetNext(current,uselocaliterator);
      current = next;
      return *this;
    }

  pContainerBaseIterator_type& operator++(int ) 
    {
      //      if( i>1 ){
      //	cout<<"++ operator functionality not supported"<<endl;
      //      }
      IterLocation<iterator_type> next = 
	pcontainer->GetNext(current,uselocaliterator);
      current = next;
      return *this;
    }


  const pContainerBaseIterator_type& operator++() const 
    {
      //IterLocation<iterator_type> next = 
      //pcontainer->GetNext(current,uselocaliterator);
      //current = next;
      return *this;
    }

  const pContainerBaseIterator_type& operator++(int i) const
    {
      if( i>1 ){
	cout<<"++ operator functionality not supported"<<endl;
      }
      //IterLocation<iterator_type> next = 
      //pcontainer->GetNext(current,uselocaliterator);
      //current = next;
      return *this;
    }


  pointer operator->() {
    return current.local_iter().get_address(); 
  }

  pointer operator->() const {
    return current.local_iter().get_address(); 
  }

   reference GetBaseElementReference(){
     return *(current.local_iter());   
   }


};//end class iterator
 

}//end namespace stapl
#endif
