template<class pRange_Type>
void
HierarchicalLoadScheduler<pRange_Type>::push(std::vector<int> newtasks)
{
        getLock(1);
        scheduling_started = true;
	for(int i=0; i<newtasks.size(); ++i)
	{			readytasks.push_back(newtasks[i]);		}
	releaseLock(1);
}

template<class pRange_Type>
std::vector<int>
HierarchicalLoadScheduler<pRange_Type>::pop()
{
        std::vector<int> rtn;
	//Wait till the scheduling is done atleast once.
    if(sch_loop_done_atleast_once==false)
	{                        return rtn;		}

    //Calculate the fraction of local tasks that has to be popped
	int lsize = local_ready_size();
    double pop_fraction = 0.2; //Fraction of local tasks going to be popped.
	int popsize = (int)(((double)lsize) * pop_fraction );
	int pop_min = 6; //minimum number of tasks to be popped

	if(popsize < pop_min)
	        popsize = pop_min;

	/*
		//Uncomment if you always want to pop only one task
	        if(popsize > 0 )
	        {
	                popsize = 1;
        	}
	*/
	
        //cout<<"In Thread "<<stapl::get_thread_id()<<" lsize = "<<lsize<<" popsize = "<<popsize<<endl;
	getLock(2);

	while( (popsize > 0 ) && (readytasks.size() > 0 ) )
	{       
			int to_pop = readytasks[readytasks.size()-1];
            popsize -= prange->get_subrange(to_pop).get_size();
            rtn.push_back(to_pop);
	        readytasks.pop_back();
    }
	releaseLock(2);

	return rtn;
}
