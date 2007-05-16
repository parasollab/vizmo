template<class pRange_Type>
HierarchicalLoadScheduler<pRange_Type>::HierarchicalLoadScheduler(pRange_Type * pr )
        : hls_thread()
{


        global_empty_flag = false;


	//Set the prange pointer
	prange = pr;

        //Set the Handle
        handle = stapl::register_rmi_object(this);


	SetHierarchy();
	Initialize_Child_Weights();
	Initialize_Child_Status();
	Initialize_Schedule_Requests();
	stapl::rmi_fence();


	scheduler_thread_created = false;
	scheduling_started = false;
	sch_loop_done_atleast_once = false;
	sch_thread_done = false;

	//initialize the lock
	pthread_mutex_init(&mutex_lock, NULL);

	//create scheduler thread
#ifdef _FULLY_FUNCTIONAL_PRANGE
	if( pthread_create(&hls_thread, NULL, Hierarchical_Load_Scheduler_Run,(void *) this ) != 0)
        {       cout << "pthread_create() error in HLScheduler constructor" << endl;
                abort();
        }


	while(!scheduler_thread_created)
	{

	        char c[100];
	        sprintf(c,"In STAPL_Thread %d Waiting for Scheduler Thread creation \n",stapl::get_thread_id());
                //cout<<c;
		//cout<<"In STAPL_Thread "<<<<" Waiting for Scheduler Thread creation "<<endl;
	}
#else
	
	//We are not going to create the scheduler thread
	scheduler_thread_created = true;
	this->Set_sch_loop_done_atleast_once(true);
	this->Set_Scheduler_Thread_Done(true);
	
	
#endif



}

//---------------------------------------------------------------------------------------------//

template<class pRange_Type>
void HierarchicalLoadScheduler<pRange_Type>::
SetHierarchy()
{

        int nodes_in_each_level = stapl::get_num_threads();// stapl::get_num_threads()/4 ; //Nodes in each level of the tree

        int x =(int) ( log((double)stapl::get_num_threads())/log((double)nodes_in_each_level) );
	if( stapl::get_num_threads() != (int)pow((double)nodes_in_each_level,x))	{	        x++;	}
        int levels = x+1;

	tree_num_levels = levels;

	for(int i=0; i<(tree_num_levels-1); ++i)
	{
	        int denom = (int)pow((double)nodes_in_each_level,i+1);
		int denom2 = (int)pow((double)nodes_in_each_level,i);

         	if(stapl::get_thread_id()%denom2 == 0 )
		{       int par= ((int)(stapl::get_thread_id()/denom))*denom;
		        parents[i]=par;
		}

		if(stapl::get_thread_id()%denom == 0 )
		{
		        map<int,double> chlds;
                        for(int j=0; j<nodes_in_each_level; ++j)
		        {

                                int newchild = 0;
	        	        newchild = stapl::get_thread_id()+(j*denom2);

				if(newchild < stapl::get_num_threads() )
				{
		        	        chlds[newchild]=0;
				}
			 }

		       children[i]=chlds;
		}

	}
}


template<class pRange_Type>
void HierarchicalLoadScheduler<pRange_Type>::
Initialize_Child_Weights()
{

	stapl::rmi_fence();

        //set the weights in the hierarchy
	for(int i=0; i<=(tree_num_levels-1); ++i)
	{

	        map<int,int>::iterator it=parents.find(i); //Find the parent in level i

		if(it!=parents.end() ) //If the parent in not null
		{
                        int parentid = it->second;
			double weight = 0;

                        if(i==0)
			{
			        //weight=log((double)stapl::get_thread_id()+1.0);
				weight=1;//(stapl::get_thread_id()+1);//*(stapl::get_thread_id()+1);

				
				local_weight = weight;

			}
			else
			{
			        //weight is the sum of weights of its children

                                childleveliterator it2= children.find(i-1); //The children in level below this;

                                if( it2 == children.end() ){    cout<<"Thread = "<<stapl::get_thread_id()<<"  Level(i) = "<<i<<" Error in HLS :: Set Hierarchy "<<endl;  cout<<" Child is not found "<<endl;     exit(-1);       }


                                for(childiterator it3 = it2->second.begin(); it3!=it2->second.end(); ++it3)
				{        weight +=it3->second;		}

			}

			//Send the weight to parent
			pair<int,pair<int,double> > x;//pair<level, pair<childid,weight> >


			x.first=i;
			x.second.first=stapl::get_thread_id();
			x.second.second=weight;
			stapl::async_rmi(parentid,getHandle(),&HLS_Type::SetChildWeight,x);

		}

		stapl::rmi_fence();
	}


}


template<class pRange_Type>
void HierarchicalLoadScheduler<pRange_Type>::
SetChildWeight(pair<int,pair<int,double> > x)
{
        int level =x.first;
	int child_id = x.second.first;
	double weight = x.second.second;

	childleveliterator it = children.find(level);
	if(it==children.end())	{       cout<<" Error in HLS::SetChildWeight "<<endl<<" Level of Children not found "<<endl;    exit(-1); }
	childiterator it2 = it->second.find(child_id);
	if(it2==it->second.end())       {      cout<<" Error in HLS::SetChildWeight"<<endl<<" Child not found in the level "<<endl;     exit(-1); }

	it2->second = weight;
}




//---------------------------------------------------------------------------------------------//

template<class pRange_Type>
void    HierarchicalLoadScheduler<pRange_Type>::
Initialize_Child_Status()
{       /*Initializes the child_status map */
	for(childleveliterator it1 = children.begin(); it1!=children.end(); ++it1)
	{
		map<int,pair<int,bool> > cstat;
		for(childiterator it2 = it1->second.begin(); it2!=it1->second.end(); ++it2)
		{
		        pair<int,bool> stat;
			stat.first=0;   //child ready task size = 0
			stat.second = false; //prange execution is not over
			cstat[ it2->first ] = stat;
		}
		child_status[it1->first]=cstat;
	}
}

//---------------------------------------------------------------------------------------------//

template<class pRange_Type>
void    HierarchicalLoadScheduler<pRange_Type>::
Initialize_Schedule_Requests()
{

	for(map<int,int>::iterator it=parents.begin(); it!=parents.end(); ++it)
	{
		ScheduleRequests[it->first];

		for(int i=it->first;  i<tree_num_levels-1; ++i)
		{
		        ScheduleRequests[it->first][i];
		}

	}


}
