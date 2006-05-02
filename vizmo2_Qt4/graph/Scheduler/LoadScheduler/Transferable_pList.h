#ifndef _trans_list_h_
#define _trans_list_h_


/*
	This is simple list , a part of which can be transferred from one processor to another
*/

template<class T>
class Transferable_pList
{
private:
		std::list<T>	theList;	//Local List
       	stapl::rmiHandle handle;	//handle
public:

	typedef typename std::list<T>::iterator iterator;
	typedef stapl::linear_boundary<iterator> LIST_BOUNDARY;
	
	typedef std::vector<T> BoundaryEnclosedDataType; 

	Transferable_pList()	{		handle = stapl::register_rmi_object(this);	}
	stapl::rmiHandle getHandle()	{	return handle;	}

	void push_back(T _t)	{ theList.push_back(_t); }
	
	iterator local_begin()	{ return theList.begin(); }
	iterator local_end()	{ return theList.end();	}
	
	std::list<LIST_BOUNDARY> CreateBoundaries(int unitsize)
	{//Divide the local pcontainer to chunks given by the unitsize.

		std::list< LIST_BOUNDARY > boundaries;
		
		int i=1;

		iterator b;
		iterator f;

		LIST_BOUNDARY  lb;

		iterator it;

		for(it= theList.begin(); it!=theList.end(); ++it)
		{
			if(i==1)
			{
				b=it;
			}

			if(i==unitsize)
			{
				f=it;
				f++;

				lb.SetData(b,f,unitsize);

				boundaries.push_back(lb);
				i = 0;
			}
			i++;
		}

		if(i!=1)
		{
			f=it;

			lb.SetData(b,f,i-1);
			boundaries.push_back(lb);
		}

		return boundaries;



	}

	

	typename std::list<T>::iterator begin()	{ return theList.begin(); }
	typename std::list<T>::iterator end()	{ return theList.end(); }

	int size()	{ return theList.size();	}


	BoundaryEnclosedDataType
	GetAndDeleteData(LIST_BOUNDARY bdry)
	{//Get the data corresponding to the boundary. Delete the returned data from the local repository

		std::vector<T>	data;
		
		typename std::list<T>::iterator it=bdry.start();
		
		typename std::list<T>::iterator last;
		for(int i=0; i<bdry.get_size(); ++i)
		{
			data.push_back(*it);
			last = it;
			++it;
		}
		
		
		
		theList.erase(bdry.start(), last);
		
		
		return data;
	}


	LIST_BOUNDARY AddData(std::vector<T> data)
	{//Add a data to the local pcontainer. Return the boundary corresponding to the local returned data.

		typename std::list<T>::iterator first,last;

		for(int i=0; i<data.size(); ++i)
		{
			//cout<<" Adding "<<data[i]<<endl;
			if(i==0)
			{
				last = first = theList.insert(theList.end(),data[i]);
			}
			else
			{
				last = theList.insert(theList.end(),data[i]);
			}
		}

		
		last++;
		LIST_BOUNDARY bdry;
		bdry.SetData(first,last,data.size());

		return bdry;

	}
	
	



	void Print()
	{	stapl::rmi_fence();
		for(int i=0; i<stapl::get_num_threads(); ++i)
		{	
			if(i==stapl::get_thread_id())
			{
				cout<<" Printing pList in Thread "<<stapl::get_thread_id()<<endl;
				for(typename std::list<T>::iterator it=begin(); it!=end(); ++it)
				{
					cout<<*it<<"  ";
				}
				cout<<endl;
			}
			if((i+1) == stapl::get_num_threads() )
			{	cout<<" ----------------------------------- "<<endl<<endl; }
			stapl::rmi_fence();
		}
	}

	void DataPrint()
	{
                cout<<" pList Size = "<<theList.size()<<"  Contents>>  ";
	        for(typename std::list<T>::iterator it=begin(); it!=end(); ++it)
		{
			cout<<*it<<"  ";
		}
		cout<<endl;
	}


};


void Print_pListBoundary(Transferable_pList<int>::LIST_BOUNDARY LB)
{
	Transferable_pList<int>::iterator it;
	it  = LB.start();
	
	
	cout<<" Start = "<<*LB.start()<<endl;
	cout<<" Finish = "<<*LB.finish()<<endl;
	cout<<" pList Boundary :: Size = "<<LB.get_size()<<" \tValues { ";
	
	for(int i=0; i<LB.get_size(); ++i)
	{
		cout<<*it<<"\t";
		it++;
	}
	
	cout<<"}"<<endl;
}


#endif
