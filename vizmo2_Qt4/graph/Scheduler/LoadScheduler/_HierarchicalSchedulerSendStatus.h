template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
SendStatusToParent()
{

        for(int L=0; L<tree_num_levels-1; ++L)
	{       //Work on each level in the hierarchy

	        pair<int, pair<int,pair<int,bool> > > stat;  //pair<level, pair<childid, pair<size,prangeover> > >;
		stat.first = L; //level
		stat.second.first = stapl::get_thread_id(); //child_id

	        if(L==0)
		{
		        //Level 0 Leaf

		        stat.second.second.first = local_ready_size();
        	        stat.second.second.second = prange->get_ddg().finished();

			int parentid = parents[L];
	                stapl::async_rmi(parentid,getHandle(),&HLS_Type::UpdateStatus,stat);
        	}
		else
		{
		        map<int,int>::iterator  p_iter = parents.find(L-1);

			if( (p_iter != parents.end()) && (stapl::get_thread_id() == p_iter->second) ) //If I am the parent of the previous level
			{

                                childleveliterator it=children.find(L-1); //Find my children in the lower level
                                if(it == children.end() )
                                {
                                        cout<<"!!! Error !!! in HierarchicalLoadScheduler::SendStatusToParent"<<endl;
	                                cout<<"Children in Level "<<(L-1)<<" not found "<<endl;
	                                exit(-1);

                                }


                                int childsum = 0;       //Compute the total size of children
                                bool ddgfinished = true; //ddgfinished in my subtree

				for(childiterator it1=it->second.begin(); it1!=it->second.end(); ++it1)
				{
				        int childid=it1->first;
					childsum +=child_status[L-1][childid].first;
					ddgfinished = ddgfinished && child_status[L-1][childid].second;
				}


		                stat.second.second.first = childsum;
		                stat.second.second.second = ddgfinished;

				map<int,int>::iterator p2_iter=parents.find(L);
                                if(p2_iter == parents.end() ) { cout<<" Fatal error in SendStatusToParent "<<endl;
				                                cout<<" Parent in Level L = "<<L<<" does not exist in Thread "<<stapl::get_thread_id()<<endl;
				                                exit(-1);
				                              }
		                int parentid = p2_iter->second;
	                        stapl::async_rmi(parentid,getHandle(),&HLS_Type::UpdateStatus,stat);
			}

		}

                stapl::rmi_fence();
	}




}

//------------------------------------------------------------------------------------//

template<class pRange_Type>
int
HierarchicalLoadScheduler<pRange_Type>::
local_ready_size()
{
        getLock(5);
        
		int sz = 0; //size of ready tasks
        for(int i=0; i<readytasks.size(); ++i)
        {
                sz += prange->get_subrange(readytasks[i]).get_size();
		}
        releaseLock(5);
		
		return sz;
}

//------------------------------------------------------------------------------------//


template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
UpdateStatus( pair<int , pair<int,pair<int,bool> > > arg)
{
	UpdateStatus(arg.first, arg.second.first, arg.second.second.first,arg.second.second.second);
}

template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::
UpdateStatus(int level,int child,int new_size,bool prangeover)
{
	if(child_status.find(level)!=child_status.end())
	{
		if(child_status[level].find(child)!=child_status[level].end())
		{
			child_status[level][child].first=new_size;
			child_status[level][child].second=prangeover;
		}
		else
		{
			cout<<"Error: In Thread "<<stapl::get_thread_id()<<" DistributedScheduler::UpdateStatus() "<<endl;
			cout<<"Child does not exist in Map "<<endl;
			system("touch error_ds_update_status");
			exit(-1);

		}
	}
	else
	{
		cout<<"Error: In Thread "<<stapl::get_thread_id()<<" DistributedScheduler::UpdateStatus() "<<endl;
		cout<<"In Thread "<<stapl::get_thread_id()<<" Level = "<<level<<" chilid = "<<child<<" new_size = "<<new_size<<" prangeover = "<<prangeover<<endl;
		cout<<"Map level does not exist "<<endl;
		system("touch error_ds_update_stat_map_level");
		exit(-1);
	}
}


//----------------------------------------------------------------------------------------------//
