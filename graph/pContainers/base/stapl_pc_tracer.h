//*************************************************************************
/*!
        \file stapl_pc_tracer.h
        \author  Alin Jula
        \date  02/08/04
        \ingroup stapl
        \brief Tracer object for pContainer operations. It records the methods that have been invoked on a pContainer in chronological order.                                                                                                                                                             
        STAPL - project
        Texas A&M University,College Station, TX, USA
*/
//*************************************************************************
                                                                                                                                                             
                                                                                                                                                             
                                                                                                                                                             
#ifndef _STAPL_PC_TRACER_H
#define _STAPL_PC_TRACER_H


#include <vector>
#include <string>
#include <stddef.h>
#include <rmitools.h>
struct trace{};
struct no_trace{};

extern double pvector_create(double ,int );
extern double pvector_access(double ,int);
extern double pvector_insert(double ,int);
extern double pvector_erase(double ,int);
extern double pvector_destroy(double ,int);
extern double pvector_iterator_plus_plus(int);
extern double pvector_iterator_minus_minus(int);
extern double pvector_iterator_plus_ptrdiff(int);
extern double pvector_iterator_minus_this(int);
extern double pvector_iterator_plus_equal(int);
extern double pvector_iterator_minus_equal(int);
extern double pvector_iterator_dereference(int);
extern double pvector_iterator_arrow(int);
 
/**
 */
class stapl_trace{
 public:
  stapl_trace(const long int &t, const size_t & s, const int& op):time(t),size(s),operation(op),stride(0),steps(0)
    {}
  //  stapl_trace(const int & _op):operation(_op),time(-1),size(0),stride(0),steps(0)
  //    {}
  long int time;
  size_t size;
  int operation;
  size_t stride;
  size_t steps;
  bool operator==(const stapl_trace& __x)
    {
      return operation==__x.operation;
    }

};

class stapl_rmi_trace {
 public:
  stapl_rmi_trace(const long int &t, const pair<int,int>& sd,const int & op, const long int & at):time(t),source_dest(sd),type(op),anchor_time(at)
    {}

  // the rmi_internal_time
  long int time;
  // the source and the destination for the armi call
  pair<int,int> source_dest;
  // type of rmi ( synch,asynch,fence,abort, etc)
  int type;
  // anchor time into the operations trace.
  long int anchor_time;

};


/**
 * Class stapl_pc_tracer
 */
class stapl_pc_tracer{

  typedef vector<stapl_trace> Sequence_type;

  typedef vector<stapl_rmi_trace> Rmi_sequence_type;

  /**
   * \brief Internal timer for stapl_pc_tracer. It gets increased
   * every time an operations has been performed on the a pcontainer. 
   */
  long int internal_timer;

  /**
   *\brief internal timer for rmi calls. This is kept separate for
   *better dealing with rmi calls, and for better determining the
   *supersteps in a program.
   */
  long int internal_rmi_timer;

  /**

   * code 0      __create;

   * code 1      __access;

   * code 2      __insert;

   * code 3      __erase;

   * code 4      __destroy;

   * code 5      __iterator_plus_plus;

   * code 6      __iterator_minus_minus;

   * code 7      __iterator_plus_ptrdiff;

   * code 8      __iterator_minus_ptrdiff;

   * code 9      __iterator_minus_this;

   * code 10     __iterator_dereference;

   * code 11     __iterator_arrow;

   * code 12     __iterator_plus_equal;

   * code 13     __iterator_minus_equal;
   */


  /*
   * code 0 - synch_rmi
   * code 1 - asynch_rmi
   * code 2 - rmi_fence
   */


  /**
   *\brief This is the raw trace of operations, as they are gathered during the execution of a program
   */
  Sequence_type __trace;
  /**
   *\brief This is the compacted trace of operations after the compacting algorithm is called
   */
  Sequence_type __compact_trace;

  /**
   *\brief This the rmi calls trace, as they are gathered during the execution of a program
   */
  Rmi_sequence_type __rmi_trace;

  /**
   *\brief Thread id
   */
  int myid;

  /**
   *\brief Kees the number of armi's exchanged beteween each part from
   *each processor (the number of processors is the first dimension,
   *and number of parts on the remote threads is the second dimenson)
   */

  vector<vector<long int> > __armi_matrix;
  


 public:
  stapl_pc_tracer():internal_timer(0)
    {
      myid=stapl::get_thread_id();
      
      for(int i=0;i<stapl::get_num_threads();++i)
	// At the beginning, each thread has only one part. The distribution algorithm from the pcontainer has to redimension this variable.
	__armi_matrix.push_back(vector<long int>(1,0));
      
    }

  ~stapl_pc_tracer()
    {}
  // PContainer methods
  
  void initialize_armi_matrix( vector<int> remote_parts)
  {
    // The previous information gets lost !!!!!!!!!
    __armi_matrix.clear();
    for(int j=0;j<remote_parts.size();++j)
      __armi_matrix.push_back(vector<long int>(remote_parts[j],0));
    
  }


  void create(size_t __size)
    { __trace.push_back(stapl_trace(internal_timer++,__size,0)); } 
  void access(size_t __size)
    { __trace.push_back( stapl_trace(internal_timer++,__size,1)); }
  void insert(size_t __size)
    { __trace.push_back(stapl_trace (internal_timer++,__size,2)); }
  void erase(size_t __size)
    { __trace.push_back(stapl_trace(internal_timer++,__size,3)); }
  void destroy(size_t __size)
    { __trace.push_back(stapl_trace(internal_timer++,__size,4)); }

  // Iterator operations

  void iterator_plus_plus(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,5)); 
    }
  void iterator_minus_minus(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,6)); 
    }
  void iterator_plus_ptfdiff(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,7)); 
    }
  void iterator_minus_ptfdiff(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,8)); 
    }
  void iterator_minus_this(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,9)); 
    }
  void iterator_dereference(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,10)); 
    }
  void iterator_arrow(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,11)); 
    }
  void iterator_plus_equal(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,12)); 
    }
  void iterator_minus_equal(size_t __size)
    {
      __trace.push_back(stapl_trace (internal_timer++,__size,13)); 
    }


  // Rmi calls
  void trace_rmi_synch(const int& dest)
    {
      __rmi_trace.push_back(stapl_rmi_trace(internal_rmi_timer++,pair<int,int> (myid,dest),0,internal_timer));
    } 
  void trace_rmi_asynch(const int& dest)
    {
      __rmi_trace.push_back(stapl_rmi_trace(internal_rmi_timer++,pair<int,int> (myid,dest),1,internal_timer));
    } 
  void trace_rmi_fence()
    {
      __rmi_trace.push_back(stapl_rmi_trace(internal_rmi_timer++,pair<int,int> (-1,-1),2,internal_timer));
    } 

  void compact_sequence()
    {
      // These are the types of operations that are taken into consideration for compaction ( the most used operations).
      // The operations that could appear in a loop 
      vector<int> operations;
      operations.push_back(1); // access
      operations.push_back(2); // insert
      operations.push_back(3); // erase
      operations.push_back(5); // ++ iterator
      operations.push_back(10); // derefence

      Sequence_type::iterator __start=__trace.begin();
      while(__start!=__trace.end())
	{
	  int op=(*__start).operation;
	  // if the operation can potentially be in a loop, analyze that
	  if ((op==1)||(op==2)||(op==3)||(op==5)||(op==10))
	    {
	      // Find the next appearence
	      Sequence_type::iterator second=find(__start + 1,__trace.end(),*__start);
	      // If there is none, insert the current operation into the compact form, fail.
	      if (second==__trace.end()) 
		{
		  __compact_trace.push_back(*__start++);
		  continue;
		}
	      // Compute the stride between consecutive appearences
	      size_t stride=distance(__start,second);
	      // Compute the span of the operation with the potential stride
	      while ( (*second).operation==op) second+=stride;
	      size_t loop_length=distance(__start,second);
	      // If the stride is 1, insert the loop in the compact form and advance to the next operation
	      if (stride==1)
		{
		  stapl_trace __item=*__start;
		  __item.stride=stride;
		  __item.steps=loop_length/stride;
		  __compact_trace.push_back(__item);
		  __start=__start + loop_length;
		  continue;
		}
	      // If the stride is not 1, then check for the same pattern for all the operations between consecutive appearences of op
	      Sequence_type::iterator _op_first,_op_second;
	      bool __loop_recognized=true;
	      // For each operation between the consecutive appearences of the current operation
	      for(int i=0;i<stride;++i)
		{
		  _op_first=_op_second=__start+i;
		  int _op=(*_op_first).operation;
		  // Check is they appear at the same stride
		  while ( (*_op_second).operation==_op) _op_second+=stride;
		  // If so, check the span of it. If it's the same with the current operation, loop candidate.
		  // If not, fail 
		  if (distance(_op_first,_op_second)!= loop_length)
		    {
		      __loop_recognized=false;
		      break;
		    }
		}
	      
	      // If a loop was recognized, add it to the compact form.
	      if (__loop_recognized)
		{
		  Sequence_type::iterator it=__start;
		  for(int i=0;i<stride;++i)
		    {
		      stapl_trace __item=*it++;
		      __item.stride=stride;
		      __item.steps=loop_length/stride;
		      __compact_trace.push_back(__item);
		    }
		  // Set the next operation to be checked.
		  __start=__start + loop_length;
		}
	      // if the loop was not recognized
	      else
		{
		  __compact_trace.push_back(*__start++);
		  continue;
		}
	    }
	  // if the current operation is not a candidate operation for recognizing loops, add it to the compact form
	  else
	    __compact_trace.push_back(*__start++);


	}
    }
  
  void report()
    {
      compact_sequence();

      printf("\n Tracing report \n");
      
      Sequence_type::iterator it;
      for(it=__compact_trace.begin();it!=__compact_trace.end(); ++it)
	{
	  if ((*it).operation==0)
	    {
	      printf("\n Construct : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);
	    }
	  if ((*it).operation==1)
	    {
	      printf("\n Access : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==2)
	    {
	      printf("\n Insert : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==3)
	    {
	      printf("\n Erase : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==4)
	    {
	      printf("\n Destroy : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==5)
	    {
	      printf("\n Iterator ++ : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==6)
	    {
	      printf("\n Iterator -- : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==7)
	    {
	      printf("\n Iterator +(ptrdiff) : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==8)
	    {
	      printf("\n Iterator -(ptrdiff) : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==9)
	    {
	      printf("\n Iterator -(iterator) : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==10)
	    {
	      printf("\n Iterator * dereference : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==11)
	    {
	      printf("\n Iterator -> arrow : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==12)
	    {
	      printf("\n Iterator += : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }
	  if ((*it).operation==13)
	    {
	      printf("\n Iterator -= : time %d and size %d",(*it).time, (*it).size);
	      if ((*it).stride!=0) printf(" stride=%d and steps=%d",(*it).stride,(*it).steps);

	    }

	}
    }


  void initialize_costs(string file_name)
    {
      /*
      ifstream    __file(file_name);
      istream_iterator<int> it(__file);
      int i=0;
      while(it!=istream_iterator<int>())
	{
	  theoretical_costs[i]=*it;
	  ++it; ++i;
	}
      */
    }


  void evaluate(int container)
    {

      double cost=0.0;
      double operation_cost;
      for( Sequence_type::iterator it=__compact_trace.begin();it!=__compact_trace.end();++it)
	{
	  operation_cost=get_cost(it,container);
	  printf("\n Operation cost  %2.2f   and times %d",operation_cost,(*it).steps);
	  if ((*it).steps !=0)
	    operation_cost=operation_cost*(*it).steps;
	  cost+=operation_cost;
	  printf("\n Cost %2.2f and code %d ",cost,it->operation);
	}
      printf("\n Overall Cost %2.2f\n",cost);
    }


  double get_cost(Sequence_type::iterator it,int container)
    {
      switch(it->operation) {
      case 0: return pvector_create(it->size,container);
      case 1: return pvector_access(it->size,container);
      case 2: return pvector_insert(it->size,container);
      case 3: return pvector_erase(it->size,container);
      case 4: return pvector_destroy(it->size,container);
      case 5: return pvector_iterator_plus_plus(container);
      case 6: return pvector_iterator_minus_minus(container);
      case 7: return pvector_iterator_plus_ptrdiff(container);
      case 8: return pvector_iterator_plus_ptrdiff(container);
      case 9: return pvector_iterator_minus_this(container);
      case 10: return pvector_iterator_dereference(container);
      case 11: return pvector_iterator_arrow(container);
      case 12: return pvector_iterator_plus_equal(container);
      case 13: return pvector_iterator_minus_equal(container);
      default: return 0;
      }
    }

};









#endif // _STAPL_PC_TRACER_H
