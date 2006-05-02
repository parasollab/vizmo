


template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::Print()
{
//Prints the contents of the scheduler data structure
/*
         for(int i=0; i<stapl::get_num_threads(); ++i)
         {
               if(i == stapl::get_thread_id())
              {


                                cout<<" Printing Hierarchical Load Scheduler in Thread "<<stapl::get_thread_id()<<endl;
                                cout<<" Printing Ready Tasks(pRanges) \t";
				for(int j=0; j<readytasks.size(); ++j)
				{
				        cout<<readytasks[j]<<"  ";
				}
				cout<<endl;
				prange->SimplePrint();

                                if((i+1)==stapl::get_num_threads())
                                {
                                        cout<<"\n --------------------------------- "<<endl<<endl;
                                }
                                cout<<endl;
              }
                        stapl::rmi_fence();
        }
*/
}

//-------------------------------------------------------------------------------------------------//
template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::PrintHierarchyOnly()
{

	for(int k=0; k<stapl::get_num_threads(); ++k)
	{
		if( k== stapl::get_thread_id() )
		{
			cout<<"Thread "<<stapl::get_thread_id()<<endl;
                	for(map<int,int>::iterator it1=parents.begin();  it1!=parents.end(); ++it1)
			{
				cout<<"\tLevel "<<it1->first<<"   Parent "<<it1->second<<endl;
			}

			for(map<int, map<int,double> >::iterator it2 = children.begin();  it2 != children.end(); ++it2)
			{
			 	cout<<"\tLevel "<<it2->first<<"   Children : { ";
                                for( map<int,double>::iterator it3= it2->second.begin(); it3!=it2->second.end(); ++it3)
				{
				        cout<<it3->first<<"("<<it3->second<<")  ";
				}


				cout<<" } "<<endl;
			}

			if(k == (stapl::get_num_threads()-1))
				cout<<"\n\t-----------------------------------------------------------------"<<endl;
		}
		stapl::rmi_fence();
	}

}

//-------------------------------------------------------------------------------------------------//
template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::PrintChildStatus()
{

	for(int k=0; k<stapl::get_num_threads(); ++k)
	{
		if( k== stapl::get_thread_id() )
		{
			cout<<"Thread "<<stapl::get_thread_id()<<endl;

			for(statusleveliterator it1 = child_status.begin(); it1!=child_status.end(); ++it1)
			{
			        cout<<"\tLevel "<<it1->first<<endl;

				for(statusiterator it2 = it1->second.begin(); it2!=it1->second.end(); ++it2)
				{   cout<<"\t\tSize of Child["<<it2->first<<"] = "<<it2->second.first<<" ("<<it2->second.second<<") "<<endl;	}
			}

			if(k == (stapl::get_num_threads()-1))
				cout<<"\n\t-----------------------------------------------------------------"<<endl;
		}
		stapl::rmi_fence();
	}

}


//-------------------------------------------------------------------------------------------------//

template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::PrintHierarchy(bool b)
{
/*
	for(int k=0; k<stapl::get_num_threads(); ++k)
	{
		if( k== stapl::get_thread_id() )
		{
			cout<<"Thread "<<stapl::get_thread_id()<<endl;
                        if(b)
			{
			        cout<<"\tPrinting Ready Tasks(pRanges) \t";
			        for(int j=0; j<readytasks.size(); ++j)
			        {
			                cout<<readytasks[j]<<"  ";
        			}
	        		cout<<endl;
			}
                        cout<<"\tReady Size = "<<local_ready_size()<<endl;
			cout<<"\tPrange Size = "<<prange->local_size()<<endl;

			if(b)
			{	        cout<<"\t";     prange->SummaryPrint();			}

			cout<<"\tPrinting Machine Hierarchy "<<endl;


			for(map<int,int>::iterator it1=parents.begin();  it1!=parents.end(); ++it1)
			{
				cout<<"\tLevel "<<it1->first<<"   Parent "<<it1->second<<endl;
			}


			for(map<int, vector<int> >::iterator it2 = children.begin();  it2 != children.end(); ++it2)
			{
			        if(it2==children.begin())       cout<<endl;

				cout<<"\tLevel "<<it2->first<<"   Children : { ";
				for(int i=0; i<it2->second.size(); ++i)
				{
					cout<<it2->second[i]<<"  ";
				}
				cout<<" } "<<endl;
			}



			for(map<int,map<int,pair<int,bool> > > ::iterator it3 = child_status.begin();  it3 !=child_status.end(); ++it3)
			{
			        if(it3==child_status.begin())   cout<<endl;

				cout<<"\tStatLevel "<<it3->first<<endl;
				for(map<int,pair<int,bool> >::iterator it4 = it3->second.begin(); it4!=it3->second.end(); ++it4)
				{
					cout<<"\t\tSize of child["<<it4->first<<"]=  "<<it4->second.first<<"  pR_Done? = "<<it4->second.second<<endl;
				}
			}


			if(k == (stapl::get_num_threads()-1))
				cout<<"\n\t-----------------------------------------------------------------"<<endl;
		}
		stapl::rmi_fence();
	}
*/
}

//-------------------------------------------------------------------------------------------------//



template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::PrintRequests()
{


	for(int i=0; i <stapl::get_num_threads(); ++i)
	{

		if(i==stapl::get_thread_id())
		{
		        cout<<"Thread = "<<stapl::get_thread_id()<<endl;
			for(requestleveliterator it=ScheduleRequests.begin(); it!=ScheduleRequests.end(); ++it)
			{
				cout<<"   Buffered Requests in LEVEL "<<it->first<<endl;

				for(requestoriginleveliterator it2 = it->second.begin(); it2!=it->second.end(); ++it2)
				{

				        if(it2->second.size() > 0 )//Print only if... the size is > 0
					{
				                cout<<"\tOrigin Level = "<<it2->first<<endl;

				                for(requestiterator it1 = it2->second.begin();  it1!=it2->second.end(); ++it1)
        				        {
	        				        cout<<"\t\t";it1->Print();
		        		        }
					}
				}
			}
			cout<<endl;
		}
		stapl::rmi_fence();
	}

}


//-------------------------------------------------------------------------------------------------//

template<class pRange_Type> void HierarchicalLoadScheduler<pRange_Type>::PrintTaskTokens()
{


	for(int i=0; i <stapl::get_num_threads(); ++i)
	{

	        if(i==stapl::get_thread_id())
		{
	        	for(availabilityiterator ait=TaskAvailabilityTokens.begin(); ait!=TaskAvailabilityTokens.end(); ++ait)
        	        {       ait->Print();	}

			cout<<endl;
		}
		stapl::rmi_fence();
	}


}
//-------------------------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------------//

