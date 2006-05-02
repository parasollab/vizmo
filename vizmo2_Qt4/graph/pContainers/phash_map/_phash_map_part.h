#ifndef _phash_map_part_h_
 
#define _phash_map_part_h_

#include "BasePart.h"
#include "hash_exceptions.h"

namespace stapl {


template<class Key,class Data, class HashFcn>
class phash_map_part
	: public BasePart< 
					pair<Key,Data>	,                      //T
					typename hash_map<Key,Data,HashFcn>::iterator ,
					typename hash_map<Key,Data,HashFcn>::const_iterator ,
					vector< pair<Key,Data> > 		//Element Set
				>
{
public:

typedef pair<Key,Data> T;
typedef hash_map<Key,Data,HashFcn>	stl_hash_map;
typedef typename stl_hash_map::iterator iterator;
typedef typename stl_hash_map::const_iterator const_iterator;

typedef T value_type;
typedef T BaseElement_type;

typedef vector< pair<Key,Data> > Element_Set_type;

typedef stl_hash_map Sequential_Container_type;
typedef stl_hash_map Container_type;
 
typedef iterator seq_iterator;

typedef const_iterator const_seq_iterator;





public:	//change into private soon

stl_hash_map 	seq_hash_map;
HashFcn		seq_hash_function;

public:

phash_map_part(PARTID _part_id,int _size)
	:	BasePart< T, iterator, const_iterator, Element_Set_type  >(_part_id),
		seq_hash_map(_size),
		seq_hash_function()
		

{
	//cout<<"phash_map_part created. Part_ID = "<<this->partid<<endl;
}


phash_map_part(PARTID _part_id)
	:	BasePart< T, iterator, const_iterator, Element_Set_type  >(_part_id),
		seq_hash_map(0),
		seq_hash_function()
		

{
	//cout<<"phash_map_part created "<<endl;
}

phash_map_part()
	:	BasePart< T, iterator, const_iterator, Element_Set_type  >(-1),
		seq_hash_map(0),
		seq_hash_function()
		

{
	//cout<<"phash_map_part created "<<endl;
}

phash_map_part(const phash_map_part<Key,Data,HashFcn> &_t,PARTID _part_id)
{
}


virtual void AddElement(const T& t, GID gid)
{
  if(	gid!= seq_hash_function(t.first)	)
    {
      cout<<" Gid != HashFcn(key) in hash_map_part "<<endl;
      throw KeyHashFcnMismatch(seq_hash_function(t.first),gid);
    }
  seq_hash_map[t.first]=t.second;
}

/*

//Commented out because it is commented out in BasePart
virtual T& GetElement(GID _gid)	
{


	pair<Key,Data> rtn;	
	const_iterator iter;

	for(iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		

		if(	seq_hash_function(iter->first)==_gid	)
			break;
	}	

	if(iter==seq_hash_map.end())
	{
		cout<<" Gid = "<<_gid<<" Not found in phash_map_part :: GetElement "<<endl;
		throw ElementNotFound(_gid);
	}

	rtn.first=iter->first;
	rtn.second=iter->second;
	return rtn;
}
*/


virtual const T& GetElement(GID _gid) const 
{

	const_iterator iter;

	for(iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		

		if(	seq_hash_function(iter->first)==_gid	)
			break;
	}	

	if(iter==seq_hash_map.end())
	{
		cout<<" Gid = "<<_gid<<" Not found in phash_map_part :: GetElement "<<endl;
		throw ElementNotFound(_gid);
	}
	return *iter;
}


virtual void SetElement(GID _gid, const T& _t)	
{

	if(	_gid!= seq_hash_function(_t.first)	)
	{
		cout<<" Gid != HashFcn(key) in hash_map_part "<<endl;
		throw KeyHashFcnMismatch(seq_hash_function(_t.first),_gid);
	}


	iterator iter;

	for(iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		

		if(	seq_hash_function(iter->first)==_gid	)
			break;
	}	

	if(iter==seq_hash_map.end())
	{
		cout<<" Gid = "<<_gid<<" Not found in phash_map_part :: SetElement "<<endl;
		throw ElementNotFound(_gid);
	}


	
	iter->second=_t.second;
	


}

virtual void DeleteElement(GID _gid)
{

	iterator iter;

	for(iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		

		if(	seq_hash_function(iter->first)==_gid	)
			break;
	}	

	if(iter==seq_hash_map.end())
	{
		cout<<" Gid = "<<_gid<<" Not found in phash_map_part :: SetElement "<<endl;
		throw ElementNotFound(_gid);
	}

	seq_hash_map.erase(iter);

}


virtual bool ContainElement(GID _gid)
{

	
	iterator iter;

	for(iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		

		if(	seq_hash_function(iter->first)==_gid	)
			break;
	}	

	if(iter==seq_hash_map.end())
	{
		return false;
	}
	else
		return true;

}

virtual bool ContainElement(GID _gid, iterator* iter)
{

	


	for((*iter)=seq_hash_map.begin();	(*iter)!=seq_hash_map.end();	++(*iter))
	{
		

		if(	seq_hash_function((*iter)->first)==_gid	)
			break;
	}	

	if((*iter)==seq_hash_map.end())
	{
		return false;
	}
	else
		return true;


}


virtual Element_Set_type GetElementSet() const
{
  cout<<" \"virtual Element_Set_type GetElementSet() const\" is not yet implemented in phash_map_part Yet !!! "<<endl;
  return Element_Set_type();

}



  virtual iterator begin()   {	return seq_hash_map.begin();	}

  virtual const_iterator begin() const { 	return seq_hash_map.begin();	}

  virtual iterator end()   {	return seq_hash_map.end();	}

  virtual const_iterator end() const  {	return seq_hash_map.end();	}


virtual size_t size() const
{

	return seq_hash_map.size();

}


virtual bool empty() const
{
	return seq_hash_map.empty();
}

virtual void DisplayPart() const 
{	cout<<"Displaying Hash_Map_Part ID = "<<partid<<"  Size = "<<seq_hash_map.size()<<endl;


	for(const_iterator iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		cout<<"Key = "<<iter->first<<"  Value = "<<iter->second<<endl;
	}
}


virtual void GetPartGids(vector<GID>& _gidvec) const
{
	for(const_iterator iter=seq_hash_map.begin();	iter!=seq_hash_map.end();	++iter)
	{
		_gidvec.push_back(	seq_hash_function(iter->first)	);
	}

}


bool ContainsKey(Key key)
{
	return seq_hash_map.find(key)!=seq_hash_map.end();
}



size_t	max_size()
{
	return seq_hash_map.max_size();
}


GID insert(Key k,Data d)
{

	seq_hash_map[k]=d;

	return seq_hash_function(k)	;
}

Data find(Key key)
{
  const_iterator iter=seq_hash_map.find(key);
	
  if(iter!=seq_hash_map.end())
    {
      return iter->second;
    }
}

int erase(Key key)
{
	seq_hash_map.erase(key);
	return 0;
}


void clear()
{
	seq_hash_map.clear();
}

};

} //end namespace stapl

#endif


