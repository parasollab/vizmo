#include "Defines.h"

#include "phash_map.h"


#include "_phash_map_part.h"


struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};



#include "hash_map_part_test.cc"


void hash_map_test()
{



	phash_map<int,int,hash<int> > hmp;
	int testcount = 1;

	bool testflag1=false;

	stapl::rmi_fence();
if(stapl::get_thread_id() == 0 )
{

	cout<<"Inserting i=0..10 and val i*i+1 in  (i+3)%num_threads "<<endl;

	for(int i=0; i<10; ++i)
	{
		hmp.insert(i,i*i+1, (i+3)%stapl::get_num_threads() );
	}

	cout<<"Getting the values....."<<endl;
	for(int i=0; i<10; ++i)
	{
		cout<<"Value of "<<i<<"  = "<<hmp.find(i)<<endl;
	}


	cout<<" Old Size = "<<hmp.size()<<endl;
	cout<<"Erasing..3,8,9..."<<endl;
	hmp.erase(3);
	hmp.erase(8);
	hmp.erase(9);

	cout<<" New Size = "<<hmp.size()<<endl;

	
}

//hmp.pDisplayPContainer();


stapl::rmi_fence();


	
}


void stapl_main(int __x,char** __p) 
{

if(stapl::get_thread_id()==0)
{
	hash_map_part_test(); 
}

hash_map_test();


	stapl::rmi_fence();

}































