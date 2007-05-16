#ifndef P_ALGO_SINGLE_H
#define P_ALGO_SINGLE_H

/* functions defined in this file
 *
 * -Nonmodifying Sequence Operations
 *
 *    UnaryFunc p_for_each(PRange1& pr1, const UnaryFunc f)
 *    Iterator1 p_find(PRange1& pr1, const T& value)
 *    Iterator1 p_find_if(PRange1& pr1, const Predicate& p)
 *    Iterator1 p_find_first_of(PRange1& pr1, Iterator first, Iterator last)
 *    Iterator1 p_find_first_of(PRange1& pr1, Iterator first, Iterator last, 
 *                              BinaryPred bp)
 *    Iterator1 p_adjacent_find(PRange1& pr1)
 *    Iterator1 p_adjacent_find(PRange1& pr1, BinaryPred bp
 *    void p_count(PRange1& pr1, const T& value, Size& S)
 *    void p_count_if(PRange1& pr1, const Predicate& P, Size& S)
 *    Iterator1 p_search(PRange1& pr1, ForIterator first, ForIterator last)
 *    Iterator1 p_search(PRange1& pr1, ForIterator first, ForIterator last, 
 *                       BinaryPred bp)
 *    Iterator1 p_find_end(PRange1& pr1, ForIterator first, ForIterator last)
 *    Iterator1 p_find_end(PRange1& pr1, ForIterator first, ForIterator last, 
 *                         BinaryPred bp)
 *    Iterator1 p_search_n(PRange1& pr1, Integer count, T value)
 *    Iterator1 p_search_n(PRange1& pr1, Integer count, T value, BinaryPred bp)
 *
 * -Modifying Sequence Operations
 *    void p_replace(PRange1& pr1, const T& old_value, const T& new_value)
 *    void p_replace_if(PRange1& pr1, Predicate pred, const T& new_value)
 *    void p_generate(PRange1& pr1, Generator gen)
 *    void p_generate_n(PRange1& pr1, size_t n, Generator gen)
 *    Iterator1 p_remove(PRange1& pr1, const T& val)
 *    Iterator1 p_remove_if(PRange1& pr1, Predicate pred)
 *    Iterator p_remove_copy(PRange1& pr1, Iterator result, const T& val)
 *    Iterator p_remove_copy_if(PRange1& pr1, Iterator result, Predicate pred)
 *    Iterator1 p_unique(PRange1& pr1)
 *    Iterator1 p_unique(PRange1& pr1, BinaryPred bp)
 *    Iterator p_unique_copy(PRange1& pr1, Iterator res)
 *    Iterator p_unique_copy(PRange1& pr1, Iterator res, BinaryPred bp)
 *    void p_reverse(PRange1& pr1)
 *    Iterator p_reverse_copy(PRange1& pr1, Iterator res)
 *    Iterator1 p_rotate(PRange1& pr1, typename Iterator1 middle)
 *
 * -Minimum and Maximum
 *    Iterator1 p_max_element(PRange1& pr)
 *    Iterator1 p_max_element(PRange1& pr1, BinaryPred bp)
 *    Iterator1 p_min_element(PRange1& pr)
 *    Iterator1 p_min_element(PRange1& pr1, BinaryPred bp)
 *
 * -Permutations
 *    bool p_next_permutation(PRange1& pr1)
 *    bool p_next_permutation(PRange1& pr1, StrictWeakOrdering cmp)
 *    bool p_prev_permutation(PRange1& pr1)
 *    bool p_prev_permutation(PRange1& pr1, StrictWeakOrdering cmp)
 */

#include <algorithm>
#include <list>

#include "p_algobase.h"

using std::for_each;
using std::generate;
using std::list;

/**
 * @ingroup for_each
 * pForEach
 */
template <class PRange1, class UrnaryFunc>
UrnaryFunc p_for_each(PRange1& pr1, UrnaryFunc func) {
  list<typename PRange1::subrangeType*> subranges;

  get_subranges_no_deps(pr1, subranges);

  typename list<typename PRange1::subrangeType*>::iterator iter = subranges.begin();
  for ( ; iter != subranges.end(); ++iter) {
    func = for_each((**iter).get_boundary().start(),
                    (**iter).get_boundary().finish(),
                    func);
  }

  return func;
}


/**
 * @ingroup generate
 * pGenerate
 */
template <class PRange, class Generator>
void p_generate(PRange& pr1, Generator gen) {
  list<typename PRange::subrangeType*> subranges;

  get_subranges_no_deps(pr1, subranges);

  typename list<typename PRange::subrangeType*>::iterator iter = subranges.begin();
  for ( ; iter != subranges.end(); ++iter) {
    generate((**iter).get_boundary().start(),
             (**iter).get_boundary().finish(),
             gen);
  }
}



/*
 * pCountIf
 */
class stapl_int {
private:
  int val;
  stapl::rmiHandle handle;
  inline void Sum(int* in, int* inout) { *inout += *in; }

public:

  stapl_int(int v) : val(v) { handle = stapl::register_rmi_object(this); }
  ~stapl_int() { stapl::unregister_rmi_object(handle); }

  inline int value() { return val; }

  int reduce() {
    int result;
    stapl::reduce_rmi( &val, &result, handle, &stapl_int::Sum, true);
    val = result;
    return val;
  }

  inline stapl_int& operator= (const stapl_int& x) {
    val = x.val;
    return *this;
  }

};

/**
 * @ingroup find
 * pCountIf
 */
template <class PRange, class Predicate>
int p_count_if(PRange& pr, Predicate pred) {
  int numLocal = 0;
  list<typename PRange::subrangeType*> subranges;
  get_subranges_no_deps(pr, subranges);
  typename list<typename PRange::subrangeType*>::iterator iter = subranges.begin();

  int numSubranges = subranges.size();
  for (; iter != subranges.end(); ++iter) {
    numLocal += count_if((**iter).get_boundary().start(),
			 (**iter).get_boundary().finish(),
			 pred);
  }

  stapl_int result(numLocal);
  result.reduce();
  return result.value();
}

#endif


/*   
 * pSort
 */

template<typename T>
struct adjustment {
  T* elements;
  int n;
  adjustment(T* p, int count) : elements(p), n(count) { }  
  void define_type(stapl::typer &t) {
    t.dynamic(elements, n);
    t.local(n);
  }
};

template<class PRange, class Compare, typename T>
class p_sample_sort : public stapl::parallel_task {
  PRange *prange;
  Compare *cmp;
  vector<T> tmp;

  bool request_received;
  bool adjust_received;
  vector<T> front_adjust;  //do this for efficiency's sake
                           //for back adjust, just push_back on tmp
  int front_pos;
  int threadID, nThreads;

  void copy_splitters(T* local, T* in, const int count) {
    copy( in, in+count, local );
  }
 
  void insert(const T i) {
    tmp.push_back( i );
  }

  //add_front & ? are only needed for manual redistribution
  //see note at bottom of execute()
  void add_front(const adjustment<T>& adj) {
/*     printf("%d: Add_Front Invoked for %d elements\n", threadID, adj.n);  */
    front_adjust.resize(adj.n);
    for (int i = 0; i < adj.n; i++)
      front_adjust[i] = adj.elements[i];
    request_received = true;
  }

  void element_request(int n) {
/*     printf("%d: Entered Element Request\n", threadID); */
    if (n > tmp.size()) {
      printf("Sorry, can't handle this");
      exit(1);
    }
    front_pos = n;
    stapl::async_rmi(threadID-1, rmiHandle, &p_sample_sort::elements_receive,
		     adjustment<T>(&(tmp[0]),n));
    request_received=true;
  }

  void elements_receive(const adjustment<T> &adj) {
    for (int i=0; i<adj.n; i++)
      tmp.push_back(adj.elements[i]);
    adjust_received = true;
  }

  void notify(void) {
    request_received=true;
  }

 public:
  p_sample_sort(PRange *pr, Compare* compare) : 
		     prange(pr), cmp(compare), request_received (false), 
		     adjust_received(false), front_pos(0) { 
    nThreads = stapl::get_num_threads();
    tmp.reserve(pr->container_global_size() / nThreads);
  }

  void execute() { 
     PRange &pr = *prange; 
     list<typename PRange::subrangeType*> subranges;

     get_subranges_no_deps(pr, subranges); 
     if (subranges.size() != 1) {
       printf("p_sort one subrang per /processor at the moment\n");
     }
     
     threadID = stapl::get_thread_id();
     int size = pr.container_global_size() / nThreads;
     //int local_size = pr.container_local_size();
     int local_size = size;
     const int oversampling = ( 128 > size ) ? size : 128;
     const int sampleRate = size / oversampling;
     
     T *localSplitters = static_cast<T*> 
       (malloc(sizeof(T) * nThreads * oversampling)); 
     
     typename list<typename PRange::subrangeType*>::iterator iter = subranges.begin(); 
 /*     for (; iter != subranges.end(); ++iter) {  */
    
       for( int i=0; i<oversampling; i++ ) 
	 localSplitters[i+threadID*oversampling] =  
	   *(((**iter).get_boundary().start()) + (i*sampleRate));         

       stapl::rmi_fence();       
       for( int i=0; i<nThreads; i++ ) {
	 stapl::broadcast_rmi( &localSplitters[i*oversampling], oversampling,
			       rmiHandle, &p_sample_sort::copy_splitters, i );
       }

       sort( &localSplitters[0], &localSplitters[nThreads*oversampling], *cmp);
       
       T *globalSplitters = static_cast<T*>( malloc(sizeof(T) * nThreads-1));
       for( int i=0; i<nThreads-1; i++ )
	 globalSplitters[i] = localSplitters[(i+1)*oversampling];
       free( localSplitters );
       
       for( int i=0; i<size; i++ ) {
	 int thread = std::lower_bound( &globalSplitters[0], &globalSplitters[nThreads-1], 
					*(((**iter).get_boundary().start()) + i), *cmp ) - &globalSplitters[0];
	 stapl::async_rmi( thread, rmiHandle, &p_sample_sort::insert, *(((**iter).get_boundary().start()) + i));
       }

       stapl::rmi_fence();            
       sort(tmp.begin(), tmp.end(), *cmp); 
       stapl::rmi_fence();
       pr.resize(tmp.size()); 
       get_subranges_no_deps(pr, subranges); 
       iter = subranges.begin(); 
        copy(tmp.begin(), tmp.end(), (**iter).get_boundary().start()); 

       //The below shouldn't really be needed. It's here to manually
       //redistribute the data to set to that existing before the 
       //sample sort messed up the relative sizes of the local containers.
       //Once the pcontainers can support a function call like 
       //(change_distribution(distribution_description, move_elements=false)
       //this can go away and the following will be sufficient:
       //   copy(tmp.begin(), tmp.end(), (**iter).get_boundary().start());
/*        if (nThreads > 1) { */
/*        //Thread 0 */
/*        if (0 == threadID) { */
/* 	 if (tmp.size() != local_size) { */
/* 	   if (tmp.size() < local_size) { */
/* 	     stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::element_request, */
/* 			      local_size - tmp.size()); */
/* 	     while (!adjust_received) { stapl::rmi_flush(); stapl::rmi_poll(); } */
/* 	   } else { */
/* 	     stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::add_front, */
/* 			      adjustment<T>(&(tmp[local_size]), tmp.size()-local_size)); */
/* 	     tmp.resize(local_size); */
/* 	   } */
/* 	 } else { */
/* 	   stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::notify); */
/* 	 } */

/* 	 copy(tmp.begin(), tmp.end(), (**iter).get_boundary().start()); */

/*        //Last Thread */
/*        } else if (nThreads-1 == threadID) { */
/* 	 while (!request_received) { stapl::rmi_flush(); stapl::rmi_poll(); } */
/* 	 if (front_adjust.size()) { */
/* 	   copy(front_adjust.begin(), front_adjust.end(), (**iter).get_boundary().start()); */
/* 	 } */
/* 	 copy (tmp.begin() + front_pos, tmp.end(), */
/* 	       (**iter).get_boundary().start() + front_adjust.size()); */

/*        //Threads in between */
/*        } else { */
/* 	 while(!request_received) { stapl::rmi_flush(); stapl::rmi_poll(); } */
/* 	 int myNewSize = tmp.size() + front_adjust.size(); */
/* 	 if (myNewSize != local_size) { */
/* 	   if (myNewSize < local_size) { */
/* 	     stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::element_request, */
/* 			      local_size - myNewSize); */
/* 	     while (!adjust_received) { stapl::rmi_flush(); stapl::rmi_poll(); } */
/* 	   } else { */
/* 	     stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::add_front, */
/* 			      adjustment<T>(&(tmp[local_size]), myNewSize-local_size)); */
/* 	     tmp.resize(local_size - front_adjust.size()); */
/* 	   } */
/* 	 } else { */
/* 	   stapl::async_rmi(threadID+1, rmiHandle, &p_sample_sort::notify); */
/* 	 } */
/* 	 if (front_adjust.size()) { */
/* 	   copy(front_adjust.begin(), front_adjust.end(), (**iter).get_boundary().start()); */
/* 	 } */
/*  	 copy (tmp.begin() + front_pos, tmp.end(),  */
/*  	       (**iter).get_boundary().start() + front_adjust.size()); 	   */
/*        } */
/*        } else { */
/* 	 copy(tmp.begin(), tmp.end(),(**iter).get_boundary().start()); */
/*      } */
       stapl::rmi_fence();
  }
};


/**   
 * @ingroup sort
 * pSort
 */
template<class PRange, class Compare>
void p_sort(PRange &pr, Compare cmp) {

  p_sample_sort<PRange,Compare,
    typename PRange::iteratorType::value_type> 
    pss(&pr, &cmp);

  stapl::execute_parallel_task( &pss );
}
