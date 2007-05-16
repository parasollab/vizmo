 
 
template<class pRange_Type>
void *
HierarchicalLoadScheduler<pRange_Type>::Schedule()
 {
	 
	 


	int scheduler_loop_count = 0;
	bool done = false;

	this->Set_Scheduler_Thread_Created(true);

	while(!this->Scheduling_Started())
	{        /* cout<<"Waiting for scheduling to start. In Hier_Load_Scheduler Thread "<<stapl::get_thread_id()<<endl; */	}

        
	do
	{

		this->SendStatusToParent();
        stapl::rmi_fence();

		/*
			this->PrintChildStatus();
        	if(stapl::get_thread_id() ==0 )
			{
		        cout<<" Send Status to Parent Over in Thread"<<stapl::get_thread_id()<<endl;
			}
		*/


	    this->CheckChildrenImbalance();
		stapl::rmi_fence();

		/*
			if(stapl::get_thread_id() ==0 )
			{
		    	    cout<<" Check Children Imbalance Over in Thread"<<stapl::get_thread_id()<<endl;
			}
			//this->PrintRequests();
			//this->PrintChildStatus();

		*/

	

	    this->ProcessRequests();
		stapl::rmi_fence();

		//cout<<" Process Requests done in Thread"<<stapl::get_thread_id()<<endl;
		
		done = this->get_global_empty_flag();

		/*
		if(false)
	        for( int i=0; i<stapl::get_num_threads(); ++i)
	        {
		        if(i==stapl::get_thread_id())
		        {
			        cout<<"In Thread ["<<stapl::get_thread_id()<<"]\tLoopC = "<<scheduler_loop_count<<"\tpRangeSize = "<<this->get_prange()->local_size()<<"\tRdy Size="<<this->local_ready_size()<<"\tGlbEmpty?= "<<done<<endl;
				//this->get_prange()->get_pcontainer()->DataPrint();
			        if( i == (stapl::get_num_threads()-1) )
			        {     	cout<<" ---------------------------------------------------- "<<endl;        }

		        }

		        stapl::rmi_fence();

	        }
		*/
		

		cout<<"In Thread ["<<stapl::get_thread_id()<<"]\tLoopC = "<<scheduler_loop_count<<"\tpRangeSize = "<<this->get_prange()->local_size()<<"\tRdy Size="<<this->local_ready_size()<<"\tGlbEmpty?= "<<done<<endl;
        stapl::rmi_fence();
		
		
		this->Set_sch_loop_done_atleast_once(true);
			
		if(done==false)
		{		
	    	    system("sleep 2");
		}
		scheduler_loop_count++;


	}
	while(done==false);


	this->Set_Scheduler_Thread_Done(true);
    return (void *)this;
		
	
	
 }
