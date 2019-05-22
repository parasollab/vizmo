#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include <cstdlib>
#include <vector>

using namespace std;

//#define OLD

#ifdef OLD
#include "Point.h"
#include "Vector.h"
#define D_type int
#else
#include "Vector2.h"
#define D_type size_t
#endif

using namespace mathtool;

//Easy timing class for tests
class Clock {
  public:
    Clock(){}

    void Start(){
      gettimeofday(&m_start, NULL);
    }

    void Stop(){
      gettimeofday(&m_end, NULL);
    }

    double Time() const {
      return (m_end.tv_sec - m_start.tv_sec) * 1000000 +
        (m_end.tv_usec - m_start.tv_usec);
    }

    friend ostream& operator<<(ostream& _os, const Clock& _c);

  private:
    timeval m_start, m_end;
};

ostream& operator<<(ostream& _os, const Clock& _c) {
  return _os << setprecision(10) << _c.Time();
}

//Generic Test function
template<class F>
double
Test(size_t _iter){
  Clock c;
  c.Start();

  F()(_iter);

  c.Stop();

  return c.Time();
}

//Random vector/point generation
template<typename T, D_type D, template<typename, D_type> class V>
V<T, D> RandV(){
  V<T, D> v;
  for(size_t i = 0; i<D; ++i)
    v[i] = (T) rand()/RAND_MAX;
  return v;
}

//random functor
template<typename T, D_type D, template<typename, D_type> class V>
struct Rand {
  void operator()(size_t _iter){
    for(size_t i = 0; i<_iter; ++i)
      RandV<T, D, V>();
  }
};

//assign functor
template<typename T, D_type D, template<typename, D_type> class V>
struct Assign {
  void operator()(size_t _iter){
    V<T, D> copy;
    for(size_t i = 0; i<_iter; ++i)
      copy = RandV<T, D, V>();
    cerr << copy << endl;
  }
};

//equality functor
template<typename T, D_type D, template<typename, D_type> class V>
struct Equality {
  void operator()(size_t _iter){
    for(size_t i = 0; i<_iter; ++i)
      if(RandV<T, D, V>() == RandV<T, D, V>())
        cerr << "They are equal!" << endl;
  }
};

//+= functor
template<typename T, D_type D, template<typename, D_type> class V>
struct PlusEq {
  void operator()(size_t _iter){
    V<T, D> sum;
    for(size_t i = 0; i<_iter; ++i)
      sum += RandV<T, D, V>();
    cerr << sum << endl;
  }
};

#ifdef OLD
template<typename T, D_type D>
struct PlusEq<T, D, Point> {
  void operator()(size_t _iter){
    Point<T, D> sum;
    for(int i = 0; i<_iter; ++i){
      Point<T, D> v = RandV<T, D, Point>();
      for(int j = 0; j<D; ++j){
        sum[j] += v[j];
      }
    }
  }
};
#endif

//-= functor
template<typename T, D_type D, template<typename, D_type> class V>
struct MinusEq {
  void operator()(size_t _iter){
    V<T, D> sum;
    for(size_t i = 0; i<_iter; ++i)
      sum -= RandV<T, D, V>();
    cerr << sum << endl;
  }
};

#ifdef OLD
template<typename T, D_type D>
struct MinusEq<T, D, Point> {
  void operator()(size_t _iter){
    Point<T, D> sum;
    for(int i = 0; i<_iter; ++i){
      Point<T, D> v = RandV<T, D, Point>();
      for(int j = 0; j<D; ++j){
        sum[j] -= v[j];
      }
    }
  }
};
#endif

//*= functor
template<typename T, D_type D>
struct MultEq {
  void operator()(size_t _iter){
    Vector<T, D> prod = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod *= (rand()%2 ? 1 : 0) + (double) rand()/RAND_MAX;
    cerr << prod << endl;
  }
};

///= functor
template<typename T, D_type D>
struct DivideEq {
  void operator()(size_t _iter){
    Vector<T, D> prod = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod /= (rand()%2 ? 1 : 0) + (double) rand()/RAND_MAX;
    cerr << prod << endl;
  }
};

//^= functor
template<typename T, D_type D>
struct MultElemEq {
  void operator()(size_t _iter){
#ifndef OLD
    Vector<T, D> copy = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      copy ^= RandV<T, D, Vector>();
    cerr << copy << endl;
#endif
  }
};

//%= functor
template<typename T, D_type D>
struct CrossProdEq {
  void operator()(size_t _iter){
    return;
  }
};

template<typename T>
struct CrossProdEq<T, 3> {
  void operator()(size_t _iter){
#ifndef OLD
    Vector<T, 3> copy = RandV<T, 3, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      copy %= RandV<T, 3, Vector>();
    cerr << copy << endl;
#endif
  }
};

//+ functor
template<typename T, D_type D, template<typename, D_type> class V>
struct Plus {
  void operator()(size_t _iter){
    Vector<T, D> copy = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      copy = copy + RandV<T, D, Vector>();
    cerr << copy << endl;
  }
};

//- functor
template<typename T, D_type D, template<typename, D_type> class V>
struct Minus {
  void operator()(size_t _iter){
    Vector<T, D> copy;
    for(size_t i = 0; i<_iter; ++i)
      copy = RandV<T, D, V>() - RandV<T, D, V>();
    cerr << copy << endl;
  }
};

//* functor
template<typename T, D_type D>
struct Mult {
  void operator()(size_t _iter){
    Vector<T, D> prod = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod = prod * ((rand()%2 ? 1 : 0) + (double) rand()/RAND_MAX);
    cerr << prod << endl;
  }
};

/// functor
template<typename T, D_type D>
struct Divide {
  void operator()(size_t _iter){
    Vector<T, D> prod = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod = prod / ((rand()%2 ? 1 : 0) + (double) rand()/RAND_MAX);
    cerr << prod << endl;
  }
};

//^ functor
template<typename T, D_type D>
struct MultElem {
  void operator()(size_t _iter){
    Vector<T, D> prod = RandV<T, D, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod = prod ^ RandV<T, D, Vector>();
    cerr << prod << endl;
  }
};

//% functor
template<typename T, D_type D>
struct CrossProd {
  void operator()(size_t _iter){
    return;
  }
};

template<typename T>
struct CrossProd<T, 2> {
  void operator()(size_t _iter){
#ifndef OLD
    T copy;
    Vector<T, 2> prod = RandV<T, 2, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      copy = prod % RandV<T, 2, Vector>();
    cerr << copy << endl;
#endif
  }
};

template<typename T>
struct CrossProd<T, 3> {
  void operator()(size_t _iter){
    Vector<T, 3> prod = RandV<T, 3, Vector>();
    for(size_t i = 0; i<_iter; ++i)
      prod = prod % RandV<T, 3, Vector>();
    cerr << prod << endl;
  }
};

//dot functor
template<typename T, D_type D>
struct DotProd {
  void operator()(size_t _iter){
    T dot;
    for(size_t i = 0; i<_iter; ++i)
      dot = RandV<T, D, Vector>() * RandV<T, D, Vector>();
    cerr << dot << endl;
  }
};

//norm functor
template<typename T, D_type D>
struct Norm {
  void operator()(size_t _iter){
    for(size_t i = 0; i<_iter; ++i)
      RandV<T, D, Vector>().norm();
  }
};

//normsqr functor
template<typename T, D_type D>
struct NormSqr {
  void operator()(size_t _iter){
    for(size_t i = 0; i<_iter; ++i)
      RandV<T, D, Vector>().normsqr();
  }
};

//normalize functor
template<typename T, D_type D>
struct Normalize {
  void operator()(size_t _iter){
    for(size_t i = 0; i<_iter; ++i)
      RandV<T, D, Vector>().normalize();
  }
};

#ifdef OLD
//point has operations = == + -
//will also write code for += and -=
template<typename T, D_type D>
void
TestPoint(size_t _iter){

  Clock tot;
  tot.Start();

  cout
    << setw(10) << "Test"
    << setw(10) << "="
    << setw(10) << "=="
    << setw(10) << "+"
    << setw(10) << "-"
    << setw(10) << "+="
    << setw(10) << "-="
    << endl;

  //test rand
  double tr = Test<Rand<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tr;

  //test =
  double tass = Test<Assign<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tass - tr;

  //test ==
  double teq = Test<Equality<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << teq - 2*tr;

  //test +
  double tp = Test<Plus<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tp - 2*tr;

  //test -
  double tm = Test<Minus<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tm - 2*tr;

  //test +=
  double tpe = Test<PlusEq<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tpe - tr;

  //test -=
  double tme = Test<MinusEq<T, D, Point> >(_iter);
  cout << setw(10) << setprecision(8) << tme - tr;

  cout << endl
    << setw(10) << "total"
    << endl;

  tot.Stop();
  double total = tot.Time();
  cout << setw(10) << setprecision(8) << total;

  cout << endl;
}
#endif

//vector has operations norm normalize = == + - * / += -= *= /= ^ * %
template<typename T, D_type D>
void
TestVector(size_t _iter){

  Clock tot;
  tot.Start();

  cout
    << setw(10) << "rand"
    << setw(10) << "="
    << setw(10) << "=="
    << endl;

  //test rand
  double tr = Test<Rand<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << tr;

  //test =
  double tass = Test<Assign<T, D, Vector> >(_iter) - tr;
  cout << setw(10) << setprecision(8) << tass;

  //test ==
  double teq = Test<Equality<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << teq - 2*tr;

  cout << endl
    << setw(10) << "+="
    << setw(10) << "-="
    << setw(10) << "*="
    << setw(10) << "/="
    << setw(10) << "^="
    << setw(10) << "%="
    << endl;

  //test +=
  double tpe = Test<PlusEq<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << tpe - tr;

  //test -=
  double tme = Test<MinusEq<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << tme - tr;

  //test *=
  double tmue = Test<MultEq<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tmue - tr/D;

  //test /=
  double tde = Test<DivideEq<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tde - tr/D;

  //test ^=
  double tmuele = Test<MultElemEq<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tmuele - tr;

  //test %=
  double tcpe = Test<CrossProdEq<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tcpe - tr;

  /*
  cout << endl
    << setw(10) << "+"
    << setw(10) << "-"
    << setw(10) << "*"
    << setw(10) << "/"
    << setw(10) << "^"
    << setw(10) << "%"
    << endl;

  //test +
  double tp = Test<Plus<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << tp - tr;

  //test -
  double tm = Test<Minus<T, D, Vector> >(_iter);
  cout << setw(10) << setprecision(8) << tm - 2*tr;

  //test *
  double tmu = Test<Mult<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tmu - tr/D;

  //test /
  double td = Test<Divide<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << td - tr/D;

  //test ^
  double tmuel = Test<MultElem<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tmuel - tr;

  //test %
  double tcp = Test<CrossProd<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tcp - tr;
*/

  cout << endl
    << setw(10) << "dot"
    << setw(10) << "norm"
    << setw(10) << "normsqr"
    << setw(10) << "normalize"
    << endl;

  //test *
  double tdp = Test<DotProd<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tdp - 2*tr;

  //test norm
  double tn = Test<Norm<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tn - tr;

  //test normsqr
  double tns = Test<Norm<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tns - tr;

  //test normalize
  double tnz = Test<Normalize<T, D> >(_iter);
  cout << setw(10) << setprecision(8) << tnz - tr;

  cout << endl
    << setw(10) << "total"
    << endl;

  tot.Stop();
  double total = tot.Time();
  cout << setw(10) << setprecision(8) << total;

  cout << endl;
}

int main(int argc, char** argv) {
  if(argc != 2){
    cerr << "Test takes 1 argument, the number of test iterations." << endl;
    return 1;
  }

  size_t iter = atoi(argv[1]);

#ifdef OLD
  //test point2f
  cout << setw(10) << "P<f, 2>" << endl;
  TestPoint<float, 2>(iter);

  //test point3f
  cout << setw(10) << "P<f, 3>" << endl;
  TestPoint<float, 3>(iter);

  //test point10f
  cout << setw(10) << "P<f, 10>" << endl;
  TestPoint<float, 10>(iter);

  //test point2d
  cout << setw(10) << "P<d, 2>" << endl;
  TestPoint<double, 2>(iter);

  //test point3d
  cout << setw(10) << "P<d, 3>" << endl;
  TestPoint<double, 3>(iter);

  //test point10d
  cout << setw(10) << "P<d, 10>" << endl;
  TestPoint<double, 10>(iter);
#endif

  //test vector2f
  cout << setw(10) << "V<f, 2>" << endl;
  TestVector<float, 2>(iter);

  //test vector3f
  cout << setw(10) << "V<f, 3>" << endl;
  TestVector<float, 3>(iter);

  //test vector10f
  cout << setw(10) << "V<f, 10>" << endl;
  TestVector<float, 10>(iter);

  //test vector2d
  cout << setw(10) << "V<d, 2>" << endl;
  TestVector<double, 2>(iter);

  //test vector3d
  cout << setw(10) << "V<d, 3>" << endl;
  TestVector<double, 3>(iter);

  //test vector10d
  cout << setw(10) << "V<d, 10>" << endl;
  TestVector<double, 10>(iter);
}
