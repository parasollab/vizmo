#ifndef _TEST_CONTAINERS_H
#define _TEST_CONTAINERS_H

#include <runtime.h>
#include <algorithm>


// testVector is a simple vector class.
template<class T> class testVector {
  T* _begin;
  T* _end;

public:
  testVector() : _begin( 0 ), _end( 0 ) {}
  testVector(const int size) : _begin( new T[size] ), _end( _begin + size ) {}
  testVector(const testVector& v) : _begin( 0 ), _end( 0 ) {
    if( v.size() != 0 ) {
      _begin = new T[v.size()];
      _end = _begin + v.size();
      std::copy( v._begin, v._end, _begin );  // copy can be optimized to use memcpy on fundamental types
    }
  }
  ~testVector() { delete[] _begin; }

  void operator=(const testVector& v) {
    if( _begin != 0 )
      delete[] _begin;
    if( v.size() != 0 ) {
      _begin = new T[v.size()];
      for( int i=0; i<v.size(); i++ )
	_begin[i] = v._begin[i];
      _end = _begin + v.size();
    }
  }

  void define_type(stapl::typer& t) {
    const int sz = size();
    t.dynamic( _begin, sz );
    t.dynamicOffset( _end, _begin, sz );
  }

  int size() const { return _end - _begin; };
  void resize(const int s) { _begin = new T[s]; _end = _begin + s; }
  T& operator[](const int index) { return _begin[index]; }
  const T& operator[](const int index) const { return _begin[index]; }

  bool operator==(const testVector& v) const {
    if( v.size() != size() )
      return false;
    for( int i=0; i<size(); ++i )
      if( _begin[i] != v._begin[i] )
	return false;
    return true;
  }
  bool operator!=(const testVector& v) const {
    return !( *this == v );
  }
};


// testVector2 extends testVector for testing purposes.  It uses the dynamic
// offsets typical of a real vector, as well as the local types of offsets.
template<class T> class testVector2 : public testVector<T> {
  double* _dPtr;
  double _d;
  int _i[3];
  int* _iPtr;

public:
  testVector2() : testVector<T>(), _dPtr( &_d ), _iPtr( &_i[1] ) {}
  testVector2(const int size) : testVector<T>( size ), _dPtr( &_d ), _iPtr( &_i[1] ) {}
  testVector2(const testVector2& v) : testVector<T>( v ), _dPtr( &_d ), _iPtr( &_i[1] ) {}
  void operator=(const testVector2& v) { testVector<T>::operator=( v ); }

  void define_type(stapl::typer& t) {
    testVector<T>::define_type( t );
    t.local( _d );
    t.local( _i, 3 );
    t.localOffset( _dPtr, _d );
    t.localOffset( _iPtr, _i[1] );
  }

  void internalStructureCheck(const int expectedSize) const {
    stapl_assert( _dPtr == &_d, "" );
    stapl_assert( _iPtr == &_i[1], "" );
    stapl_assert( testVector<T>::size() == expectedSize, "" );
  }
};


// testList is a simple doubly-linked list class.
template<class T> class testList {
  struct node {
    T _element;
    node* _next;
    node* _prev;
    node(const T& element, node* const next, node* const prev) 
      : _element( element ), _next( next ), _prev( prev ) {}
    void define_type(stapl::typer& t) {
      t.local( _element );
      t.dynamic( _next ); // recursively define the rest of the list, until _next == 0
      t.dynamicOffset( _prev, _prev ); // as the recursion unwinds, the _prev offsets will be valid
    }
  };
  node* _head;

  class listIterator {
    node* _cur;
  public:
    explicit listIterator(node* const pos) : _cur( pos ) {}
    void operator++()    { _cur = _cur->_next; }
    void operator++(int) { _cur = _cur->_next; }
    void operator--()    { _cur = _cur->_prev; }
    void operator--(int) { _cur = _cur->_prev; }
    T& operator*() { return _cur->_element; }
    bool operator!=(const listIterator& l) { return this->_cur != l._cur; }
  };

  class constListIterator {
    node* _cur;
  public:
    explicit constListIterator(node* const pos) : _cur( pos ) {}
    void operator++()    { _cur = _cur->_next; }
    void operator++(int) { _cur = _cur->_next; }
    void operator--()    { _cur = _cur->_prev; }
    void operator--(int) { _cur = _cur->_prev; }
    const T& operator*() { return _cur->_element; }
    bool operator!=(const constListIterator& l) { return this->_cur != l._cur; }
  };

public:
  typedef listIterator iterator;
  typedef constListIterator const_iterator;

  iterator begin() { return listIterator( _head ); }
  iterator end() { return listIterator( 0 ); }
  const_iterator begin() const { return constListIterator( _head ); }
  const_iterator end() const { return constListIterator( 0 ); }

  testList() : _head( 0 ) {}
  testList(const testList& l) : _head( 0 ) {
    for( const_iterator i=l.begin(); i!=l.end(); ++i )
      insert_back( *i );    
  }
  ~testList() {
    while( _head != 0 )
      remove_front();
  }

  void operator=(const testList& l) const {
    while( _head != 0 )
      remove_front();
    for( const_iterator i=l.begin(); i!=l.end(); ++i )
      insert_back( *i );
  }

  void define_type(stapl::typer& t) {
    t.dynamic( _head ); 
  }

  int size() const {
    int _size = 0;
    for( const_iterator i=begin(); i!=end(); ++i )
      _size++;
    return _size;
  }
  void insert_front(const T& element) {
    if( _head == 0 ) {
      _head = new node( element, 0, 0 );
      _head->_prev = _head;
    }
    else {
      _head = new node( element, _head, _head->_prev );
      _head->_next->_prev = _head;
    }
  }
  void insert_back(const T& element) {
    if( _head == 0 ) {
      _head = new node( element, 0, 0 );
      _head->_prev = _head;
    }
    else {
      _head->_prev = new node( element, 0, _head->_prev );
      _head->_prev->_prev->_next = _head->_prev;
    }
  }
  void remove_front() {
    stapl_assert( _head != 0, "testList is empty, can't remove_front" );
    node* const tmp = _head;
    _head = _head->_next;
    if( _head != 0 )
      _head->_prev = tmp->_prev;
    delete tmp;
  }

  bool operator==(const testList& l) const {
    if( l.size() != size() )
      return false;
    const_iterator i = begin();
    const_iterator iL = l.begin();
    for( ; i!=end(); ++i, ++iL )
      if( *i != *iL )
	return false;
    return true;
  }
  bool operator!=(const testList& l) const {
    return !( *this == l );
  }
};

#endif

