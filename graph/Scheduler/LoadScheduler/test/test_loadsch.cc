#define _STAPL_LOAD_SCHEDULER_
/*
  The flag _STAPL_LOAD_SCHEDULER_ comments out the rmi_poll from STAPL executor ( p_for_all)
  
      If you are using the load scheduler. Then you can't have an RMI Poll in p_for_all "RIGHT NOW". 
      This is because the load scheduler creates a helper thread, which also does RMI. 
      The RMI library presently can't support RMI on two threads ( main computation thread and load scheduling thread)
      at the same time. So this poll has to be removed. 
    
      When the RMI_Thread is fully implemented ( when RMI could be performed on both threads ) then this comment could 
      be taken out.
*/



#define _FULLY_FUNCTIONAL_PRANGE    

/*
 If the flag _FULLY_FUNCTIONAL_PRANGE is declared then the Transferable_pRange is used to do the test.
 Else the STAPL pRang is used. If the STAPL pRange is used then no data will be shipped. 
 The appropriate methods in pRange will be called by the load scheduler but no load 
rebalancing wil occuur as  the methods in STAPL pRange resonsible for shipping data are not implemented
*/



#include <stdlib.h>
#include <iostream>
#include <pvector.h>
#include <pRange.h>
#include <LoadScheduler.h>
#include <runtime.h>


void load_test_transferable_prange();  
void load_test_simple_prange();  
  
void stapl_main(int argc, char **argv) 
{

if(false) //For compile sake only
{  
  load_test_transferable_prange();  
  load_test_simple_prange();  
}


#ifdef   _FULLY_FUNCTIONAL_PRANGE    
  load_test_transferable_prange();  
#else 
  load_test_simple_prange();  
#endif

}





int _gid_ = 200;
int _next_gid() {  return _gid_++; }

void   load_test_transferable_prange()
{
  /*
    This function uses transferable pRange to test the load scheduler.
    The load could be rebalanced.
  */

  
  typedef Transferable_pList<int> PLIST;
  typedef stapl::linear_boundary<PLIST::iterator> LIST_BOUNDARY;
  typedef Transferable_pRange<LIST_BOUNDARY,IndependentTasksDDG, PLIST> LIST_PRANGE;
  
  PLIST plist;
  int nElems = 800;
  
  if(stapl::get_thread_id() == 0)
  {
    for(int i = 0; i<nElems; ++i)  {    plist.push_back(i);  }
  }
  
  //plist.DataPrint();
  
  list<LIST_BOUNDARY> boundaries = plist.CreateBoundaries(5);
  
  for(list<LIST_BOUNDARY>::iterator it = boundaries.begin(); it!=boundaries.end(); ++it)
  {
    //Print_pListBoundary(*it);
  }
  
  LIST_PRANGE prange(&plist);
  
  
  for(list<LIST_BOUNDARY>::iterator it = boundaries.begin(); it!=boundaries.end(); ++it)
  {
      prange.add_subrange(*it);
  }
  
  ListWorkFunction lwf;
  
  stapl::rmi_fence();
  
    
  HierarchicalLoadScheduler<LIST_PRANGE>  hls(&prange);
    
    prange.get_ddg().reset();
  stapl::rmi_fence();
  
  p_for_all(prange,lwf,hls);
    
  

  
  stapl::rmi_fence();
       
  lwf.SendToThread(0);
    
  stapl::rmi_fence();

      
    if(stapl::get_thread_id() == 0)
  {
      lwf.Test(nElems);  
  }

    
  stapl::rmi_fence();
  
  

}


void load_test_simple_prange()
{
  
  /*
    This function uses STAPL pRange to test the load scheduler.
    No load balacing will be done. . 
   The appropriate methods in pRange will be called by the load scheduler but no load 
  rebalancing wil occuur as  the methods in STAPL pRange resonsible for shipping data are not implemented
  */

  typedef Transferable_pList<int> PLIST;
  typedef stapl::linear_boundary<PLIST::iterator> LIST_BOUNDARY;
  typedef stapl::pRange<LIST_BOUNDARY,PLIST,IndependentTasksDDG> LIST_PRANGE;
  
  PLIST plist;
  int nElems = 800;
  
  if(stapl::get_thread_id() == 0)
  {
    for(int i = 0; i<nElems; ++i)  {    plist.push_back(i);  }
  }
  
  //plist.DataPrint();
  
  list<LIST_BOUNDARY> boundaries = plist.CreateBoundaries(5);
  
  for(list<LIST_BOUNDARY>::iterator it = boundaries.begin(); it!=boundaries.end(); ++it)
  {
    //Print_pListBoundary(*it);
  }
  
  
  IndependentTasksDDG ddg;

  int offset = 10000;
  LIST_PRANGE prange(&plist,LIST_BOUNDARY(plist.local_begin(),plist.local_end()),offset);
  prange.set_ddg(&ddg);
  
  
  
  for(list<LIST_BOUNDARY>::iterator it = boundaries.begin(); it!=boundaries.end(); ++it)
  {
      int _gid = _next_gid();
      LIST_PRANGE subrange(_gid,stapl::get_thread_id(),&plist,*it,offset,&prange);
      prange.add_subrange(subrange);
      prange.get_ddg().AddTask(_gid);
  }
  
  
  cout<<" Num Subranges = "<<prange.get_num_subranges() <<endl;
  
  
  
  ListWorkFunction lwf;
  HierarchicalLoadScheduler<LIST_PRANGE>  hls(&prange);
  
  prange.get_ddg().reset();
  stapl::rmi_fence();
  p_for_all(prange,lwf,hls);

  stapl::rmi_fence();
  
  
  
  stapl::rmi_fence();
       
  lwf.SendToThread(0);
    
  stapl::rmi_fence();

      
    if(stapl::get_thread_id() == 0)
  {
      lwf.Test(nElems);  
  }

    
  stapl::rmi_fence();
  
  
}
