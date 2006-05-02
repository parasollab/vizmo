#ifndef _Ind_Task_DDG_h_
#define _Ind_Task_DDG_h_

#include "pDDG.h"

#include <set>

//class IndependentTasksDDG : public BaseDDG
/*
	Unable to inherit IndependentTasksDDG from DDG base class. Compiler error probably
	due to header file loop
*/

class IndependentTasksDDG 
{
private:

	set<int>  subranges;
	set<int>  readysubranges;
	set<int>   processedsubranges;
public:

  
  
  
  

	void AddTask(int x)	{		subranges.insert(x);	}
	void DeleteTask(int x)	{		subranges.erase(x);	}

	virtual bool finished()
	{

                if(processedsubranges.size() != subranges.size())
		{
		        return false;
		}
		else //size equal
                {
		        if(processedsubranges == subranges )
			{
                                return true;
			}
			else
			{
			        cout<<" Error in IndependentTaskDDG::finished() "<<endl;
				cout<<" Size of subrange and processed subrange same..."<<endl;
				cout<<" But the two sets do not match in eqality test "<<endl;
                                exit(-1);
				return false;
			}
		}
                return readysubranges.size() == 0;
	}

	virtual void reset()
	{
		//clear readysubranges and add tasks in subranges to readysubranges

		readysubranges.clear();
		processedsubranges.clear();

		if(readysubranges.size() != 0 )
		{
			cout<<" Error in IndependentTasksDDG::reset() "<<endl;
			cout<<" Size != 0 After resetting "<<endl;
			exit(-1);
		}

		readysubranges = subranges;

		if(readysubranges.size() != subranges.size() )
		{
			cout<<" Error in IndependentTasksDDG::reset() "<<endl;
			cout<<" Size mismatch after equating "<<endl;
			exit(-1);
		}

	}

	
	virtual std::vector<int> get_ready()
	{
		std::vector<int> rdy;

		for(std::set<int>::iterator it= readysubranges.begin(); it!=readysubranges.end(); ++it)
		{
			rdy.push_back(*it);
		}

		readysubranges.clear();

		if(readysubranges.size() != 0 )
		{
			cout<<" Error in IndependentTasksDDG::get_ready() "<<endl;
			cout<<" Size != 0 After resetting "<<endl;
			exit(-1);
		}

		return rdy;

	}

	virtual void processed(const int &x)
	{
                processedsubranges.insert(x);
		return;
	}

	void Print()
	{
		for(int i=0; i<stapl::get_num_threads(); ++i)
		{
			if(i==stapl::get_thread_id())
			{
				cout<<" Printing the nodes in DDG in Thread["<<stapl::get_thread_id()<<"]   ";
				for(std::set<int>::iterator it= subranges.begin(); it!=subranges.end(); ++it)
				{
					cout<<*it<<"  ";

				}
				cout<<endl;
			}
			//if((i+1) == stapl::get_num_threads() )
			//{	cout<<" ----------------------------------- "<<endl<<endl; }
			stapl::rmi_fence();
		}
	}

	void SimplePrint()
	{
				cout<<" Printing the nodes in DDG in Thread["<<stapl::get_thread_id()<<"]   ";
				for(std::set<int>::iterator it= subranges.begin(); it!=subranges.end(); ++it)
				{
					cout<<*it<<"  ";

				}
				cout<<endl;


	}
};



#endif
