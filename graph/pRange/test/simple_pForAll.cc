#include <iostream>
#include <runtime.h>
#include <pRange.h>
#include <pvector.h>

using std::cout;
using std::endl;

typedef stapl::pvector<int> pvectorType;
typedef stapl::linear_boundary<pvectorType::iterator> linear_boundaryType;
typedef stapl::pRange<linear_boundaryType, pvectorType> prangeType;

class work_func {
private:
  int x;
public:
  void operator()(prangeType::subrangeType& pr) {
    for (prangeType::iteratorType i  = pr.get_boundary().start();
                                  i != pr.get_boundary().finish();
                                ++i) {
      *i += 1;
    }
  }
};

void stapl_main(int argc, char **argv) {

  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();
  if (id == 0)
    cout << "Testing simple executor call with " << nThreads << " threads... ";

  pvectorType vec(100);
  prangeType  pr(&vec, linear_boundaryType(vec.local_begin(), vec.local_end()));
  stapl::pDDG<stapl::DDGTask, stapl::DDGWeight>* ddg = new stapl::pDDG<stapl::DDGTask, stapl::DDGWeight>();
  stapl::DDGTask vert(pr.get_subranges().begin()->get_global_id());
  ddg->AddVertex(vert, pr.get_subranges().begin()->get_global_id(), id);
  ddg->initialize_base_num_predecessors();
  ddg->initialize_predecessors_left();
  ddg->reset();
  pr.set_ddg(ddg);


  stapl::defaultScheduler sched;
  work_func func;
  p_for_all(pr, func, sched);

  if (id == 0)
    cout << "Passed\n";
}
