// This example implements pi_inner_product().  It basically calls
// std::inner_product() for each thread's portion of the calculation, then
// uses stapl::reduce_rmi() to combine the results.  Note, two versions are
// provided.  One, simple, performs the inner_product as described.  The other
// handles the more complicated case where a given's thread's input may be
// empty, as required by STL.

#include <iostream>
#include <pArray.h>
#include <pRange.h>
#include <pAlgorithm.h>
#include <numeric>
#include <runtime.h>

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

template<class pRange1, class pRange2, class T>
struct p_inner_product : public stapl::parallel_task {
  pRange1 pr1;
  pRange2 pr2;
  T result;

  p_inner_product(pRange1& p1, pRange2& p2, T init)
    : pr1(p1), pr2(p2), result(init) {}

  struct combo {
    int valid;
    T datum;
    combo() : valid(false), datum(0) {}
    void define_type(stapl::typer& t) { t.local(valid); t.local(datum); }
  };

  void sum(combo* in, combo* inout) {
    if( in->valid )
      if( inout->valid ) { inout->datum += in->datum; }
      else { inout->valid = true; inout->datum = in->datum; }
  }

  void execute() {
    combo c, rtn;
    if (0 == pr1.get_num_subranges()) {
      if (pr1.get_boundary().start() != 
          pr1.get_boundary().finish()) {
        c.valid = true;
        if (stapl::get_thread_id() == 0) {
          c.datum = inner_product(pr1.get_boundary().start(), 
                                  pr1.get_boundary().finish(), 
                                  pr2.get_boundary().start(), 
                                  result);
        } else {
          typename pRange1::iteratorType first1 = pr1.get_boundary().start();
          typename pRange2::iteratorType first2 = pr2.get_boundary().start();
          result = (*first1) * (*first2);
          c.datum = inner_product(++first1, 
                                  pr1.get_boundary().finish(), 
                                  ++first2, 
                                  result);
        }
      }
    } else {
      vector<typename pRange1::subrangeType>& subranges = pr1.get_subranges();
      typename vector<typename pRange2::subrangeType>::iterator j = pr2.get_subranges().begin();
      for (typename vector<typename pRange1::subrangeType>::iterator i  = subranges.begin();
                                              i != subranges.end();
                                            ++i, ++j) {
        if (i->get_boundary().start() != 
            i->get_boundary().finish()) {
          c.valid = true;
          if (stapl::get_thread_id() == 0) {
            c.datum = inner_product(i->get_boundary().start(), 
                                    i->get_boundary().finish(), 
                                    j->get_boundary().start(), 
                                    result);
          } else {
            typename pRange1::iteratorType first1 = i->get_boundary().start();
            typename pRange2::iteratorType first2 = j->get_boundary().start();
            result = (*first1) * (*first2);
            c.datum = inner_product(++first1, 
                                    i->get_boundary().finish(), 
                                    ++first2, 
                                    result);
          }
        }
      }
    }
    stapl::reduce_rmi( &c, &rtn, rmiHandle, &p_inner_product::sum, true );
    result = rtn.datum;
  }
};

void stapl_main(int argc, char *argv[]) {
  typedef stapl::pArray<double> pcontainerType;
  typedef stapl::linear_boundary<pcontainerType::iterator> boundaryType;
  typedef stapl::pRange<boundaryType, pcontainerType> prangeType;


  int id     = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if (id == 0) {
    cout << "Testing inner_product with " << nThreads << " threads... ";
    if (argc <= 1) {
      cout << "Failed:  Usage: " << argv[0] << " [numints]\n";
      return;
    }
  }
  int nElems = atoi(argv[1]);
  pcontainerType input(nElems);

  prangeType pr(&input, boundaryType(input.local_begin(), input.local_end()));
  prangeType pr2(&input, boundaryType(input.local_begin(), input.local_end()));

  int start = id * (nElems/nThreads) + 1;
  p_generate(pr, Sequence<double>(start,1));

  p_inner_product<prangeType,prangeType,double> pinner(pr, pr2, 0);
  stapl::execute_parallel_task( &pinner );

  if (id == 0) {
    double ans = nElems * (nElems + 1) * (2 * nElems + 1) / 6;
    if (pinner.result == ans)
      cout << "Passed\n";
    else
      cout << "Failed: " << pinner.result << " != " << ans << endl;
  }
}
