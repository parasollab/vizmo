#ifndef SIMPLEPOINTERJUMPING_H
#define SIMPLEPOINTERJUMPING_H

/**
 * \file SimplePointerJumping.h
 * \author Tao Huang
 * \date 01/19/2005
 * \*ingroup stapl
 * \brief Simple Pointer Jumping pAlgorithm.
 *
 * Applies Wyllie's Algorithm (called as "Pointer jumping" or 
 * "Short Cutting") on the given pRange;
 * The computation on the incoming value and own value during
 * each iteration is specified by the input workfunction TODOFunc. 
 */

#include <runtime.h>
#include <rmitools.h>
#include <BasePart.h>
#include <BasePContainer.h>

namespace stapl {

//the computation direction
//Fix me!
// 
//There should be a method in pRange to decide whether it is 
// a Forwarding pRange or reverse. 
//Since there is no such way now, we need a boolean "direction"
// to specifically show the direction of computation, which is
// actually the same as the direction (normal pRange or reverse)
// of the input pRange for the pAlgorithm.

#ifndef Forward	
#define Forward true
#endif

#ifndef Backward
#define Backward false
#endif

/* class for communication, used in Simple Pointer Jumping */
template<class ELEMENT>
class comm_simplepointerjumping: public BasePObject {
    vector<ELEMENT>* tmpelets; 
    vector<Location>* newprevs;
    vector<Location>* newnexts;

    /* next component tells me to update my next link
     * store the received link temporarily */
    inline void GetFromNext(PARTID mypartid, Location newnext) {
      (*newnexts)[mypartid] = newnext;
    }

    /* previous component tells me to update my value and my prev link
     * store the received value and link temporarily */
    inline void GetValueFromPrev_triple(_StaplTriple<PARTID, Location, ELEMENT> _arg) {
      PARTID   mypartid = _arg.first;
      Location newprev  = _arg.second;
      ELEMENT  tmpelet     = _arg.third;
      (*tmpelets)[mypartid] = tmpelet;
      (*newprevs)[mypartid] = newprev;
    }
  public:
    comm_simplepointerjumping(vector<ELEMENT>& _tmpelets, vector<Location>& _newprevs, vector<Location>& _newnexts) {
      tmpelets = &_tmpelets;
      newprevs = &_newprevs;
      newnexts = &_newnexts;
      register_this(this);
    }

    ~comm_simplepointerjumping() {
      rmiHandle handle = this->getHandle();
      if (handle > -1)
        stapl::unregister_rmi_object(handle);
    }

    /* send my value to next component */
    inline void TellValueToNext(Location next, Location prev, ELEMENT myelet) {
      PARTID partid = next.partid();
      _StaplTriple<PARTID, Location, ELEMENT> _arg(partid, prev, myelet);
      stapl::async_rmi(next.locpid(), this->getHandle(), &comm_simplepointerjumping::GetValueFromPrev_triple, _arg);
      return;
    }
      
    /* send my next link to previous component */
    inline void TellPrev(Location& prev, Location& next) {
      stapl::async_rmi(prev.locpid(), this->getHandle(), &comm_simplepointerjumping::GetFromNext, prev.partid(), next);
      return;
    }

    inline void AddFlags(bool* in, bool* inout) {
            *inout = (*inout) & (*in);
    } 
        
    /* Pointer Jumping stops only if all thread has flag as TRUE */
    inline bool AllDone(bool myflag) {
      bool ret;
      reduce_rmi(&myflag, &ret, this->getHandle(), &comm_simplepointerjumping::AddFlags, true);
     return ret;
    }
};

/** 
 * SimplePointerJumping pAlgorithm.
 * @param prx a pRange or a LinearOrderPContainer or a pArray which is required 
 * to have only 1 element per subpContainer;
 * @param todofunc a WorkFunction which specifies the operation applied on the 
 * elements during pointer jumping iterations;
 * @param direction a boolean which specifies the computation direction (which is 
 * actually same as the direction of prx: normal pRange or reverse pRange);
 * @param pure a boolean. If want the computation result in each subpRange comes 
 * only from its preceding subpRanges, set this argument as true, otherwise the 
 * result will comes from the preceding subpRanges including itself.
 * @return void.
 */
template <class PRANGE, class TODO>
void SimplePointerJumping(PRANGE& prx, TODO& todofunc, bool direction=Forward, bool pure=false) {
    typedef typename PRANGE::ContainerType::value_type ELEMENT;
    int nprocs = stapl::get_thread_id();
    bool done=false, flag;
    vector<typename PRANGE::subrangeType> subprxv = prx.get_subranges();
    int npart = subprxv.size();
    vector<ELEMENT> tmpelets(npart), oldvalues(npart);
    vector<Location> oldprevs, oldnexts, newprevs, newnexts;
    Location invloc(-1, INVALID_PART);
    comm_simplepointerjumping<ELEMENT> result(tmpelets, newprevs, newnexts);
    typename PRANGE::iteratorType pxit;

    for (PARTID partid=0; partid<npart; partid++) {
      if (direction == Forward) {
	oldprevs.push_back(prx.GetPreviouspRange(partid) );
	oldnexts.push_back(prx.GetNextpRange(partid) );
      } else {
	oldprevs.push_back(prx.GetNextpRange(partid) );
	oldnexts.push_back(prx.GetPreviouspRange(partid) );
      }
      //cout<<"#("<<get_thread_id()<<","<<partid<<") origp=("<<oldprevs[partid].locpid()<<","<<oldprevs[partid].partid()<<"), orign=("<<oldnexts[partid].locpid()<<","<<oldnexts[partid].partid()<<")"<<" oldvalue="<<*(subprxv[partid].get_boundary().start())<<endl;
      oldvalues[partid] = *(subprxv[partid].get_boundary().start() );
      pxit = subprxv[partid].get_boundary().start();
      *pxit = todofunc.identity() ;
      newprevs.push_back(invloc); newnexts.push_back(invloc);
    }

    stapl::rmi_fence();

    int iter=0;
    while (!done) {
      done = true; flag = true;
      stapl::rmi_fence();
      
      //send
      PARTID partid = 0;
      for (PARTID partid=0; partid<npart; partid++) {
        pxit = subprxv[partid].get_boundary().start();
	if (oldnexts[partid].locpid() != INVALID_PID &&  oldnexts[partid].partid() != INVALID_PART) {
	  //cout<<"iter="<<iter<<" #("<<stapl::get_thread_id()<<","<<partid<<") Tell value todofunc("<<(*pxit)<<","<<oldvalues[partid]<<")="<<todofunc( (*pxit), oldvalues[partid])<<" and Prevloc ("<<oldprevs[partid].locpid()<<","<<oldprevs[partid].partid()<<") to Next("<<oldnexts[partid].locpid()<<","<<oldnexts[partid].partid()<<")"<<endl;
	  result.TellValueToNext(oldnexts[partid], oldprevs[partid], todofunc( (*pxit), oldvalues[partid]) );
	  flag = false;
	}

	if (pure==true) { if (iter==0) *pxit = todofunc.identity(); }

	if (oldprevs[partid].locpid() != INVALID_PID && oldprevs[partid].partid()!= INVALID_PART) {
	  //cout<<"#("<<stapl::get_thread_id()<<","<<partid<<") Tell Nextloc("<<oldnexts[partid].locpid()<<","<<oldnexts[partid].partid()<<") to Prev("<<oldprevs[partid].locpid()<<","<<oldprevs[partid].partid()<<")"<<endl;
	  result.TellPrev(oldprevs[partid], oldnexts[partid]);
	  flag = false;
	} else tmpelets[partid] = todofunc.identity();

      } //end for...send

      stapl::rmi_fence();

      //after receive
      for (PARTID partid=0; partid<npart; partid++) {
        pxit = subprxv[partid].get_boundary().start();
	if (oldprevs[partid].locpid() != INVALID_PID && oldprevs[partid].partid()!= INVALID_PART) {
	  oldprevs[partid] = newprevs[partid];
	  //cout<<"#"<<get_thread_id()<<" in iter "<<iter<<" part "<<partid<<" incomingvalue="<<tmpelets[partid]<<", myoldvalue="<<(*pxit)<<endl;
 	  *pxit = todofunc(tmpelets[partid], (*pxit) );
 	} 
	if (oldnexts[partid].locpid() != INVALID_PID &&  oldnexts[partid].partid() != INVALID_PART) {
	  oldnexts[partid] = newnexts[partid];
	}
      }

      stapl::rmi_fence();

      //decide whether to stop
      done = result.AllDone(flag); 
      iter++; 
      stapl::rmi_fence();
    } //end while
}

}//end namespace

#endif
