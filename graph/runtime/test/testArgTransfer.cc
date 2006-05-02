// This test stresses all combinations of basic/pointer/reference and
// non-const/const data types as arguments to RMI calls, specified via
// stapl::typer.  It also stresses a variety of fundamental types, and
// user-defined objects that use local and dynamic variables.
//
// The types are passed as arguments to async_rmi and as return values from
// sync_rmi.  Various orders of invocation are attempted to stress the
// internal buffer handling.

#include "testContainers.h"

#include <runtime.h>
#include <stdio.h>


// localObject is a hierarchical structure that contains many common C++
// fundamental types.  It stresses the basic RMI transfer mechanisms (i.e., no
// dynamic variables, just local variables).  It also stresses correctly
// transferring inherited classes.
struct local2 {
  float        _f;
  unsigned int _i;
  local2(const int i) : _f(i), _i(i) {}
  bool operator==(const local2& l) const { return _f==l._f && _i==l._i; }
  bool operator!=(const local2& l) const { return !( *this == l ); }
  void define_type(stapl::typer& t) { t.local(_f); t.local(_i); }
  void inc(const int i) { _f+=i; _i+=i; }
};

struct local1 {
  local2 _l;
  char   _c;
  bool   _b;
  local1(const int i) : _l(i),  _c(i), _b(true)  {}
  bool operator==(const local1& l) const { return _l==l._l && _c==l._c && _b==l._b; }
  bool operator!=(const local1& l) const { return !( *this == l ); }
  void define_type(stapl::typer& t) { t.local(_l); t.local(_c); t.local(_b); }
  void inc(const int i) { _l.inc(i); _c+=i; }
};

struct localObjectAbstractBase {
  virtual void inc(const int i) = 0;
};

struct localObjectBase : public localObjectAbstractBase {
  int _i;
  localObjectBase() : _i(0) {}
  localObjectBase(const int i) : _i(i) {}
  void define_type(stapl::typer& t) { t.local(_i); }
  virtual void inc(const int i) { _i+=i; }
};

struct localObjectBase2 {
  double _d[2];
  localObjectBase2() { _d[0]=0;  _d[1]=0; }
  localObjectBase2(const int i) { _d[0]=i;  _d[1]=i; }
  void define_type(stapl::typer& t) { t.local(_d, 2); }
};

struct localObject : public localObjectBase, public localObjectBase2 {
  local1 _l;
  localObject() : localObjectBase(), localObjectBase2(), _l(0) {}
  localObject(const int i) : localObjectBase(i), localObjectBase2(i), _l(i) {}
  bool operator==(const localObject& l) const { return _i==l._i && _d[0]==l._d[0] && _d[1]==l._d[1] && _l==l._l; }
  bool operator!=(const localObject& l) const { return !( *this == l ); }
  void define_type(stapl::typer& t) { localObjectBase::define_type(t); localObjectBase2::define_type(t); t.local(_l); }
  virtual void inc(const int i) { _i+=i; _d[0]+=i; _d[1]+=i; _l.inc(i); }
};


// dynamicObject is a hierarchical structure that contains dynamically
// allocated variables.  It stresses the dynamic mechanisms.  It does not
// provide a default (i.e., no argument) constructor to test if an
// implementation relies on more than the copy constructor and typer.
struct dynamic1 {
  float _f;
  int*  _i;
  dynamic1() : _f(-1) { _i=new int(-1); }
  dynamic1(const int i) : _f(i) { _i=new int(i); }
  dynamic1(const dynamic1& d) : _f(d._f) { _i=new int(*d._i); }
  ~dynamic1() { delete _i; }
  void set(const int i) { _f=i; *_i=i; }
  bool operator==(const dynamic1& d) const { return _f==d._f && *_i==*d._i; }
  bool operator!=(const dynamic1& d) const { return !( *this == d ); }
  void define_type(stapl::typer& t) { t.local(_f); t.dynamic(_i); }
  void inc(const int i) { _f+=i; *_i+=i; }
};

struct dynamicObject {
  int*      _i;
  dynamic1  _d[2];
  dynamic1* _d2;
  dynamicObject(const int i) { _i=new int(i); _d[0].set(i); _d[1].set(i); _d2=new dynamic1[2]; _d2[0].set(i); _d2[1].set(i); }
  dynamicObject(const dynamicObject& d) { _i=new int(*d._i); _d[0].set(*_i); _d[1].set(*_i); _d2=new dynamic1[2]; _d2[0].set(*_i); _d2[1].set(*_i); }
  ~dynamicObject() { delete _i; delete[] _d2; }
  bool operator==(const dynamicObject& d) const { return *_i==*d._i && _d[0]==d._d[0] && _d[1]==d._d[1] && _d2[0]==d._d2[0] && _d2[1]==d._d2[1]; }
  bool operator!=(const dynamicObject& d) const { return !( *this == d ); }
  void define_type(stapl::typer& t) { t.dynamic(_i); t.local(_d, 2); t.dynamic(_d2, 2); }
  void inc(const int i) { *_i+=i; _d[0].inc(i); _d[1].inc(i); _d2[0].inc(i); _d2[1].inc(i); }
};


// complexObject combines all the test containers available!  It stresses
// all the typer mechanisms at once!
struct complexObject {
  testVector<localObject> v;
  testList<dynamicObject> l;
  testVector<localObject>* vP;
  testList<localObject>* lP;
  complexObject(const int size) : v( size ), l() {
    vP = new testVector<localObject>( size );
    lP = new testList<localObject>();
  };
  complexObject(const complexObject& c) : v( c.v ), l( c.l ) {
    vP = new testVector<localObject>( *c.vP );
    lP = new testList<localObject>( *c.lP);
  }
  bool operator==(const complexObject& c) const {
    return (c.v == v) && (c.l == l) && (*c.vP == *vP) && (*c.lP == *lP);
  }
  bool operator!=(const complexObject& c) const {
    return !( *this == c );
  }
  void define_type(stapl::typer& t) {
    t.local( v );
    t.local( l );
    t.dynamic( vP );
    t.dynamic( lP );
  }
};


struct p_test : public stapl::parallel_task {
  int _nThreads; // number of participating threads
  int _id;       // local id
  int _lt, _rt;  // neighbor id's

  p_test() {
    _nThreads = stapl::get_num_threads();
    _id = stapl::get_thread_id();
    _rt = ( _id == _nThreads - 1 ) ? 0 : _id + 1;
    _lt = ( _id == 0 ) ? _nThreads - 1 : _id - 1;
  }

  bool               fB(bool t)                 { return t; }
  char               fC(char t)                 { return t; }
  signed char        fSC(signed char t)         { return t; }
  unsigned char      fUC(unsigned char t)       { return t; }
  signed short int   fSSI(signed short int t)   { return t; }
  unsigned short int fUSI(unsigned short int t) { return t; }
  signed int         fSI(signed int t)          { return t; }
  unsigned int       fUI(unsigned int t)        { return t; }
  signed long        fSL(signed long t)         { return t; }
  unsigned long      fUL(unsigned long t)       { return t; }
  float              fF(float t)                { return t; }
  double             fD(double t)               { return t; }
  long double        fLD(long double t)         { return t; }

  void local_b_copyByValue(const localObject lt) {
    stapl_assert( lt == localObject(_lt), "" );
  }
  void local_p_copyByValue(const localObject* const lt) {
    stapl_assert( *lt == localObject(_lt), "" );
  }
  void local_r_copyByValue(localObject& lt) {
    stapl_assert( lt == localObject(_lt), "" );
    lt.inc( 1 );
  }
  void dynamic_b_copyByValue(dynamicObject rt) {
    stapl_assert( rt == dynamicObject(_rt), "" );
    rt.inc( 1 );
  }
  void dynamic_p_copyByValue(const dynamicObject* const rt) {
    stapl_assert( *rt == dynamicObject(_rt), "" );
  }
  void dynamic_r_copyByValue(dynamicObject& rt) {
    stapl_assert( rt == dynamicObject(_rt), "" );
    rt.inc( 1 );
  }

  localObject local_b_return(localObject* const rt) {
    stapl_assert( *rt == localObject(_rt), "" );
    rt->inc( 1 );
    return *rt;
  }
  localObject* local_p_return(localObject* const rt) {
    stapl_assert( *rt == localObject(_rt), "" );
    rt->inc( 1 );
    return new localObject( *rt );
  }
  localObject& local_r_return(localObject* const rt) {
    stapl_assert( *rt == localObject(_rt), "" );
    rt->inc( 1 );
    return *( new localObject(*rt) );
  }
  dynamicObject dynamic_b_return(dynamicObject* const lt) {
    stapl_assert( *lt == dynamicObject(_lt), "" );
    lt->inc( 1 );
    return *lt;
  }
  dynamicObject* dynamic_p_return(dynamicObject* const lt) {
    stapl_assert( *lt == dynamicObject(_lt), "" );
    lt->inc( 1 );
    return new dynamicObject( *lt );
  }
  dynamicObject& dynamic_r_return(dynamicObject* const lt) {
    stapl_assert( *lt == dynamicObject(_lt), "" );
    lt->inc( 1 );
    return *( new dynamicObject(*lt) );
  }

  const localObject  local_b_return0()     { return localObject(_id); }
  const localObject* local_p_return0()     { return new localObject(_id); }
  const localObject& local_r_return0()     { localObject* o = new localObject(_id); return *o; }
  const dynamicObject  dynamic_b_return0() { return dynamicObject(_id); }
  const dynamicObject* dynamic_p_return0() { return new dynamicObject(_id); }
  const dynamicObject& dynamic_r_return0() { dynamicObject* o = new dynamicObject(_id); return *o; }

  void offset_pointers(const int size, testVector2< testVector2<double> >& v) {
    v.internalStructureCheck( size );
    for( int i=0; i<size; ++i ) {
      v[i].internalStructureCheck( i );
      for( int j=0; j<i; ++j )
	stapl_assert( v[i][j] == _lt, "" );
    }
  }

  void dynamic_types(const int size, const testList<int>& l) {
    int count = l.size();
    stapl_assert( size == count, "" );
    for( testList<int>::const_iterator i=l.begin(); i!=l.end(); ++i )
      stapl_assert( *i == --count, "" );
  }

  complexObject complex_type(complexObject& c) {
    return c;
  }

  void bvector_type(vector<bool>& v, int _sz){
    vector<bool>::iterator it = v.begin();
    stapl_assert(v.size() == _sz,"bvector::Inavlid size received");
    int i = 0;
    bool b = true;
    while(i < v.size()){
      stapl_assert(v[i]==b,"bvector::Invalid value received");
      b = !b;
      ++i;
    }
  }

  vector<bool> bvector_sync_type(void){
      vector<bool> vb;
      bool b = true;
      int i = 0;
      while(i < 100){
	vb.push_back(b);
	b = !b;
	++i;
      }
      return vb;
  }

  void dvector_type(vector<double>& v, int _sz){
    stapl_assert(v.size() == _sz,"dvector::Inavlid size received");
    int i = 0;
    while(i < v.size()){
      stapl_assert(v[i]==i,"dvector::Invalid value received");
      ++i;
    }
  }

  vector<double> dvector_sync_type(void){
    vector<double> vb;
    int i = 0;
    while(i < 100){
      vb.push_back(i);
      ++i;
    }
    return vb;
  }


  void execute() {
    localObject   oL( _id );
    dynamicObject oD( _id );

    // Test the fundamental types.
    stapl_assert( false== stapl::sync_rmi(_rt, rmiHandle, &p_test::fB, false), "" );
    stapl_assert( '1'  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fC, '1'), "" );
    stapl_assert( '2'  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fSC, (signed char)'2'), "" );
    stapl_assert( '3'  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fUC, (unsigned char)'3'), "" );
    stapl_assert( 4    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fSSI, (signed short)4), "" );
    stapl_assert( 5    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fUSI, (unsigned short)5), "" );
    stapl_assert( 6    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fSI, 6), "" );
    stapl_assert( 7    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fUI, 7U), "" );
    stapl_assert( 8    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fSL, 8L), "" );
    stapl_assert( 9    == stapl::sync_rmi(_rt, rmiHandle, &p_test::fUL, 9UL), "" );
    stapl_assert( 1.0  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fF, 1.0f), "" );
    stapl_assert( 1.1  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fD, 1.1), "" );
    stapl_assert( 1.2  == stapl::sync_rmi(_rt, rmiHandle, &p_test::fLD, (long double)1.2), "" );

    // Test if user-defined types are really copy-by-value (i.e., argument
    // modifications within the RMI function aren't seen after the RMI).
    localObject testL( _id );
    stapl::async_rmi( _rt, rmiHandle, &p_test::local_p_copyByValue, &oL );
    stapl::async_rmi( _rt, rmiHandle, &p_test::local_r_copyByValue,  oL );
    stapl::async_rmi( _rt, rmiHandle, &p_test::local_b_copyByValue,  oL );
    stapl::rmi_fence();
    stapl_assert( oL == testL, "" );

    dynamicObject testD( _id );
    stapl::async_rmi( _lt, rmiHandle, &p_test::dynamic_b_copyByValue,  oD );
    stapl::async_rmi( _lt, rmiHandle, &p_test::dynamic_p_copyByValue, &oD );
    stapl::async_rmi( _lt, rmiHandle, &p_test::dynamic_r_copyByValue,  oD );
    stapl::rmi_fence();
    stapl_assert( oD == testD, "" );

    // Test if return values are transferred correctly (1 & 0 arg sync_rmi).
    testD.inc( 1 );
    dynamicObject& t1 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_r_return, &oD );
    stapl_assert(  t1 == testD, "" );
    dynamicObject  t2 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_b_return, &oD );
    stapl_assert(  t2 == testD, "" );
    dynamicObject* t3 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_p_return, &oD );
    stapl_assert( *t3 == testD, "" );
    stapl::rmi_fence();

    testL.inc( 1 );
    localObject*   t4 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_p_return, &oL );
    stapl_assert( *t4 == testL, "" );
    localObject&   t5 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_r_return, &oL );
    stapl_assert(  t5 == testL, "" );
    localObject    t6 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_b_return, &oL );
    stapl_assert(  t6 == testL, "" );
    stapl::rmi_fence();

    localObject testL2( _lt );
    const localObject  t7 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_b_return0 );
    stapl_assert(      t7 == testL2, "" );
    const localObject* t8 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_p_return0 );
    stapl_assert(     *t8 == testL2, "" );
    const localObject& t9 = stapl::sync_rmi( _lt, rmiHandle, &p_test::local_r_return0 );
    stapl_assert(      t9 == testL2, "" );
    stapl::rmi_fence();

    dynamicObject testD2( _rt );
    const dynamicObject* t10 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_p_return0 );
    stapl_assert(       *t10 == testD2, "" );
    const dynamicObject& t11 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_r_return0 );
    stapl_assert(        t11 == testD2, "" );
    const dynamicObject  t12 = stapl::sync_rmi( _rt, rmiHandle, &p_test::dynamic_b_return0 );
    stapl_assert(        t12 == testD2, "" );
    stapl::rmi_fence();

    // Test offset pointers
    if( _id % 2 == 0 ) {
      const int vSize = 100;
      testVector2< testVector2<double> > v( vSize );
      for( int i=1; i<vSize; ++i ) {
	v[i].resize( i );
	for( int j=0; j<i; ++j )
	  v[i][j] = _id;
      }
      stapl::async_rmi( _rt, rmiHandle, &p_test::offset_pointers, vSize, v );
    }
    stapl::rmi_fence();

    // Test dynamically-linked types
    if( _id % 2 == 0 ) {
      const int lSize = 10;
      testList<int> l;
      for( int i=0; i<lSize; i++ )
	l.insert_front( i );
      stapl::async_rmi( _lt, rmiHandle, &p_test::dynamic_types, lSize, l );
      l.insert_front( lSize );
      l.insert_front( lSize+1 );
      l.insert_front( lSize+2 );
      stapl::async_rmi( _rt, rmiHandle, &p_test::dynamic_types, lSize+3, l );
    }
    stapl::rmi_fence();

    // Test complex aggregate types
    if( _id % 2 == 0 ) {
      const int cSize = _id + 1;
      complexObject oC( cSize );
      for( int i=0; i<cSize; ++i ) {
	oC.l.insert_front( oD );
	oC.lP->insert_front( oL );
      }
      complexObject t13 = stapl::sync_rmi( _rt, rmiHandle, &p_test::complex_type, oC );
      stapl_assert( t13 == oC, "" );
    }

    stapl::rmi_fence();

    //test vector
    if( _id > 0 ) {
      vector<double> vb;
      vector<double>::iterator it = vb.begin();
      int i = 0;

      while(i < 100){
	vb.push_back(i);
	stapl::async_rmi(_lt,
			 rmiHandle,
			 &p_test::dvector_type,vb,i+1);	
	++i;
      }

      vb = stapl::sync_rmi(_lt,
			   rmiHandle,
			   &p_test::dvector_sync_type);
      stapl_assert(vb.size() == 100,"dvector::Inavlid size received");
      i = 0;
      while(i < vb.size()){
	stapl_assert(vb[i]==i,"dvector::Invalid value received");
	++i;
      }
    }
    stapl::rmi_fence();

    //test bitvector
    if( _id > 0 ) {
      vector<bool> vb;
      vector<bool>::iterator it = vb.begin();
      bool b = true;
      int i = 0;
      while(i < 100){
	vb.push_back(b);
	stapl::async_rmi(_lt,
			 rmiHandle,
			 &p_test::bvector_type,vb,i+1);	
	b = !b;
	++i;
      }
      vb = stapl::sync_rmi(_lt,
			   rmiHandle,
			   &p_test::bvector_sync_type);
      stapl_assert(vb.size() == 100,"bvector::Inavlid size received");
      i = 0;
      b = true;
      while(i < vb.size()){
	stapl_assert(vb[i]==b,"bvector::Invalid value received");
	b = !b;
	++i;
      }
    }
    stapl::rmi_fence();
  }
};


void stapl_main(int argc, char *argv[]) {
  p_test pt;
  stapl::execute_parallel_task( &pt );
#ifndef _TEST_QUIET
  printf( "%d successfully passed!\n", stapl::get_thread_id() );
#endif
}
