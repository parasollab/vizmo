#ifndef LISTRANKING_H
#define LISTRANKING_H
#include <runtime.h>
#include <rmitools.h>
#include "pList.h"

namespace stapl {

  template<class T>
    class ListRanking : public BasePObject {

    typedef typename pList<T>::list_piterator NextIter;

    //assume only one part, if multiple parts, use vector<NextIter>
    //some trivial changes in the functions
    T nextvalue;
    PID  nextpid;
    bool done;

    public:
    ListRanking() : done(false) {
      this->register_this(this);
      rmi_fence();
    }

    ~ListRanking() {}

    void define_type(stapl::typer &t) 
      {
	t.local(nextvalue);
	t.local(nextpid);
      }

    void Ranking(pList<T>& myplist) {

      //initial local ranking
      InitLocalRanking(myplist);

#ifdef STAPL_DEBUG
      myplist.DisplayPContainer();
#endif

      //set the nextpid of last part to -1
      NextIter globalend;
      globalend = myplist.end();
      nextpid = get_thread_id();
      if( nextpid ==  globalend.base_pid() ) nextpid = -1;
      stapl::rmi_fence();

      //initial nextpid to point to the thread of next part
      NextIter it(&myplist);
      it = myplist.local_end();
      it.set_uselocaliterator(false);   //switch to global iter
      ++it;
      if(nextpid != -1) nextpid = it.base_pid();
      stapl::rmi_fence();

      UpdateRank(myplist);

#ifdef STAPL_DEBUG
      myplist.DisplayPContainer();
#endif

      while(!done) {
	PtrJumping();
	UpdateRank(myplist);
      }
      stapl::rmi_fence();
    }

    void UpdateRank(pList<T>& myplist) {
      if(nextpid != -1) {
	nextvalue = GetNextValue();
	NextIter it(&myplist);
	for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	  *it += nextvalue;   
	}
      }
      else 
	done = true;
    }

    T GetNextValue() {
      T tmp = sync_rmi(nextpid, getHandle(), 
		       &ListRanking<T>::GetValue);
      rmi_poll();  //make sure everyone received the old nextvalue
      nextvalue = tmp;
    }

    void PtrJumping() {
     PID tmppid = sync_rmi(nextpid, getHandle(), 
				  &ListRanking<T>::GetNextPid);
     rmi_poll();  //make sure everyone received the old nextvalue
     nextpid = tmppid;
    }

    protected:

    T GetValue() {
      return nextvalue;
    }

    PID GetNextPid() {
      return nextpid;
    }

    void InitLocalRanking(pList<T>& myplist) {
      int dist = 0;
      //operator-- needs to be implemented
      NextIter it(&myplist);
      for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	*it += dist;
	dist = *it;
      }
      for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	*it = dist - *it + 1;
      }
      nextvalue = dist;
      cout<<"dist:"<<dist<<endl;
    }

  };

} //end namespace

#endif
