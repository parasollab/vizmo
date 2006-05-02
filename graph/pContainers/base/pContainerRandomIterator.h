/*!
	\file GlobalIterator.h
	\date Jan. 17, 03
	\ingroup stapl
	\brief General bidirectional Iterator class. To access data globally
	random access iterator can be derived from this / or 
	define a global pointer????

	Base on LocalIterator written by Gabi.

*/

#ifndef PCONTAINERRANDOMITERATOR_H
#define PCONTAINERRANDOMITERATOR_H

#include "DistributionDefines.h"
#include "splBaseElement.h"
#include "BasePart.h"

namespace stapl {

template <class PCONTAINER>
class pContainerRandomIterator : public pContainerBaseIterator<PCONTAINER>
{
 public:
  typedef typename PCONTAINER::pContainer_Part_type Part_type;
  typedef typename PCONTAINER::pContainer_Part_type::iterator _Iterator;
  typedef typename PCONTAINER::value_type _value;

  typedef typename iterator_traits<_Iterator>::iterator_category  iterator_category;
  typedef typename iterator_traits<_Iterator>::value_type value_type;
  typedef typename iterator_traits<_Iterator>::difference_type difference_type;
  typedef typename iterator_traits<_Iterator>::pointer pointer;
  //typedef typename iterator_traits<_Iterator>::reference reference;
  typedef _Iterator iterator_type;


  pointer operator->() {
    return this->current.local_iter().get_address(); 
  }

};//end class iterator
 

}//end namespace stapl
#endif
