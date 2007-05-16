#include "phash_map.h"
using namespace stapl;

void hpart()
{
        
  	int id=stapl::get_thread_id();
	int nprocs=stapl::get_num_threads();

	phash_map_part< int,int,hash<int> > hmp(0,3);	

	cout<<" hmp Part ID  = "<<hmp.GetPartId()<<endl;
	phash_map_part< int,int,hash<int> > hmp2(8);
	cout<<" hmp2 Part ID = "<<hmp2.GetPartId()<<endl;
	hmp2.SetPartId(11);
	cout<<"hmp2 Part ID (new) = "<<hmp2.GetPartId()<<endl;




	pair<int,int> x;

	int testcount = 1;

	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Adding 7 elements key=i and value=i*i "<<endl;
	for(int i=10; i<17; ++i)
	{
		x.first=i;
		x.second=i*i*i;

		hmp.AddElement(x,x.first);
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	vector<GID> gidvec;
	hmp.GetPartGids(gidvec);
	cout<<" Printing the GIDs in hmp "<<endl;
	for(int i=0; i<gidvec.size(); ++i)
	{		cout<<gidvec[i]<<"  ";	}
	cout<<endl;
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;

	

	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" This Tests hmp.GetElement() "<<endl;
	for(int i=0; i<gidvec.size(); ++i)
	{

		x=hmp.GetElement(gidvec[i]);

		cout<<" x.first = "<<x.first<<"   x.second = "<<x.second<<endl;
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing set element ... setting to i*10 "<<endl;
	for(int i=10; i<17; ++i)
	{
		x.first=i;
		x.second=i*10;

		hmp.SetElement(x.first,x);
	}
	//hmp.DisplayPart();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;




	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing Delete Element .... deleting GIDs 11 and 13 "<<endl;
	hmp.DeleteElement(11);
	hmp.DeleteElement(13);
	//hmp.DisplayPart();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;



	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing  ContainElement(const GID _gid)  "<<endl;
	for(int i=8; i<18; ++i)
	{

		cout<<" GID = "<<i<<"  ";

		if(hmp.ContainElement(i))
		{
			cout<<" Exists ";			
		}
		else
		{
			cout<<" Does not exist ";
		}	
		
		cout<<endl;
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;

	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing  ContainElement(const GID _gid, iterator*)  "<<endl;
	for(int i=8; i<18; ++i)
	{

		phash_map_part< int,int,hash<int> >::iterator it;

		cout<<" GID = "<<i<<"  ";

		if(hmp.ContainElement(i,&it))
		{
			cout<<" value = "<<it->second;			
		}
		else
		{
			cout<<" Does not exist ";
		}	
		
		cout<<endl;
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;



	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;

	cout<<" Testing Iterators.... "<<endl;
	phash_map_part< int,int,hash<int> >::iterator b;
	phash_map_part< int,int,hash<int> >::const_iterator cb;

	b=hmp.begin(); 
	b->second = 56;
	cb=hmp.begin();
	//cb->second = 34;
	cout<<"  Begin Key = "<<b->first<<"   Value = "<<b->second<<endl;
	cout<<"  Begin Key = "<<cb->first<<"   Value = "<<cb->second<<endl;

	for(; b!=hmp.end(); ++b)
	{
		cout<<" Key = "<<b->first<<"   Value = "<<b->second<<endl;
	}
	cout<<endl<<endl;
	for(; cb!=hmp.end(); ++cb)
	{
		cout<<" Key = "<<cb->first<<"   Value = "<<cb->second<<endl;
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing size and empty "<<endl;
	cout<<" Size hmp = "<<hmp.size()<<endl;
	cout<<" Empty hmp = "<<hmp.empty()<<endl;
	cout<<" Size hmp2 = "<<hmp2.size()<<endl;
	cout<<" Empty hmp2 = "<<hmp2.empty()<<endl;
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


cout<<" ********* Testing Hash_Map_Part specific functions......... "<<endl;
{

	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Max Size hmp = "<<hmp.max_size()<<endl;
	cout<<" Max Size hmp2 = "<<hmp2.max_size()<<endl;
	hmp.DisplayPart();
	cout<<" Clearing ... "<<endl;
	hmp.clear();
	hmp.DisplayPart();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;

	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Insert Test Key 20 - 30    Value key * 3 "<<endl;
	for(int i=20; i<=30; ++i)
	{
	  int _gid = hmp.insert(i,i+3);
#ifdef STAPL_DEBUG
	  cout<<" i = "<<i<<" Gid = "<< _gid <<endl;
#endif
	}
	hmp.DisplayPart();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;



	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Testing find "<<endl;
	cout<<" Finding 20..... 30 .... "<<endl;
	for(int i=20; i<=30; ++i)
	{
		int data = hmp.find(i);
#ifdef STAPL_DEBUG
		cout<<" Key = "<<i<<"  Data = "<<data<<endl;
#endif
	}
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	cout<<" Test ,... deleting 23 26 27 "<<endl;
	hmp.erase(23);
	hmp.erase(26);
	hmp.erase(27);	
	hmp.DisplayPart();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;


/*	cout<<"\n>>>>Test "<<testcount<<"  Starting "<<endl;
	phash_map_part< int,int,hash<int> >::Element_Set_type est=hmp.GetElementSet();
	cout<<"<<<<Test "<<testcount++<<"  Ending "<<endl<<endl;
*/

}

	return;
}



void stapl_main(int __x,char** __p) 
{
	hpart();




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

stapl::rmi_fence();


}







