template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
ProcessRequests()
{

        requestleveliterator rlit=ScheduleRequests.find(0);     //Find requests in level 0
        if(rlit==ScheduleRequests.end())
        {
                        cout<<" Error in HLS::Process Requests"<<endl;
	                cout<<" Scheduler Requests in Level 0 is not found "<<endl;
	                exit(-1);
        }

        for(int level=0; level<=tree_num_levels-2; ++level)
	{

                if(stapl::get_thread_id() == 0 )
		{
		        //cout<<" Processing Requests in Level "<<level<<endl;
		}


	        requestoriginleveliterator rolit=rlit->second.find(level);
        	if(rolit==rlit->second.end())
	        {
	                cout<<" Error in HLS::Process Requests"<<endl;
		        cout<<" Scheduler Requests in Origin Level "<<level<<" not found "<<endl;
		        exit(-1);
	        }
	        list<ScheduleRequest> * requests = &rolit->second;


		double initial_fraction=1;
                for(requestiterator it1 = requests->begin(); requests->size()>0; )
		{

		        int lsize = local_ready_size(); // Size of the local ready queue
			getLock(3);
                        double fraction_to_send = it1->getFraction()/initial_fraction;
			int sendsize = (int) ((lsize * fraction_to_send)+0.01);

			vector<int>     tosend; //ids of ready subranges to be sent
                        int x = 0;
			for(;  readytasks.size()>0; )
			{
                                x+= prange->get_subrange(readytasks[readytasks.size()-1]).get_size();

				if( x > sendsize )
				{
				        break;
				}

				tosend.push_back(readytasks[readytasks.size()-1]);

				readytasks.pop_back();
			}


			prange->SendSubRanges(tosend,it1->getTargetPid());

			stapl::async_rmi(it1->getTargetPid(),getHandle(),&HLS_Type::AcceptReadyTasks,tosend);

			//Request is satisfied... clear the request
			initial_fraction -=it1->getFraction();
			list<ScheduleRequest>::iterator itold = it1;
			it1++;
			requests->erase(itold);

                        releaseLock(3);

		}

		if(stapl::get_thread_id() == 0 )
		{
		        //        cout<<" ----------- At the end of Level "<<level<<" -------------- "<<endl;
		}
		stapl::rmi_fence();

        /*
		for(int i=0; i<stapl::get_num_threads(); ++i)
		{

		        if(i==stapl::get_thread_id())
			{
                                cout<<"In Thread "<<stapl::get_thread_id()<<" Size of local tasks = "<<local_ready_size()<<endl;
			}
			stapl::rmi_fence();
                }
		stapl::rmi_fence();
        */

        }


}

//-------------------------------------------------------------------//

template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
AcceptReadyTasks(vector<int> newreadytasks)
{

getLock(4);
        for(int i=0; i<newreadytasks.size(); ++i)
	{
                readytasks.push_back(newreadytasks[i]);
	}
releaseLock(4);
}

//-------------------------------------------------------------------//
