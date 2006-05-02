#include <stdlib.h>
#include <iostream>
#include <functional>

#include <pArray.h>
#include <pRange.h>
#include <pAlgorithm.h>
#include <runtime.h>

using namespace std;
using namespace stapl;


//Functor to initialize a container
//The arguments to the constructor are the start value 
//  and the step size of the sequence.
template<class T> class Sequence {
private:
  T counter, oldCtr, step;
public:
  Sequence(T start = 0, T st = 1) : counter(start), step(st) { }
  T operator()() {
    oldCtr = counter;
    counter += step;
    return oldCtr;
  }
};


//Functor to print an element
//The argument to the constructor is the stream the element will be written to.
template<class T> struct print : public unary_function<T, void>
{
  print(ostream& out) : os(out), count(0) {}
  void operator() (T x) { os << x << ' '; ++count; }
  ostream& os;
  int count;
};


//Functor to negate an element.
//The work function has the name my_negate since the standard defines a 
//Negate<T> work function itself.
template<class T> struct my_negate : public unary_function<T, void>
{
  my_negate() {}
  void operator() (T& x) {  x *= -1; }
};


//Functor to sum two elements together and store the result of the sum
//in the second element.
template<class T> struct sum : public binary_function<T, T, void>
{
  sum() {}
  void operator() (T& x, T& y) { y += x; }
};


//Functor to sum two elements together and store the result of the sum in
//a thrid element
template<class T> struct sum2 
{
  sum2() {}
  void operator() (T& x, T& y, T& z) { z = x + y; }
};

void stapl_main(int argc, char **argv) {

  typedef pArray<int> pcontainer;
  typedef stapl::pRange<linear_boundary<pcontainer::iterator>, pcontainer> prange;

  //The number of elements in each container is passed in from the command line.
  int nElems = atoi( argv[1] );
  int nThreads = stapl::get_num_threads();
  int threadID = stapl::get_thread_id();

  if (threadID == 0)
    cout << "Testing pRange with p_for_each on " << nThreads << " threads...";

  //Declare the pContainers.
  //In nElems isn't evenly divisible by the number of threads then the 
  //part of the pContainer on thread p - 1 will contain the extra elements.
  pcontainer a(nElems);
  pcontainer b(nElems);
  pcontainer c(nElems);
  pcontainer d(nElems);
  pcontainer e(nElems);

  //Declare a pRange on each pContainer.  
  //Using the local_begin and local_end iterators to construct the 
  //linear_boundary produces one leaf pRange per thread that 
  //encapsulates all the elements in a container on the thread.

  prange a_pr(&a, linear_boundary<pcontainer::iterator>(a.local_begin(), 
                                                        a.local_end()));

  prange b_pr(&b, linear_boundary<pcontainer::iterator>(b.local_begin(), 
                                                        b.local_end()));

  prange c_pr(&c, linear_boundary<pcontainer::iterator>(c.local_begin(), 
                                                        c.local_end()));

  prange d_pr(&d, linear_boundary<pcontainer::iterator>(d.local_begin(), 
                                                        d.local_end()));

  prange e_pr(&e, linear_boundary<pcontainer::iterator>(e.local_begin(), 
                                                        e.local_end()));

  rmi_fence();

  int elements_per_thread = nElems / nThreads;

  //Initialize the elements of the first pContainer
  //to the values from 0 .. nElems - 1
  p_generate(a_pr, Sequence<int>(elements_per_thread * threadID, 1));

  //Initialize the elements of the second pContainer
  //to the values from 0 .. -1 * (nElems - 1)
  p_generate(b_pr, Sequence<int>(-1 * elements_per_thread * threadID, -1));

  //Initialize the elements of the thrid pContainer to 0
  p_generate(c_pr, Sequence<int>(0,0));

  //Copy the elements of A to D
  p_copy(a_pr, d_pr);

  //Nefate the elements of D using the work function above.
  p_for_each(d_pr, my_negate<int>());

  //D now contains consecutive negative integers and 0 beginning with 0.
  //This is what B contains, compare the two to verify.
  bool passed1 = p_equal(b_pr, d_pr);
  if (!passed1)
    cout << get_thread_id() << ": Error in p_for_each on single pRange: ";

  //Sum the elements of A and D, storing the result in D.
  sum<int> sum_func;
  p_for_each(a_pr, d_pr, sum_func);

  //D now contains all zeros.
  //This is what C contains, compare the two to verify.
  bool passed2 = p_equal(c_pr, d_pr);
  if (!passed2)
    cout << get_thread_id() << ": Error in p_for_each on two pRanges: ";

  //Sum the values of A and B using the work function above.
  //The results are stored in E
  p_for_each(a_pr, b_pr, e_pr, sum2<int>());

  //E now contains all zeros.
  //This is what C has, compare the two to verify.
  bool passed3 = p_equal(c_pr, e_pr);
  if (!passed3)
    cout << get_thread_id() << ": Error in p_for_each on three pRanges: ";

  if ((threadID == 0) && (passed1 && passed2 && passed3))
    cout << "Passed\n";
  else if (threadID == 0)
    cout << "Failed\n";

  rmi_fence();
}
