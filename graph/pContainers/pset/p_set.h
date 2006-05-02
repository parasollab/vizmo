////////////////////////////////////////////////////////////////////////////
// File :   p_set.h           
// Despcription : Class description p_set : based on  2,3 tree
// Author  : Alin Jula
// Date :       08/03/02                                                       
// Last update :08/03/02
// Texas A&M University,College Station, TX
///////////////////////////////////////////////////////////////////////////

#ifndef _STAPL_PSET_H
#define _STAPL_PSET_H
#include <23tree_container.h>
#include <runtime.h>
#include <vector>
#include <runtime.h>
//#include "private_mechanism.h"
#include "rmitools.h"
#include "BasePContainer.h"

using namespace _STLP_STD;


namespace stapl {
template <class _Key, __DFL_TMPL_PARAM(_Compare,less<_Key>), 
                     _STLP_DEFAULT_ALLOCATOR_SELECT(_Key) >
  class p_set :public  BasePContainer<p_set<_Key,_Compare,_Alloc>,vector<_Key>,pair<int,int>  >
{
public:
// typedefs:
  typedef _Key     key_type;
  typedef _Key     value_type;
  typedef _Compare key_compare;
  typedef _Compare value_compare;
private:
  typedef _23tree<key_type, value_type, 
    _Identity<value_type>, key_compare, _Alloc> _Rep_type;
public:
  typedef typename _Rep_type::pointer pointer;
  typedef typename _Rep_type::const_pointer const_pointer;
  typedef typename _Rep_type::reference reference;
  typedef typename _Rep_type::const_reference const_reference;
  typedef typename _Rep_type::const_iterator const_iterator;
  typedef const_iterator iterator;
  typedef typename _Rep_type::const_reverse_iterator reverse_iterator;
  typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
  typedef typename _Rep_type::size_type size_type;
  typedef typename _Rep_type::difference_type difference_type;
  typedef typename _Rep_type::allocator_type allocator_type;

private:
  _Rep_type __data;  // 2-3 tree representing p_set
public:

  // allocation/deallocation

  p_set() : __data(_Compare(), allocator_type()) 
    {
      register_object();
    }

  explicit p_set(const _Compare& __comp,
	       const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    {  
      register_object();
    }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  p_set(_InputIterator __first, _InputIterator __last)
    : __data(_Compare(), allocator_type())
    {
      register_object();
      __data.insert_unique(__first, __last); 
    }

#else
  p_set(const value_type* __first, const value_type* __last) 
    : __data(_Compare(), allocator_type()) 
    {
      register_object();
      __data.insert_unique(__first, __last); 
    }

  p_set(const value_type* __first, 
      const value_type* __last, const _Compare& __comp,
      const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    { 
      register_object();
      __data.insert_unique(__first, __last); 
    }

  p_set(const_iterator __first, const_iterator __last)
    : __data(_Compare(), allocator_type()) 
    {
      register_object();
      __data.insert_unique(__first, __last); 
    }

  p_set(const_iterator __first, const_iterator __last, const _Compare& __comp,
      const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    {
      register_object();
      __data.insert_unique(__first, __last); 
    }
#endif /* _STLP_MEMBER_TEMPLATES */

  p_set(const p_set<_Key,_Compare,_Alloc>& __x) : __data(__x.__data) 
    {
      register_object();
    }
  p_set<_Key,_Compare,_Alloc>& operator=(const p_set<_Key, _Compare, _Alloc>& __x)
  { 

    __data = __x.__data; 
    return *this;
  }


  void register_object()
    {
      this->Handle=register_rmi_object(this);
    }

  // accessors:

  key_compare key_comp() const { return __data.key_comp(); }
  value_compare value_comp() const { return __data.key_comp(); }
  allocator_type get_allocator() const { return __data.get_allocator(); }

  iterator local_begin() const { return __data.begin(); }
  iterator local_end() const { return __data.end(); }
  reverse_iterator local_rbegin() const { return __data.rbegin(); } 
  reverse_iterator local_rend() const { return __data.rend(); }

  /**
   *\b GLOBAL
   */

  bool empty() const 
    {
      vector<size_t>  __empty(get_num_threads());
      __empty[get_thread_id()]=__data.empty();
      for(int i=0;i<get_num_threads();i++)
	broadcast(i,&__empty[i],1);
      for(int i=0;i<get_num_threads();i++)
	if (__empty[i]==false ) return false;
      
    return true;
    }

  /**
   *\b LOCAL
   */
  bool local_empty()
    {
      return __data.empty();
    }

  /**
   * \b GLOBAL
   */
  size_type size() const 
    {
      vector<size_t> sizes(get_num_threads());
      size_t global_size=0;
      sizes[get_thread_id()]=__data.size();
      for(int i=0;i<get_num_threads();i++)
	{
	  broadcast(i,&sizes[i],1);
	  global_size+=sizes[i];
	}      
    return global_size;
    }

  size_type local_size() const
    {
      return __data.size();
    }

  size_type max_size() const { return __data.max_size(); }
  void swap(p_set<_Key,_Compare,_Alloc>& __x) { __data.swap(__x.__data); }

  // insert/erase
  pair<iterator,bool> insert( const value_type& __x) { 
    typedef typename _Rep_type::iterator _Rep_iterator;
    pair<_Rep_iterator, bool> __p = __data.insert_unique(__x); 
    return pair<iterator, bool>(__REINTERPRET_CAST(const iterator&,__p.first), __p.second);
  }
  iterator insert(iterator __position, const value_type& __x) {
    typedef typename _Rep_type::iterator _Rep_iterator;
    return __data.insert_unique((_Rep_iterator&)__position, __x);
  }
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert(_InputIterator __first, _InputIterator __last) {
    __data.insert_unique(__first, __last);
  }
#else
  void insert(const_iterator __first, const_iterator __last) {
    __data.insert_unique(__first, __last);
  }
  void insert(const value_type* __first, const value_type* __last) {
    __data.insert_unique(__first, __last);
  }
#endif /* _STLP_MEMBER_TEMPLATES */
  void erase(iterator __position) { 
    typedef typename _Rep_type::iterator _Rep_iterator;
    __data.erase((_Rep_iterator&)__position); 
  }
  size_type erase(const key_type& __x) { 
    return __data.erase(__x); 
  }
  void erase(iterator __first, iterator __last) { 
    typedef typename _Rep_type::iterator _Rep_iterator;
    __data.erase((_Rep_iterator&)__first, (_Rep_iterator&)__last); 
  }
  void clear() { __data.clear(); }

  // p_set operations:

  iterator find(const key_type& __x) const { return __data.find(__x); }

  size_type count(const key_type& __x) const { 
    return __data.find(__x) == __data.end() ? 0 : 1 ; 
  }
  iterator lower_bound(const key_type& __x) const {
    return __data.lower_bound(__x);
  }
  iterator upper_bound(const key_type& __x) const {
    return __data.upper_bound(__x); 
  }
  pair<iterator,iterator> equal_range(const key_type& __x) const {
    return __data.equal_range(__x);
  }

  /** 
   * DIstribution methods
   */
void MergeSubContainers ()
  {
  }
//BuildSubContainer (const SUBTASK &, CONTAINER &)
 void BuildSubContainer(const Subtask_type & a, Container_type & b)
   {
   }
 //ComputeDistribution (const _STL::vector<size_t, _STL::allocator<size_t> > &) 
 void ComputeDistribution(const vector<size_t>& __splitters)
   {
   }

};

template <class _Key, __DFL_TMPL_PARAM(_Compare,less<_Key>), 
                     _STLP_DEFAULT_ALLOCATOR_SELECT(_Key) >
class p_multiset :public  BasePContainer<p_multiset<_Key,_Compare,_Alloc>,vector<_Key>,pair<int,int>  >
 {
public:
  // typedefs:

  typedef _Key     key_type;
  typedef _Key     value_type;
  typedef _Compare key_compare;
  typedef _Compare value_compare;
private:
  typedef _23tree<key_type, value_type, 
                  _Identity<value_type>, key_compare, _Alloc> _Rep_type;
public:
  typedef typename _Rep_type::pointer pointer;
  typedef typename _Rep_type::const_pointer const_pointer;
  typedef typename _Rep_type::reference reference;
  typedef typename _Rep_type::const_reference const_reference;
  typedef typename _Rep_type::const_iterator const_iterator;
  typedef const_iterator iterator;
  typedef typename _Rep_type::const_reverse_iterator reverse_iterator;
  typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
  typedef typename _Rep_type::size_type size_type;
  typedef typename _Rep_type::difference_type difference_type;
  typedef typename _Rep_type::allocator_type allocator_type;

private:
  _Rep_type __data;  // 2,3 tree representing p_multiset
public:
  // allocation/deallocation

  p_multiset() : __data(_Compare(), allocator_type()) {}
  explicit p_multiset(const _Compare& __comp,
                    const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    {
      register_object();
    }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  p_multiset(_InputIterator __first, _InputIterator __last)
    : __data(_Compare(), allocator_type())
    {
      register_object();
      __data.insert_equal(__first, __last); 
    }

  template <class _InputIterator>
  p_multiset(_InputIterator __first, _InputIterator __last,
           const _Compare& __comp,
           const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : __data(__comp, __a) 
    {
      register_object(); 
      __data.insert_equal(__first, __last);
    }

#else

  p_multiset(const value_type* __first, const value_type* __last)
    : __data(_Compare(), allocator_type())
    { 
      register_object();
      __data.insert_equal(__first, __last);
    }

  p_multiset(const value_type* __first, const value_type* __last,
           const _Compare& __comp,
           const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    { 
      register_object();
      __data.insert_equal(__first, __last);
    }

  p_multiset(const_iterator __first, const_iterator __last)
    : __data(_Compare(), allocator_type())
    {
      register_object();
      __data.insert_equal(__first, __last);
    }

  p_multiset(const_iterator __first, const_iterator __last,
           const _Compare& __comp,
           const allocator_type& __a = allocator_type())
    : __data(__comp, __a) 
    {
      register_object(); 
      __data.insert_equal(__first, __last);
    }
  
#endif /* _STLP_MEMBER_TEMPLATES */

  p_multiset(const p_multiset<_Key,_Compare,_Alloc>& __x) : __data(__x.__data) 
    {
      register_object();
    }

  p_multiset<_Key,_Compare,_Alloc>&
  operator=(const p_multiset<_Key,_Compare,_Alloc>& __x) {
    __data = __x.__data; 
    return *this;
  }

 void register_object()
    {
      this->Handle=register_rmi_object(this);
    }

  // accessors:

  key_compare key_comp() const { return __data.key_comp(); }
  value_compare value_comp() const { return __data.key_comp(); }
  allocator_type get_allocator() const { return __data.get_allocator(); }

  iterator local_begin() const { return __data.begin(); }
  iterator local_end() const { return __data.end(); }
  reverse_iterator local_rbegin() const { return __data.rbegin(); } 
  reverse_iterator local_rend() const { return __data.rend(); }


  /**
   *\b GLOBAL
   */
  bool empty() const 
    {
      vector<size_t>  __empty(get_num_threads());
      __empty[get_thread_id()]=__data.empty();
      for(int i=0;i<get_num_threads();i++)
	broadcast(i,&__empty[i],1);
      for(int i=0;i<get_num_threads();i++)
	if (__empty[i]==false ) return false;
      
    return true;
    }
 /**
   *\b LOCAL
   */
 bool local_empty() const 
    {
      return __data.empty();
    }
  /**
   *\b GLOBAL
   */
  size_type size() const 
    {
      vector<size_t> sizes(get_num_threads());
      size_t global_size=0;
      sizes[get_thread_id()]=__data.size();
      for(int i=0;i<get_num_threads();i++)
	{
	  broadcast(i,&sizes[i],1);
	  global_size+=sizes[i];
	}      
    return global_size;
    }

 /**
   *\b LOCAL
   */
  size_type local_size() const 
    {
      __data.size();
    }

  size_type max_size() const { return __data.max_size(); }
  void swap(p_multiset<_Key,_Compare,_Alloc>& __x) { __data.swap(__x.__data); }

  // insert/erase
  iterator insert(const value_type& __x) { 
    return __data.insert_equal(__x);
  }
  iterator insert(iterator __position, const value_type& __x) {
    typedef typename _Rep_type::iterator _Rep_iterator;
    return __data.insert_equal((_Rep_iterator&)__position, __x);
  }

#ifdef _STLP_MEMBER_TEMPLATES  
  template <class _InputIterator>
  void insert(_InputIterator __first, _InputIterator __last) {
    __data.insert_equal(__first, __last);
  }
#else
  void insert(const value_type* __first, const value_type* __last) {
    __data.insert_equal(__first, __last);
  }
  void insert(const_iterator __first, const_iterator __last) {
    __data.insert_equal(__first, __last);
  }
#endif /* _STLP_MEMBER_TEMPLATES */
  void erase(iterator __position) { 
    typedef typename _Rep_type::iterator _Rep_iterator;
    __data.erase((_Rep_iterator&)__position); 
  }
  size_type erase(const key_type& __x) { 
    return __data.erase(__x); 
  }
  void erase(iterator __first, iterator __last) { 
    typedef typename _Rep_type::iterator _Rep_iterator;
    __data.erase((_Rep_iterator&)__first, (_Rep_iterator&)__last); 
  }
  void clear() { __data.clear(); }

  // p_multiset operations:

  iterator find(const key_type& __x) const { return __data.find(__x); }

  size_type count(const key_type& __x) const { return __data.count(__x); }
  iterator lower_bound(const key_type& __x) const {
    return __data.lower_bound(__x);
  }
  iterator upper_bound(const key_type& __x) const {
    return __data.upper_bound(__x); 
  }
  pair<iterator,iterator> equal_range(const key_type& __x) const {
    return __data.equal_range(__x);
  }
};

} //end namespace stapl;
#endif //_STAPL_PSET_H
