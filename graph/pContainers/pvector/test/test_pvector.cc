#include <stdio.h>
#include <stdlib.h>
#include <runtime.h>
#include "pvector.h"
#include <algorithm>
#include <assert.h>
#include <map>




#define VALUE 8
typedef int  ELEM ;
using namespace stapl;




bool check_properties(pvector<ELEM> & _pv, size_t N, ELEM val)
{


  print_tag<int>(_pv.tag());
  assert(_pv.myid==get_thread_id());
  assert(_pv.nprocs=get_num_threads());
  for(int i=0;i<N;i++)
    _pv.Set(i,i);
  rmi_fence();
  ELEM val_get;
  for(int i=0;i<N;i++)
   {
     val_get=_pv.Get(i);
     if (val_get!=i)
       printf("\n Get=%d and i=%d on processor %d",val_get,i,get_thread_id());
     
     assert(_pv.Get(i)==i);
     
     val_get=_pv[i];

     if (val_get!=i)
       printf("\n [%d]=%d and i=%d on processor %d",i,val_get,i,get_thread_id());
     assert(_pv[i]==i);

   }
  rmi_fence();
  if (get_thread_id()==0) printf("\n     Set and Get  checked......");

  printf("\n Thread %d -- Size=%d",get_thread_id(),_pv.size());
  printf("\n Thread %d -- Capacity=%d",get_thread_id(),_pv.capacity());

  assert(_pv.capacity()>=_pv.size());
  if (get_thread_id()==0) printf("\n     capacity  checked......");

  printf("\n Thread %d -- Local Capacity=%d",get_thread_id(),_pv.local_capacity());

  assert(_pv.local_capacity()>=_pv.local_size());
  if (get_thread_id()==0) printf("\n     local_capacity  checked......");
  _pv.reserve(N+N);
  assert(_pv.capacity()>=2*N);
  if (get_thread_id()==0) printf("\n     reserve  checked......");
  if (get_thread_id()==0) printf("\n     All methods from pvector.h have been checked");
  assert(_pv.size()==N);
  if (get_thread_id()==0) printf("\n     size()  checked......");
  printf("\n Thread %d -- local Size=%d",get_thread_id(),_pv.local_size());

  assert(_pv.local_size()==(N/get_num_threads()+((N%get_num_threads())> get_thread_id())));
  if (get_thread_id()==0) printf("\n     local_size()  checked......");

  printf("\n Thread %d -- Empty=%d",get_thread_id(),_pv.empty());

  assert(_pv.empty()==(N==0));
  if (get_thread_id()==0) printf("\n     empty()  checked......");
  printf("\n Thread %d -- Local Empty=%d",get_thread_id(),_pv.local_empty());

  assert(_pv.local_empty()==(N==0));
  if (get_thread_id()==0) printf("\n     local_empty()  checked......");
  if (N!=0)
    {
      pvector<ELEM>::iterator it=_pv.local_begin();
      ELEM x=*it -1  ;
      while(it!=_pv.local_end())
	{
	  assert(*it==(x+1));
	  x=*it;
	  ++it;
	}
    }
  if (get_thread_id()==0) printf("\n     local_begin and local_end checked......");
  /********* Testing the iterators**************************************************/
  pvector<ELEM>::iterator P_begin,P_end, it, _iterator;
  P_begin=_pv.local_begin();
  P_end=_pv.local_end();
  int i=0;
  ptrdiff_t diff1,diff2;

  for( it=_pv.local_begin();it!=_pv.local_end();++it,++i)
    {
      diff1=it-P_begin;
      diff2=it-P_end;;
      if ((P_begin+i)!=it)
	  {
	    printf("\n The ++ iterators are different at iteration %d-- %d and %d",i,*(P_begin+i),*it);
	    exit(0);
	  }
    }
printf("\n Operators ++() and +(int)    checked................done");
  i=0;
  for( it=(_pv.local_end()-1);it!=_pv.local_begin();--it,++i)
    {
      _iterator=P_end-(1+i);
      if (_iterator!=it)
	{
	  printf("\n The -- iterators are different at iteration %d-- %d and %d",i,*(_iterator),*it);
	  exit(0);
	}

    }
  if (P_begin!=it)
    {
      printf("\n The -- iterators are different at iteration %d-- %d and %d",i,*P_begin,*it);
      exit(0);
    }
printf("\n Operators --() and -(int)    checked................done");

  
  /************************* Done testing the iterators************************************/
  /*
  int i=_pv.local_size();
  int d=_pv.size();
  int y=0;
  pvector<ELEM>::iterator start=_pv.begin();
  pvector<ELEM>::iterator stop=_pv.end();
  
  if (!(start!=stop)) printf("\n The iterators are equal");
  else printf("\n The iterators are NOT equal");
  if (get_thread_id()==0)
    {
      it=_pv.begin();
      while (it!=_pv.end())
	{
	  printf("\n %d",*it);
	  fflush(stdout);
	  if (it!=stop) printf("\n Before different");
	  else printf("\n Before Equal");
	  ++it;
	  y++;
	  printf("\n %d and %d",d ,y);
	  if (d==y) break;
	  printf("\n ++ -------");
	  if (it!=stop) printf("\n After different");
	  else printf("\n After Equal");

	  //	if (y==i) break;
	}
      printf("\n Done printing");
    }
  */

  _pv.insert(_pv.local_begin(),6);
  _pv.update();
  if (*(_pv.local_begin())==6) printf("\n COrrect");
	else printf("\n Incorrect");
  return true;
}





void stapl_main(int argc, char *argv[]) {
  if (argc<2)
    {
      printf("\n Usage driver pvector : > driver_pvector #elements");
      exit(0);
    }

  map<PID,vector<pair<GID,long int> > > my_dist;
  vector<pair<GID,long int> > dist1,dist2;
  dist1.push_back(pair<GID,long int>(0,14));
  dist1.push_back(pair<GID,long int>(25,35));
  dist1.push_back(pair<GID,long int>(35,56));
  dist1.push_back(pair<GID,long int>(67,100));

  dist2.push_back(pair<GID,long int>(14,25));
  dist2.push_back(pair<GID,long int>(56,67));

  

  my_dist[0]=dist1;
  my_dist[1]=dist2;
  
  //  my_dist[2]=pair<int,int>(63,71);
  //  my_dist[3]=pair<int,int>(71,99);
  int N=atoi(argv[1]);
  int i=0;
  pvector<ELEM,no_trace,base_distribution> _test_dist_map(my_dist);
  
  
  printf("\n Local size %d size %d th_id %d", _test_dist_map.local_size(),_test_dist_map.size(),get_thread_id());

  rmi_fence();



  return;

  /*
  ///////////////////////////////////////////////////////////////////////////////////
  if (get_thread_id()==0) printf("\ntesting implicit pvector constructor......");
  pvector<ELEM> P1;//implicit
  rmi_fence();
  //  check_properties(P1,0,ELEM());
  rmi_fence();
  if (get_thread_id()==0)  printf("\ndone");
  ///////////////////////////////////////////////////////////////////////////////////
  if (get_thread_id()==0)printf("\ntesting  pvector constructor (N)......");
  pvector<ELEM> P2(N);
  rmi_fence();
  P2.update();
  check_properties(P2,N,ELEM());

  P2.Distribute(my_dist);
  // exit(0);
  
  //  check_properties(P2,N,ELEM());
  rmi_fence();
  if (get_thread_id()==0)printf("\ndone");
  
  ///////////////////////////////////////////////////////////////////////////////////
  if (get_thread_id()==0)printf("\ntesting  pvector constructor (N,VALUE)......");
  pvector<ELEM> P3(N,VALUE);
  rmi_fence();
  check_properties(P3,N,VALUE);
  if (get_thread_id()==0)printf("\ndone");

  */  


  //Constructing the STAPL pvector

  //  for(pvector<ELEM>::iterator i = P2.local_begin(); i != P2.local_end(); ++i) { *i += 1; }

  
  // Inserting in the beginning of the STL vector
  //  P2.insert(P2.begin(),0);
  // Erasing from the beginning of the  STL vector
  //  P2.erase(P2.begin());
  // Traversing the elements of the STAPL vector using an iterator
  // Doing ++ on the elements so the compiler cannot deadcode this loop

  /*
  for(pvector<ELEM>::iterator it=P2.local_begin();it!=P2.local_end();++it)
    ++i;
  */
  /*
  P2.clear();
  if (get_thread_id()==0) printf("\n\n\n Pvector test driver has successfully completed  !!! \n");
  rmi_fence();

  //////////////////////////////////////////////////////////////////////////////////////////
 
  P2.Distribute(my_dist);
  */
  //  check_properties(P2,N,ELEM());


}


