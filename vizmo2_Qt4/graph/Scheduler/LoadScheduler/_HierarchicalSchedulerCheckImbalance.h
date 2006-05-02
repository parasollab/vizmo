template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
CheckChildrenImbalance()
{
//////////////OLD ONE

	for(map<int, vector<int> >::iterator it1 = children.begin();	it1 != children.end(); ++it1)
	{
		if( it1->first != 0)
		{
			cout<<" Not Supported yet in CheckChildrenImbalance "<<endl;
			system("touch error_ds_check_children_imbalancee");
			exit(-1);
		}
		int level=it1->first;

		vector<int> *ch = &it1->second;
		int childsum = 0;
		for(int i=0; i<(*ch).size(); ++i)
		{
			int childid=(*ch)[i];
			childsum += child_status[level][childid].first;
		}
		double avg = (childsum*1.0) / (stapl::get_num_threads()*1.0);


		if(childsum == 0 )
		{
                        for(int i=0; i<(*ch).size(); ++i)
		        {
			        int childid=(*ch)[i];
				stapl::async_rmi(childid,getHandle(),&HLS_Type::set_global_empty_flag,true);
		        }
		}
		//cout<<" Sum = "<<childsum<<endl;
		//cout<<" Average = "<<avg<<endl;

		map<int,map<int,double> > allocation; //re - allocation matrix
		map<int,double> surplus;
		map<int,double> deficit;

		//Calculate surplus and deficit tasks

		bool redistribute = false;

		for(int i=0; i<(*ch).size(); ++i)
		{	double diff = avg - child_status[level][ (*ch)[i] ].first;


			double stat_avg_ratio = child_status[level][ (*ch)[i] ].first / avg ;

			if(avg == 0 )
			        stat_avg_ratio = 1;

			//cout<<" Child Status ["<<(*ch)[i]<<"] = "<<child_status[level][ (*ch)[i] ]<<"   stat_avg_ratio = "<<stat_avg_ratio <<endl;

                        if(stat_avg_ratio<0.25) redistribute = true;


			if(diff > 0)	{	deficit[(*ch)[i]]=diff;		}
			else if (diff < 0){	surplus[(*ch)[i]]=-diff;	}
			else	{	surplus[(*ch)[i]]=0;		}
			for(int j=0; j<(*ch).size(); ++j)
			{		allocation[ (*ch)[i] ] [ (*ch)[j] ]= 0;			}
		}


		double deftotal = 0;
		for(map<int,double>::iterator it=deficit.begin(); it!=deficit.end(); ++it)
		{	deftotal+=it->second;	}
		//cout<<" Total deficit = "<<deftotal<<endl;

		//Dont redistribute if there is no great difference in child loads
		if(redistribute == false )
		{
		        //cout<<" Ignoring the deficit "<<endl;
		        continue;
		}




		/*cout<<" SURPLUS "<<endl;
		for(map<int,double>::iterator it=surplus.begin(); it!=surplus.end(); ++it)
		{	cout<<"Child id "<<it->first<<"  Surplus "<<it->second<<endl;	}

		cout<<" DEFICIT "<<endl;
		for(map<int,double>::iterator it=deficit.begin(); it!=deficit.end(); ++it)
		{	cout<<"Child id "<<it->first<<"  Deficit "<<it->second<<endl;		}
                */

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

		for(int i=0; i<(*ch).size(); ++i)
		{
			double surp=0;
			for(int j=0; j<(*ch).size(); ++j)
			{	surp+=allocation[ (*ch)[i] ] [ (*ch)[j] ];			}

			map<int,double>::iterator it=surplus.find((*ch)[i]);
			double rsurp=0;
			if(it != surplus.end() ){	rsurp = it->second;		}
			//cout<<"Surp = "<<surp<<" Rsurp = "<<rsurp<<endl;
			if(fabs(rsurp-surp)>0.01){	cout<<"Debug error 1 "<<endl; 			system("touch error_check_children_imb_deb1");	exit(-1);	}

		}

		for(int j=0; j<(*ch).size(); ++j)
		{	double defi=0;
			for(int i=0; i<(*ch).size(); ++i){defi+=allocation[ (*ch)[i] ] [ (*ch)[j] ];}

			map<int,double>::iterator it=deficit.find((*ch)[j]);
			double rdefi=0;
			if(it != deficit.end() ){rdefi = it->second;}
			//cout<<"Defi = "<<defi<<"  RDefi = "<<rdefi<<endl;
			if(fabs(defi-rdefi)>0.01){cout<<"Debug error 2 "<<endl;			system("touch error_deb2_check_children_imbalance");	exit(-1);}

		}
		//

                /*
                cout<<endl;
		cout<<"\t";	for(int i=0; i<(*ch).size(); ++i){cout<<"["<<(*ch)[i]<<"]\t";}	cout<<"Surplus"<<endl;

		for(int i=0; i<(*ch).size(); ++i)
		{	cout<<"["<<(*ch)[i]<<"]\t";

			double surp=0;
			for(int j=0; j<(*ch).size(); ++j)
			{
				cout<<allocation[ (*ch)[i] ] [ (*ch)[j] ]<<"\t";
				surp+=allocation[ (*ch)[i] ] [ (*ch)[j] ];
			}
			cout<<surp<<endl;
		}
                */

		int redistmessagecount = 0;
		for(int i=0; i<(*ch).size(); ++i)
		{

			for(int j=0; j<(*ch).size(); ++j)
			{
				if(allocation[ (*ch)[i] ] [ (*ch)[j] ] > 0.0001)
				{
					double fraction = (allocation[ (*ch)[i] ] [ (*ch)[j] ])/(1.0*child_status[level][(*ch)[i]].first);
					ScheduleRequest sr((*ch)[j],fraction,level);
					stapl::async_rmi((*ch)[i],getHandle(),&HLS_Type::AcceptRequest,sr);

					redistmessagecount++;
				}
			}
		}

		//cout<<" Redistibution Messages Count = "<<redistmessagecount<<endl;

		for(int i=0; i<(*ch).size(); ++i)
		{	for(int j=0; j<(*ch).size(); ++j)
			{
				double alloc= (int) ( allocation[ (*ch)[i] ] [ (*ch)[j] ] );

				child_status[level][(*ch)[i]].first -=(int) alloc;
				child_status[level][(*ch)[j]].first +=(int) alloc;

			}

		}

                /*
		for(map<int,map<int,int> > ::iterator it3 = child_status.begin();  it3 !=child_status.end(); ++it3)
		{
			cout<<"\tExpected StatLevel After Satisfying Requests  "<<it3->first<<endl;
			for(map<int,int>::iterator it4 = it3->second.begin(); it4!=it3->second.end(); ++it4)
			{
				cout<<"\t\tSize of child["<<it4->first<<"]=  "<<it4->second<<endl;
			}
		}
                */




	}
}



//----------------------------------------------------------------------------------------------//
template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
AcceptRequest(ScheduleRequest sr)
{


	if(sr.getLevel() != 0)
	{
		cout<<"Level not ready yet in Accept Request "<<endl;
			system("touch error_accept_request");
		exit(-1);
	}
	map<int, list<ScheduleRequest> >::iterator it = ScheduleRequests.find(sr.getLevel());

	if(it == ScheduleRequests.end() )
	{

		cout<<"Error in DistributedScheduler::AcceptRequest "<<endl;
			system("touch error_accept_req_error2");
		exit(-1);
	}
	it->second.push_back(sr);
}

//----------------------------------------------------------------------------------------------//



