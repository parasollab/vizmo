#ifndef _Lst_Wk_Fn_h_
#define _Lst_Wk_Fn_h_

class ListWorkFunction
{
private:
		
	typedef Transferable_pList<int> PLIST;
	typedef stapl::linear_boundary<PLIST::iterator> LIST_BOUNDARY;
	typedef stapl::pRange<LIST_BOUNDARY, PLIST, IndependentTasksDDG> LIST_PRANGE;

	vector<int> data;
	stapl::rmiHandle handle;

public:
	

	ListWorkFunction()
	{
		 handle = stapl::register_rmi_object(this);
	}
	void operator ()	(LIST_PRANGE prange)
	{

		cout<<" ID = "<<prange.get_global_id()<<endl;
		LIST_BOUNDARY boundary = prange.get_boundary();
		
		
		PLIST::iterator it = boundary.start();  
		
		for(int i=0; i<boundary.get_size(); ++i)
		{
			
			foo(*it);
			data.push_back(*it);
			it++;
		}
	}
	
	
	void operator ()	(LIST_BOUNDARY boundary)
	{
		//cout<<" ID = "<<boundary.GetID()<<endl;
		
		PLIST::iterator it = boundary.start();  
		
		for(int i=0; i<boundary.get_size(); ++i)
		{
			
			foo(*it);
			data.push_back(*it);
			it++;
		}

	}

	void SendToThread(int id)
	{
		if(stapl::get_thread_id() != id)
		{
			stapl::async_rmi(id,handle,&ListWorkFunction::AddData,data);
		}
	}
	
	
	void AddData(vector<int> newv)
	{    
		for(int i=0; i<newv.size(); ++i)
		{   
			data.push_back(newv[i]);	                
		}
	}
	
	void Test(int sz)
	{
	    cout<<" sz = "<<sz<<endl;
		cout<<" data.size() = "<<data.size()<<endl;
        if(sz != data.size() )
		{		        cout<<" Size Mismatch !!!!!!!! Error "<<endl;		}

		sort(data.begin(),data.end());
        cout<<" Data from Test in Thread {"<<stapl::get_thread_id()<<"} --->  ";
	    for(int i=0; i<sz; ++i)
		{        //cout<<data[i]<<"  ";
			if(i != data[i] )
			{        cout<<endl<<" ************************ ERROR !!!! mismatch in position "<<i<<" *************** "<<endl;			}
		}
		cout<<" !! TEST SUCCESS !! "<<endl;
		cout<<endl;
	}

	void foo(int n=1)
	{	
        if(n<0) n=1;

		//cout<<" Working in Thread "<<stapl::get_thread_id()<<"  n = "<<n<<endl;

       for(int i=0; i<n; ++i)
        {
        	int limit = 5;
	        for(int i=0; i<limit; ++i)
	        {
	        	double x=sin(4.235)*sin(3.56);
				
				
	        }
        }
		
		//cout<<" Work Over in Thread "<<stapl::get_thread_id()<<"  n = "<<n<<endl;
	}

	
};


#endif

