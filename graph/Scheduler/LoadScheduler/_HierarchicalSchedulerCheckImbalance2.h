template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
CheckChildrenImbalance()
{
bool pf1,pf2,pf3,pf4,pf5;
pf1=false;
pf2=false;
pf3=false;
pf4=false;
pf5=false;



//////////////NEW ONE...............



for(int level=tree_num_levels-2; level>=0; --level)
{
        //Checks load imbalance top down.



	map<int,int>::iterator parent_iterator = parents.find(level);

	if( (parent_iterator!=parents.end() && (stapl::get_thread_id() == parent_iterator->second ) ) )
	{
	        //I am parent of Level L-1

		if(pf1) cout<<" Thread "<<stapl::get_thread_id()<<" is a parent of Level = "<<level<<endl;

		childleveliterator ch_lvl_it = children.find(level);

		if(ch_lvl_it==children.end())
		{       cout<<" Error in HierarchicalLoadScheduler::CheckChildrenImbalance "<<endl;
			cout<<" Did not find any information about children in level "<<level<<endl;
			exit(-1);
		}




		map<int,double> *chln = &ch_lvl_it->second;

                double childsum = 0;
		double weightedchildsum = 0;
		double sumchildweights = 0; //sum of child weights
                bool ddgfinished = true;        //ddgfinished in my subtree

		if(pf1)
		{
                        cout<<" Children in this level are "<<endl;
                        for(childiterator it=chln->begin(); it!=chln->end(); ++it)
		        {
		                cout<<" ID="<<it->first<<" Size= "<<child_status[level][it->first].first<<" Weight= "<<it->second<<endl;
		        }
		}


		for(childiterator it=chln->begin(); it!=chln->end(); ++it)
		{
		        int childid=it->first;
			childsum +=  (child_status[level][childid].first*1.0);
			weightedchildsum += ( (child_status[level][childid].first*1.0) / (1.0*children[level][childid])); //Divide the childsum with the weight of the child
			sumchildweights +=  children[level][childid];
			ddgfinished = ddgfinished && child_status[level][childid].second;
                }

		if(pf2)
		{
		        cout<<"In thread "<<stapl::get_thread_id()<<" ChildSum = "<<childsum<<endl;
		        cout<<"In thread "<<stapl::get_thread_id()<<" Weightedchildsum = "<<weightedchildsum<<endl;
		        cout<<"In thread "<<stapl::get_thread_id()<<" SumChildWeights = "<<sumchildweights<<endl;
		        cout<<"In thread "<<stapl::get_thread_id()<<" DDgfinished ? = "<<ddgfinished<<endl;
                }
                double wavg = (weightedchildsum*1.0)/(chln->size()*1.0);
		if(pf2) cout<<"In thread "<<stapl::get_thread_id()<<" Weighted average = "<<wavg<<endl;

		double twavg = ( childsum * 1.0 )/(sumchildweights * 1.0 ); //Targetted weighted average
		if(pf2) cout<<"In thread "<<stapl::get_thread_id()<<" Targetted weighted average = "<<twavg<<endl;



		if(level==(tree_num_levels-2))
		{
		        if(pf2)
        		{
	        	        cout<<"In thread "<<stapl::get_thread_id()<<" Child Sum = "<<childsum<<endl;
		        	cout<<"In thread "<<stapl::get_thread_id()<<" Weighted Child Sum = "<<weightedchildsum<<endl;
		        }

		        if(childsum == 0 )
		        {
			        if(pf2) cout<<" Level = "<<level<<endl;
				pair<int,bool>  x;
				x.first=level;
				x.second=true;
				set_global_empty_flag(x);

        			if(false)
			        for(childiterator it=chln->begin(); it!=chln->end(); ++it)
			        {

			                int childid=it->first;
					pair<int,bool>  x;
					x.first=level;
					x.second=true;

					set_global_empty_flag(x) ;
			                //stapl::async_rmi(childid,getHandle(),&HLS_Type::set_global_empty_flag,x);
		                }

		        	goto Skip1;
		        }
		}


                map<int,map<int,double> > allocation; //re - allocation matrix
		map<int,double> surplus;
		map<int,double> deficit;

		bool redistribute = false;

                for(childiterator it=chln->begin(); it!=chln->end(); ++it)
		{       int childid=it->first;
			double stat_avg_ratio = ((child_status[level][childid].first*1.0)/(1.0*children[level][childid])) / wavg ;

			if(wavg == 0 )
			        stat_avg_ratio = 1;

			if(pf2) cout<<"In Thread "<<stapl::get_thread_id()<<"\tChildStatus["<<childid<<"]= "<<child_status[level][childid].first<<"\tChWt= "<<children[level][childid]<<"\twavg= "<<wavg<<"\tstat_avg_ratio= "<<stat_avg_ratio <<endl;

			if(stat_avg_ratio<0.25) redistribute = true;


			double diff = (twavg*it->second) - child_status[level][childid].first;



			if(diff > 0)	{	deficit[childid]=diff;		}
			else if (diff < 0){	surplus[childid]=-diff;	}
			else	{	surplus[childid]=0;		}


			for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
			{		allocation[ it->first ] [ it2->first ] = 0;			}

		}
		//Dont redistribute if there is no great difference in child loads

        	if(redistribute == false )
	        {
	                cout<<"In thread "<<stapl::get_thread_id()<<" IGNORING the imbalance in level "<<level<<endl;
	                goto Skip1;
	        }
		else
		{
		        cout<<"In thread "<<stapl::get_thread_id()<<" REDISTRIBUTING data in Level "<<level<<endl;
		}


		if(pf3)
		{
	                cout<<" SURPLUS "<<endl;
	                for(map<int,double>::iterator it=surplus.begin(); it!=surplus.end(); ++it)
	                {	cout<<"Child id "<<it->first<<"  Surplus "<<it->second<<endl;	}

	                cout<<" DEFICIT "<<endl;
	                for(map<int,double>::iterator it=deficit.begin(); it!=deficit.end(); ++it)
	                {	cout<<"Child id "<<it->first<<"  Deficit "<<it->second<<endl;		}

                        cout<<" Allocation Matrix "<<endl<<"\t";
		        for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1){  cout<<"["<<it1->first<<"]\t";}  cout<<endl;
                        for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
                        {       cout<<"["<<it1->first<<"]\t";
                                for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
	        	        {       cout<<allocation[it1->first][it2->first]<<"\t";       }
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
		{	if( s > d )
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

		if(pf4)
		{
                        cout<<" Allocation Matrix "<<endl<<"\t";
		        for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1){  cout<<"["<<it1->first<<"]\t";}  cout<<endl;
                        for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
                        {       cout<<"["<<it1->first<<"]\t";
                                for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
		                {       cout<<allocation[it1->first][it2->first]<<"\t";       }
		                cout<<endl;
	                }

                }

		for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
		{	double surp=0;
			for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
			{	surp+=allocation[ it1->first ] [ it2->first ];			}
			map<int,double>::iterator it=surplus.find(it1->first);
			double rsurp=0;
			if(it != surplus.end() ){	rsurp = it->second;		}
			//cout<<"Surp = "<<surp<<" Rsurp = "<<rsurp<<endl;
			if(fabs(rsurp-surp)>0.01){	cout<<"Debug error 1 "<<endl; 			system("touch error_check_children_imb_deb1");	exit(-1);	}
		}

		for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
		{	double defi=0;
		        for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
			{       defi+=allocation[(int) it1->first ] [(int) it2->second ];      }
			map<int,double>::iterator it=deficit.find((int)it2->second);
			double rdefi=0;
			if(it != deficit.end() ){rdefi = it->second;}
			//cout<<"Defi = "<<defi<<"  RDefi = "<<rdefi<<endl;
			if(fabs(defi-rdefi)>0.01){cout<<"Debug error 2 "<<endl;			system("touch error_deb2_check_children_imbalance");	exit(-1);}
		}


		int redistmessagecount = 0;
		for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
		{

		        for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
			{


				if(allocation[ it1->first ] [ it2->first]  > 0.0001)
				{
					double fraction = (allocation[ it1->first ] [ it2->first ])/(1.0*child_status[level][ it1->first ].first);

					if(pf5)
					{
					        cout<<" numerator = "<<allocation[ it1->first ] [ it2->first ]<<endl;
					        cout<<" denominator = "<<child_status[level][ it1->first ].first<<endl;
					        cout<<" fraction = "<<fraction<<endl;
					}
					ScheduleRequest sr(it2->first,fraction,level,level);
					stapl::async_rmi(it1->first,getHandle(),&HLS_Type::AcceptRequest,sr);

					redistmessagecount++;
				}
			}
		}

		//cout<<" Redistibution Messages Count = "<<redistmessagecount<<endl;

		for(childiterator it1=chln->begin(); it1!=chln->end(); ++it1)
		{
		        for(childiterator it2=chln->begin(); it2!=chln->end(); ++it2)
			{
				double alloc= (int) ( allocation[ it1->first ] [ it2->first ] );

				child_status[level][it1->first].first -=(int) alloc;
				child_status[level][it2->first].first +=(int) alloc;

			}

		}



	}//ifstatement end




        int p;
	Skip1:  p=0;


	stapl::rmi_fence();
	set_local_global_empty_flag(stapl::sync_rmi(0,getHandle(),&HLS_Type::get_global_empty_flag));
	stapl::rmi_fence();

	//cout<<" In Thread "<<stapl::get_thread_id()<<" ggf = "<<get_global_empty_flag()<<endl;
        if(get_global_empty_flag())
	        break;

	if(level != 0 )
	{
	        map<int,int>::iterator parent_iterator2 = parents.find(level-1);
	        if( (parent_iterator2!=parents.end() && (stapl::get_thread_id() == parent_iterator2->second ) ) )
	        {        	ProcessBufferedRequestInfo(level);         	}
	}

	stapl::rmi_fence();


        //PrintChildStatus();
}//for loop over levels end

//Now we have sent out all the requests for scheduling... we should not find the appropriate processors for satisfying these requests
//PrintRequests();

if(get_global_empty_flag())
        return;

bool pf6,pf7,pf8,pf9;
pf6=pf7=pf8=pf9 = false;
pf6 = true;


requestleveliterator rlit=ScheduleRequests.find(0);     //Find requests in level 0
if(rlit==ScheduleRequests.end())
{
        cout<<" Error in HLS::Check Imbalance "<<endl;
	cout<<" Scheduler Requests in Level 0 is not found "<<endl;
	exit(-1);
}


stapl::rmi_fence();
for(int level=tree_num_levels-2; level>0; --level)
{
        //Now the requests are with the leaf nodes..in level 0... send them each to the origin node....

        requestoriginleveliterator rolit=rlit->second.find(level);
	if(rolit==rlit->second.end())
	{
	        cout<<" Error in HLS::Check Imbalance "<<endl;
		cout<<" Scheduler Requests in Origin Level "<<level<<" not found "<<endl;
		exit(-1);
	}
	list<ScheduleRequest> * requests = &rolit->second;

	double total_fraction_send = 0;
	for(requestiterator rit=requests->begin(); rit!=requests->end(); ++rit)
	{
	        if(rit->getOriginLevel() != level )
		{      cout<<" Error... Origin info mismatch in the the map and the stored information "<<endl;
        	       exit(-1);
		}



		TaskAvailabilityToken tat(stapl::get_thread_id(),rit->getOriginLevel(),rit->getOriginLevel(),rit->getFraction(),local_weight);

		stapl::async_rmi(rit->getTargetPid(),getHandle(),&HLS_Type::AcceptTaskAvailabilityToken,tat);



	}




	rolit->second.clear();

        stapl::rmi_fence();


	for(int k=level; k>=1; --k )
	{
                ProcessTaskAvailabilityTokens(k);
	}

        stapl::rmi_fence();


}//second for loop over levels end




}//function end


template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
AcceptTaskAvailabilityToken(TaskAvailabilityToken at)
{

        TaskAvailabilityTokens.push_back(at);
}

//----------------------------------------------------------------------------------------------//
template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
AcceptRequest(ScheduleRequest sr)
{
	requestleveliterator it = ScheduleRequests.find(sr.getLevel());

	if(it == ScheduleRequests.end() )
	{

	       cout<<"Error in HLS::AcceptRequest ::Level not found"<<endl;
	       system("touch error_accept_req_error2");
	        exit(-1);
	}

	requestoriginleveliterator it2 = it->second.find(sr.getOriginLevel());

	if(it2 == it->second.end() )
	{
	        cout<<"Error in HLS::AcceptRequest :: OriginLevel not found "<<endl;
		exit(-1);
	}

        it2->second.push_back(sr);
}

//----------------------------------------------------------------------------------------------//
