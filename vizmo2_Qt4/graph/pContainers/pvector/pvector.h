//*************************************************************************
/*!
	\file pvector.h
	\author  Alin Jula	
	\date  02/03/04
	\ingroup stapl
	\brief STAPL pvector container class. 

	STAPL - project
	Texas A&M University,College Station, TX, USA
*/
//*************************************************************************



#ifndef _STAPL_PVECTOR_H
#define _STAPL_PVECTOR_H

#include <vector>
#include <algorithm>
#include <numeric>
#include <runtime.h>
#include "VectorPart.h"
#include "../base/stapl_pc_tracer.h"
#include "../base/LinearOrderPContainer.h"
#include "../distribution/VectorDistribution.h"
//#include <pRange.h>


using namespace std;
#ifdef _TIMER_EXPERIMENT
extern stapl::timer tr_PV_I_islocal,tr_PV_I_lookup,tr_PV_I_retrieve_val,tr_PV_GG_lookup,tr_PV_GG_retrieve_gid,tr_PV_I_get_value;
extern double t_PV_I_islocal,t_PV_I_lookup,t_PV_I_retrieve_val,t_PV_GG_lookup,t_PV_GG_retrieve_gid,t_PV_I_get_value;
#endif


namespace stapl {
/**
 * @addtogroup pvector
 * @{
 **/
  /**
     \b pvector class 
     template arguments : _Tp - the type of the element stored in the pvector
                          distribution_tag_class - the type of the distribution used by that specific instance of pvector.
			  It could be either "vector_distribution" or "base_distribution". The default one is "vector_distribution"
     e.g. pvector<int> a - creates an instance that uses the vector_distribution 
          pvector<int,base_distribution> creates an instance that uses the base_distribution
     They could coexist in the same program, i.e. there could be 2 different instances , one instatiated with vector_distribution and the other instantiated with base_distribution.

     Using vector_distribution, the pvector does not rely on the
     global id to access elements, but rather relies on the
     index. This saves space ( not keeping a gid for each element, not
     keeping the element location map, part thread map. On the other
     hand, it requires to synchronize the pvector after a series of
     modifying methods being called ( insert, erase, push_back, etc)
     in order to assure a coherency on all threads.

     Using base_distribution relies on accessing elements based on
     their gid. This one is general, abd it assures consistency across
     all threads without explicitly doing so. On the other hand, it
     uses more space ( gid attached with each element, maps for
     element location, part thread, and part ordering) and it is
     slower than the vector_distribution since it needs to do
     additional bookkeeping for each operation it performs, in order
     to keep the pvector consistent on all threads at all times.
     
     See pvector example for a more detailed explanation.
     
  */
  template <class _Tp,class trace_class=no_trace,class distribution_tag_class=base_distribution>
    class pvector :  
                    public LinearOrderPContainer<VectorPart<_Tp,pair<GID,long int>,distribution_tag_class >  ,
    VectorDistribution<_Tp,VectorPart<_Tp,pair<GID,long int>,distribution_tag_class >,distribution_tag_class > , 
    trace_class,
    random_access_iterator_tag, 
    stapl_element_tag >
{

 public:
  void define_type(stapl::typer &t)  {
    stapl_assert(1,"pVector define_type used.\n");
  }

  /* This type */
  typedef pvector<_Tp,trace_class,distribution_tag_class> this_type;

  /* Base class type */
  typedef   LinearOrderPContainer<VectorPart<_Tp,pair<GID,long int>,distribution_tag_class >,VectorDistribution<_Tp ,VectorPart<_Tp,pair<GID,long int>,distribution_tag_class >,distribution_tag_class > , trace_class, random_access_iterator_tag ,stapl_element_tag > _Base_type;

  /* Overdefine the value_type from the base for ease of use */
  typedef typename _Base_type::value_type value_type;
  typedef typename _Base_type::Container_type Container_type;
  typedef VectorPart<_Tp,pair<GID,long int>,distribution_tag_class > pContainer_Part_type;
  typedef pair<GID,long int> Element_Set_type;
  typedef typename _Base_type::parts_internal_iterator parts_internal_iterator;


  /** Local Iterator to element type*/
    typedef typename _Base_type::iterator  iterator;

  /** Local Const Iterator to element type*/
  typedef typename _Base_type::const_iterator  const_iterator;

 /** Vector Part type*/
  typedef   VectorPart<_Tp,pair<GID,long int>,distribution_tag_class > VectorPart_type;

  /** Iterator that iterates on elements in a part*/
  typedef typename VectorPart_type::iterator part_iterator;

 /** pRange type */
 // typedef  pRange<linear_boundary<iterator>,this_type>  pRange_type;


 private:

 /**
    This variable is for storing the elements during a distribution process. It is a temp variable used to transfer the elements in a distribution process.
  */
  vector<pContainer_Part_type> _gathering_during_distribution;

 public:
  //==============================================
  // Constructors 
  //================================================

  /**
     \b  Default constructor 
     It registers the pvector with armi, gives the distribution object access to its parts and updates the pvector across all threads
  */
 pvector() 
    {
      this->register_this(this); 
      this->dist.set_parts(&(this->pcontainer_parts));

      _construct_trace(0,trace_class());

      update();
    }


  /**
     \b  Constructor 
     It creates a part with n/p elements.

     It registers the pvector with armi, gives the distribution object access to its parts and updates the pvector across all threads
  */
  pvector(size_t __n): _Base_type(__n) 
    { 

      this->register_this(this); 
      // setting the offset
      long int offset=(__n/this->nprocs)*this->myid + (( __n%this->nprocs) > this->myid ? this->myid : __n%this->nprocs );

      this->pcontainer_parts[0]->SetOffset(offset);

      this->dist.set_parts(&(this->pcontainer_parts));


      _construct_trace(__n/this->nprocs,trace_class());
        
      update();


    }
  /**
     \b  Constructor 
     It creates a part with n/p elements, all of value given as parameter (__value).
     
     It registers the pvector with armi, gives the distribution object access to its parts and updates the pvector across all threads
  */
  pvector(size_t __n, const _Tp& __value) :_Base_type(__n,__value)
    { 
      this->register_this(this); 
      long int offset=(__n/this->nprocs)*this->myid + (( __n%this->nprocs) > this->myid ? this->myid :  __n%this->nprocs);
      this->pcontainer_parts[0]->SetOffset(offset);
      this->dist.set_parts(&(this->pcontainer_parts));

      _construct_trace(__n/this->nprocs,trace_class());

      update();
    }
  
  /**
     \b  Copy Constructor 
     Copies the contents of the parameter pvector.

     It registers the pvector with armi, gives the distribution object access to its parts and updates the pvector across all threads
  */

 pvector(const this_type & __x):_Base_type(__x)
    { 
      this->register_this(this);
      this->dist.set_parts(&(this->pcontainer_parts));

      _construct_trace(local_size(),trace_class());
      //      update();
    }


 /**
  */
 pvector(typename _Base_type::Distribution_Info_type& __dist_map)
 {
   //printf("\n Start Constructor");
   this->register_this(this);
   this->pcontainer_parts.clear();
   vector<GID> v;
   for(int i=0;i< __dist_map[myid].size();++i)
   {
     size_t part_size=__dist_map[myid][i].second-__dist_map[myid][i].first;
     v=this->dist.get_n_gids(part_size);
     VectorPart_type * new_part=new VectorPart_type(part_size,v,VectorPart_type::value_type(),i); 
     new_part->SetOffset(__dist_map[myid][i].first);
     pcontainer_parts.push_back(new_part); 
   }
   this->dist.set_parts(&(this->pcontainer_parts));

   //printf("\n NUmber of parts %d",this->pcontainer_parts.size());
   update();
   update_distribution_info();
   
   rmi_fence();
   update();
   //for(int j=0;j<this->pcontainer_parts.size();++j)
   //printf("\n Thread %d has part %d with size %d and offset %d",this->myid, this->pcontainer_parts[j]->GetPartId(),this->pcontainer_parts[j]->size(),this->pcontainer_parts[j]->GetOffset());
 }

  /**
     \b  Desctructor
     It un-registers the pvector with armi and deallocates all the data members
  */


  ~pvector() {
    rmiHandle handle = this->getHandle();
		if (handle > -1)
      unregister_rmi_object(handle);
    _destroy_trace(local_size(),trace_class());
  }


  // ===============================================================
  // Methods specific to pVector
  //================================================================

 /**
    returns the maxumun size a pvector can have
  */
  size_t max_size() {return size_t(-1)/sizeof(_Tp);}
 /**
    \b Collective call
    Returns the summed capacity of all pvector respresentatives (on all threads)
  */
  size_t capacity()
    { 
      size_t __capacity =0;
      
      for(int i=0; i<this->nprocs; ++i)
	{
	  if(this->myid == i) __capacity += local_capacity();
	  else
	    {
	      _trace_rmi_synch(i,trace_class());
	    __capacity+=sync_rmi(i,this->getHandle(),&this_type::local_capacity);
	    }
	}

      return __capacity;   
      
    }
 /**
    Local call
    Returns the local capacity the pvector representative has.
    It sums all the capacities of all local parts
  */
  size_t local_capacity()
    {
      size_t __local_capacity=0;
      for(typename _Base_type::parts_internal_iterator it=this->pcontainer_parts.begin();it!=this->pcontainer_parts.end();it++)
	__local_capacity += (*it)->capacity();
      return __local_capacity;
    }

 /**
    It returns the value of the index. It does NOT return a reference, 
    but rather a copy of the element, therefore it cannot be used on the left hand side of an expression.
    It is a read-only operation( for write see "set(index,value)");
  */

  value_type operator[](size_t index)
    {
      _access_trace(local_size(),trace_class());
      return operator_index(index,distribution_category(dist));
    }


 /**
    It returns the value of the index. It is similar to the [] ooperator, but it performs bound checks.
  */
  value_type at (size_t __n)
    {
      _access_trace(local_size(),trace_class());

      printf("\n pvector at() - bound checks have NOT been performed. To be implemented");
      return operator[](__n);
    }


  /**
     It sets a value to the index-th element ( this index does not have to be local). 
     This is the method to be used  if elements in the pvector need to be written.

  */
  bool Set(size_t index,value_type  val)
    {
      _access_trace(local_size(),trace_class());
      pair<int,int> __range;
    
      // If it's in the local parts, get it from there
      for(parts_internal_iterator it=this->pcontainer_parts.begin();it!=this->pcontainer_parts.end();++it)
	{
	  __range=(**it).GetRange();
	  if ((index>=__range.first)&&(index<__range.second))
	    {
	      (**it)[index-__range.first]=val;
	      return true;
	    }
	}
      // if it's remote
      // It's not in the local parts.
      int owner=dist.LookUpIndex(index).first;

      _trace_rmi_synch(owner,trace_class());
      return sync_rmi(owner,this->getHandle(),&this_type::Set,index,val);
    }
  /**
     It returns the value of the index. It does NOT return a reference, 
     but rather a copy of the element, therefore it cannot be used on the left hand side of an expression.
     It is a read-only operation( for write see "set(index,value)");
  */
  value_type Get(size_t index)
    {
      _access_trace(local_size(),trace_class());
      return operator[](index);
    }

 /**
    It reserves memory for the pvector representative. The amount of memory it allocates for the pvector respresentative is n/p, 
    amount that is split equally between the parts. Each part then reserves that amount of memory.
  */

  void reserve (size_t __n)
    {
      size_t per_processor=(__n/this->nprocs) + ( __n%this->nprocs > this->myid);

      size_t per_part=(per_processor/this->pcontainer_parts.size()) +1 ; // extra one*sizeof(_Tp) bytes for the last threads

      for(typename _Base_type::parts_internal_iterator it=this->pcontainer_parts.begin();it!=this->pcontainer_parts.end();it++)
	(*it)->reserve(per_part);
      // Make sure that all processor have already reserved the memory by the time this 

      _trace_rmi_fence(trace_class());
      rmi_fence();
    }


  /** 
      This function HAS to be called after the pvector representatives have modified the number of elements in their own data ( if elements have been inserted or erased).
      
      a)It follows the part order ( which is present in the distribution object) and each part sends a message to its successor to let it know about the number of elements 
      so far (similar to the partial sum). This is needed to assure consistency among all threads.
      
      b) It creates a global image for the distribution about the ranges of indicies each pvector representative has. This is needed for accessing elements based on the index.

      See pvector example.
   */

 /**
    Push_back method. If the distribution is set for
    base_distribution, the push_back is called from
    LinearOrderPContainer. If the distribution is vector_distribution,
    then it just calls push back on the last part.
  */

 void push_back(const value_type & val)
 {
   _insert_trace(local_size(),trace_class());

   _push_back(val, distribution_tag_class());
 }


  void update()
    {
      _trace_rmi_fence(trace_class());
      rmi_fence();
      //###########
      // Propagate the offset to all the parts.
      //###########
      this->dist.clear_offset_maps();

      propagate_offsets();

      
      // synchronizes the parts image to each processor
      synch_parts();

      _trace_rmi_fence(trace_class());
      rmi_fence();
      this->dist.sort_ranges();
    }

 /**
    This method distributes the elements using a block distribution algorithm. If the block size is not provided, then the default size is n/p.
    E.g. A pvector with 20 elements on 2 threads on which a BlockDistribute(5) is called, distributes the elements so that the first thread has
    [0,5) [10,15) ranges of elements and the second thread has [5,10) and [15,20) ranges of elements.

    It computes the distribution map for the block distribution and it calls the general Distribute method with the block distribution map.
    It also updates the part order, element location and part map in the distribution object IF the pvector object has been instantiated with base distribution.
    If the pvector has been instatiated with vector_distribution, these distribution updates are not performed.
  */


 void BlockDistribute(size_t block_size=-1)
 {

   size_t no_elements=size();
   size_t count=0;
   // If the block size was not provided, assign it to n/p
   if (block_size == -1)
   {
     block_size = no_elements/ this->nprocs;
     if (no_elements%this->nprocs!=0)
       ++block_size;
   }
   // create the block distribution map
   map<PID,vector<pair<GID,long int> > > _block_map;
   vector<vector<pair<GID,long int> > > dist_vectors(this->nprocs);
   int i=0;
   while (count < no_elements)
   {
     for(i=0;(i<this->nprocs)&&(count<=(no_elements- block_size));++i)
       {
	 dist_vectors[i].push_back(pair<GID,long int>(count,count+block_size));
	 count+=block_size;
       }
     if ((count>(no_elements-block_size))&&(i<this->nprocs))
       {
	 dist_vectors[i].push_back(pair<GID,long int>(count,no_elements));
	 count=no_elements;
       }
   }
   for( i=0;i<this->nprocs;++i)
   _block_map[i]=dist_vectors[i];
   /*
   for(int i=0;i<dist_vectors.size();i++)
   {
     for(int j=0;j<dist_vectors[i].size();++j)
       {
	 printf("\n Processor %d part %d range (%d,%d)",i,j,dist_vectors[i][j].first,dist_vectors[i][j].second);
       }
   }
   */
   // call general distribute method with the newly created block distribution map
   Distribute(_block_map);
 }



 /**
    Returns a prange of the current pvector. Each part is assigned a new subrange, and all the parts in a pvector representative are set to belong to the
    prange that represents it.
  */
 /*
 pRange_type get_prange() 
 {
   pRange_type _prange(this);
   // if there is only 1 part
   if (this->pcontainer_parts.size()==1)
   {
     _prange.set_boundary(linear_boundary<iterator>(local_begin(),local_end()));
   }
   // if there are more than 1 part
   else
   {
     for(parts_internal_iterator part_it=this->pcontainer_parts.begin();part_id!=this->pcontainer_parts.end();++part_it)
     {
       // Get the part begin and end iterators and convert them to splLocalIterators
       iterator begin_part(this,part_it,(*part_it)->begin());
       iterator end_part(this,part_it,(*part_it)->end());
       // add the subrange corresponding to the part
       _prange.add_subrange(pRange_type (this,linear_boundary<iterator> (begin_part,end_part)));
     }
   }
   return _prange;
 }
 */

 /**
    Returns the type of the distribution the current pvector representative uses (vector_distribution or base_distribution)
  */

 typename  VectorDistribution<_Tp ,VectorPart<_Tp,pair<GID,long int>,distribution_tag_class >,distribution_tag_class >::distribution_tag tag() 
 {
   return distribution_category(dist);
 }

 /**
    Prints each part of the pvector, and calls distribution print if the argument "dist" is passed as true
 */

 void print(const bool dist=false)
    {
      printf("\n NUmber of parts %d",this->pcontainer_parts.size());
      for(typename vector<pContainer_Part_type*>::iterator it=this->pcontainer_parts.begin();it!=this->pcontainer_parts.end();++it)
      {
	printf("\n Part id %d, size %d, range (%d,%d) and offset %d---------Thread %d",(*it)->GetPartId(), (*it)->size(),(*it)->GetRange().first,(*it)->GetRange().second,(*it)->GetOffset(),get_thread_id());
	(*it)->print();
      }
      
      if (dist)
      {
	printf("\n Distribution\n");
	this->dist.print();
      }
    }
 
 void print_ranges()
 {
   vector<pair<int,int> > r=this->dist.get_local_offset_map();
   for(int i=0;i<r.size();++i)
   printf("\n Local range [%d,%d] on thread %d",r[i].first,r[i].second,get_thread_id());
   vector< Pid_Partid_Range_triplet >  rg=this->dist.get_global_offset_map();
   for(int i=0;i<rg.size();++i)
   printf("\n Global range [%d:-%d,%d] on thread %d",rg[i].pid,rg[i].range.first,rg[i].range.second,get_thread_id());

 }


 void report()
 {__tracer.report();}
 
 void evaluate(int container )
 {__tracer.evaluate(container);}
 

 vector<pair<int,int> >  get_local_offset_map()
 {
   return this->dist.get_local_offset_map();
 }


 /******************************** These are methods called by the public methods of the pvector******************************/
 /* They do not need to be accessible to the user*/



 protected:



  //=========================================================
  // Methods used in data distribution 
  //==========================================================

 /**
    This function takes in a vector of ranges ( that are supposed to
    be distributed on a thread) and creates parts of those elements in
    these ranges. These parts will be shipped to the destination
    thread by the Distribute method.

    It creates new parts ( it duplicates elements if they are in those ranges).

    Called by Distribute(distribution_map)

  */


  void BuildSubContainer( const vector<pair<GID, long int> >& ranges, 
			  vector<pContainer_Part_type>& subparts)
  {
    
    
    for(vector<pair<GID,long int> >::const_iterator range_it=ranges.begin();range_it!=ranges.end();++range_it)
      {
	pair<GID,long int> _range=*range_it;
	for(typename _Base_type::parts_internal_iterator part_it=_Base_type::pcontainer_parts.begin();part_it!=_Base_type::pcontainer_parts.end();++part_it)
	  {
	    typename _Base_type::pContainer_Part_type part_to_be_sent;
	    (*part_it)->make_vector_from_gid_index_maps();
	    pair<int,int> part_range=(*part_it)->GetRange();

	    if (_range.first > part_range.first)
	      if (_range.first < part_range.second)
		{

		  int max= _range.second > part_range.second ? part_range.second : _range.second;
		  // (_range.first, max) has to be paked
		  // Set the data
		  part_to_be_sent.insert(part_to_be_sent.begin(),
					 (*part_it)->begin() + (_range.first - (*part_it)->GetOffset()),
					 (*part_it)->begin() + (max - (*part_it)->GetOffset()),
					 (*part_it)->get_gids_begin()+(_range.first - (*part_it)->GetOffset()));

		  // Set the offset
		  part_to_be_sent.SetOffset(_range.first);
		  part_to_be_sent.make_vector_from_gid_index_maps();
		  // Ready to be shipped
		  subparts.push_back(part_to_be_sent);
		}
	      else 
		{
		  //This distribution range is not in this part.
		  continue;
		}
	    else
	      {
		if (_range.second < part_range.first)
		  {
		    //	This distribution range and the current part do not overlap.
		    continue;
		  }
		if (_range.second < part_range.second)
		  {
		    // (part_range.first,_range.second) to be paked
		    // Set the data
		    part_to_be_sent.insert(part_to_be_sent.begin(),
					   (*part_it)->begin() + (part_range.first - (*part_it)->GetOffset()),
					   (*part_it)->begin() + (_range.second - (*part_it)->GetOffset()),
					   (*part_it)->get_gids_begin()+ (part_range.first - (*part_it)->GetOffset()));
		    // Set the offset
		    part_to_be_sent.SetOffset(part_range.first);
		    part_to_be_sent.make_vector_from_gid_index_maps();
		  
		    // Ready to be shipped
		    subparts.push_back(part_to_be_sent);
		  }
		else 
		  // the whole part has to be packed (part_range.first, part_range.second)
		  {
		    subparts.push_back(**part_it);
		  }
	      }// else 
	  }// end for parts
      }// end for range
  }



 /**
    This method is invoked by the Distribute method when a part is
    received by a thread. This method assigns the newly received part
    into the temporary holder "_gathering_during_distribution". At the
    merge phase, this temporary is emptied into the parts of the
    pvector.

    Called by Distribute(distribution_map)

  */


  int SetSubContainer( vector<pContainer_Part_type>  _ct)
	{
	  //	  printf("\n ENTRY IN SUBCONTAINER %d and SIZE %d",get_thread_id(), _ct.size());
	  _gathering_during_distribution.insert(_gathering_during_distribution.end(),_ct.begin(),_ct.end());
	  
	  return OK;   //to use sync_rmi
	  
	}

 /**
    This method is invoked after the distribution has taken place. At
    this step, the parts are sorted according to their order (
    offsets) and checks are done to see if 2 consecutive parts can be
    merged as one. If there are no more consecutive parts to be
    merged, the distribution information is updated. Update() method
    is called. The part order is set and the newly distributed
    elements' location is updated in the distribution object.

    Called by Distribute(distribution_map)

  */



 void MergeSubContainer()
  {
    // sort the part according to their offsets
    sort(_gathering_during_distribution.begin(),_gathering_during_distribution.end());
    
    this->pcontainer_parts.clear();
    this->dist.clear_offset_maps();
    _trace_rmi_fence(trace_class());
    rmi_fence();
    //    rmi_fence();
    //    pDisplayPContainer();
    int part_count=0;
    //    printf("\n Gathering size %d",_gathering_during_distribution.size());
    for(int i=0;i<_gathering_during_distribution.size();)
      {
	//	_gathering_during_distribution[i].print();
	// merging if adjecent parts are continuous
	int j=i+1;
	int new_part_size=_gathering_during_distribution[i].size();
	//	printf("\n Current part size %d",new_part_size);
	// find how many of the consecutive parts should be merged and what the size of the new part is
	
	while(j<_gathering_during_distribution.size())
	  {
	    //	    printf("\n J=%d and SIZE %d",j, _gathering_during_distribution.size());
	    if (_gathering_during_distribution[j-1].GetRange().second == _gathering_during_distribution[j].GetRange().first )
	      {
		new_part_size+=_gathering_during_distribution[j].size();
		//		printf("\n part[j] size %d",_gathering_during_distribution[j].size());

		++j;
	      }
	    else break;
	  }

	//	printf("\n After merging Current part size %d",new_part_size);

	// create a new part
	typename _Base_type::pContainer_Part_type * new_part=new typename _Base_type::pContainer_Part_type;
	new_part->reserve(new_part_size);
	int k=i;
	// insert into the new part all the consecutive parts that should be merged
	while(k<j)
	  {
	    //	    printf("\n This is merging of the parts--size of map %d",_gathering_during_distribution[k].size_map());
	    new_part->insert(new_part->end(),_gathering_during_distribution[k].begin(),_gathering_during_distribution[k].end(),_gathering_during_distribution[k].get_gids_begin());
	    ++k;
	  }
	new_part->SetOffset(_gathering_during_distribution[i].GetOffset());
	new_part->SetPartId(part_count);
	part_count++;
	// put the new part into the stable storage
	//	printf("\n part to be inserted size %d",new_part->size());

	//	new_part->print();
	_Base_type::pcontainer_parts.push_back(new_part);
	i=j;

      }


    _gathering_during_distribution.clear();
    //    pDisplayPContainer();



    synch_parts();
    _trace_rmi_fence(trace_class());
    rmi_fence();
    update_distribution_info();
    //    propagate_offsets();

    _trace_rmi_fence(trace_class());
    rmi_fence();
  }




 /**
    Sets the part ranges it has received from thread "proc" in its own global view of the part ranges. 

    Called by synch_parts()
  */
 void update_global_offsets(vector<pair<int,int> >x, int proc)
 {
   this->dist.set_offsets(x,proc);
 }

 /**
    Given a part id and an index , it retrieves the index from the part with that part_id


    Called by operator[](index, vector_distribution)
  */

 value_type get_element_from_part(size_t index, int part_id)
 {
   return (*(this->pcontainer_parts[part_id]))[index - this->pcontainer_parts[part_id]->GetRange().first];
 }


 /**
    Given a part id and a GID , it retrieves the GID of the element whose index is passed as argument
    
    Called by the operator[](index, base_distribution)
  */

 GID get_gid_from_part(size_t index,int part_id)
 {
   (*(this->pcontainer_parts[part_id])).get_gid(index);
 }


 /**

 This is the operator[] specialized to work with the Vector
 Distribution It check to see if it's its local parts. If not, the
 index searching is performed by the distribution object, which finds
 the part that has the index (checking the global part ranges data structure).
 
 Called by operator[](index)
 
 */
 
 value_type  operator_index(size_t __n, vector_distribution  x)
 {

   value_type val;
   /*      
	   pair<int,int> __range;
	   
	   // If it's in the local parts, get it from there
	   for(parts_internal_iterator it=this->pcontainer_parts.begin();it!=this->pcontainer_parts.end();++it)
	   {
	   __range=(**it).GetRange();
	   if ((__n>=__range.first)&&(__n<__range.second))
	   {
	   return ((**it)[__n-__range.first]);
	   }
	   }
   */
   // It's not in the local parts.
#ifdef _TIMER_EXPERIMENT
   tr_PV_I_islocal=start_timer();
#endif 
   int __part=this->dist.IsLocalIndex(__n);
#ifdef _TIMER_EXPERIMENT
   t_PV_I_islocal+=stop_timer(tr_PV_I_islocal);
#endif 

   if (__part!=-1)
   {
     //     size_t __offset=this->pcontainer_parts[__part]->GetRange().first;
#ifdef _TIMER_EXPERIMENT
     tr_PV_I_get_value=start_timer();
#endif 

     val=(*(this->pcontainer_parts[__part]))[__n-this->pcontainer_parts[__part]->GetRange().first ];
#ifdef _TIMER_EXPERIMENT
     t_PV_I_get_value+=stop_timer(tr_PV_I_get_value);
#endif 
     return val;
   }
#ifdef _TIMER_EXPERIMENT
   tr_PV_I_lookup=start_timer();
#endif 
   pair<int,int> location=dist.LookUpIndex(__n);
#ifdef _TIMER_EXPERIMENT
   t_PV_I_lookup+=stop_timer(tr_PV_I_lookup);
   tr_PV_I_retrieve_val=start_timer();
#endif 
   if (location.first!=this->myid)
   {
     _trace_rmi_synch(location.first,trace_class());
     val=sync_rmi(location.first,this->getHandle(),&this_type::get_element_from_part,__n,location.second);
   }
   else val=get_element_from_part(__n,location.second);
#ifdef _TIMER_EXPERIMENT
   t_PV_I_retrieve_val+=stop_timer(tr_PV_I_retrieve_val);
#endif 

   return val;
 }

 /**
   This is the operator[] specialized to work with the base
   distribution It checks if the local parts have that index. If so,
   it gets the gid of that element. If it's not local, it calls
   get_gid(__n) which returns the gid of the element from that index.
   It calls GetElement (from BasePContainer ) to retrive the value.


   Called by operator[](index)

 */
  value_type operator_index(size_t __n, base_distribution x)
    {
      // locate gid
      // return dat
      GID _gid=get_gid(__n);
      if (_gid==-1)
      { 
	printf("\n Pvector operator[%d] (on thread %d) did not find the corresponding GID\n",__n,get_thread_id());
	exit(0);
      }
      value_type val=GetElement(_gid);
      return val;
      
    }

 /**
    Each pvector distribution object ( base of vector) has a global
    view of all the parts' ranges. This image is replicated on each
    thread. This map is needed to locate an element by its index.

    Each thread sends their parts' ranges to everybody

    Called by update()
  */


 void synch_parts()
 {
   vector<pair<int,int> >offsets;
   for(typename _Base_type::parts_internal_iterator part_it=_Base_type::pcontainer_parts.begin();part_it!=_Base_type::pcontainer_parts.end();++part_it)
   {
     offsets.push_back( (*part_it)->GetRange());
   }      
   for(int i=0;i<this->nprocs;++i)
   {
     if (i!=this->myid)
     {     
       _trace_rmi_asynch(i,trace_class());
       async_rmi(i,this->getHandle(),&this_type::update_global_offsets,offsets,this->myid);
     }
     else
     update_global_offsets(offsets,myid);
   }
 }


 /**
    This method propagates (updates) the offsets of all the parts. The
    processor which has the first part ( this information is given by
    the distribution part order) sends a message to the thread that
    has the successor ot the current part , with the updated
    offset. At its turn, the receiving processor updates that part's
    offset and sends a message to the next part, until all the parts
    get their offsets updated.

    This creates a chain of rmi's that follow exactly the same pattern as the part order stored in the distribution object.

    Called by update()

  */

 void propagate_offsets()
 {
      Location previous=GetDistribution().LookUpBdBeginInfo(0);
      Location next=GetDistribution().LookUpBdEndInfo(0);
      //
      //      printf("\n Processor %d part %d has previous (%d,%d)",get_thread_id(),0,previous.locpid(),previous.partid());
      //      printf("\n Processor %d part %d has next (%d,%d)",get_thread_id(),0,next.locpid(),next.partid());

      if (previous.partid()==INVALID_PART)
	update_part(0,0);
      _trace_rmi_fence(trace_class());
      rmi_fence();

 }


 /**
    This method receives an part id and a size. It updates the offset
    of the part whose id is passed as a parameter and looks in the
    distribution for the part successor. It sends then an rmi message
    to its successor with the new offset ( the size that it received +
    its current size)


    Called by  propagate_offsets() which is called by update()
  */

  void update_part(PARTID part_id, size_t _size)
 {

   (*(this->parts_begin()+ part_id))->SetOffset(_size);

   Location l=GetDistribution().LookUpBdEndInfo(part_id);
   //printf("\n Thread %d tries to set the offset for the part %d Offset=%d....will send a message for thread %d and part %d",get_thread_id(),part_id,_size,l.locpid(),l.partid());

   if (l.partid() != INVALID_PART)
       {
	 //	 printf("\n Message !! from Thread %d to Thread %d part %d",get_thread_id(),l.locpid(),l.partid());
	 _trace_rmi_asynch(l.locpid(),trace_class());
	 async_rmi(l.locpid(),this->getHandle(),&this_type::update_part,l.partid(),_size+(*(this->parts_begin()+ part_id))->size());
       }
   //   printf("\n THREAD %d DONE",get_thread_id());
   return;
   
 }
 /**
    Update distribution information for the vector distribution : nothing to be done
  */
  void _update_distribution_info(vector_distribution x)
    {
      return;
    }
  

 /**
    Update distribution information for the base distribution The
    parts order, element location and the cache is flushed. It adds
    each part to the part map, along with each element in the part to
    the element location map ( if complete_location_map defined). It
    also sets the new part order.

    Called by update_distribution_info()

  */
 void _update_distribution_info(base_distribution x)
    {
      // CLear the previous distribution info
      this->dist.FlushPartsMap();
      this->dist.FlushLocationMap();
      this->dist.FlushCache();

      // Add each part to the part map
      for(typename _Base_type::parts_internal_iterator part_it=_Base_type::pcontainer_parts.begin();part_it!=_Base_type::pcontainer_parts.end();++part_it)
	{
	  AddPart2PartIDMap((**part_it));
#ifdef COMPLETE_LOCATION_MAP
	  // Add each element to the element_location_map
	  AddPart2LocationMap((**part_it));
#endif
	  
	}
      // UPdate the boundary part info ( the order of parts)
      //      this->dist.update_part_bdry();
      return;
    }

 /**
    It updates the distribution information.  This method is called
    after each distribution. It selects the right
    _update_distribution_info based on the kind of distribution the
    pvector uses.

    Called by MergeSubContainer();
  */
  
  void update_distribution_info()
    {
      this->dist.sort_ranges();
      _update_distribution_info(distribution_tag_class());
      // UPdate the boundary part info ( the order of parts)
      this->dist.update_part_bdry();
    }


 /**
    It finds the location that has this index ( based on the
    distribution object global view parts' ranges) and sends and rmi
    to get the gid for that element.
  */

 GID get_gid(size_t index)
 {
   GID _gid;
#ifdef _TIMER_EXPERIMENT
   tr_PV_GG_lookup=start_timer();
#endif 
   pair<int,int> location=dist.LookUpIndex(index);   
#ifdef _TIMER_EXPERIMENT
   t_PV_GG_lookup+=stop_timer(tr_PV_GG_lookup);
   tr_PV_GG_retrieve_gid=start_timer();
#endif 
   if (location.first==this->myid)
     _gid=get_gid_from_part(index,location.second);
   else
   {
     _trace_rmi_synch(location.first,trace_class());
     _gid=sync_rmi(location.first,this->getHandle(),&this_type::get_gid_from_part, index,location.second);
   }
#ifdef _TIMER_EXPERIMENT
   t_PV_GG_retrieve_gid+=stop_timer(tr_PV_GG_retrieve_gid);
#endif 
   return _gid;
 }

 /** 
     Helper for the push_back method with base distribution. This
     helper calls the push_back from LinearOrderPContainer which does
     all the distribution updates.
  */
 void _push_back(const value_type & val, base_distribution x)
 {
   _Base_type::push_back(val);
 }

 /** 
     Helper for the push_back method with vector distribution. This
     helper calls the push_back on the last part without any distribution updates ( not necessary)

  */
 void _push_back(const value_type & val, vector_distribution x)
 {
   (*(_Base_type::pcontainer_parts.end()-1))->push_back(val);
 }





};//end class pvector




template <class T>
void print_tag(base_distribution x)
{
  printf("\n This is a Base distribution");
}

template <class T>
void print_tag(vector_distribution x)
{
  printf("\n This is a Vector distribution");
}

}// end namespace stapl

//@}
#endif
