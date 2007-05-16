template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
ProcessTaskAvailabilityTokens(int level)
{

        bool pf1,pf2,pf3,pf4,pf5,pf6;
	pf1=pf2=pf3=pf4=pf5=pf6=false;
	pf1=true;
	pf2=true;
	pf3=true;
	pf4=true;
	pf5=true;
	pf6=true;

        pf1=pf2=pf3=pf4=pf5=pf6=false;





	if(stapl::get_thread_id() == 0 )
        if(pf1) cout<<"*************** In Thread "<<stapl::get_thread_id()<<"Level = "<<level<<"   Processing availability tokens... Size = "<<TaskAvailabilityTokens.size()<<endl;




        if(pf2)
	{       cout<<" Availability Tokens "<<endl;
	        for(availabilityiterator ait=TaskAvailabilityTokens.begin(); ait!=TaskAvailabilityTokens.end(); ++ait)
	        {	        if(pf1) ait->Print();	}
	}


	double totalreceivedsize = 0;
	for(availabilityiterator ait=TaskAvailabilityTokens.begin(); ait!=TaskAvailabilityTokens.end(); ++ait)
	{
                if(level!=ait->getLevel())
		{
		        cout<<"Error Level mismatch in HLS::ProcessTaskAvailabilityToken "<<endl;
			cout<<" Level = "<<level<<" Level = "<<ait->getLevel()<<endl;
			exit(-1);
		}map<int,list<TaskAvailabilityToken>  > TaskAvailabilityTokens;
	        totalreceivedsize += (ait->getFraction()*ait->getWeight() );
	}

	int avl_size= TaskAvailabilityTokens.size();


        stapl::rmi_fence();

if(avl_size > 0 )
{
        if(pf2) cout<<" In Thread "<<stapl::get_thread_id()<<" Total Received Size = "<<totalreceivedsize<<endl;

        childleveliterator ch_lvl_it = children.find(level-1);
        if(ch_lvl_it == children.end() ){ cout<<" Error in ProcessBufferedRequests"<<endl;      cout<<" Could not find the child in level "<<(level-1)<<endl;	exit(-1);}
	map<int,double> * chld= &ch_lvl_it->second;
        if(pf3) for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1){     cout<<" Child ID = "<<it1->first<<" Weight = "<<it1->second<<endl;	}


        double totalchildweight=0;
        for(childiterator it=chld->begin(); it!=chld->end(); ++it)
        {       int childid=it->first;
                double childweight = it->second;
		totalchildweight += childweight;
        }
	if(pf3) cout<<"In Thread "<<stapl::get_thread_id()<<" Total child weight = "<<totalchildweight<<endl;

	map<int,double> deficit;//corresponding to children;
	for(childiterator it=chld->begin(); it!=chld->end(); ++it)
        {       int childid=it->first;
                double childweight = it->second;
	        deficit[childid]=totalreceivedsize * (childweight/totalchildweight);
        }

	map<int,double> surplus;//corresponding to task availability tokens
        for(availabilityiterator ait=TaskAvailabilityTokens.begin(); ait!=TaskAvailabilityTokens.end(); ++ait)
	{
                surplus[ait->getReplyingProcessor()] = (ait->getFraction()*ait->getWeight());
	}

	map<int,map<int,double> > allocation;//actual allocation requested data
        for(availabilityiterator it1=TaskAvailabilityTokens.begin(); it1!=TaskAvailabilityTokens.end(); ++it1)
	{
	        for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2)
	        {
                        allocation[it1->getReplyingProcessor()][it2->first]=0;
	        }
	}
	if(pf4)
	{
                cout<<" SURPLUS "<<endl;
                for(map<int,double>::iterator it=surplus.begin(); it!=surplus.end(); ++it)
                {	cout<<"Child id "<<it->first<<"  Surplus "<<it->second<<endl;	}

                cout<<" DEFICIT "<<endl;availabilityiterator ait_last = TaskAvailabilityTokens.end();
                for(map<int,double>::iterator it=deficit.begin(); it!=deficit.end(); ++it)
                {	cout<<"Child id "<<it->first<<"  Deficit "<<it->second<<endl;		}

		cout<<" Allocation Matrix "<<endl<<"\t";
		for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2){ cout<<"["<<it2->first<<"]\t";    }       cout<<endl;
		for(availabilityiterator it1=TaskAvailabilityTokens.begin(); it1!=TaskAvailabilityTokens.end(); ++it1)
	        {
                        cout<<"["<<it1->getReplyingProcessor()<<"]\t";
	                for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2)
	                {
                                cout<<allocation[it1->getReplyingProcessor()][it2->first]<<"\t";
	                }
			cout<<endl;
	        }

        }


	map<int,double>::iterator dit=deficit.begin();
	map<int,double>::iterator sit=surplus.begin();

        double d,s;
        s=sit->second;
        d=dit->second;

        //Calculate the allocation matrix
        while(  (dit!=deficit.end()) && (sit!=surplus.end())  )
        {	 if( s > d )
	        {	s = s - d;
		        allocation[sit->first][dit->first] = d;
		        dit++;			d=dit->second;
	        }
	        else if ( s < d)
	        {	d= d - s;
		        allocation[sit->first][dit->first] = s;
		        sit++;			s=sit->second;
	        }
	        else
	        {	allocation[sit->first][dit->first] = s;
		        sit++;			dit++;
		        s=sit->second;		d=dit->second;
	        }
        }

        if(pf5)
	{
		cout<<" Allocation Matrix "<<endl<<"\t";
		for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2){ cout<<"["<<it2->first<<"]\t";    }       cout<<endl;
		for(availabilityiterator it1=TaskAvailabilityTokens.begin(); it1!=TaskAvailabilityTokens.end(); ++it1)
	        {
                        cout<<"["<<it1->getReplyingProcessor()<<"]\t";
	                for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2)
	                {
                                cout<<allocation[it1->getReplyingProcessor()][it2->first]<<"\t";
	                }
			cout<<endl;
	        }

        }





	int task_avl_size=TaskAvailabilityTokens.size(); //Store this as it will change later
	availabilityiterator it1=TaskAvailabilityTokens.begin();

	for(int i=0; i<task_avl_size; ++i)
	{

	        for(childiterator it2=chld->begin(); it2!=chld->end(); ++it2)
		{
		        if(allocation[ it1->getReplyingProcessor() ] [ it2->first]  > 0.0001)
			{
		                double new_fraction =  (allocation[ it1->getReplyingProcessor() ] [ it2->first])/(it1->getWeight()*1.0);
				if(level>1)
				{
				        TaskAvailabilityToken tat(it1->getReplyingProcessor(),level-1,it1->getOriginLevel(),new_fraction,it1->getWeight());
					stapl::async_rmi(it2->first,getHandle(),&HLS_Type::AcceptTaskAvailabilityToken,tat);
				}
				if(level == 1 )
				{
                                        ScheduleRequest sr(it2->first,new_fraction,it1->getOriginLevel(),0);
					stapl::async_rmi(it1->getReplyingProcessor(),getHandle(),&HLS_Type::AcceptRequest,sr);
				}

			}
		}

		availabilityiterator itold = it1;

		++it1;

		if(pf6) {       cout<<" Erasing token "; itold->Print();        }
		TaskAvailabilityTokens.erase(itold);

	}




}

stapl::rmi_fence();

//PrintTaskTokens();

//PrintRequests();


}
