#ifndef LISTRANKING_H
#define LISTRANKING_H
#include <runtime.h>
#include <rmitools.h>
#include "pList.h"

namespace stapl {
  template <class PCONT>
  class localranking_prange : public BasePObject{ //work function

    typedef stapl::linear_boundary<typename PCONT::iterator> linear_boundary;
    typedef stapl::pRange<linear_boundary, PCONT> prange;
    deque<int>* accumv;
    rmiHandle handle;

  public: 

    localranking_prange(deque<int>& _accumv ) {
        accumv = &_accumv;
	if (handle==-1)
        handle = stapl::register_rmi_object(this);
    }

    ~localranking_prange() {
	handle = this->getHandle();
	if (handle > -1)
    	  stapl::unregister_rmi_object(handle);
    }

    int operator()(prange& pr) {
	int  counter = 0;
	for (typename prange::iteratorType it = pr.get_boundary().start(); it!= pr.get_boundary().finish(); ++it) {
  	  (*it).SetRank(counter++);
	}
	for (typename prange::iteratorType it = pr.get_boundary().start(); it!= pr.get_boundary().finish(); ++it) {
	  (*it).SetRank(counter-1-((*it).GetRank())); //correct local rank
	}
	accumv->push_back(counter); //#elets = accumulation
    }
  };

  template<class PCONT>
  class updaterank_prange : public BasePObject{ //work function

    typedef stapl::linear_boundary<typename PCONT::iterator> linear_boundary;
    typedef stapl::pRange<linear_boundary, PCONT> prange;
    deque<int> tempv;
    rmiHandle handle;

  public: 

    updaterank_prange(deque<int>& _tempv) {
	tempv = _tempv;
	if (handle==-1)
        handle = stapl::register_rmi_object(this);
    }

    ~updaterank_prange() {
	handle = this->getHandle();
	if (handle > -1) 
  	stapl::unregister_rmi_object(handle);
    }

    int operator()(prange& pr) {
	int i=0;
	int temp = tempv.front();
	tempv.pop_front();
	for(typename prange::iteratorType it = pr.get_boundary().start(); it!= pr.get_boundary().finish(); ++it) {
	  (*it).SetRank((*it).GetRank()+temp);  
	}
   
    }
  };

  template<class PCONT>
  class comm_next_value: public BasePObject {
    PCONT* _pcont;
    vector<int>* _tmpvals;
    vector<Location>* _newprevs;
    vector<Location>* _newnexts;

    inline void GetFromNext(_StaplTriple<PARTID, Location, int> _arg) {
      PARTID _mypartid = _arg.first;
      Location _newnext = _arg.second;
      int _tmpval = _arg.third;
      (*_newnexts)[_mypartid] = _newnext;
      (*_tmpvals)[_mypartid] = _tmpval;
    }

    inline void GetFromPrev(PARTID _mypartid, Location _newprev) {
      (*_newprevs)[_mypartid] = _newprev;
    }

  public:
    comm_next_value(PCONT& pcont, vector<int>& tmpvals, vector<Location>& newprevs, vector<Location>& newnexts) {
      _pcont = &pcont;
      _tmpvals = &tmpvals;
      _newprevs = &newprevs;
      _newnexts = &newnexts;
      rmiHandle handle = this->getHandle();
      if (handle == -1)
      register_this(this);
    }

    ~comm_next_value() { 
	    /*
      _tmpvals->clear();
      _newprevs->clear();
      _newnexts->clear();
      */
      rmiHandle handle = this->getHandle();
      if (handle > -1)
	stapl::unregister_rmi_object(handle);
    }

    void TellPrev(Location& prev, Location& next, int myval) {
      _StaplTriple<PARTID, Location, int> _arg(prev.partid(), next, myval);
      stapl::async_rmi(prev.locpid(), this->getHandle(), &comm_next_value::GetFromNext, _arg);
      return;
    }

    void TellNext(Location& next, Location& prev) {
      stapl::async_rmi(next.locpid(), this->getHandle(), &comm_next_value::GetFromPrev, next.partid(), prev);
      return;
    }

    void AddFlags(bool* in, bool* inout) {*inout = (*inout) & (*in);}

    bool AllDone(bool myflag) {
      bool ret;
      reduce_rmi(&myflag, &ret, this->getHandle(), &comm_next_value::AddFlags, true);
      return ret;
    }
  };


  template<class PCONT>
  int GlobalRanking(PCONT& pcont, deque<int>* accumv, deque<int>* tempv) {
    bool done=false, flag;
    vector<int> tmpvals;
    vector<Location> newprevs,newnexts;

    for (PARTID partid=0; partid<pcont.get_num_parts(); partid++) {
      tmpvals.push_back(0);
      Location zero(-1, INVALID_PART);
      newprevs.push_back(zero);
      newnexts.push_back(zero);
    }	      

    comm_next_value<PCONT> result(pcont, tmpvals, newprevs, newnexts);

    int iter=0;

    while (!done) {
      done = true;
      flag = true;

      for (PARTID partid=0; partid<pcont.get_num_parts(); partid++) {
        Location prev = pcont.GetDistribution().LookUpBdBeginInfo(partid);
        Location next = pcont.GetDistribution().LookUpBdEndInfo(partid);
	if (prev.locpid() != INVALID_PID && prev.partid() != INVALID_PART) {
	  result.TellPrev(prev,next,(*accumv)[partid]);
	  flag = false;
	}
        if (next.locpid() != INVALID_PID && next.partid() != INVALID_PART) {
	  result.TellNext(next,prev);
	  flag = false;
	}
      } 
      stapl::rmi_fence();

      for (PARTID partid=0; partid<pcont.get_num_parts(); partid++) {
        Location prev = pcont.GetDistribution().LookUpBdBeginInfo(partid);
        Location next = pcont.GetDistribution().LookUpBdEndInfo(partid);
	if (prev.locpid() != INVALID_PID && prev.partid() != INVALID_PART) {
	  prev = newprevs[partid];
	}
        if (next.locpid() != INVALID_PID && next.partid() != INVALID_PART) {
	  next = newnexts[partid];
	  (*tempv)[partid] += tmpvals[partid];
	  (*accumv)[partid] += tmpvals[partid];
	}
	pcont.InitBoundaryInfo(partid, prev, next);
      }
      
      done = result.AllDone(flag);
      iter++;
      stapl::rmi_fence();
    } 
    stapl::rmi_fence();
  }


  template<class PCONT>
  void ListRanking(PCONT& pcont ) {
	/* pcont must have an int. field: rank, 2 methods: SetRank, GetRank */
	  //we use 3 steps, 1 and 3 use prange, 2 is a method with 
	  //pcontainer as an argument
	  //b/c prange doesn't allow fence, and prange doesn't keep
	  //boundary info., both of which are required by GlobalRanking.
    typedef stapl::linear_boundary<typename PCONT::iterator> linear_boundary;
    typedef stapl::pRange<linear_boundary, PCONT> prange;
    typedef stapl::defaultScheduler scheduler;

    deque<int> accumv;
    deque<int> tempv;
    vector<Location> orig_beforev;
    vector<Location> orig_nextv;

    for (PARTID partid=0; partid<pcont.get_num_parts(); partid++) {
	tempv.push_back(0);
        Location before = pcont.GetDistribution().LookUpBdBeginInfo(partid);
        Location next = pcont.GetDistribution().LookUpBdEndInfo(partid);
	orig_beforev.push_back(before);
	orig_nextv.push_back(next);
    }

    prange pr1(&pcont);
    pcont.get_prange(pr1);
    scheduler s1;
    localranking_prange<PCONT> lr(accumv);
    stapl::rmi_fence();
    p_for_all<prange, localranking_prange<PCONT>, scheduler> (pr1, lr, s1);
    stapl::rmi_fence();

    GlobalRanking<PCONT>(pcont, &accumv, &tempv);
    stapl::rmi_fence();
    stapl::rmi_fence();
    int i = 0;
    for(deque<int>::iterator it = tempv.begin(); it!= tempv.end(); ++it,i++) {
    } 
    stapl::rmi_fence();
    stapl::rmi_fence();

    for(PARTID partid=0; partid<pcont.get_num_parts(); partid++) {
      pcont.InitBoundaryInfo(partid,orig_beforev[partid], orig_nextv[partid]);
    }
    stapl::rmi_fence();
    stapl::rmi_fence();
    pcont.DisplayPContainer();
    stapl::rmi_fence();

    prange pr2(&pcont);
    pcont.get_prange(pr2);
    scheduler s2;
    stapl::rmi_fence();
    updaterank_prange<PCONT> up(tempv);
    stapl::rmi_fence();
    stapl::rmi_fence();
    p_for_all<prange, updaterank_prange<PCONT>, scheduler> (pr2, up, s2);
    stapl::rmi_fence();
  }

}
#endif
