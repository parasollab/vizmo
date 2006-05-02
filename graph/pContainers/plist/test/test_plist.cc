#include "Defines.h"
#include "pList.h"

using namespace stapl;


class work_function1{
 public:
  work_function1(){}
  void operator()(pList<int>::PRange::subrangeType& pr1){
    
  }
};

class work_function2{
 public:
  work_function2(){}
  void operator()(pList<int>::PRange::subrangeType& pr1, pList<int>::PRange::subrangeType& pr2){
    
  }
};


void stapl_print(const char* s){
  if(stapl::get_thread_id() == 0)
    cout << s << flush;
}

void stapl_main(int argc,char** argv) {

  int myid=stapl::get_thread_id();
  int nprocs=stapl::get_num_threads();
  
  if(argc < 5) {
    cout<< "Usage: exe nGID GID, GID1 Check_PID" <<endl;
    exit(1);
  }

  int n_gid = atoi(argv[1]);//size of the pList that will be used.
  int in_gid = atoi(argv[2]);
  int in_gid1 = atoi(argv[3]);
  PID in_pid = atoi(argv[4]);

  Location loc;
  int tmp,k,n=nprocs;
  int t = myid*n;
  
  //=======================================
  // building a pList
  //=======================================
  stapl_print("testing implicit pList constructor...");  
  stapl::pList<int> myplist;
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing AddElements...");  
  list<int> seqlist;
  for(k=0; k<n_gid; k++) {
    seqlist.push_back(k+myid*n);
  }
  myplist.AddElements(seqlist);
  stapl::rmi_fence();
  myplist.DisplayPContainer();
  pList<int>::iterator it_1=myplist.local_begin();
  for(k=0; it_1!=myplist.local_end(); ++it_1,++k) {
    if(*it_1 != (k+myid*n)) 
      cout<<"ERROR:: while testing AddElements\n";
  }
  stapl::rmi_fence();  
  stapl_print("Passed\n");

  //==========================================
  // testing get_prange and pList(PRange, bool)
  //=========================================
  stapl::pList<int>::PRange mypr(&myplist);
  myplist.get_prange(mypr);
  stapl::pList<int> copy1(mypr, true);
  pList<int>::iterator it_copy1=copy1.local_begin();
  for(k=0; it_copy1!=copy1.local_end(); ++it_copy1,++k) {
    if(*it_copy1 != (k+myid*n)) 
      cout<<"ERROR:: while testing get_prange or pList(PRange,true)\n";
  }
  stapl::rmi_fence();
  stapl::pList<int> copy2(mypr, false);
  copy2.DisplayPContainer(); 
  if (copy2.local_size() != myplist.get_num_parts() ) 
      cout<<"ERROR:: while testing get_prange or pList(PRange,true)\n";
  
  stapl::rmi_fence(); 
  stapl_print("Passed\n");

  //=======================================
  // testing Get/Set/Delete methods 
  //=======================================
  stapl_print("testing Get/Set element...");  
  tmp = 88+myid;
  if(myid == 0) {
    myplist.SetElement(in_gid,tmp);
  }
  stapl::rmi_fence();

  if(myid == 0) {
    try{
 	tmp = myplist.GetElement(in_gid);
    } 
    catch(stapl::ElementNotFound& e){
      e.Report();
    }
    if(tmp != 88 + myid){
      cout<<"ERROR: while testing Get/SetElement"<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing delete element...");  
  if(myid == 0) myplist.DeleteElement(in_gid);

  loc = myplist.Lookup(in_gid);
  if(loc.ValidLocation()) {
    cout<<"ERROR while deleting an element from pList"<<endl;
  }
  stapl::rmi_fence();
  //myplist.DisplayPContainer();
  stapl_print("Passed\n");
  
  //=======================================
  // testing size/empty/IsLocal/Lookup methods 
  //=======================================
  stapl_print("testing size(), empty()...\n");  
  int tt = myplist.size();
  bool isemp = myplist.empty();
  cout<<"   size: "<<tt<<" Empty? "<<isemp<<endl;
  stapl::rmi_fence();
  stapl_assert(tt == nprocs * n_gid - 1,"ERROR while getting the size of the pList");
  stapl_assert(isemp == false,"ERROR while testing IsEmpty");
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing IsLocal(), Lookup()...");  
  if(myid == 0) {
    PARTID id=99;
    bool t = myplist.IsLocal(in_gid1,id);
    cout<<"local?"<<t<<"partid:"<<id<<endl;
    if(!t) {
      loc = myplist.Lookup(in_gid1);
      cout<<"Location:"<<loc<<endl;
    }
  }
  stapl::rmi_fence();
  stapl_print("Passed\n");
  //=======================================
  // testing local iterator 
  //=======================================
  //to test plist with multiple parts in each thread
  //new parts are only added when redistribution
  //rmi for list is not ready, so added part like this
  //the user should not use addpart directly
  stapl_print("testing AddPart(), AddElement2Part()...");  
  PARTID partid = myplist.AddPart();
  tmp = 100 + myid;
  myplist.AddElement2Part(tmp, partid);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl_print("testing local iterators...");  
  pList<int>::iterator it=myplist.local_begin();
  for(; it!=myplist.local_end(); ++it) {
    *it = (myid+999);
  }
  stapl::rmi_fence();
  it=myplist.local_begin();
  for(; it!=myplist.local_end(); ++it) {
    if(*it != (myid+999)){
      cout<<"ERROR while setting element value using iterators"<<endl;
    }
  }
  stapl::rmi_fence(); 
  stapl_print("Passed\n");
  //=======================================
  // testing pList specific methods
  //=======================================
  stapl_print("testing front, push_front, pop_front...");  
  pList<int> myplist1(n_gid, 99);
  stapl::rmi_fence();
  for(int j=0; j<n_gid/nprocs; j++) {
    GID gid = myid+nprocs*j;
    if(myplist1.IsLocal(gid) )
      myplist1.SetElement(gid,myid*n_gid/nprocs+j);
  }
  stapl::rmi_fence();


  //----------------------------------------
  //   front/push_front/pop_front
  //----------------------------------------
  it = myplist1.front();
  tmp = *it;

  if(myid == 0) {
    myplist1.push_front(t);
  }  
  stapl::rmi_fence();
  it = myplist1.front();
  if(myid == 0)
    stapl_assert(*it == t,"ERROR while performing push_front(1)");
  //  myplist1.DisplayPContainer();
  stapl::rmi_fence();
  
  if(myid == 0) {
    myplist1.push_back(t+1);
  }

  stapl::rmi_fence();
  /*
  it = myplist1.front();
  if(myid == 0)
    stapl_assert(*it == t+1,"ERROR while performing push_front(2)");

  if(myid == 0) {
    myplist1.pop_front();
    myplist1.pop_back();
  }
  stapl::rmi_fence();
  it = myplist1.front();
  stapl_assert(*it == tmp,"ERROR while performing pop_front(1)");
  stapl_print("Passed\n");

  //----------------------------------------
  //   back/push_back/pop_back
  //----------------------------------------
  stapl_print("testing back, push_back, pop_back...");  
  it = myplist1.back();
  tmp = *it;

  if(myid == 0) {
    myplist1.push_back(t);
  }  
  stapl::rmi_fence();
  it = myplist1.back();
  if(myid == 0)
    stapl_assert(*it == t,"ERROR while performing push_back(1)");
  //  myplist1.DisplayPContainer();
  stapl::rmi_fence();
  
  if(myid == 0) {
    myplist1.push_back(t+1);
  }

  stapl::rmi_fence();
  return;
  it = myplist1.back();
  if(myid == 0)
    stapl_assert(*it == t+1,"ERROR while performing push_back(2)");

  if(myid == 0) {
    myplist1.pop_back();
    myplist1.pop_back();
  }
  stapl::rmi_fence();
  it = myplist1.back();
  stapl_assert(*it == tmp,"ERROR while performing pop_back(1)");
  */
  stapl_print("Passed\n");

  stapl_print("testing get_prange/p_for_all...");  
  pList<int>::PRange pr(&myplist);  
  pList<int>::PRange pr1(&myplist);  
  myplist.get_prange(pr);
  myplist.get_prange(pr1);
  work_function1 W1;
  work_function2 W2;
  stapl::defaultScheduler scheduler;
  p_for_all(pr,W1,scheduler);
  p_for_all(pr,pr1,W2,scheduler);
  stapl::rmi_fence();
  stapl_print("Passed\n");

  stapl::rmi_fence();
}
