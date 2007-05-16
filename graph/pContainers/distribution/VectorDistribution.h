//*************************************************************************
/*!
	\file VectorDistribution.h
	\author  Alin Jula	
	\date  01/21/03
	\ingroup stapl
	\brief 
	STAPL - project
	Texas A&M University,College Station, TX, USA
*/
//*************************************************************************





#ifndef _STAPL_VECTORDISTRIBUTION_H
#define _STAPL_VECTORDISTRIBUTION_H

#include "LinearOrderDistribution.h"

namespace stapl {

  //forward declaration
  class Pid_Partid_Range_triplet;
  struct within_triplet;

/**
 * @addtogroup pvector
 * @{
 **/

  /**
     VectorDistribution class. This class inherits from the
     BaseDistribution. It has all the methods that Base Distribution
     has. It adds methods that deal specifically with the distribution
     mechanism based on index rather than GID ( the base distribution
     does it based on GID). So this class has both functionalities to
     deal with distribution mechanism, based on index and based on
     GID.

     If the distribution_tag_class is passed as vector_distribution,
     then VectorDistribution uses methods based on index in
     distribution management exclusively. If the
     distribution_tag_class is passed as base_distribution,
     VectorDistribution uses methods based on GID in distribution
     management exclusively.
   */

template <class _Tp, class Part_type, class distribution_tag_class>
class VectorDistribution : public 
  LinearOrderDistribution<typename vector<splBaseElement<_Tp> >::iterator>
{
 protected:
  /**
     This is a pointer to the pcontainer parts. The access to parts is
     needed when location an element based on its
     index. Global_offset_map could be used to get the same
     information, but global_offset_map is consistency across threads
     only at synchronization points (given the threads insert/erase
     elements from their data).
   */
  vector<Part_type*> * parts;

  /**
     This is a global image of the ranges that ALL parts belonging to
     a pvector have. This image is replicated in each
     VectorDistribution object. The consistency is assured at any
     gloabl synchronization point in the program. In between those
     synchronization points, gloabl_offset_map might not represent the
     correct parts' ranges ( if the pvector inserts/erases
     elements). 

     For each processor, and for each part on that
     processor it holds the ranges of those parts.
   */
  //  vector<vector<pair<int,int> > > global_offset_map;
  vector<pair<int,int> > local_offset_map;
  vector< Pid_Partid_Range_triplet > global_offset_map;

 
 public:
  /** This type VectorDistribution */
  typedef VectorDistribution<_Tp,Part_type,distribution_tag_class> this_type;

  /** Distribution type */
  typedef distribution_tag_class distribution_tag;


  typedef typename vector<splBaseElement<_Tp> >::iterator Vector_Iterator_type;
  typedef pair<Vector_Iterator_type,Vector_Iterator_type> Vector_Range_type;
  
 protected:
   /**
     Making the data members from BaseDistribution accessible to VectorDistribution whitout the use of "this->data_member"
   */
  using LinearOrderDistribution<Vector_Iterator_type>::element_location_map; 
  using LinearOrderDistribution<Vector_Iterator_type>::element_location_cache; 
  using LinearOrderDistribution<Vector_Iterator_type>::myid;
  using LinearOrderDistribution<Vector_Iterator_type>::nprocs;

 public:
  //===========================
  //constructors & destructors
  //===========================
  /**
     Default constructor: registers the VectorDistribution object with ARMI and sets the size of global_offset_map.
   */
  VectorDistribution() 
    {
      this->register_this(this);
      //      new (&global_offset_map)  vector<vector<pair<GID,long int> > >(get_num_threads());
    }
  VectorDistribution(const this_type & __dist)
    {
      local_offset_map=__dist.local_offset_map;
      global_offset_map=__dist.global_offset_map;
    } 


  /**
     Destructor :unregisters the VectorDistribution object from ARMI.
   */
  ~VectorDistribution() 
    {
      rmiHandle handle = this->getHandle();
      if (handle > -1)
	unregister_rmi_object(handle);
    }

 //===========================
 //public methods
 //===========================
  
  // protected:

  /**
     Sets a pointer to the pcontainer parts ( pcontainer that uses this VectorDistribution object).
   */

  void set_parts(vector<Part_type*> * __x)
    {
      parts=__x;
    }

  /**
     This method checks to see if an index belongs to the local
     pcontainer parts. If so, it returns the id of the part that has
     that index. If the index is not in the local parts, it returns
     -1.

     This method uses protected data member parts in its
     functionality.
   */

  /*
  // returns partid 
  int IsLocalIndex(size_t index)
    {
      pair<int,int> __range;
      for(typename vector<Part_type*>::iterator p_it=parts->begin();p_it!=parts->end();++p_it)
	{
	  __range=(**p_it).GetRange();
	  if( (__range.first<= index)&&( index<__range.second)) // belongs to this range
	    return (p_it - parts->begin());
	}
      return -1;
    }
  */

  /**
     This method checks to see if an index belongs to the local
     pcontainer parts. If so, it returns the id of the part that has
     that index. If the index is not in the local parts, it returns
     -1.

     This method uses protected data member global_offset_map in its
     functionality.
   */
  int IsLocalIndex(size_t index)
    {

      if (local_offset_map.size()>5)
	{
	  // Do a binary search 
	  vector<pair<int,int> >::iterator it=lower_bound(local_offset_map.begin(),local_offset_map.end(),index,within_range());
	  if (it==local_offset_map.end())
	       return -1;
	  else 
	    return (it-local_offset_map.begin());
	}


      else 
	{
	  // This code does a blind search in the parts for the index
	  for(int i=0;i<local_offset_map.size();++i)
	    if ( (local_offset_map[i].first<= index)&&( index< local_offset_map[i].second))
	      return i;
	  return -1;
	}
      
    }
  // returns thread owner and part id of that index
  /**
     This methods returns the thread id and the part id that has the
     index passed as argument. It checks the local parts first, and if
     index is not local, it asks every thread ( every
     VectorDistribution objects) if they has that index.

     It returns a pair of part id and thread id. If the index is not
     found, it returns <-1,-1>.
   */
  /*
  pair<int,PARTID> LookUpIndex(size_t __index)
    {
      int part_id=IsLocalIndex(__index);

      if (part_id!=-1)
	return pair<int,int>(get_thread_id(), part_id);
	
      // it's not local. send a request to ? 
      for(int i=0;i<nprocs;++i)
	{
	  if (i!=get_thread_id())
	    part_id = stapl::sync_rmi(i,this->getHandle(),&this_type::IsLocalIndex, __index);
	  if (part_id!=-1)
	    return pair<int,int>(i,part_id);
	}
      return pair<int,int>(-1,-1);
    }
  */
  /**
     This methods returns the thread id and the part id that has the
     index passed as argument. It checks the local parts first, and if
     index is not local, it locates the index based on the
     global_offset_map.

     It returns a pair of part id and thread id. If the index is not
     found, it returns <-1,-1>.
   */
  int s()
    {
      return global_offset_map.size();
    }
  pair<int,int> LookUpIndex(size_t __index)

    {
      
      if (global_offset_map.size()>5)
	{
	  // Do a binary search 
	  within_triplet t;
	  vector<Pid_Partid_Range_triplet >::iterator it=lower_bound(global_offset_map.begin(),global_offset_map.end(),__index , within_triplet());
	  
	  if (it==global_offset_map.end())
	    {
	      printf("\n Global INdex %d was not found on thread %d",__index,get_thread_id());
	      for(it=global_offset_map.begin();it!=global_offset_map.end();++it)
		{
		  printf("\n Range %d,%d  and comp %d\n",(*it).range.first,(*it).range.second,__index);
		}
	      return pair<int,int>(-1,-1);
	    }
	  else
	    {
	      //	      printf("\n Global Index %d found in range [%d,%d)",__index,it->range.first,it->range.second);
	      return pair<int,int>(it->pid,it->partid);
	    }
	}
      else
	{
	  pair<int, int> _range;
	  //	  printf("\n MAP SIZE %d",global_offset_map.size());
	  for(int i=0;i<global_offset_map.size();++i)
	    {
	      _range=global_offset_map[i].range;
	      //	      printf("\n Thread %d Searching for index %d in range (%d,%d)",get_thread_id(),__index,_range.first,_range.second);
	      if ( (_range.first<= __index)&&( __index< _range.second))
		return pair<int,int>(global_offset_map[i].pid,global_offset_map[i].partid);
	    }
	  //	  printf("\n BLIND SEARCH INdex %d was not found ",__index);
	  printf("\n Global INdex %d was not found on thread %d",__index,get_thread_id());
	  printf("\n Size of global offset map  %d ",global_offset_map.size(),get_thread_id());
		  
	  for(vector<Pid_Partid_Range_triplet >::iterator it=global_offset_map.begin();it!=global_offset_map.end();++it)
	    {
	      printf("\n Range %d,%d  and comp %d\n",(*it).range.first,(*it).range.second,__index);
	    }
	  return pair<int,int>(-1,-1);
	}
    }
  /**
     Returns the distribution tag class that the VectorDistribution
     object has been instantiated with.
   */
  virtual distribution_tag tag() {return distribution_tag();}


  /**
     Sets the parts' ranges from a specific thread into the
     global_offset_map.
   */

  void set_offsets(vector<pair<int, int> > x, int proc)
    {
      for(int i=0;i<x.size();++i)
	global_offset_map.push_back(Pid_Partid_Range_triplet(proc,i,x[i]));
      if (proc==myid)
	  local_offset_map=x;
    }

  void sort_ranges()
    {
      // sort accroding to their offset, which would give us the order.
      sort(global_offset_map.begin(),global_offset_map.end());
      sort(local_offset_map.begin(),local_offset_map.end());

    }

  /**
     This method updates the part order after a distribution has been
     invoked( the distribution changes the part order). It relies of
     the fact that global_offset_map data member has been updated
     after the distribution. It sorts all the parts' ranges from
     global_offset_map according to their offset, and sets the
     predecessors and successors for each local part.
   */
  void update_part_bdry()
    {
      // CLear the previous info about the part order
      this->partbrdyinfo.clear();

      
      for(int i=0;i<global_offset_map.size();++i)
	if ( global_offset_map[i].pid==this->myid)
	  {
	    //if it's the first part
	    if (i==0)
	      {
		// If the successor it's valid
		if ((i+1)<global_offset_map.size())
		  this->partbrdyinfo.push_back(pair<Location,Location> ( Location(-1,INVALID_PART),Location(global_offset_map[i+1].pid,global_offset_map[i+1].partid)));
		// If the successor it's invalid
		else
		    this->partbrdyinfo.push_back(pair<Location,Location> ( Location(-1,INVALID_PART),Location(-1,INVALID_PART)));
	      }
	    // if it's not the first part
	    else
	      {
		// If the successor it's valid
		if ((i+1)<global_offset_map.size())
		  this->partbrdyinfo.push_back(pair<Location,Location> ( Location(global_offset_map[i-1].pid,global_offset_map[i-1].partid),Location(global_offset_map[i+1].pid,global_offset_map[i+1].partid)));
		// if the successor it's invalid
		else
		    this->partbrdyinfo.push_back(pair<Location,Location> ( Location(global_offset_map[i-1].pid,global_offset_map[i-1].partid),Location(-1,INVALID_PART)));
	      }
	      
	  }// end if this is myid part
      
    }




  /**
     Prints the VectorDistribution object
   */

  void print()
    {
      DisplayElementLocationMap();
      for(int i=0;i<global_offset_map.size();++i)
	  {
	    printf("\n Thread %d Part ID=%d on thread %d has this range(%d,%d)",get_thread_id(),global_offset_map[i].partid,global_offset_map[i].pid,global_offset_map[i].range.first,global_offset_map[i].range.second);
	  }
    }

  

  /**
   * Returns the local offset map
   */

  vector<pair<int,int> > get_local_offset_map()
    {
      return local_offset_map;
    }
  /**
   * Returns the global offset map
   */

  vector< Pid_Partid_Range_triplet > get_global_offset_map()
    {
      return global_offset_map;
    }

  /**
   * Clears the global and local offset maps
   */
  void clear_offset_maps()
    {
      global_offset_map.clear();
      local_offset_map.clear();
    }

  /**
   */
  vector<GID> get_n_gids(size_t n)
    {
      return _get_n_gids(n,distribution_tag());
    }
  vector<GID> _get_n_gids(size_t n, vector_distribution  x)
    {
      return vector<GID>();
    }
  vector<GID> _get_n_gids(size_t n, base_distribution  x)
    {
      vector<GID> v;
      this->GetNextGids(n,v);
      return v;
    }

};

/**
   This class is a helper class used in sorting the parts' range
   according to the parts' offsets. It is a triplet ( processor id,
   part id, and part range) with the arithmetic operators overloaded
   based on the range comparison.
 */
class Pid_Partid_Range_triplet
  {
  public:
    typedef Pid_Partid_Range_triplet this_type;
    //data
    /** Processor id*/
    PID pid;
    /** part id*/
    PARTID partid;
    /** Range */
    pair<int,int>  range;

    /** Default constructor*/
    Pid_Partid_Range_triplet(PID _pid,PARTID _partid, pair<int,int> _range)
      {
	pid=_pid;
	partid=_partid;
	range=_range;
      }

    //operators
    /** Range based comparison ( [a,b) < [c,d) iff a<b).*/
    bool operator<(const this_type& x) const
      {
	return (range.first < x.range.first); 
      }
    /** Range based comparison ( [a,b) <= [c,d) iff a<=b).*/
   bool operator<=( const this_type& x)const
      {
	return (range.first <= x.range.first); 
      }
   /** Range based comparison ( [a,b) >= [c,d) iff a>=b).*/
   bool operator>=( const this_type& x)const
      {
	return !(*this < x); 
      }
   /** Range based comparison ( [a,b) > [c,d) iff a>b).*/

   bool operator>( const this_type& x)const 
      {
	return !(*this <= x);
      }
   /** Range based comparison ( [a,b) == [c,d) iff a==b AND b==d and proc_id1==proc_id2).*/
   bool operator==(const this_type& x) const
     {
       return (pid==x.pid)&&(range.first==x.range.first)&&(range.second==x.range.second);
      }

  };

 struct within_triplet //:public binary_function<Pid_Partid_Range_triplet,size_t,bool>
 {
   bool operator()(const Pid_Partid_Range_triplet& __triplet,const size_t& value) const 
      {
	if (__triplet.range.second <= value) 
	  return true;
	else 
	  return false;
	
      }
   within_triplet() {}
  };

// vector_distribution tag() {return vector_distribution();}


}//end namespace stapl
//@}
#endif



