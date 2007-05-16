#include "pList.h"

using namespace stapl;

class wf{
 public:
  wf(){}
  void operator()(pList<int>::pListPRange& pr1, pList<int>::pListPRange& pr2){
    
  }
};

void stapl_main(int argc,char** argv) {

  int myid=stapl::get_thread_id();
  int nprocs=stapl::get_num_threads();
  
  if(argc < 5) {
    cout<< "Usage: exe nGID GID Check_PID, repeats" <<endl;
    exit(1);
  }

  int n_gid  = atoi(argv[1]);
  int in_gid = atoi(argv[2]);
  PID in_pid = atoi(argv[3]);
  int reps   = atoi(argv[4]);

  int n=2;
  int t = myid*n;
  double etime=0;
  timer stime;

  pList<int> myplist(n_gid,55);
  stapl::rmi_fence();
  //=======================================
  // testing global iterator 
  //=======================================
  pList<int> myplist1(n_gid, 99);
  stapl::rmi_fence();
  for(int j=0; j<n_gid/nprocs; j++) {
    GID gid = myid+nprocs*j;
    if(myplist1.IsLocal(gid) )
      myplist1.SetElement(gid,myid*n_gid/nprocs+j);
  }

  //myplist1.DisplayPContainer();
  if(myid == in_pid) myplist1.GetDistribution().DisplayBoundaryInfo();

  pList<int>::iterator git3;

  git3=myplist1.begin(); 
  for(; git3!=myplist1.end(); ++git3) {
    //cout<<"dereference of iterator: "<<*git3<<endl;
  }
  stapl::rmi_fence(); 

  //=======================================
  // testing Global pList bookkeeping methods
  //=======================================

  if(myid == in_pid) {
    myplist.push_front(t);
    myplist.push_back(t+1);
    myplist.push_front(t+2);
    myplist.push_back(t+3);
  }
  stapl::rmi_fence();
  //myplist.DisplayPContainer();

  stapl::rmi_fence();

  if(myid == in_pid) {
    myplist.pop_front();
    myplist.pop_back();
  }
  stapl::rmi_fence();
  //myplist.DisplayPContainer();

  pList<int>::iterator git(&myplist);
  pList<int>::iterator git1(&myplist);
  if(myid == in_pid) {
    git=myplist.begin();
    ++git; ++git;
    git1 = myplist.insert(git, t+1000);
  }
  stapl::rmi_fence();
  //myplist.DisplayPContainer();

  rmi_flush();

  if(myid == in_pid) {
    ++git1; 
    myplist.erase(git1);
  }
  stapl::rmi_fence();
  //myplist.DisplayPContainer();

  //=======================================
  // building a pList
  //=======================================
  int i;
  double ave=0., std=0., var=0.;
  double tt[100];
  for(i=0;i<reps;i++){
    list<int> seqlist1;
    stime=start_timer();
    for(int k=0; k<n_gid*nprocs; k++) {
      seqlist1.push_back(k+myid*n);
    }
    etime =stop_timer(stime);
    ave += etime;
    tt[i]=etime;
  }
  ave = ave/(double)reps;
  for(i=0; i<reps; i++) {
    var += (tt[i]-ave)*(tt[i]-ave);
  }
  var = var/(double)reps;
  std = sqrt(var);
  cout<<"THREAD: "<<myid<<" stl time (sec.): "<<ave<<" with std="<<std<<endl;

  list<int> seqlist;
  for(int k=0; k<n_gid; k++) {
    seqlist.push_back(k+myid*n);
  }

  ave=0.; var=0.;
  for(i=0;i<reps;i++){
    stime=start_timer();
    myplist.AddElements(seqlist);
    etime =stop_timer(stime);
    ave += etime;
    tt[i]=etime;
  }
  ave = ave/(double)reps;
  for(i=0; i<reps; i++) {
    var += (tt[i]-ave)*(tt[i]-ave);
  }
  var = var/(double)reps;
  std = sqrt(var);
  cout<<"THREAD: "<<myid<<" plist time (sec.): "<<ave<<" with std="<<std<<endl;

  //myplist.DisplayPContainer();

  //=======================================
  // testing local iterator 
  //=======================================
  //to test plist with multiple parts in each thread
  //new parts are only added when redistribution
  //rmi for list is not ready, so added part like this
  //the user should not use addpart directly
  PARTID partid = myplist.AddPart();
  int tmp = 100 + myid;
  if(myid == 0){
    Location next0(1,1);
    Location before0(-1,INVALID_PART);
    myplist.InitBoundaryInfo(0,before0,next0);
    Location next1(1,0);
    Location before1(1,1);
    myplist.InitBoundaryInfo(1,before1,next1);
  }
  else{
    Location next0(0,0);
    Location before0(0,1);
    myplist.InitBoundaryInfo(0,before0,next0);
    Location next1(0,1);
    Location before1(0,0);
    myplist.InitBoundaryInfo(1,before1,next1);
  }
  myplist.AddElement2Part(tmp, partid);
  myplist.AddElement(211);
  stapl::rmi_fence();
  //myplist.DisplayPContainer();
  pList<int>::iterator it=myplist.local_begin();
  //for(; it!=myplist.local_end(); it++) {
  //*it=myid+999;
  //cout<<"for thread "<<myid<<" data is "<<*it<<endl;
  //}

  pList<int>::pListPRange pr(&myplist);  
  pList<int>::pListPRange pr1(&myplist);  

  myplist.get_prange(pr);
  myplist.get_prange(pr1);
  wf W;
  stapl::defaultScheduler scheduler;
  p_for_all(pr,pr1,W,scheduler);
  stapl::rmi_fence();
  pr.get_ddg().pDisplayGraph();
  
  //myplist.DisplayPContainer();
  myplist.DisplayBoundaryInfo();
  return;

  //=======================================
  // testing Get/Set/Delete methods 
  //=======================================
  int tmp1 = myplist.GetElement(in_gid);
  cout<<"**Thread "<<myid<<" get element gid "<<in_gid
      <<" with data "<<tmp1<<endl;
  int tmp2=88+myid;
  stapl::rmi_fence();
  myplist.SetElement(in_gid,tmp2);
  stapl::rmi_fence();
  cout<<"==Thread "<<myid<<" once set element gid "<<in_gid
      <<" data "<<myplist.GetElement(in_gid)<<endl;

  stapl::rmi_fence();
  if(myid == in_pid) myplist.DeleteElement(in_gid+1);


  stapl::rmi_fence();
  myplist.DisplayPContainer();

  //=======================================
  // testing size/empty/IsLocal/Lookup methods 
  //=======================================
  int ttt = myplist.size();
  cout<<"size: "<<ttt<<" Empty? "<<myplist.empty()<<endl;
  stapl::rmi_fence();

  if(myid == in_pid) {
    Location t = myplist.Lookup(in_gid);
    cout<<"T:"<<t<<endl;
  }
  stapl::rmi_fence();

  if(myid == in_pid) {
    PARTID id=99;
    bool t = myplist.IsLocal(in_gid,id);
    cout<<"local?"<<t<<"partid:"<<id<<endl;
  }

  stapl::rmi_fence();
  

  //=======================================
  // testing distribution/redistribution 
  //=======================================
// typedef pair<list<BaseElement<int> >::iterator,list<BaseElement<int> >::iterator> Listrange;
//   vector<pair<Listrange,PID> > sched;
//   Listrange tmp;
//   int m= (int) n/nprocs;
//   list<BaseElement<int> >::iterator t1=seqlist.begin();
//   for(int i=0; i<nprocs; i++) {
//     list<BaseElement<int> >::iterator t2 = t1;
//     for(int j=0; j<m; j++) {
//       t2++;
//     }
//     tmp.first = t1; 
//     tmp.second = t2;
//     pair<Listrange,PID> tp(tmp,i);
//     sched.push_back(tp);
//     t1=++t2;
//   }
//   vector<vector<pair<Listrange,PID> > > distinfo(1,sched);
//   myplist.DisplayDistributionInfo(distinfo);
//   myplist.Distribute(distinfo);

}
