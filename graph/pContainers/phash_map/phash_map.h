#ifndef _phash_map_h_
#define _phash_map_h_

#include <runtime.h>
#include "BaseDistribution.h"
#include "BasePContainer.h"
#include  "_phash_map_part.h"
#include "phash_iterators.h"

namespace stapl {

template<class Key,class Data, class HashFcn>
class phash_map : public BasePContainer
  <	   
  phash_map_part<Key,Data,HashFcn> ,   
  BaseDistribution< 	vector< pair<Key,Data> > >,
  no_trace,
  bidirectional_iterator_tag,
  stapl_base_element_tag	  
		>

{

public:
  
  typedef BasePContainer< phash_map_part<Key,Data,HashFcn>, BaseDistribution<vector<pair<Key,Data> > >,no_trace,bidirectional_iterator_tag, stapl_base_element_tag >	BasePContainer_type;

 typedef typename BasePContainer_type::pContainerPart_type pContainerPart_type;
 typedef typename BasePContainer_type::Element_Set_type Element_Set_type;

 typedef phash_map<Key,Data,HashFcn> phash_map_type;
 typedef typename BasePContainer_type::value_type value_type;
 typedef typename BasePContainer_type::pContainerPart_type pContainer_Part_type;



HashFcn		seq_hash_function;

phash_map()
	:BasePContainer_type()
{ 

	this->register_this(this);

	dist.register_this(&dist);
	AddPart();
}



phash_map(int _size)
	:BasePContainer_type()
{

    this->register_this(this);

    PARTID partid= part_id_counter++;
    pContainer_Part_type* _ct = new pContainer_Part_type(partid,_size);
    _ct->SetPartId(partid);
    pcontainer_parts.push_back(_ct);
}


virtual GID AddElement(const value_type& _t) 
{
    GID _gid = seq_hash_function(_t.first);
    BasePContainer_type::AddElement(_t,_gid);
    return _gid;
}

private:
GID _insert(Key key,Data value)
{
	value_type t;
	t.first=key;
	t.second=value;
	return AddElement(t);
}

public:
GID insert(Key key,Data data,PID p)
{
	//Insert in a remote processor.

	if( p == stapl::get_thread_id())
	{
		return _insert(key,data);
	}
	else
	{
		return stapl::sync_rmi(p,this->getHandle(),&phash_map_type::_insert,key,data);
	}
}

Data find(Key key)
{

	GID gid=seq_hash_function(key);
	PARTID partid;

	assert(gid != INVALID_GID);
	if(IsLocalKey(key,partid))
	{
		return pcontainer_parts[partid]->find(key);
	}
	else
	{
		PID pid = dist.Lookup(gid).locpid();
		assert(pid != INVALID_PID); 
		return stapl::sync_rmi( pid,
								getHandle(),
								&phash_map_type::find,
								key
							   );
	}
}

int erase(Key key)
{
	GID gid=seq_hash_function(key);
	PARTID partid;

	
	int rtn;
	assert(gid != INVALID_GID);
	if(IsLocalKey(key,partid))
	{
		rtn = pcontainer_parts[partid]->erase(key);
		dist.DeleteFromLocationMap(gid);
		dist.DeleteFromPartIDMap(gid);
	}
	else
	{

		PID pid = dist.Lookup(gid).locpid();
		assert(pid != INVALID_PID);
		rtn=stapl::sync_rmi( pid,
								getHandle(),
								&phash_map_type::erase,
								key
							   );
		dist.DeleteFromCache(gid);
	}

	return rtn;

}


bool IsLocalKey(Key key) const
{
	PARTID partid;
	return IsLocalKey(key,partid);
}

bool IsLocalKey(Key key,PARTID& partid) const
{
	bool found=false;
	for(int i=0; i<pcontainer_parts.size(); ++i)
	{
		if(pcontainer_parts[i]->ContainsKey(key))
		{
			found=true;
			partid=i;
		}
	}
	return found;
}

virtual void MergeSubContainer()	{}

int SetSubContainer(const vector<pContainerPart_type>  _ct){
  printf("\n This is -SetSubContainer- method from phash_map ...... NOT YET IMPLEMENTED");
  return OK;
}

void BuildSubContainer( const vector<Element_Set_type>&, 
				      vector<pContainerPart_type>&)
{
printf("\n This is -BuildSubContainer- method from phash_map ...... NOT YET IMPLEMENTED");
}


};
		
} //end namespace stapl

#endif		

