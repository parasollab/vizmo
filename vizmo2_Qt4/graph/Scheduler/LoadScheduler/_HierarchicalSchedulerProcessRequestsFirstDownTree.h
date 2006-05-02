
template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
ProcessBufferedRequestInfo(int level)
{




        bool pf1,pf2,pf3,pf4,pf5,pf6;

	pf1=false;
	pf2=false;
	pf3=false;
	pf4=false;
	pf5=false;
	pf6=false;


        pf1=pf2=pf3=pf4=pf5=pf6=false;

	if(pf1) cout<<" Processing Buffered Request Info  in Thread "<<stapl::get_thread_id()<<"  Level = "<<level<<endl;
	requestleveliterator rlit = ScheduleRequests.find(level);
	if(rlit==ScheduleRequests.end())
	{      cout<<"In thread "<<stapl::get_thread_id()<<" Error :: Count not find level "<<level<<" in ScheduleRequests "<<endl;		exit(-1);	}

        childleveliterator ch_lvl_it = children.find(level-1);
        if(ch_lvl_it == children.end() ){ cout<<" Error in ProcessBufferedRequests"<<endl;      cout<<" Could not find the child in level "<<(level-1)<<endl;	exit(-1);}
	map<int,double> * chld= &ch_lvl_it->second;
        if(pf1) for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1){     cout<<" Child ID = "<<it1->first<<" Weight = "<<it1->second<<endl;	}



	for(int origlevel=tree_num_levels-2; origlevel>=level; --origlevel)
	{
	        if(pf2)
		        cout<<">In Thread "<<stapl::get_thread_id()<<"  Level = "<<level<<"  origlevel = "<<origlevel<<"<"<<endl;


	        requestoriginleveliterator rolit=rlit->second.find(origlevel);
		if(rolit==rlit->second.end())
		{       cout<<" In Thread "<<stapl::get_thread_id()<<" Error:: Origlevel = "<<origlevel<<" not found in schedule requests "<<endl;
			exit(-1);
		}



		list<ScheduleRequest> * requests = &rolit->second;
                if(requests->size()==0)
	        {
		        if(pf2) cout<<"In thread "<<stapl::get_thread_id()<<" No requests here ...."<<endl;
			if(pf2) cout<<"\n--------------------------------------------"<<endl;
		        continue;
		}


		if(pf3)
		{
		        cout<<"Printing requests "<<endl;
	                for(requestiterator it1=requests->begin(); it1!=requests->end(); ++it1)	{    it1->Print();	}
                }

		double childweightsum = 0;
                for(childiterator it=chld->begin(); it!=chld->end(); ++it)
	        {
	                int childid=it->first;
		        childweightsum +=  it->second*1.0;
                }
		if(childweightsum == 0 )
	        {
		        if(pf3) cout<<"childweightsum = 0"<<endl;
		        if(pf3) cout<<"\n--------------------------------------------"<<endl;
		        continue;
		}

	        if(pf4) cout<<" ChildweightSum = "<<childweightsum<<endl;

		double totalfractionrequested=0;
        	for(requestiterator rit=requests->begin(); rit!=requests->end(); ++rit)
	        {
	                totalfractionrequested+=rit->getFraction();
	        }
	        if(pf4) cout<<"Total fraction requested = "<<totalfractionrequested<<endl;

	        map<int,double> deficit;//corresponding to request;
	        for(requestiterator rit=requests->begin(); rit!=requests->end(); ++rit)
	        {	        deficit[rit->getTargetPid()]=rit->getFraction()*childweightsum;	}

	        map<int,double> surplus;//corresponding to children
                for(childiterator it=chld->begin(); it!=chld->end(); ++it)
	        {       int childid = it->first;
	                surplus[childid]=it->second*totalfractionrequested; //The amount that is going to be transferred
	        }

	        map<int,map<int,double> > allocation;//actual allocation of data

		for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1)
                {
                        for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2)
                        {       allocation[it1->first][it2->getTargetPid()]=0;  }

                }

		if(pf5)
		{
                        cout<<" SURPLUS "<<endl;
                        for(map<int,double>::iterator it=surplus.begin(); it!=surplus.end(); ++it)
                        {	cout<<"Child id "<<it->first<<"  Surplus "<<it->second<<endl;	}

                        cout<<" DEFICIT "<<endl;
                        for(map<int,double>::iterator it=deficit.begin(); it!=deficit.end(); ++it)
                        {	cout<<"Child id "<<it->first<<"  Deficit "<<it->second<<endl;		}

                        cout<<" Allocation Matrix "<<endl<<"\t";
	                for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2){  cout<<"["<<it2->getTargetPid()<<"]\t";}  cout<<endl;
                        for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1)
                        {       cout<<"["<<it1->first<<"]\t";
                                for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2)
                                {       cout<<allocation[it1->first][it2->getTargetPid()]<<"\t";       }
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

                if(pf6)
		{
	                cout<<" Allocation Matrix "<<endl<<"\t";
	                for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2){  cout<<"["<<it2->getTargetPid()<<"]\t";}  cout<<endl;
                        for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1)
                        {       cout<<"["<<it1->first<<"]\t";
                                for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2)
                                {       cout<<allocation[it1->first][it2->getTargetPid()]<<"\t";       }
                                cout<<endl;
                        }
                }

		for(childiterator it1=chld->begin(); it1!=chld->end(); ++it1)
                {
                        for(requestiterator  it2=requests->begin(); it2!=requests->end(); ++it2)
		        {
		                if(allocation[ it1->first ] [ it2->getTargetPid()]  > 0.0001)
			        {
		                        double fraction =  (allocation[ it1->first ] [ it2->getTargetPid()])/(1.0*it1->second);
			                ScheduleRequest sr(it2->getTargetPid(),fraction,it2->getOriginLevel(),level-1);
				        stapl::async_rmi(it1->first,getHandle(),&HLS_Type::AcceptRequest,sr);

					if(pf6) sr.Print();
			        }
		        }
	        }


	        rolit->second.clear();



		if(pf6) cout<<"\n--------------------------------------------"<<endl;
	}

	if(pf6) cout<<"\n\n*****************************************"<<endl;



}
