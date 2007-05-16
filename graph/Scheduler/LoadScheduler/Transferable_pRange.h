 #ifndef _Trans_pRange_h_
 #define _Trans_pRange_h_




 
 #include <hash_map>

template<class BOUNDARY,class DDGTYPE,class PCONTAINER>
class Transferable_pRange
{
private:

	PCONTAINER * pc;
	std::hash_map<int,BOUNDARY> subranges;
	int gid_generator;
	stapl::rmiHandle handle;
	DDGTYPE ddg;
	typedef typename std::hash_map<int,BOUNDARY>::iterator subrangeiterator;

	typedef Transferable_pRange<BOUNDARY,DDGTYPE,PCONTAINER>	pRange_Type;

public:
	typedef BOUNDARY BOUNDARY_TYPE;

	DDGTYPE& get_ddg(){	return ddg; };
	stapl::rmiHandle getHandle()	{	return handle;	}
	PCONTAINER * get_pcontainer()   {       return pc; }

	Transferable_pRange(PCONTAINER * PC)
	{	pc = PC;
		gid_generator = stapl::get_thread_id();;
		handle = stapl::register_rmi_object(this);
	}

	BOUNDARY_TYPE & get_subrange( int id)
	{
		typename std::hash_map<int,BOUNDARY>::iterator it = subranges.find(id);
		if(it == subranges.end())
		{        
				cout<<" Error, appropriate Sub Range ( ID = "<<id<<" )not found in Transferable_pList::get_subrange()"<<endl;
				exit(-1);
		}
		
		
		return it->second;
		
	}
	

	void add_subrange(BOUNDARY_TYPE boundary)
	{
		
		boundary.SetID(gid_generator);
		subranges[gid_generator] = boundary;
		ddg.AddTask(boundary.GetID());
		

		gid_generator+= stapl::get_num_threads();
	}

	

	void Print()
	{	stapl::rmi_fence();
		for(int i=0; i<stapl::get_num_threads(); ++i)
		{
			if(i == stapl::get_thread_id())
			{
				cout<<" pRange in Thread "<<stapl::get_thread_id()<<endl;
				for(subrangeiterator it= subranges.begin();     it !=subranges.end();	it++)
				{
					it->second->Print();
				}
				ddg.SimplePrint();
				cout<<" GID Generator = "<<gid_generator<<endl;
				if((i+1) == stapl::get_num_threads() )
				{	cout<<" ----------------------------------- "<<endl<<endl; }
				else
					cout<<endl;
			}
			stapl::rmi_fence();
		}
	}


	int local_size()
	{       int sz = 0;
		
        	for(subrangeiterator it= subranges.begin();	                it !=subranges.end();			it++)
			{	
					sz +=it->second.get_size();		
			}
			return sz;
	}

	void SimplePrint()
	{
		cout<<" pRange in Thread "<<stapl::get_thread_id()<<endl;
		for(subrangeiterator it= subranges.begin();       it !=subranges.end(); 	it++)
		{			it->second.Print();		}

        	ddg.SimplePrint();
		cout<<" GID Generator = "<<gid_generator<<endl;
	}


	void SummaryPrint()
	{        cout<<"pRange in Thread{"<<stapl::get_thread_id()<<"} --> ";
		for(subrangeiterator it= subranges.begin();        it !=subranges.end();	it++)
		{		        cout<<it->second.GetID()<<"("<<it->second.size()<<")  ";		}
                cout<<endl;
	}


	void SendSubRange(int id,int _pid)
	{

		here(1);
		
		if(_pid == stapl::get_thread_id() )
		{       //nothing to do ... return
			return;
		}

        subrangeiterator it  = subranges.find(id);
		if(it==subranges.end())
		{	cout<<" Error, appropriate pRange not found in Send Subrange"<<endl;
			exit(-1);
		}

		BOUNDARY bdry = it->second;
		subranges.erase(it);

		ddg.DeleteTask(bdry.GetID());

		std::pair<typename PCONTAINER::BoundaryEnclosedDataType,int> data_to_send;
		data_to_send.first = pc->GetAndDeleteData(bdry);
		data_to_send.second = bdry.GetID();
		stapl::async_rmi(_pid,getHandle(),&pRange_Type::ReceiveSubRange,data_to_send);
	}

	void ReceiveSubRange(std::pair< typename PCONTAINER::BoundaryEnclosedDataType ,int > _data)
	{		ReceiveSubRange(_data.first,_data.second);	}

	void ReceiveSubRange(typename PCONTAINER::BoundaryEnclosedDataType d,int _id)
	{	BOUNDARY bdry = pc->AddData(d);
		bdry.SetID(_id);
                subranges[_id] = bdry;
		ddg.AddTask(bdry.GetID());
	}


	void SendSubRanges(vector<int> tosend,int _pid)
	{
		
		
	    if(_pid == stapl::get_thread_id() )
		{       //nothing to do ... return
			return;
		}

		vector< std::pair<typename PCONTAINER::BoundaryEnclosedDataType,int> > boundaries_to_send;
		for(int i=0; i<tosend.size(); ++i)
		{
			
			
		        int id = tosend[i];
                subrangeiterator it  = subranges.find(id);
		        if(it==subranges.end())
		        {
			        cout<<" Error, appropriate pRange not found in Send Subrange Chunk"<<endl;
			        exit(-1);
		        }
				
				
				BOUNDARY bdry = it->second;
		        subranges.erase(it);
        		ddg.DeleteTask(bdry.GetID());

				std::pair<typename PCONTAINER::BoundaryEnclosedDataType,int> data_to_send;
				
		        data_to_send.first = pc->GetAndDeleteData(bdry);
		        data_to_send.second = bdry.GetID();
				
	
				boundaries_to_send.push_back(data_to_send);
		}

                stapl::async_rmi(_pid,getHandle(),&pRange_Type::ReceiveSubRanges,boundaries_to_send);
		
	}

	void ReceiveSubRanges(vector<std::pair< typename PCONTAINER::BoundaryEnclosedDataType ,int > > _data_vector)
	{
	        for(int i=0; i<_data_vector.size(); ++i)
		{
			ReceiveSubRange(_data_vector[i].first,_data_vector[i].second);
		}
	}


};



 
 
 #endif
