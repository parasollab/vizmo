#ifndef _STAPL_RUNTIME_EXECUTOR
#define _STAPL_RUNTIME_EXECUTOR

#include <list>
#include <queue>
#include <vector>
#include <iostream>

namespace stapl {

  class defaultScheduler {
  private:
    std::queue<int> prange_ids;
  public:
    void push(std::vector<int>& ids) { 
      for (std::vector<int>::iterator i = ids.begin(); 
           i != ids.end(); ++i) {
        prange_ids.push(*i);
      }
    }
    
    std::vector<int> pop() {
      int id = prange_ids.front();
      prange_ids.pop();
      std::vector<int> ready(1,id);
      return ready;
    }
    
    bool empty() { return prange_ids.empty(); }
    bool global_empty() { return prange_ids.empty(); }
  };
  


  
  template <class pRange, class Function, class pScheduler>
  void p_for_all(pRange& pr, 
                 Function& func, 
                 pScheduler& sched) {

    std::vector<int> ready_ids;
    while ( (!pr.get_ddg().finished()) || (!sched.global_empty()) ) {
      ready_ids = pr.get_ddg().get_ready();
      sched.push(ready_ids);
      
      if (!sched.empty()) {
        std::vector<int> subrange_ids = sched.pop();
        for (std::vector<int>::iterator i = subrange_ids.begin(); 
             i != subrange_ids.end(); ++i) {
          typename pRange::subrangeType& subrange = pr.get_subrange(*i);
          func(subrange);
          pr.get_ddg().processed(*i);
	  stapl::rmi_flush();
        }
      }
      stapl::rmi_poll();
    } 
    pr.get_ddg().reset();
    stapl::rmi_fence();
  }
  

  template <class pRange1, class pRange2, class Function, class pScheduler>
  void p_for_all(pRange1& pr1,
		 pRange2& pr2,
                 Function& func, 
                 pScheduler& sched) {

    std::vector<int> ready_ids;

    while ( (!pr1.get_ddg().finished()) || (!sched.global_empty()) ) {
      ready_ids = pr1.get_ddg().get_ready();
      sched.push(ready_ids);
      
      if (!sched.empty()) {
        std::vector<int> subrange_ids = sched.pop();
        for (std::vector<int>::iterator i = subrange_ids.begin(); 
             i != subrange_ids.end(); ++i) {
          typename pRange1::subrangeType& subrange1 = pr1.get_subrange(*i);
	  typename pRange2::subrangeType& subrange2 = pr2.get_subrange(*i);
          func(subrange1,subrange2);
          pr1.get_ddg().processed(*i);
	  stapl::rmi_flush();
        }
      }
      stapl::rmi_poll();
    } 
    pr1.get_ddg().reset();
    stapl::rmi_fence();
  }

  
  template <class pRange, class Function, class pScheduler>
    void p_for_all(std::vector<pRange>& pranges, 
                   std::vector<Function>& funcs, 
                   pScheduler& sched) {

    int numPranges = pranges.end() - pranges.begin();
    int donePrangeCount = 0;
    
    typename std::vector<Function>::iterator func = funcs.begin();
    typename std::vector<pRange>::iterator pr = pranges.begin();
    typename std::vector<bool> donePranges(numPranges, false);
    typename std::vector<pScheduler> scheds(numPranges, sched);

    while(donePrangeCount < numPranges) {
      for (int pr = 0; pr < pranges.size(); ++pr) {
        pRange &prange = pranges[pr];
        pScheduler &scheduler = scheds[pr];

        if (!donePranges[pr]) {
          stapl::rmi_poll();
          std::vector<int> ready_ids;
          ready_ids = prange.get_ddg().get_ready();

          while ((ready_ids.size() != 0) || (!scheduler.empty())) {
            scheduler.push(ready_ids);

            if (!scheduler.empty()) {
              std::vector<int> subrange_ids = scheduler.pop();
              for (std::vector<int>::iterator i = subrange_ids.begin();
                   i != subrange_ids.end(); ++i) {
                typename pRange::subrangeType& subrange = prange.get_subrange(*i);
                (funcs[pr])(subrange);
                 prange.get_ddg().processed(*i);
		 stapl::rmi_flush();
              }
            }
            ready_ids = prange.get_ddg().get_ready();

          }
          if ( prange.get_ddg().finished() && scheduler.global_empty() ) {
            donePrangeCount++;
            donePranges[pr] = true;
          }
        }
      }
    }
    for(typename std::vector<pRange>::iterator it= pranges.begin(); it != pranges.end();++it){
      it->get_ddg().reset();
    }
    stapl::rmi_fence();
  }

} //end namespace stapl
#endif // _STAPL_RUNTIME_EXECUTOR
