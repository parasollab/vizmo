#include <runtime.h>
#include <rmitools.h>
#include "../ListRanking.h"

using namespace stapl;

void stapl_main(int argc,char** argv) {

  int myid=stapl::get_thread_id();
  int nprocs=stapl::get_num_threads();
  
  if(argc < 3) {
    cout<< "Usage: exe listsize, repeats" <<endl;
    exit(1);
  }

  int n_gid = atoi(argv[1]);
  int reps = atoi(argv[2]);

  double etime=0;
  timer stime;
  
  pList<int> myplist(n_gid,1);
  stapl::rmi_fence();
  ListRanking<int> mylistranking;
  stapl::rmi_fence();


  int i, n=5;
  double ave=0., std=0., var=0., mini=1000.;
  double tt[100];
  for(i=0;i<reps;i++){
    stime=start_timer();
    mylistranking.Ranking(myplist);
    etime =stop_timer(stime);
    ave += etime;
    if(etime < mini) mini = etime;
    tt[i]=etime;
  }
  ave = ave/(double)reps;
  for(i=0; i<reps; i++) {
    var += (tt[i]-ave)*(tt[i]-ave);
  }
  var = var/(double)reps;
  std = sqrt(var);
  cout<<"THREAD: "<<myid<<" plist time (sec.): MIN: "<<mini
      <<" AVE: "<<ave<<" with std="<<std<<endl;

  stapl::rmi_fence();
  

  //sequential list ranking:
  ave=0.; var=0., mini=1000.;
  list<int> seqlist(n_gid);
  for(i=0;i<reps;i++){
    stime=start_timer();
    list<int>::iterator it=seqlist.begin();
    for(; it!=seqlist.end(); ++it) {
      *it = 1;
    }
    int first = 1;
    for(it=seqlist.begin(); it!=seqlist.end(); ++it) {
      *it = *it + first;
      first = *it;
    }
    etime =stop_timer(stime);
    if(etime < mini) mini = etime;
    ave += etime;
    tt[i]=etime;
  }
  ave = ave/(double)reps;
  for(i=0; i<reps; i++) {
    var += (tt[i]-ave)*(tt[i]-ave);
  }
  var = var/(double)reps;
  std = sqrt(var);
  cout<<"THREAD: "<<myid<<" seq time (sec.):  MIN: "<<mini
      <<" AVE: "<<ave<<" with std="<<std<<endl;
}
