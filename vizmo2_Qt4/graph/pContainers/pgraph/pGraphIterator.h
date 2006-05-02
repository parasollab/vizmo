#ifndef _PGRAPHITERATOR
#define _PGRAPHITERATOR
#include "Defines.h"
#include "pGraph.h"
/*
the following code was adapted from "_iterator.h"
belonging to stlport
*/

/*
 * The functionality right now is the same as for the old VI;
 * we have to reimplement some of the methods to skip gost nodes 
 * or add new iterators for DFS/BFS like traversal;
 * !!!!!  The constant vertices iterator has to be implemnted;
 */

template <class _Iterator>
class pGraph_vi : 
  public iterator<typename iterator_traits<_Iterator>::iterator_category,
                  typename iterator_traits<_Iterator>::value_type,
                  typename iterator_traits<_Iterator>::difference_type,
                  typename iterator_traits<_Iterator>::pointer,
                  typename iterator_traits<_Iterator>::reference>
{
 protected:

  _Iterator current;
  _Iterator begin;
  _Iterator end;

  typedef pGraph_vi<_Iterator> _Self;

 public:
  typedef typename iterator_traits<_Iterator>::iterator_category  iterator_category;
  typedef typename iterator_traits<_Iterator>::value_type value_type;
  typedef typename iterator_traits<_Iterator>::difference_type difference_type;
  typedef typename iterator_traits<_Iterator>::pointer pointer;
  typedef typename iterator_traits<_Iterator>::reference reference;
  typedef _Iterator iterator_type;

 public:
  void define_type(stapl::typer &t) {
    stapl_assert(1,"pGraphIterator define_type used.\n");
  }

  pGraph_vi() {}
  
  //explicit   //not hp compatible
    pGraph_vi(iterator_type __x, iterator_type __begin, iterator_type __end) 
    : current(__x), begin(__begin), end(__end) {}

  pGraph_vi(const _Self& __x) : current(__x.current), begin(__x.begin), end(__x.end) {}
  
  /*
  void define_type(stapl::typer &t){
    //this define type is here for compatibility issues
    //Some code doesn't compile whitout; You shouldn't use the values inside 
    //when you unpack because the pointers are valid in some other memory space
    t.local(current);
    t.local(begin);
    t.local(end);
  }
  */

  //              = operator
  _Self& operator = (const _Self& __x) { 
    current = __x.base(); 
    begin = __x.base_begin();
    end = __x.base_end();
    return *this; 
  } 


  iterator_type base() const { return current; }
  iterator_type base_begin() const { return begin;}
  iterator_type base_end() const { return end; }

  void SetCurrent(_Iterator _sit){
    //this is a shortcut to make the old pGraph pIsVertex work
    //you should not call ++ on it
    current = _sit;
  }

  reference operator*() const {
    return *current;
  }

  pointer operator->() const { 
    return &*current; 
  }


  //the -- operator is not safe;
  _Self& operator--() {
    --current;
    //next line specific to pgraph
    while(current->IsGhost() && current != begin) --current;
    return *this;
  }

  _Self operator--(int) {
    _Self __tmp = *this;
    
    --current;
    //next line specific to pgraph
    while(current->IsGhost() && current != begin) --current;

    return __tmp;
  }

  _Self& operator++() {

    ++current;
    //next line specific to pgraph
    while(current->IsGhost() && current != end) ++current;

    return *this;
  }

  _Self operator++(int) {
    _Self __tmp = *this;
    ++current;
    //next line specific to pgraph
    while(current->IsGhost() && current != end) ++current;
    return __tmp;
  }
  
  bool operator==(_Self& _x) const {
    return (current == _x.base());
  }

  bool operator!=(const _Self& _x) const {
    return (current != _x.base());
  }
};



//perhaps the next iterator we don't need it ;
//it is hard to because begin doesn't point before real beggining 
//the way end points after the real end;

template <class _Iterator>
class pGraph_rvi : 
  public iterator<typename iterator_traits<_Iterator>::iterator_category,
                  typename iterator_traits<_Iterator>::value_type,
                  typename iterator_traits<_Iterator>::difference_type,
                  typename iterator_traits<_Iterator>::pointer,
                  typename iterator_traits<_Iterator>::reference>
{
 protected:

  _Iterator current;
  typedef pGraph_rvi<_Iterator> _Self;

 public:
  typedef typename iterator_traits<_Iterator>::iterator_category  iterator_category;
  typedef typename iterator_traits<_Iterator>::value_type value_type;
  typedef typename iterator_traits<_Iterator>::difference_type difference_type;
  typedef typename iterator_traits<_Iterator>::pointer pointer;
  typedef typename iterator_traits<_Iterator>::reference reference;
  typedef _Iterator iterator_type;

 public:
  pGraph_rvi() {}
  explicit pGraph_rvi(iterator_type __x) : current(__x) {}
  pGraph_rvi(const _Self& __x) : current(__x.current) {}
  
  //              = operator
  _Self& operator = (const _Self& __x) { current = __x.base(); return *this; } 
  _Self& operator = (const iterator_type& __it) { current = __it; return *this; } 
  _Self& operator = (iterator_type& __it) { current = __it; return *this; } 

  iterator_type base() const { return current; }


  pointer operator->() const { 
    //?? test
    return &(operator*()); 
  }

  reference operator*() const {
    //?? test
    _Iterator __tmp = current;
    return *--__tmp;
  }

  _Self& operator++() {
    --current;

    while((current-1)->IsGhost()) --current;

    return *this;
  }

  _Self operator++(int) {
    _Self __tmp = *this;

    --current;
    while((current-1)->IsGhost()) --current;

    return __tmp;
  }

  //_Self& operator--() {
  //  ++current;
  //  while((current-1)->IsGhost()) --current;
  //  return *this;
  //}

  //_Self operator--(int) {
  //  _Self __tmp = *this;
  //  ++current;
  //  return __tmp;
  //}
  
  bool operator==(_Self& _x) const {
    return (current == _x.base());
  }

  bool operator!=(_Self& _x) const {
    return (current != _x.base());
  }

  bool operator!=(_Self _x) const {
    return (current != _x.base());
  }

  //_Self operator+(difference_type __n) const {
  //  return _Self(current - __n);
  //}
  //_Self& operator+=(difference_type __n) {
  //  current -= __n;
  //  return *this;
  //}
  //_Self operator-(difference_type __n) const {
  //  return _Self(current + __n);
  //}
  //_Self& operator-=(difference_type __n) {
  //  current += __n;
  //  return *this;
  //}

  reference operator[](difference_type __n) const { return *(*this + __n); }   
};


#endif
