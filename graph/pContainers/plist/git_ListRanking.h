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
    NextIter nextit;
    bool done;
    NextIter globalend;  //optimization, save a local copy

    public:
    ListRanking() : done(false) {
      this->register_this(this);
      rmi_fence();
    }

    ~ListRanking() {}

    void define_type(stapl::typer &t) 
      {
	t.local(nextit);
	t.local(done);  //actually not used
      }

    void Ranking(pList<T>& myplist) {

      //initial local ranking
      InitLocalRanking(myplist);

#ifdef STAPL_DEBUG
      myplist.DisplayPContainer();
#endif

      //initial nextit to point to begin of next part
      NextIter it(&myplist);
      it = myplist.local_end();
      it.set_uselocaliterator(false);   //switch to global iter
      ++it;
      nextit = it;
      globalend = myplist.end();
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
      if(nextit != globalend) {      
	T tmp = *nextit;
	rmi_poll();   //rmi call in *nextit
	NextIter it(&myplist);
	for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	  *it += tmp;   
	}
      }
      else done = true;
    }

    void PtrJumping() {
     NextIter tmpit = sync_rmi(nextit.base_pid(), getHandle(), 
				  &ListRanking<T>::GetNextIter);
     rmi_poll();  //make sure everyone received the old nextit
     nextit = tmpit;
    }

    protected:

    NextIter GetNextIter() {
      return nextit;
    }

    void InitLocalRanking(pList<T>& myplist) {
      NextIter it(&myplist);
      int dist = 0;
      //operator-- needs to be implemented
      for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	*it += dist;
	dist = *it;
      }
      for(it=myplist.local_begin(); it!=myplist.local_end(); ++it) {
	*it = dist - *it + 1;
      }
    }

  };

} //end namespace

#endif
