#ifndef _Hier_Load_Sch_h_
#define _Hier_Load_Sch_h_

#include <list>
#include <vector>
#include <map>
#include "ScheduleRequest.h"

class BaseScheduler
{
	public:
		virtual void * Schedule()=0;
		
};

void * Hierarchical_Load_Scheduler_Run(void *hls_address);
template <class pRange_Type>
class HierarchicalLoadScheduler : public BaseScheduler
{
private:

	vector<int> readytasks;
	pRange_Type * prange;

	double local_weight; //weight of the local thread

    map<int, map<int,double> > children; //map<level, map<childid,weight> >Children "at" different levels in the hierarchy//Level 0 is the bottom most level
	map<int,int>	parents; //map<level,parentid> "My" Parents at different levels in the hierarchy. //level 0 parent is the bottommost parent
	map<int, map<int,pair<int,bool> > > child_status;//map<level, map<childid,pair<size,prangeover> > > The size of data in child nodes
	map<int, map<int,list<ScheduleRequest> > > ScheduleRequests; //map<level,map<originlevel, list<Requests> > >....schedule requests to be satisfied by this level...


	//map<int,list<TaskAvailabilityToken>  > TaskAvailabilityTokens;
	list<TaskAvailabilityToken>  TaskAvailabilityTokens;

    pthread_t hls_thread; // Thread associated with the load scheduler where the actual load balancing computation are being done.
    pthread_mutex_t mutex_lock;

    stapl::rmiHandle handle; //rmi handle for communication

    bool scheduler_thread_created;
	bool scheduling_started; //only after somebody pushed tasks in
	bool sch_loop_done_atleast_once; //The scheduler thread loop is executed atleast once


    bool sch_thread_done;
	bool global_empty_flag;

	int tree_num_levels; //number of levels in the tree... height of the tree-1.  The level numbering starts from 0

	typedef map<int, map<int,double> >::iterator childleveliterator;
	typedef map<int,double>::iterator childiterator;

	typedef map<int, map<int,pair<int,bool> > >::iterator statusleveliterator;
    typedef map<int,pair<int,bool> >::iterator statusiterator;

	typedef map<int, map<int,list<ScheduleRequest> > >::iterator  requestleveliterator;
	typedef map<int,list<ScheduleRequest> >::iterator  requestoriginleveliterator;
	typedef list<ScheduleRequest>::iterator requestiterator;

	//typedef map<int,list<TaskAvailabilityToken> >::iterator availabilityleveliterator;
	typedef list<TaskAvailabilityToken>::iterator availabilityiterator;


	typedef HierarchicalLoadScheduler<pRange_Type>  HLS_Type;
public:



	HierarchicalLoadScheduler(pRange_Type * pr );
	void SetHierarchy();
	void Initialize_Child_Weights();
	void SetChildWeight(pair<int,pair<int,double> > x);
	void Initialize_Child_Status();
	void Initialize_Schedule_Requests();
	void Set_Scheduler_Thread_Created(bool b){	scheduler_thread_created = b;	}

	void Print();
	void PrintHierarchyOnly();
	void PrintChildStatus();
	void PrintHierarchy(bool=true);
	void PrintRequests();
	void PrintTaskTokens();


	pRange_Type * get_prange()      { return prange;  }
	stapl::rmiHandle getHandle()    { return handle; }

	void push(std::vector<int> newtasks);
    std::vector<int>	pop();

	virtual void * Schedule();

	bool global_empty()	
	{   
		#ifdef _FULLY_FUNCTIONAL_PRANGE		
			return ( global_empty_flag && sch_thread_done ); 
		#else
			prange->get_ddg().finished();
		#endif
	}
	bool empty()	{ return global_empty(); }
    bool Scheduling_Started (){ return scheduling_started; }


	void Set_Scheduler_Thread_Done(bool b){	sch_thread_done = true; }
	bool Scheduler_Thread_Done()		{	return sch_thread_done;	}

	void Set_sch_loop_done_atleast_once(bool b)    { sch_loop_done_atleast_once = b; }



	void SendStatusToParent();
	int local_ready_size(); //Size of local tasks that are ready for execution;
	void UpdateStatus( pair<int , pair<int,pair<int,bool> > > arg);
	void UpdateStatus(int level,int child,int new_size,bool prangeover);


	void CheckChildrenImbalance();
	void AcceptRequest(ScheduleRequest sr);
	void ProcessBufferedRequestInfo(int level);
	void AcceptTaskAvailabilityToken(TaskAvailabilityToken at);
	void ProcessTaskAvailabilityTokens(int level);



	void ProcessRequests();
	void AcceptReadyTasks(vector<int> newreadytasks);

	void getLock(int x)	{	//cout<<"LOCKED in \t"<<stapl::get_thread_id()<<"  ["<<x<<"]"<<endl;
	                                        pthread_mutex_lock (&mutex_lock);
	                        }
	void releaseLock(int x)	{
	                		//cout<<"UNLOCKED in \t"<<stapl::get_thread_id()<<"  ["<<x<<"]"<<endl;
					pthread_mutex_unlock (&mutex_lock);
	                        }

        bool get_global_empty_flag()    {       return global_empty_flag; };
	void set_local_global_empty_flag(bool b)
	{
               // cout<<" Receiving in Thread "<<stapl::get_thread_id()<<" bool b = "<<b<<endl;
	        global_empty_flag = b;
	};

	void set_global_empty_flag(pair<int,bool> x)
	{



	        int level = x.first;
		bool flag = x.second;
		//cout<<" Setting global empty flag in "<<stapl::get_thread_id()<<"    Level = "<<level<<endl;

                childleveliterator ch_lvl_it = children.find(level);
		if(ch_lvl_it==children.end())
		{       cout<<" Error in HierarchicalLoadScheduler::SetGlobalEmptyFlag"<<endl;
		        cout<<" In Thread "<<stapl::get_thread_id()<<"  ";
			cout<<" Did not find any information about children in level "<<level<<endl;
			exit(-1);
		}

                map<int,double> *chln = &ch_lvl_it->second;

		for(childiterator it=chln->begin(); it!=chln->end(); ++it)
		{     //  cout<<" Thread "<<stapl::get_thread_id()<<" Child = "<<it->first<<" Level = "<<level<<endl;
		}

		for(childiterator it=chln->begin(); it!=chln->end(); ++it)
		{


		        if(level==0)
			{
			        int childid=it->first;
                                stapl::async_rmi(childid,1,&HLS_Type::set_local_global_empty_flag,flag);
			}
			else
			{
	                        int childid=it->first;
			        pair<int,bool>  y;
		                y.first=level-1;
			        y.second=flag;
			        stapl::async_rmi(childid,getHandle(),&HLS_Type::set_global_empty_flag,y);
			}

		 }


	        global_empty_flag = flag;
	};


	void dummyfunc(int p)   {       int k; k=p; }



};

#include "_HierarchicalSchedulerConstructor.h"
#include "_HierarchicalSchedulerHelperFunctions.h"
#include "_HierarchicalSchedulerUserInterface.h"
#include "_HierarchicalSchedulerSendStatus.h"
//#include "_HierarchicalSchedulerCheckImbalance.h"
#include "_HierarchicalSchedulerCheckImbalance2.h"
#include "_HierarchicalSchedulerProcessRequests.h"
#include "_HierarchicalSchedulerProcessRequestsFirstDownTree.h"
#include "_HierarchicalSchedulerProcessRequestsSecondDownTree.h"
#include "_HierarchicalSchedulerSchedule.h"
#include "HierarchicalLoadSchedulerThread.h"


#endif
