////////////////////////////////////////////////////////////////////////////
// File :   23tree.h           
// Despcription : Class description for 2,3 tree
// Author  : Alin Jula
// Date :       02/25/00                                                       
// Last update :08/02/02
// Texas A&M University,College Station, TX
///////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <functional>
#include <vector>
#include <stack>
using namespace _STLP_STD;

/*
 *\class _23tree_node 
 *\brief : node structure : keeps 8 pointers:
 * left_link - pointer to the left child
 * mid_link - pointer to the middle child
 * right_link - pointer to the right child (if 0 then there is not right child)
 * parent_link - pointer to the parent
 * left_value - pointer to the node with the maximum value  in the left  subtree.
 * mid_value - pointer to the node with the maximum value  in the middle  subtree.
 * right_value - pointer to the node with the maximum value  in the right subtree.
 * value - pointer to the node with the maximum value  in this subtree (it's either mid_value or right_value)
 * __count_node - number of nodes in its subtree.
 * IMPORTANT - if the node is a leaf , then the mid_link and right_link are used as links for the leaf predecessor and successor 
 * this is used by the 23tree_iterator.The leaves in the 23tree are kept in a double-linked list (using mid_link and right_link as 
 * predecessor and successor pointers)
 **/

template<class T>
class _23tree_node {
  public :
  
    typedef _23tree_node<T>* _Link_type;
  typedef  T* _Value_link_type;
  typedef T _Value_type;
  _23tree_node() 
    {
      left_value=mid_value=right_value=value=NULL;      
      left_link=mid_link=right_link=parent_link=NULL;
      __count_node=1;
    }
  // data
  _Value_link_type left_value;
  _Value_link_type mid_value;
  _Value_link_type right_value;
  _Value_link_type value;

  _Link_type left_link;
  _Link_type mid_link;
  _Link_type right_link;
  _Link_type parent_link;

  size_t __count_node;

  //methods
  bool isLeaf() 
    {
      return (left_link==0);
    } // Alin : assuming the invariant that a node is a leaf iff its left link is nonNULL
  bool first_level_above_leaf()
    {
      return ((left_link!=0)&&(left_link->left_link==0));
    }
  size_t get_height()
    {
      size_t height=0;
      _Link_type __x=this;
      while (__x!=0)
	{++height;__x=__x->left_link;}
      return height;
    }

};



/*
 *\class _23_tree_iterator
 *\brief Iterator defined on the 23tree. Bidirectional iterator which uses the mid_link and right_link of the leaves for move
 * backwards or forward.
 */

template <class _Value, class _Traits>
struct _23_tree_iterator
{
  //types
  typedef _23tree_node<_Value>*        _Base_ptr;
  typedef bidirectional_iterator_tag iterator_category;
  typedef ptrdiff_t                  difference_type;
  typedef _Value value_type;
  typedef typename _Traits::reference  reference;
  typedef typename _Traits::pointer    pointer;
  typedef _23_tree_iterator<_Value, _Traits> _Self;

  //pointer to the current node
  _Base_ptr __node;

  //operator ==

  bool operator==(const _23_tree_iterator& __y) const
  {
    return __node == __y.__node;
  }

  bool operator!=(const _23_tree_iterator& __y) const
{
  return __node != __y.__node;
}

  _23_tree_iterator() 
  { 
    __node = 0; 
  }
  _23_tree_iterator(_Base_ptr __x) 
  { 
    __node = __x; 
  }
  _23_tree_iterator(const _23_tree_iterator<_Value, _Nonconst_traits<_Value> >& __it) 
  { 
    __node = __it.__node; 
  }

  reference operator*() const { 
    return *(__node->value); 
  }
  

  _Self& operator++() 
  { 
    __node = __node->right_link; 
    return *this; 
  }
  _Self operator++(int) {
    _Self __tmp = *this;
    __node = __node->right_link;
    return __tmp;
  }
    
  _Self& operator--() 
  { 
    __node = __node->mid_link; 
    return *this; 
  }
  _Self operator--(int) {
    _Self __tmp = *this;
    __node = __node->mid_link;
    return __tmp;
  }
};
/*
template <class _Value, class _Traits>
bool _23_tree_iterator<class _Value, class _Traits>::operator==(const _23_tree_iterator& __y) const
{
  return __node == __y.__node;
}


template <class _Value, class _Traits>
bool _23_tree_iterator<class _Value, class _Traits>::operator!=(const _23_tree_iterator& __y) const 
{
  return __node != __y.__node;
}
*/



/* 
 *\class _23tree
 *\brief 23tree container which deals also with equal keys.
 */

template <class _Key, class _Value, class _KeyOfValue,  class _Compare,  _STLP_DEFAULT_ALLOCATOR_SELECT(_Value)>
class _23tree {
  protected :
    typedef _23tree_node<_Value> * _Base_ptr;
  _Base_ptr __header;
  _Compare _key_compare;

   long int __node_alloc;
   long int __value_alloc;

public:

   typedef _Key key_type;
   typedef _Value value_type;
   typedef value_type* pointer;
   typedef const value_type* const_pointer;
   typedef value_type& reference;
   typedef const value_type& const_reference;
   typedef _23tree_node<_Value>* _Link_type;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef bidirectional_iterator_tag _Iterator_category;
   
   typedef _23_tree_iterator<value_type, _Nonconst_traits<value_type> > iterator;
   typedef _23_tree_iterator<value_type, _Const_traits<value_type> > const_iterator;
   
  _STLP_DECLARE_BIDIRECTIONAL_REVERSE_ITERATORS;

  typedef typename _Alloc_traits<value_type, _Alloc>::allocator_type allocator_type;
  typedef _23tree<_Key,_Value,_KeyOfValue,_Compare ,allocator_type> this_type;

 public:
  _23tree() : _key_compare(_Compare())
    { initialize_tree();}
  
  _23tree(const _Compare& __comp) : _key_compare(__comp)
    { initialize_tree();}

  _23tree(const _Compare& __comp, const allocator_type & a) : _key_compare(__comp)
    { initialize_tree();}

  _23tree(const _23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc>& __x) 
    {
      __copy(__x);
    }

  ~_23tree()
    { 
      clear();
      __deallocate_node(__header);
      printf("\n Nodes not freed %d",__node_alloc);
      printf("\n Values not freed %d",__value_alloc);
      
    }


  
  _Compare key_comp() const { return _key_compare; }

  iterator begin() 
    {
      return iterator(leftmost()); 
    }
  const_iterator begin() const 
    { 
      return const_iterator(leftmost()); 
    }
  iterator end() 
    { 
      return iterator(this->__header); 
    }
  const_iterator end() const 
    { 
      return const_iterator(this->__header); 
    }
  
  reverse_iterator rbegin() 
    { 
      return reverse_iterator(end()); 
    }
  const_reverse_iterator rbegin() const 
    { 
      return const_reverse_iterator(end()); 
    }
  reverse_iterator rend() 
    { 
      return reverse_iterator(begin()); 
    }
  const_reverse_iterator rend() const 
    { 
      return const_reverse_iterator(begin());
    } 

  bool empty() const 
    {
      return  __header->parent_link==__header;
    }

  size_type max_size() const 
    { 
      return size_type(-1); 
    }

  void clear() 
    {      
      if (!empty())
	__erase(__root());
    }
  



  void swap(_23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __t) {
    _STLP_STD::swap(this->__header, __t.__header);
    _STLP_STD::swap(_key_compare, __t._key_compare);
  }


 protected:
  value_type * __allocate_value()
    { 
      __value_alloc++;
      return new value_type; 
    }
  _Base_ptr  __allocate_node()
    {
      __node_alloc++;
      return new _23tree_node<_Value>;
    }
  this_type * __allocate_tree() 
    {
      this_type * p=new this_type;
      p->initialize_tree();
      return p;
    }
      

  void __deallocate_node(_Base_ptr __x) 
    {
      delete __x; 
      __node_alloc--;
    }
  void __deallocate_value(value_type* __x) 
    { delete __x;
    __value_alloc--;
    }
  void __deallocate_tree(this_type * __x) 
    {
      delete __x;
    }

  _Base_ptr __root() { return __header->parent_link;}

  _Base_ptr __root() const { return __header->parent_link;}


  _Base_ptr leftmost() const 
    {
      return __header->left_link;
    }
  _Base_ptr rightmost() const
    {
      return __header->right_link;
    }
    
 public:

  pair<iterator,bool> insert_unique(const  value_type &val)
    {
      bool node_exists=false;

      _Base_ptr where=find_node_to_insert(val,node_exists);

      _Base_ptr new_node;

      if (!node_exists)
	{
	  new_node=__allocate_node();
	  
	  new_node->value=__allocate_value();
	  
	  *(new_node->value)=val;

	  addson(where,new_node);
	  
	  return pair<iterator,bool>(iterator(new_node),true);

	}
      else 
	return pair<iterator,bool>(end(),false);
    }


  iterator insert_equal(const value_type& val)
    {
      bool node_exists=false;

      _Base_ptr where=find_node_to_insert(val,node_exists);
      
      _Base_ptr new_node=__allocate_node();
      
      new_node->value=__allocate_value();
	  
      *(new_node->value)=val;

      
      addson(where,new_node);
      
      return iterator(new_node);
    }

#ifdef _STLP_MEMBER_TEMPLATES  
  template<class _II>
  void insert_equal(_II __first, _II __last) {
    for ( ; __first != __last; ++__first)
      insert_equal(*__first);
  }
  template<class _II>
  void insert_unique(_II __first, _II __last) {
    for ( ; __first != __last; ++__first)
      insert_unique(*__first);
  }
#else /* _STLP_MEMBER_TEMPLATES */
  void insert_unique(const_iterator __first, const_iterator __last) {
    for ( ; __first != __last; ++__first)
      insert_unique(*__first);
  }
  void insert_unique(const value_type* __first, const value_type* __last) {
    for ( ; __first != __last; ++__first)
      insert_unique(*__first);
  }
  void insert_equal(const_iterator __first, const_iterator __last) {
    for ( ; __first != __last; ++__first)
      insert_equal(*__first);
  }
  void insert_equal(const value_type* __first, const value_type* __last) {
    for ( ; __first != __last; ++__first)
      insert_equal(*__first);
  }
#endif /* _STLP_MEMBER_TEMPLATES */







  size_type size() const
    {
      if (empty()) return 0;
      else 
	return __root()->__count_node;
    }


 void erase(iterator __position) 
   {
     printf("\n To be implemented");

   }
 

  size_t erase (const value_type& val)
    {
      if (empty()) return 0;
      if (__root()->isLeaf())
	{
	  if (_KeyOfValue()(val)==_KeyOfValue()(*(__root()->value))) 
	    {
	      __deallocate_value(__root()->value);
	      __header->parent_link=__header;	      
	      __header->left_link=__header->right_link=__header;
	    }
	  return 0;
	}
      bool done, son1;
      bool duplicate;// stores whether or not there are some other nodes with the same value
      do { // erase also all the nodes with the same value
	duplicate=false;
	done=son1=false;
	if (__root()->isLeaf())
	  if (_KeyOfValue()(*(__root()->value))==_KeyOfValue()(val))
	    {
	      __deallocate_node(__root());
	      __header->left_link=__header;
	      __header->right_link=__header;
	      __header->parent_link=__header;
	    }
	  else {duplicate=false;break;}
	delete_node(__root(),val,done,son1,duplicate);
	if (__root()->mid_link==0)
	  {
	    _Base_ptr __interim_root=__root()->left_link;
	    __deallocate_node(__root());
	    __header->parent_link=__interim_root;
	    __interim_root->parent_link=__header;
	  }
      }
      while ( duplicate);

      return size();
    }

  void erase(iterator __first, iterator __last) {
    if (__first == begin() && __last == end())
      clear();
    else
      while (__first != __last) erase(__first++);
  }

  void erase(const key_type* __first, const key_type* __last) {
    while (__first != __last) erase(*__first++);
  }

 iterator find(const key_type& __key) 
   {
    if (empty()) return end();
      if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
	return end();
      _Base_ptr __x=__root();
      while (!__x->isLeaf())
	{
	  if ((_key_compare(__key,_KeyOfValue()(*(__x->left_value)))) ||  (_KeyOfValue()(*(__x->left_value))==__key))

	    __x=__x->left_link;
	  else 
	    if ((_key_compare(__key,_KeyOfValue()(*(__x->mid_value))))||(_KeyOfValue()(*(__x->mid_value))==__key))
	      __x=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if ((_key_compare(__key,_KeyOfValue()(*(__x->right_value))))||(_KeyOfValue()(*(__x->right_value))==__key))
		  __x=__x->right_link;
		else
		  return end();
	      else
		return end();
	}
      // __x is leaf
      if (__key=_KeyOfValue()(*(__x->value)))
	return iterator(__x);
      else 
	return end();

   }
 
 const_iterator find(const key_type& __key) const { 
    if (empty()) return end();
      if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
	return end();
      _Base_ptr __x=__root();
      while (!__x->isLeaf())
	{
	  if ((_key_compare(__key,_KeyOfValue()(*(__x->left_value))))||(_KeyOfValue()(*(__x->left_value))==__key))
	    __x=__x->left_link;
	  else 
	    if ((_key_compare(__key,_KeyOfValue()(*(__x->mid_value))))||(_KeyOfValue()(*(__x->mid_value))==__key))
	      __x=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if ((_key_compare(__key,_KeyOfValue()(*(__x->right_value))))||(_KeyOfValue()(*(__x->right_value))==__key))
		  __x=__x->right_link;
		else
		  return end();
	      else
		return end();
	}
      // __x is leaf
      if (__key=_KeyOfValue()(*(__x->value)))
	return iterator(__x);
      else 
	return end();
 }


 size_type count(const key_type& __x) const
   {
     pair<const_iterator, const_iterator> __p = equal_range(__x);
     size_type __n = distance(__p.first, __p.second);
     return __n;
   }
 
  iterator lower_bound(const key_type& __key) 
    { 
     if (empty()) return end();
      if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
	return end();
      _Base_ptr __x=__root();
      while (!__x->isLeaf())
	{
	  if ((_key_compare(__key,_KeyOfValue()(*(__x->left_value))))||(_KeyOfValue()(*(__x->left_value))==__key))
	    __x=__x->left_link;
	  else 
	    if ((_key_compare(__key,_KeyOfValue()(*(__x->mid_value))))||(_KeyOfValue()(*(__x->mid_value))==__key))
	      __x=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if ((_key_compare(__key,_KeyOfValue()(*(__x->right_value))))||(_KeyOfValue()(*(__x->right_value))==__key))
		  __x=__x->right_link;
		else
		  return end();
	      else
		return end();
	}
      // __x is leaf
      
      return iterator(__x);
      
    }
 

  const_iterator lower_bound(const key_type& __key) const 
    { 

      if (empty()) return end();
      if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
	return end();
      _Base_ptr __x=__root();
      while (!__x->isLeaf())
	{
	  if ((_key_compare(__key,_KeyOfValue()(*(__x->left_value))))||(_KeyOfValue()(*(__x->left_value))==__key))
	    __x=__x->left_link;
	  else 
	    if ((_key_compare(__key,_KeyOfValue()(*(__x->mid_value))))||(_KeyOfValue()(*(__x->mid_value))==__key))
	      __x=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if ((_key_compare(__key,_KeyOfValue()(*(__x->right_value))))||(_KeyOfValue()(*(__x->right_value))==__key))
		  __x=__x->right_link;
		else
		  return end();
	      else
		return end();
	}
      // __x is leaf
      
      return iterator(__x);
    }
  

  iterator upper_bound(const key_type& __key) 
    { 


     if (empty()) return end();
     if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
       return end();
     _Base_ptr __y,__x;
     __y=__x=__root();
     while (!__x->isLeaf())
	{
	  if (!(_key_compare(_KeyOfValue()(*(__x->left_value)),__key)||(_KeyOfValue()(*(__x->left_value))==__key)))
	    __y=__x->left_link;
	  else 
	    if (!(_key_compare(_KeyOfValue()(*(__x->mid_value)),__key)||(_KeyOfValue()(*(__x->mid_value))==__key)))
	      __y=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if (!(_key_compare(_KeyOfValue()(*(__x->right_value)),__key)||(_KeyOfValue()(*(__x->right_value))==__key)))
		  __y=__x->right_link;
		else
		  if (__x==__y) return end();
		  else {}
	      else
		if (__x==__y) return end();
	       
	  __x=__y;
	}

     return iterator(__x);
    }
  
  const_iterator upper_bound(const key_type& __key) const 
    { 
      if (empty()) return end();
      if (_key_compare(__key,_KeyOfValue()(*(leftmost()->value)))||_key_compare(_KeyOfValue()(*(rightmost()->value)),__key))
	return end();
      _Base_ptr __y,__x;
      __y=__x=__root();
      while (!__x->isLeaf())
	{
	  if (!(_key_compare(_KeyOfValue()(*(__x->left_value)),__key)||(_KeyOfValue()(*(__x->left_value))==__key)))
	    __y=__x->left_link;
	  else 
	    if (!(_key_compare(_KeyOfValue()(*(__x->mid_value)),__key)||(_KeyOfValue()(*(__x->mid_value))==__key)))
	      __y=__x->mid_link;
	    else
	      if (__x->right_link!=0)
		if (!(_key_compare(_KeyOfValue()(*(__x->right_value)),__key)||(_KeyOfValue()(*(__x->right_value))==__key)))
		  __y=__x->right_link;
		else
		  if (__x==__y) return end();
		  else {}
	      else
		if (__x==__y) return end();
	  __x=__y;
	}
      
      return iterator(__x);
    }
  
  pair<iterator,iterator> equal_range(const key_type& __key) 
    {
    return pair<iterator, iterator>(lower_bound(__key), upper_bound(__key));
    }
  
  pair<const_iterator, const_iterator> equal_range(const key_type& __key) const 
    {
      return pair<const_iterator,const_iterator>(lower_bound(__key),
						 upper_bound(__key));
    }




  pair<_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*,_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*>   split(value_type val)
    {

      if (empty()) 
	{
	  _23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >* p= __allocate_tree();
	  return pair<_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*,_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*>(p,this);
	}
      _Base_ptr  t1,t2;
      t1=__allocate_node();
      t2=__allocate_node();
      stack<_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >* > __stack1,__stack2;
      _Base_ptr __x=__root();
      while (!__x->isLeaf())
	{
	  // check the left value
	  if (_key_compare(_KeyOfValue()(*(__x->left_value)),_KeyOfValue()(val)))
	      __stack1.push(node_to_tree(__x->left_link));
	  else
	    {
	      if (__x->right_link!=0)
		  __stack2.push(node_to_tree(__x->right_link));
	      __stack2.push(node_to_tree(__x->mid_link));
	      __x=__x->left_link;
	      continue;
	    }
	  // check the mid value

	  if (_key_compare(_KeyOfValue()(*(__x->mid_value)),_KeyOfValue()(val)))
	      __stack1.push(node_to_tree(__x->mid_link));
	  else 
	    {
	      if (__x->right_link!=0)
		  __stack2.push(node_to_tree(__x->right_link));
	      __x=__x->mid_link;
	      continue;
	    }
	  
	  // the right value


	  if (__x->right_value!=0)
	    {

	      if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->right_value)))||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->right_value))))
	      __x=__x->right_link;
		
	    else
	      {
		  printf("\n This value is not in this tree"); 
		  _23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >* p= __allocate_tree();
		  return pair<_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*,_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*>(p,this);
	      }
	    }
	}

      // __x is leaf
      if (_key_compare(_KeyOfValue()(*(__x->value)),_KeyOfValue()(val)))
	  __stack1.push(node_to_tree(__x));
      else
	  __stack2.push(node_to_tree(__x));
      // done separating the nodes
      
      _23tree<_Key,_Value,_KeyOfValue(),_Compare, _Alloc >* current_tree,*left_tree,*right_tree; 
      left_tree=__allocate_tree();
      right_tree=__allocate_tree();
      if (__stack1.size()!=0)
	{
	  left_tree=__stack1.top();
	  __stack1.pop();
	  while (!__stack1.empty())
	    {
	      current_tree=__stack1.top();
	      __stack1.pop();
	      current_tree->splice(*left_tree);
	      left_tree=current_tree;
	    }
	}
      if (__stack2.size()!=0)
	{
	  right_tree=__stack2.top();
	  __stack2.pop();
	  
	  while (!__stack2.empty())
	    {
	      current_tree=__stack2.top();
	      __stack2.pop();
	      right_tree->splice(*current_tree);
	    }
	}

      // clear this tree
      __header->parent_link=__header;
      __header->left_link=__header;
      __header->right_link=__header;
      return pair<_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*,_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >*>(left_tree,right_tree);
      
    }



  void splice(_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc > & __second_tree)
    // Assumption : all the elements in the second tree are greater than all the elements of this tree
    {
      if (__second_tree.empty()) {return ;}
      if (empty()) 
	{
	  __deallocate_node(__header);
	  __header=__second_tree.__header;
	  __second_tree.initialize_tree();
	  return ;
	}
      if (_key_compare(_KeyOfValue()(*(__second_tree.leftmost()->value)),_KeyOfValue()(*(rightmost()->value))))
	{
	  printf("\n Second tree has elements which are less than the maximum element in the primary tree. Splice cannot be performed\n");
	  return;
	}

      if (__root()->get_height()==__second_tree.__root()->get_height())
	{
	  //create a new root and set the links
	  _Base_ptr  new_root=__allocate_node();
	  new_root->parent_link=__header;
	  new_root->left_link=__root();
	  new_root->mid_link=__second_tree.__root();
	  new_root->left_value=__root()->value;
	  new_root->mid_value=__second_tree.__root()->value;
	  new_root->value=new_root->mid_value;
	  __root()->parent_link=new_root;
	  __second_tree.__root()->parent_link=new_root;
	  __header->parent_link=new_root;
	  //set the iterators
	  __header->right_link->right_link=__second_tree.__header->left_link;
	  __second_tree.__header->left_link->mid_link=__header->right_link;
	  __second_tree.__header->right_link->right_link=__header;
	  __header->right_link=__second_tree.__header->right_link;

	  //update the node count
	  new_root->__count_node=new_root->left_link->__count_node + new_root->mid_link->__count_node;
	}
      else 
	{
	  _Base_ptr __x=__root();
	  _Base_ptr __y=__second_tree.__root();
	  size_t this_height, that_height;
	  this_height=__x->get_height();
	  that_height=__y->get_height();

	  _Base_ptr left_x,right_x,left_y,right_y;
	  left_x=leftmost();
	  right_x=rightmost();
	  left_y=__second_tree.leftmost();
	  right_y=__second_tree.rightmost();

	  bool set_by_traversing=false;
	  
	  if (this_height < that_height)
	    {
	      while (that_height - 1  > this_height)	
		{
		  __y=__y->left_link;
		  --that_height;
		}
	      
	      if (_KeyOfValue()(*(__y->value))==_KeyOfValue()(*(__root()->value)))
		set_by_traversing=true;

	      __second_tree.addson(__y,__root()) ;

	      if (set_by_traversing)
		{
		  __second_tree.set_iterators_by_traversing();
		  __header->parent_link=__second_tree.__header->parent_link;
		  __header->parent_link->parent_link=__header;
		  __header->left_link=__second_tree.__header->left_link;
		  __header->right_link=__second_tree.__header->right_link;
		  __header->left_link->mid_link=__header;
		  __header->right_link->right_link=__header;
		}
	      else
		{
		  right_x->right_link=left_y;
		  left_y->mid_link=right_x;
		  
		  
		  __header->parent_link=__second_tree.__header->parent_link;
		  __header->parent_link->parent_link=__header;
		  __header->left_link=left_x;
		  __header->left_link->mid_link=__header;
		  __header->right_link=right_y;
		  __header->right_link->right_link=__header;
		}
	    }
	  else
	    {
	      while (this_height - 1 > that_height)
		{
		  if (__x->right_link!=0) __x=__x->right_link;
		  else __x=__x->mid_link;
		  --this_height;
		}

	      if (_KeyOfValue()(*(__x->value))==_KeyOfValue()(*(__second_tree.__root()->value)))
		set_by_traversing=true;


	      addson(__x,__second_tree.__root());

	      if (set_by_traversing)
		  set_iterators_by_traversing();
	      else
		{
		  right_x->right_link=left_y;
		  left_y->mid_link=right_x;
		  __header->left_link=left_x;
		  __header->right_link=right_y;
		  __header->left_link->mid_link=__header;
		  __header->right_link->right_link=__header;
		}



	    }
	}
   
      //empty the second tree
      
      	  __second_tree.__header->parent_link=__second_tree.__header;
	  __second_tree.__header->left_link=__second_tree.__header;
	  __second_tree.__header->right_link=__second_tree.__header;

      return;

    }



 public:


  void __copy (this_type & __second_tree)
    {
      if (__second_tree.empty()) return;
      vector<_Base_ptr> current_nodes;
      int i=0;
      for(iterator it=__second_tree.begin();it!=__second_tree.end();++it)
	{
	  _Base_ptr __x=__allocate_node();
	  __x->value=__allocate_value();
	  *(__x->value)=*it;
	  current_nodes.push_back(__x);
	  if (i!=0)
	    {
	      current_nodes[i-1]->right_link=__x;
	      __x->right_link=current_nodes[i-1];
	    }
	  i++;
	}

      vector<_Base_ptr> nodes;

      while(current_nodes.size()!=1)
	{
	  i=0;

	  while((current_nodes.size()-i)>4)
	    {

	      _Base_ptr __x=__allocate_node();
	      //set links
	      __x->left_link=current_nodes[i];
	      __x->mid_link=current_nodes[i+1];
	      __x->right_link=current_nodes[i+2];
	      //set parent links
	      current_nodes[i]->parent_link=current_nodes[i+1]->parent_link=current_nodes[i+2]->parent_link=__x;
	      //set values;
	      __x->left_value=__x->left_link->value;
	      __x->mid_value=__x->mid_link->value;
	      __x->right_value=__x->right_link->value;
	      __x->value=__x->right_value;
	      // set node count
	      __x->__count_node= __x->left_link->__count_node + __x->mid_link->__count_node + __x->right_link->__count_node;
	      nodes.push_back(__x);
	      i=i+3;
	    }

	  switch(current_nodes.size()-i) {
	  case 2:
	    {

	      _Base_ptr __x=__allocate_node();
	      //set links
	      __x->left_link=current_nodes[i];
	      __x->mid_link=current_nodes[i+1];
	      //set parent links
	      current_nodes[i]->parent_link=current_nodes[i+1]->parent_link=__x;
	      //set values;
	      __x->left_value=__x->left_link->value;
	      __x->mid_value=__x->mid_link->value;
	      __x->value=__x->mid_value;
	      //set node count
	      __x->__count_node= __x->left_link->__count_node + __x->mid_link->__count_node ;
	      nodes.push_back(__x);
	      break;
	    }
	  case 3:
	    {

	      _Base_ptr __x=__allocate_node();
	      //set links
	      __x->left_link=current_nodes[i];
	      __x->mid_link=current_nodes[i+1];
	      __x->right_link=current_nodes[i+2];
	      //set parent links
	      current_nodes[i]->parent_link=current_nodes[i+1]->parent_link=current_nodes[i+2]->parent_link=__x;
	      //set values;
	      __x->left_value=__x->left_link->value;
	      __x->mid_value=__x->mid_link->value;
	      __x->right_value=__x->right_link->value;
	      __x->value=__x->right_value;
	      // set node count
	      __x->__count_node= __x->left_link->__count_node + __x->mid_link->__count_node + __x->right_link->__count_node;
	      nodes.push_back(__x);
	      break;
	    }
	  case 4:
	    {

	      _Base_ptr __x=__allocate_node();
	      //set links
	      __x->left_link=current_nodes[i];
	      __x->mid_link=current_nodes[i+1];
	      //set parent links
	      current_nodes[i]->parent_link=current_nodes[i+1]->parent_link=__x;
	      //set values;
	      __x->left_value=__x->left_link->value;
	      __x->mid_value=__x->mid_link->value;
	      __x->value=__x->mid_value;
	      //set node count
	      __x->__count_node= __x->left_link->__count_node + __x->mid_link->__count_node ;
	      nodes.push_back(__x);
	      i=i+2;
	      _Base_ptr __y=__allocate_node();
	      //set links
	      __y->left_link=current_nodes[i];
	      __y->mid_link=current_nodes[i+1];
	      //set parent links
	      current_nodes[i]->parent_link=current_nodes[i+1]->parent_link=__y;
	      //set values;
	      __y->left_value=__y->left_link->value;
	      __y->mid_value=__y->mid_link->value;
	      __y->value=__y->mid_value;
	      //set node count
	      __y->__count_node= __y->left_link->__count_node + __y->mid_link->__count_node ;
	      nodes.push_back(__y);
	      break;
	    }
	  default:
	    {
	      printf("\n Impossible case in copy function");
	      exit(0);
	    }
	  }

	  current_nodes=nodes;
	  nodes.clear();
	}
      
      __header->parent_link=current_nodes[0];
      current_nodes[0]->parent_link=__header;
      _Base_ptr __aux=current_nodes[0];
      while (!__aux->isLeaf())
	__aux=__aux->left_link;
      __header->left_link=__aux;//set leftmost
      __aux=current_nodes[0];
      while(!__aux->isLeaf())
	{
	  if (__aux->right_link!=0)
	    __aux=__aux->right_link;
	  else 
	    __aux=__aux->mid_link;
	}
      __header->right_link=__aux;//set rightmost
      __header->left_link->mid_link=__header;
      __header->right_link->right_link=__header;
      return;
    }


 protected:

  void initialize_tree()
    {
      __node_alloc=0;
      __value_alloc=0;
      __header=__allocate_node();

      __header->parent_link=__header;
      __header->left_link=__header;
      __header->right_link=__header;
    }

  void __erase(_Base_ptr __x) // erase without balancing
    {
      while (__x!=0)
	{
	  
	  if (__x->isLeaf())
	    {
	      __deallocate_value(__x->value);
	      __deallocate_node(__x);
	      break;
	    }
	  if (__x-> right_link!=0)
	    __erase(__x->right_link);
	  if (__x->mid_link!=0)
	    __erase(__x->mid_link);
	  _Base_ptr __left=__x->left_link;
	  __deallocate_node(__x);
	  __x=__left;
	}
    }


  // sets up the iterators when a node is deleted.

  void delete_iterator(_Base_ptr __node)
    {
      if (__node->mid_link!=__header)
	__node->mid_link->right_link=__node->right_link;
      if (__node->right_link!=__header)
	__node->right_link->mid_link=__node->mid_link;
      return;
    }



  // Finds a node in the tree from where the insert procedure should start working.
  _Base_ptr find_node_to_insert(const  key_type& val, bool &node_already_exists)
    {
      if (__header==__header->parent_link) return __header;
      if (size()==1)
	{
	  if (_KeyOfValue()(*(__root()->value))==_KeyOfValue()(val))
	    node_already_exists=true;
	  return __header;
	}

      _Base_ptr __x=__root();
      // find the path to the node which is one level above leaf
      
      while (!__x->first_level_above_leaf())
	{
	  // find the right path
	  
	  /*
	  if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->left_value)))
||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->left_value)))) // if less than left value
	  */
	  if ((_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->left_value)))) || (_KeyOfValue()(val) == _KeyOfValue()(*(__x->left_value)) ) )  
	    {
	      __x=__x->left_link;
	      continue;
	    }
	  
	  if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->mid_value)))
||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->mid_value)))) // exists a mid value and less than mid value
	    {
	      __x=__x->mid_link;
	      continue;
	    }
	  if (__x->right_link==0) 
	      __x=__x->mid_link;
	  else
	      __x=__x->right_link;
	}
      
      // if node is in one of its children, then it already exists (this is for insert_unique on insert_equal)
      
      if (((__x->left_link!=0)&&(_KeyOfValue()(*(__x->left_value))==_KeyOfValue()(val)))
	  ||((__x->mid_link!=0)&&(_KeyOfValue()(*(__x->mid_value))==_KeyOfValue()(val)))
	  ||((__x->right_link!=0)&&(_KeyOfValue()(*(__x->right_value)==_KeyOfValue()(val)))))
	node_already_exists=true;
      
      return __x;
    }

void addson(_Base_ptr  where,_Base_ptr  __node)
{
  _Base_ptr __x= where;
  if (__header->parent_link==__header)
    {
      __header->parent_link=__node;
      __header->left_link=__node;
      __header->right_link=__node;
      __node->parent_link=__x;
      __node->mid_link=__header;
      __node->right_link=__header;
      return ;
    }	
  while ((__x!=__header)||(__x->mid_link==0))
    {      
      if (__x->mid_link==0)
	{
	  _Base_ptr __new_root=__allocate_node();
	  //create a new node to hold the 2 chidlren
	  //this is the new root
	  __new_root->parent_link=__header;
	  // arrange the chidlren
	  if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(__x->parent_link->value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->parent_link->value))))
	    {
	      // exception (44,45), (45,45)
	      if ((_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->parent_link->value)))&&(!__node->isLeaf())&&(_key_compare(_KeyOfValue()(*(__x->parent_link->left_value)),_KeyOfValue()(*(__node->left_value)))))
		{
		  __new_root->mid_link=__node;
		  __new_root->left_link=__x->parent_link;
		  if(__node->isLeaf())
		    {
		      __node->mid_link=__new_root->left_link;//node backwards
		      __node->right_link=__header;//node forward
		      __header->right_link=__node;// set rightmost
		__node->mid_link->right_link=__node;//successor
		    }
		}
	      else
		{
		  __new_root->mid_link=__x->parent_link;
		  __new_root->left_link=__node;
		  if(__node->isLeaf())
		    {
		      __node->mid_link=__header;//node backwards
		      __header->left_link=__node;//set leftmost
		      __node->right_link=__new_root->mid_link;//node forward
		      __node->right_link->mid_link=__node;//successor
		    }
		}
	      
	    }
	  else {
	    
	    __new_root->mid_link=__node;
	    __new_root->left_link=__x->parent_link;
	    if(__node->isLeaf())
	      {
		__node->mid_link=__new_root->left_link;//node backwards
		__node->right_link=__header;//node forward
		__header->right_link=__node;// set rightmost
		__node->mid_link->right_link=__node;//successor
	      }
	  }
	  //set their parents
	  __new_root->left_link->parent_link=__new_root;
	  __new_root->mid_link->parent_link=__new_root;
	  //set the values in the newly created root
	  __new_root->left_value=__new_root->left_link->value;
	  __new_root->value = __new_root->mid_value = __new_root->mid_link->value;
	  //set it as root
	  __header->parent_link=__new_root;
	  //update the node count
	  __new_root->__count_node=__new_root->left_link->__count_node+__new_root->mid_link->__count_node;
	  return;
	}


      if (__x->right_link==0)
	{
	  if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(__x->left_value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->left_value))))
	    {
	      if ((_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->left_value)))&&(!__node->isLeaf())&&(_key_compare(_KeyOfValue()(*(__x->left_link->left_value)),_KeyOfValue()(*(__node->left_value)))))
		  // exception (eg. x=(44,45) node=(45,45)
		{
		  printf("\n EXCEPTION !!!!!!!!!!!");
		  __x->right_value=__x->mid_value;
		  __x->mid_value=__node->value;
		  __x->right_link=__x->mid_link;
		  __x->mid_link=__node;
		  __node->parent_link=__x;
		  __x->value=__x->right_value;
		  __x->__count_node=__x->left_link->__count_node +__x->mid_link->__count_node+ __x->right_link->__count_node;
		  
		  update_node(__x->parent_link,__x);
		  // set the iterators if node is leaf
		  if (__node->isLeaf())
		    {
		      __node->mid_link=__x->left_link;//node backwards
		      __node->right_link=__x->right_link;//node forward
		      __node->mid_link->right_link=__node;//predecessor
		      __node->right_link->mid_link=__node;//successor
		      
		    }
		  return;
		}
	      __x->right_value=__x->mid_value;
	      __x->mid_value=__x->left_value;
	      __x->left_value= __node->value;
	      __x->right_link=__x->mid_link;
	      __x->mid_link=__x->left_link;
	      __x->left_link= __node;
	      __node->parent_link= __x;
	      __x->value=__x->right_value;
	      __x->__count_node=__x->left_link->__count_node +__x->mid_link->__count_node+ __x->right_link->__count_node;
	      update_node(__x->parent_link,__x);
	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__x->mid_link->mid_link;//node backwards
		  __node->right_link=__x->mid_link;//node forward
		  if (__node->mid_link!=__header)
		    __node->mid_link->right_link=__node;//predecessor
		  else __header->left_link=__node;//set leftmost 
		  __node->right_link->mid_link=__node;//successor
		      
		}
	      return;
	      
	    }
	  if (_key_compare(_KeyOfValue()(*(__node->value)), _KeyOfValue()(*(__x->mid_value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->mid_value))))
	    {
		  // exception (eg. x=(44,45) node=(45,45)

	      if ((_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->mid_value)))&&(!__node->isLeaf())&&(_key_compare(_KeyOfValue()(*(__x->mid_link->left_value)),_KeyOfValue()(*(__node->left_value)))))
		{
		  __x->right_value=__node->value;
		  __x->right_link=__node;
		  __node->parent_link=__x;
		  __x->value=__x->right_value;
		  __x->__count_node=__x->left_link->__count_node +__x->mid_link->__count_node+ __x->right_link->__count_node;
		  
		  update_node(__x->parent_link,__x);
		  // set the iterators if node is leaf
		  if (__node->isLeaf())
		    {
		      __node->mid_link=__x->mid_link;//node backwards
		      __node->right_link=__x->mid_link->right_link;//node forward
		      __node->mid_link->right_link=__node;//predecessor
		      if (__node->right_link!=__header)
			__node->right_link->mid_link=__node;//successor
		      else __header->right_link=__node;//set rightmost
		      
		    }
		  
		  
		  return;
		}
	      __x->right_value=__x->mid_value;
	      __x->mid_value=__node->value;
	      __x->right_link=__x->mid_link;
	      __x->mid_link=__node;
	      __node->parent_link=__x;
	      __x->value=__x->right_value;
	      __x->__count_node=__x->left_link->__count_node +__x->mid_link->__count_node+ __x->right_link->__count_node;

	      update_node(__x->parent_link,__x);
	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__x->left_link;//node backwards
		  __node->right_link=__x->right_link;//node forward
		  __node->mid_link->right_link=__node;//predecessor
		  __node->right_link->mid_link=__node;//successor
		      
		}

	      return;
	    }
	  else {
	    __x->right_value=__node->value;
	    __x->right_link=__node;
	    __node->parent_link=__x;
	    __x->value=__x->right_value;
	      __x->__count_node=__x->left_link->__count_node +__x->mid_link->__count_node+ __x->right_link->__count_node;

	    update_node(__x->parent_link,__x);
	    // set the iterators if node is leaf
	    if (__node->isLeaf())
	      {
		__node->mid_link=__x->mid_link;//node backwards
		__node->right_link=__x->mid_link->right_link;//node forward
		__node->mid_link->right_link=__node;//predecessor
		if (__node->right_link!=__header)
		  __node->right_link->mid_link=__node;//successor
		else __header->right_link=__node;//set rightmost
		
	      }


	    return;
	    
	  }
	  
	}
      else
	{
	  _Base_ptr  __sibling=__allocate_node();

	  if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(__x->left_value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->left_value))))
	    {
	      __sibling->left_value = __node->value;
	      __sibling->mid_value  = __x->left_value;
	      __sibling->left_link  = __node;
	      __sibling->mid_link= __x->left_link;
	      __x->left_value=__x->mid_value;
	      __x->mid_value=__x->right_value;

	      __x->left_link=__x->mid_link;
	      __x->mid_link=__x->right_link;
	      
	      __sibling->left_link->parent_link=__sibling;
	      __sibling->mid_link->parent_link=__sibling;
	      __sibling->right_link=0;  
	      __x->right_link=0;
	      __x->right_value=0;
	      __sibling->value=__sibling->mid_value;
	      __x->value=__x->mid_value;

	      __x->__count_node=__x->left_link->__count_node+ __x->mid_link->__count_node;
	      __sibling->__count_node=__sibling->left_link->__count_node+ __sibling->mid_link->__count_node;

	      update_node(__x->parent_link,__x);

	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__sibling->mid_link->mid_link;//node backwards
		  __node->right_link=__sibling->mid_link;//node forward
		  if (__node->mid_link!=__header)
		    __node->mid_link->right_link=__node;//predecessor
		  else __header->left_link=__node;//set leftmost 
		  __node->right_link->mid_link=__node;//successor
		      
		}

	      __x=__x->parent_link;
	      __node=__sibling;


	      continue;
	    }	
	  if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(__x->mid_value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->mid_value))))
	    {

	      __sibling->left_value=__x->left_value;
	      __sibling->mid_value=__node->value;
	      __sibling->left_link=__x->left_link;
	      __sibling->mid_link=__node;

	      __x->left_value=__x->mid_value;
	      __x->mid_value=__x->right_value;

	      __x->left_link=__x->mid_link;
	      __x->mid_link=__x->right_link;
	      
	      __sibling->left_link->parent_link=__sibling;
	      __sibling->mid_link->parent_link=__sibling;
	      __sibling->right_link=0;  
	      __x->right_link=0;
	      
	      __x->right_value=0;

	      __sibling->value=__sibling->mid_value;
	      __x->value=__x->mid_value;
	      update_node(__x->parent_link,__x);


	      __x->__count_node=__x->left_link->__count_node+ __x->mid_link->__count_node;
	      __sibling->__count_node=__sibling->left_link->__count_node+ __sibling->mid_link->__count_node;

	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__sibling->left_link;//node backwards
		  __node->right_link=__x->left_link;//node forward
		  __node->mid_link->right_link=__node;//predecessor
		  __node->right_link->mid_link=__node;//successor
		      
		}
	      __x=__x->parent_link;
	      __node=__sibling;
	      continue;
		}
	  if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(__x->right_value)))||(_KeyOfValue()(*(__node->value))==_KeyOfValue()(*(__x->right_value))))
	  {

	    __sibling->left_value=__x->left_value;
	    __sibling->mid_value=__x->mid_value;
	    __sibling->left_link=__x->left_link;
	    __sibling->mid_link=__x->mid_link;
	    __sibling->left_link->parent_link=__sibling;
	    __sibling->mid_link->parent_link=__sibling;
	    __sibling->right_link=0;  
	    __x->left_link=__node;
	    __x->mid_link=__x->right_link;
	    __x->left_value=__node->value;
	    __x->mid_value=__x->right_value;

	    __x->right_link=0;
	    __x->right_value=0;
	    __x->value=__x->mid_value;
	    __sibling->value=__sibling->mid_value;
	    __node->parent_link=__x;


	      __x->__count_node=__x->left_link->__count_node+ __x->mid_link->__count_node;
	      __sibling->__count_node=__sibling->left_link->__count_node+ __sibling->mid_link->__count_node;

	    update_node(__x->parent_link,__x);
	    
	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__sibling->mid_link;//node backwards
		  __node->right_link=__x->mid_link;//node forward
		  __node->mid_link->right_link=__node;//predecessor
		  __node->right_link->mid_link=__node;//successor
		      
		}



	    __x=__x->parent_link;
	    __node=__sibling;
	    continue;
	  }
	else
	  {
	    __sibling->left_value=__x->left_value;
	    __sibling->mid_value=__x->mid_value;	
	    __sibling->left_link=__x->left_link;
	    __sibling->mid_link=__x->mid_link;
	    __sibling->left_link->parent_link=__sibling; 
	    __sibling->mid_link->parent_link=__sibling;
	    __sibling->right_link=0;  

	    __x->left_link=__x->right_link;
	    __x->mid_link=__node;
	    __x->left_value=__x->right_value;
	    __x->mid_value=__node->value;
	    
	    __node->parent_link=__x;

	    __x->right_link=0;
	    __x->right_value=0;
	    __x->value=__x->mid_value;
	    __sibling->value=__sibling->mid_value;


	      __x->__count_node=__x->left_link->__count_node+ __x->mid_link->__count_node;
	      __sibling->__count_node=__sibling->left_link->__count_node+ __sibling->mid_link->__count_node;


	    update_node(__x->parent_link,__x);

	    
	      // set the iterators if node is leaf
	      if (__node->isLeaf())
		{
		  __node->mid_link=__x->left_link;//node backwards
		  __node->right_link=__x->left_link->right_link;//node forward
		  __node->mid_link->right_link=__node;//predecessor
		  if (__node->right_link!=__header)
		    __node->right_link->mid_link=__node;//successor
		  else __header->right_link=__node;//set rightmost
		      
		}

	    __x=__x->parent_link;
	    __node=__sibling;
	    continue;

	  }



	} 
      
    }
}
 
 void update_node(_Base_ptr old_node,_Base_ptr new_node)
   {
     _Base_ptr __old=old_node;
     _Base_ptr __new=new_node;

     while (__old!=__header)
       {
	 
	 //update the node count;
      	 __old->__count_node=__old->left_link->__count_node;
	 if (__old->mid_link!=0) 
	   __old->__count_node+= __old->mid_link->__count_node;
	 if (__old->right_link!=0)
	   __old->__count_node+=__old->right_link->__count_node;
	 
	 //done update node count;
	 if (__old->left_link==__new) 
	     {
	       __old->left_value=__new->value;
	       if (__old->mid_link==0)
		 __old->value=__old->left_value;
	     }
	 else 
	   if (__old->mid_link==__new) 
	     {
	       __old->mid_value=__new->value;
	       if (__old->right_link==0)
		 __old->value=__old->mid_value;

	     }
	   else 
	     if (__old->right_link==__new) 
	       {
		 __old->right_value=__new->value;
		 __old->value=__old->right_value;
	       }

	     else printf("\n Impossible case in update_node");
	 __new=__old;
	 __old=__old->parent_link;
       }
   }

 void delete_node(_Base_ptr __initial_node,const value_type & val, bool & done, bool &son1, bool &duplicate)
   {
     _Base_ptr __x=__initial_node;
     
     _Base_ptr child;
     if (__x->first_level_above_leaf())
     {       
       if (__x->right_link==0) //has only two nodes
	 { 
	   if (_KeyOfValue()(*(__x->left_link->value)) == _KeyOfValue()(val)) // ...and it's the left link
	     {
	       // check if there are nodes with the same value (duplicate)
	       if (__x->left_link->mid_link!=__header) // if it's not the first mode in the tree
		 if (_KeyOfValue()(*(__x->left_link->mid_link->value)) == _KeyOfValue()(val)) duplicate=true;
	       if (__x->left_link->right_link!=__header) // if it's not the last node in the tree
		 if (_KeyOfValue()(*(__x->left_link->right_link->value)) == _KeyOfValue()(val)) duplicate=true;
		     // done checking for duplicates
	       delete_iterator(__x->left_link);
	       if (leftmost()==__x->left_link)
		 __header->left_link=__x->mid_link;
	       __deallocate_node(__x->left_link);
	       __x->left_link=__x->mid_link;
	       __x->mid_link=0;
	       //update values
	       __x->left_value=__x->left_link->value;
	       __x->mid_value=0;
	       __x->value=__x->left_value;
	       //update node count
	       __x->__count_node=__x->left_link->__count_node;

	       //		 update_node(__x,__x->left_link);
	       son1=true;
	       done=false;
	     }			
	   else 
	     if (_KeyOfValue()(*(__x->mid_link->value)) ==_KeyOfValue()( val))
	       {
	       // check if there are nodes with the same value (duplicate)
	       if (__x->mid_link->mid_link!=__header) // if it's not the first mode in the tree
		 if (_KeyOfValue()(*(__x->mid_link->mid_link->value)) == _KeyOfValue()(val)) duplicate=true;
	       if (__x->mid_link->right_link!=__header) // if it's not the last node in the tree
		 if (_KeyOfValue()(*(__x->mid_link->right_link->value)) == _KeyOfValue()(val)) duplicate=true;
		     // done checking for duplicates
		 delete_iterator(__x->mid_link);
		 if (rightmost()==__x->mid_link)
		   __header->right_link=__x->left_link;
		 __deallocate_node(__x->mid_link);

		 __x->mid_link=0;
		 __x->mid_value=0;
		 __x->value=__x->left_value;
	       //update node count
	       __x->__count_node=__x->left_link->__count_node;

		 son1=true;
		 done=false;
	       }
	     else 
	       {
		 done=true;
		 son1=false;
	       }
	 } // end right_link==0
       else // it has three children
	 if (_KeyOfValue()(*(__x->left_link->value)) == _KeyOfValue()(val))//...and it's the left child
	   {
	     // check if there are nodes with the same value (duplicate)
	     if (__x->left_link->mid_link!=__header) // if it's not the first mode in the tree
	       if (_KeyOfValue()(*(__x->left_link->mid_link->value)) ==_KeyOfValue()( val)) duplicate=true;
	     if (__x->left_link->right_link!=__header) // if it's not the last node in the tree
	       if (_KeyOfValue()(*(__x->left_link->right_link->value)) == _KeyOfValue()(val)) duplicate=true;
		   // done checking for duplicates
	     delete_iterator(__x->left_link);
	     if (leftmost()==__x->left_link)
	       __header->left_link=__x->mid_link;
	     __deallocate_node(__x->left_link);

	     __x->left_link=__x->mid_link;
	     __x->mid_link=__x->right_link;
	     __x->right_link=0;
	     __x->left_value=__x->mid_value;
	     __x->mid_value=__x->right_value;
	     __x->right_value=0;
	     son1=false;
	     done=true;
	   }	
	 else 
	   if (_KeyOfValue()(*(__x->mid_link->value)) == _KeyOfValue()(val))//...and it's the mid child
	     {
	       if (__x->mid_link->mid_link!=__header) // if it's not the first mode in the tree
		 if (_KeyOfValue()(*(__x->mid_link->mid_link->value)) == _KeyOfValue()(val)) duplicate=true;
	       if (__x->mid_link->right_link!=__header) // if it's not the last node in the tree
		 if (_KeyOfValue()(*(__x->mid_link->right_link->value)) == _KeyOfValue()(val)) duplicate=true;
		     // done checking for duplicates
	       delete_iterator(__x->mid_link);
	       __deallocate_node(__x->mid_link);

	       __x->mid_link=__x->right_link;
	       __x->mid_value=__x->right_value;
	       __x->right_link=0;
	       __x->right_value=0;
	       son1=false;
	       done=true;
	     }		
	   else 
	     if (_KeyOfValue()(*(__x->right_link->value)) == _KeyOfValue()(val))//...and it's the right child
	       {
	       if (__x->right_link->mid_link!=__header) // if it's not the first mode in the tree
		 if (_KeyOfValue()(*(__x->right_link->mid_link->value)) == _KeyOfValue()(val)) duplicate=true;
	       if (__x->right_link->right_link!=__header) // if it's not the last node in the tree
		 if (_KeyOfValue()(*(__x->right_link->right_link->value)) == _KeyOfValue()(val)) duplicate=true;
		     // done checking for duplicates
		 delete_iterator(__x->right_link);
		 if (rightmost()==__x->right_link)
		   __header->right_link=__x->mid_link;
		 __deallocate_node(__x->right_link);

		 __x->right_link=0;
		 __x->right_value=0;
		 __x->value=__x->mid_value;
		 //		 update_node(__x,__x->mid_link);
		 son1=false;
		 done=true;
	       }
	     else 
	       { 

		 done=true; 
		 son1=false;
	       }
       __x->__count_node=__x->left_link->__count_node;
       if (__x->mid_link!=0) 
	 __x->__count_node+= __x->mid_link->__count_node;
       update_node(__x,__x->left_link);

       return;
     }
     else {
       if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->left_value)))||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->left_value))))
	 child=__x->left_link;
       else 
	 if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->mid_value)))||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->mid_value))))
	   child=__x->mid_link;
	 else 
	   if (__x->right_link!=0)
	     if (_key_compare(_KeyOfValue()(val),_KeyOfValue()(*(__x->right_value)))||(_KeyOfValue()(val)==_KeyOfValue()(*(__x->right_value))))
	       child=__x->right_link;
	     else 
	       {
		 child=0;
	       }
	   else 
	     {
	       child=0;
	     }
     }

     if (child==0) return;
     bool son11=false;
     delete_node(child,val,done,son11,duplicate);
     
     if (son11)
       {
	 
	 _Base_ptr __aux=0;
	 _Base_ptr only_one=child;
	 if (__x->left_link->right_link!=0)  
	     __aux=__x->left_link;
	 else 
	   if (__x->mid_link->right_link!=0) 
	     __aux=__x->mid_link;
	   else 
	     if (__x->right_link!=0)
	       if (__x->right_link->right_link!=0)
		 __aux=__x->right_link;
	       else {;}//printf("\n Message 3 - delete_node-could not find an auxiliary node with 3 children")
	     else {;}//printf("\n Message 4 - delete_node - could not find an auxiliary node with 3 children")
	 
	 if (__aux!=0)
	   {
	     _Base_ptr array[7];
	     for(int y=0;y<7;y++) array[y]=0;
	     int ind=0;
	     if (__x->left_link->left_link!=0) array[ind++]=__x->left_link->left_link;
	     if (__x->left_link->mid_link!=0) array[ind++]=__x->left_link->mid_link;
	     if (__x->left_link->right_link!=0) array[ind++]=__x->left_link->right_link;
	     if (__x->mid_link->left_link!=0) array[ind++]=__x->mid_link->left_link;
	     if (__x->mid_link->mid_link!=0) array[ind++]=__x->mid_link->mid_link;
	     if (__x->mid_link->right_link!=0) array[ind++]=__x->mid_link->right_link;
	     
	     if (__x->right_link!=0)
	       {
		 if (__x->right_link->left_link!=0) array[ind++]=__x->right_link->left_link;
		 if (__x->right_link->mid_link!=0) array[ind++]=__x->right_link->mid_link;
		 if (__x->right_link->right_link!=0) array[ind++]=__x->right_link->right_link;
		 
	       }
	     //	     printf("\n Index is %d and count = %d",ind,__count);
	     switch(ind) {
	     case 3: break;
	     case 4:
	       {
		 //		 printf("\n CASE 4");
		 // arrange (2 2)

		 // LEFT LINK
		 //update children
		 __x->left_link->left_link=array[0];
		 __x->left_link->mid_link=array[1];
		 __x->left_link->right_link=0;		 
		 //update values
		 __x->left_link->left_value=__x->left_link->left_link->value; 
		 __x->left_link->mid_value=__x->left_link->mid_link->value;
		 __x->left_link->value=__x->left_link->mid_value;
		 __x->left_link->right_value=0;
		 // update parents
		 __x->left_link->left_link->parent_link=__x->left_link;
		 __x->left_link->mid_link->parent_link=__x->left_link;
		 //MID LINK
		 //update children
		 __x->mid_link->left_link=array[2];
		 __x->mid_link->mid_link=array[3];
		 __x->mid_link->right_link=0;
		 
		 //update values
		 __x->mid_link->left_value=__x->mid_link->left_link->value;
		 __x->mid_link->mid_value=__x->mid_link->mid_link->value;
		 __x->mid_link->right_value=0;
		 __x->mid_link->value=__x->mid_link->mid_value;
		 //update parents
		 __x->mid_link->left_link->parent_link=__x->mid_link;
		 __x->mid_link->mid_link->parent_link=__x->mid_link;

		 //NODE
		 __x->left_value=__x->left_link->value;
		 __x->mid_value=__x->mid_link->value;
		 __x->value=__x->mid_link->value;
		 break;
	       }
	     case 5:
	       {
		 //		 printf("\n CASE 5");
		 //arrange (3 2)
		 //LEFT LINK
		 //update children
		 __x->left_link->left_link=array[0];
		 __x->left_link->mid_link=array[1];
		 __x->left_link->right_link=array[2];
		 //update values
		 __x->left_link->left_value=__x->left_link->left_link->value;
		 __x->left_link->mid_value=__x->left_link->mid_link->value;
		 __x->left_link->right_value=__x->left_link->right_link->value;
		 __x->left_link->value=__x->left_link->right_value;
		 //update parents
		 __x->left_link->left_link->parent_link=__x->left_link;
		 __x->left_link->mid_link->parent_link=__x->left_link;
		 __x->left_link->right_link->parent_link=__x->left_link;

		 //MID LINK
		 //update children
		 __x->mid_link->left_link=array[3];
		 __x->mid_link->mid_link=array[4];
		 __x->mid_link->right_link=0;
		 //update values
		 
		 __x->mid_link->left_value=__x->mid_link->left_link->value;
		 __x->mid_link->mid_value=__x->mid_link->mid_link->value;
		 __x->mid_link->right_value=0;
		 __x->mid_link->value=__x->mid_link->mid_value;
		 //update parents
		 __x->mid_link->left_link->parent_link=__x->mid_link;
		 __x->mid_link->mid_link->parent_link=__x->mid_link;
		 //NODE
		 
		 __x->right_link=0;
		 __x->left_value=__x->left_link->value;
		 __x->mid_value=__x->mid_link->value;
		 __x->value=__x->mid_link->value;
		 break;
	       }
	     case 6:
	       {
		 //		 printf("\n CASE 6");
	       // arrange the children into 2 node of 3 children (3,3)
	       __x->left_link->left_link=array[0];
	       __x->left_link->mid_link=array[1];  
	       __x->left_link->right_link=array[2];
	       __x->mid_link->left_link=array[3];
	       __x->mid_link->mid_link=array[4];
	       __x->mid_link->right_link=array[5];
	       //LEFT LINK
	       // update values of left link
	       __x->left_link->left_value=__x->left_link->left_link->value;
	       __x->left_link->mid_value=__x->left_link->mid_link->value;
	       __x->left_link->right_value=__x->left_link->right_link->value;
	       __x->left_link->value=__x->left_link->right_value;

	       // update the parents for the newly created children of left link
	       __x->left_link->left_link->parent_link=__x->left_link;
	       __x->left_link->mid_link->parent_link=__x->left_link; 
	       __x->left_link->right_link->parent_link=__x->left_link;

	       //MID LINK
	       __x->mid_link->left_value=__x->mid_link->left_link->value;
	       __x->mid_link->mid_value=__x->mid_link->mid_link->value;
	       __x->mid_link->right_value=__x->mid_link->right_link->value;
	       __x->mid_link->value=__x->mid_link->right_value; 
	       // update the parents for the newly created children of mid link
	       __x->mid_link->left_link->parent_link=__x->mid_link;
	       __x->mid_link->mid_link->parent_link=__x->mid_link;
	       __x->mid_link->right_link->parent_link=__x->mid_link;    

	       //CURRENT NODE
	       // set the values for the __x
	       __x->left_value=__x->left_link->value;
	       __x->mid_value=__x->mid_link->value;
	       __x->value=__x->mid_link->value;
	       // delete the right child. It doesn't exist anymore
	       __deallocate_node(__x->right_link);
	       __x->right_value=0;
	       __x->right_link=0;
	       done=true;
	       break;
	       }
	     case 7:
	       {
		 //		 printf("\n CASE 7");
	       // arrange the nodes (3 2 2 )
               __x->left_link->left_link=array[0];
	       __x->left_link->mid_link=array[1];
	       __x->left_link->right_link=array[2];
	       __x->mid_link->left_link=array[3];
	       __x->mid_link->mid_link=array[4];
	       __x->mid_link->right_link=0;
	       __x->right_link->left_link=array[5];
	       __x->right_link->mid_link=array[6];		
	       __x->right_link->right_link=0;		

	       //LEFT LINK
	       //update values in left link
	       __x->left_link->left_value=__x->left_link->left_link->value;
	       __x->left_link->mid_value=__x->left_link->mid_link->value;
	       __x->left_link->right_value=__x->left_link->right_link->value;
	       __x->left_link->value=__x->left_link->right_link->value;
	       //update parents in left link 
	       __x->left_link->left_link->parent_link=__x->left_link;
	       __x->left_link->mid_link->parent_link=__x->left_link; 
	       __x->left_link->right_link->parent_link=__x->left_link;
	       //MID LINK
	       //update values in mid link
	       __x->mid_link->left_value=__x->mid_link->left_link->value;
	       __x->mid_link->mid_value=__x->mid_link->mid_link->value;
	       __x->mid_link->right_value=0;//__x->mid_link->right_link->value;
	       __x->mid_link->value=__x->mid_link->mid_value;
	       //updates parents in mid link
	       __x->mid_link->left_link->parent_link=__x->mid_link;   
	       __x->mid_link->mid_link->parent_link=__x->mid_link;   
	       //RIGHT LINK
	       //update values in right link
	       __x->right_link->right_value=0;
	       __x->right_link->mid_value=__x->right_link->mid_link->value;;
	       __x->right_link->left_value=__x->right_link->left_link->value;
	       __x->right_link->value=__x->right_link->mid_value;
	       //updates parents in right link
	       __x->right_link->left_link->parent_link=__x->right_link;
	       __x->right_link->mid_link->parent_link=__x->right_link;

	       //NODE
	       //update values in the current node;
	       __x->left_value=__x->left_link->value;
	       __x->mid_value=__x->mid_link->value;
	       __x->right_value=__x->right_link->value;
	       __x->value=__x->right_link->value;
	       done=true;
	       break;
	       }
	       
	     default:
	       {
		 printf("\n Impossible case in deleting a node - message 2 -- # of chidlren = %d",ind);
		 exit(0);
	       }
	       
	     }// end switch;
	     //update node count;
	     //LEFT LINK
	     __x->left_link->__count_node = __x->left_link->left_link->__count_node + __x->left_link->mid_link->__count_node ;
	     if (__x->left_link->right_link!=0)
	       __x->left_link->__count_node+=__x->left_link->right_link->__count_node;

	     //MID LINK
	     __x->mid_link->__count_node = __x->mid_link->left_link->__count_node + __x->mid_link->mid_link->__count_node ;
	     if (__x->mid_link->right_link!=0)
	       __x->mid_link->__count_node+=__x->mid_link->right_link->__count_node;
	     //RIGHT LINK
	     if (__x->right_link!=0)
	       {
		 __x->right_link->__count_node = __x->right_link->left_link->__count_node + __x->right_link->mid_link->__count_node ;
		 if (__x->right_link->right_link!=0)
		   __x->right_link->__count_node+=__x->right_link->right_link->__count_node;
	       }
	     // NODE
	     __x->__count_node=__x->left_link->__count_node+ __x->mid_link->__count_node;
	     if (__x->right_link!=0)
	       __x->__count_node+=__x->right_link->__count_node;
	     
	     son1=false;
	     update_node(__x->parent_link,__x);
	   }
     else  
       { 
	 if (child==__x->left_link)
	   {
	     // mid link has 2 children. Move then to the right, make room for the child's child
	     __x->mid_link->right_link=__x->mid_link->mid_link;
	     __x->mid_link->mid_link=__x->mid_link->left_link;
	     
	     __x->mid_link->right_value=__x->mid_link->mid_value;
	     __x->mid_link->mid_value=__x->mid_link->left_value;
	     // add the child's child as the left link to the mid
	     __x->mid_link->left_link=child->left_link;
	     // Set the parent and value
	     __x->mid_link->left_link->parent_link=__x->mid_link;
	     __x->mid_link->left_value=__x->mid_link->left_link->value;
	     __x->mid_link->value=__x->mid_link->right_value;
	     //update the node count;
	     __x->mid_link->__count_node=__x->mid_link->left_link->__count_node + __x->mid_link->mid_link->__count_node + __x->mid_link->right_link->__count_node ;
	     // deallocate the left link since it's empty
	     __deallocate_node(__x->left_link);
	     // set the left link to the mid link
	     __x->left_link=__x->mid_link;
	     __x->left_value=__x->mid_value;

	     if (__x->right_link==0) 
	       {
		 __x->mid_link=0;
		 __x->mid_value=0;
		 son1=true;
		 __x->value=__x->left_value;
	       }
	     else 
	       {
		 __x->mid_link=__x->right_link;
		 __x->right_link=0;	
		 __x->mid_value=__x->right_value;
		 __x->right_value=0;
		 __x->value=__x->mid_value;
		 son1=false;
	       }

	   }
	 else 
	   if(child==__x->mid_link)
	     {
	       __x->left_link->right_link=child->left_link;
	       __x->left_link->value  =  __x->left_link->right_value  =  __x->left_link->right_link->value;
	       __x->left_link->right_link->parent_link=__x->left_link;
	       __x->left_link->__count_node=__x->left_link->left_link->__count_node + __x->left_link->mid_link->__count_node + __x->left_link->right_link->__count_node ;

	       // deallocate mid link
	       __deallocate_node(__x->mid_link);
	       __x->mid_link=0;
	       
	       if (__x->right_link==0)
		 {
		   son1=true;
		   __x->mid_value=0;
		 }
	       else 
		 {
		   __x->mid_link=__x->right_link;
		   __x->right_link=0;
		   __x->mid_value=__x->right_value;
		   __x->right_value=0;
		 }
	       update_node(__x,__x->left_link);

	     }
	   else
	     if (child==__x->right_link)
	       {

		 // move child's only child to the __x->mid_link since this one has only 2 children
		 __x->mid_link->right_link=child->left_link;
		 __x->mid_link->value  =   __x->mid_link->right_value  =  __x->mid_link->right_link->value;
		 __x->mid_link->right_link->parent_link=__x->mid_link;

		 __x->mid_link->__count_node=__x->mid_link->left_link->__count_node + __x->mid_link->mid_link->__count_node + __x->mid_link->right_link->__count_node ;

		 __deallocate_node(__x->right_link);
		 __x->right_link=0;
		 __x->right_value=0;
		 update_node(__x,__x->mid_link);
		}
	     else
	       {
		 printf("\n COuld not find a node to add the only child left to it");
		 exit(0);

	       }
	 if (__x->mid_link==0) son1=true;
	 //update node count
	 __x->__count_node=__x->left_link->__count_node + __x->mid_link->__count_node;

       }
       }
     
     else {son1=false;}
     
     
     
}

_23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc >* node_to_tree(_Base_ptr __x)
   {
     _23tree<_Key,_Value,_KeyOfValue,_Compare, _Alloc > *__tree=__allocate_tree();
     __tree->__header->parent_link=__x;
     __x->parent_link=__tree->__header;
     _Base_ptr __aux=__x;
     while (!__aux->isLeaf())
       __aux=__aux->left_link;
     __tree->__header->left_link=__aux;
     __aux=__x;
     while(!__aux->isLeaf())
       {
	 if (__aux->right_link!=0)
	   __aux=__aux->right_link;
	 else 
	   __aux=__aux->mid_link;
       }
     __tree->__header->right_link=__aux;
     __tree->__header->left_link->mid_link=__tree->__header;
     __tree->__header->right_link->right_link=__tree->__header;

     return __tree;
   }

 protected:
 bool verify_23tree(_Base_ptr __node)
   {
     if(!__node->isLeaf())
       {
	 // check parents
	 if (__node->left_link->parent_link!=__node) 
	   {
	     printf("\n Parent Link of left link is not properly set");
	     print_values(__node);
	     return false;
	   }
	 if (__node->mid_link->parent_link!=__node) 
	   {
	     printf("\n Parent Link of mid link is not properly set");
	     return false;
	   }
	 // check values (left,mid, right and value)
	 if (__node->left_value!=__node->left_link->value) 
	   {
	     printf("\n Left Value is not properly set");
	     print_values(__node);
	     return false;
	   }
	 if (__node->mid_value!=__node->mid_link->value) 
	   {
	     printf("\n Mid Value is not properly set");
	     print_values(__node);
	     return false;
	   }
	 if (!(_key_compare(_KeyOfValue()(*(__node->left_value)),_KeyOfValue()(*(__node->mid_value)))||(_KeyOfValue()(*(__node->left_value))==_KeyOfValue()(*(__node->mid_value))))) 
	   {
	     printf("\n Left Value is greater than Mid Value");
	     print_values(__node);
	     return false;
	   }
	 if (__node->right_link!=0)
	   {
	     if (__node->right_link->parent_link!=__node) 
	       {
		 printf("\n Parent Link of Right link is not properly set");
		 return false;
	       }
	     if (__node->right_value!=__node->right_link->value) 
	       {
		 printf("\n Right Value is not properly set");
		 print_values(__node);
		 return false;
	       }
	     if (__node->value!=__node->right_link->value) 
	       {
		 printf("\n Node Value is not properly set (with right link)");
		 print_values(__node);
		 return false;
	       }
	     if (!(_key_compare(_KeyOfValue()(*(__node->mid_value)),_KeyOfValue()(*(__node->right_value)))||(_KeyOfValue()(*(__node->mid_value))==_KeyOfValue()(*(__node->right_value)))))

	       {
		 printf("\n Mid Value is greater than Right Value");		 	     
		 print_values(__node);
		 return false;
	       }
	     if (__node->__count_node != (__node->left_link->__count_node + __node->mid_link->__count_node + __node->right_link->__count_node))
	       {
		 printf("\n The Node Count is not properly set (with right link)");
		 return false;
	       }
	     return (verify_23tree(__node->left_link))&&(verify_23tree(__node->mid_link))&&(verify_23tree(__node->right_link)) ;
	   }
	 else
	   {
	     if (__node->value!=__node->mid_link->value) 
	       {
		 printf("\n Node Value is not properly set");
		 print_values(__node);
		 return false;
	       }
	     if (__node->__count_node != (__node->left_link->__count_node + __node->mid_link->__count_node))
	       {
		 printf("\n The Node Count is not properly set ");
		 return false;
	       }
	     return (verify_23tree(__node->left_link))&&(verify_23tree(__node->mid_link)) ;
	   }
       }
     else // is a leaf
       {
	 if (_key_compare(_KeyOfValue()(*(__node->value)),_KeyOfValue()(*(leftmost()->value)))) 
	   {
	     printf("\n The Leaf Value is less than the minimum in the tree");
	     return false;
	   }
	 if (_key_compare(_KeyOfValue()(*(rightmost()->value)),_KeyOfValue()(*(__node->value)))) 
	   {
	     printf("\n The Leaf Value is Greater than the maximum in the tree");
	     print_values(__node);
	     print_values(rightmost());
	     print();
	     return false;
	   }
	 //	 if (__node->mid_link!=__header) // for g++
	 if ((__node->mid_link!=0)&&(__node->mid_link!=__header))
	   if (!(_key_compare(_KeyOfValue()(*(__node->mid_link->value)),_KeyOfValue()(*(__node->value)))||(_KeyOfValue()(*(__node->mid_link->value))==_KeyOfValue()(*(__node->value)))))
	     {
	       printf("\n Left Iterator value is greater than the node value");
	       printf("\n Node value %d and left iterator value %d",*(__node->value),*(__node->mid_link->value));
	       return false;
	     }
	 //	 if (__node->right_link!=__header)//for g++
	 if ((__node->right_link!=0)&&(__node->right_link!=__header))
	   if (_key_compare(_KeyOfValue()(*(__node->right_link->value)),_KeyOfValue()(*(__node->value))))
	     {
	       printf("\n Right iterator value is smaller than the node value ");
	       printf("\n Node value %d and right iterator value %d",*(__node->value),*(__node->right_link->value));
	       return false;
	     }
	 return true;
       }
   }

 public:

 bool verify()
   {
     if (empty()) return true;
     vector<_Value> v=serialize();
     for(typename vector<_Value>::iterator it=v.begin();it!=v.end();it++)
       {
	 if (it!=v.begin())
	   {
	     typename vector<_Value>::iterator it_aux=it;
	     it_aux--;
	     if (_key_compare(_KeyOfValue()(*it),_KeyOfValue()(*it_aux)))
	       {printf("\n The tree is not ordered");
	       printf("\n %d is followed by %d",*it_aux,*it);
	       print_tree();
	       return false;
	       }
	   }
	 typename vector<_Value>::iterator it_aux1=it;
	 it_aux1++;
	 if (it_aux1!=v.end())
	   if (_key_compare(_KeyOfValue()(*it_aux1),_KeyOfValue()(*it)))
	     {
	       printf("\n The tree is not ordered");
	       printf("\n %d is followed by %d",*it_aux1,*it);
	       print_tree();
	       return false;
	     }
       }
     _Base_ptr __x=__root();
     while(__x->left_link!=0) __x=__x->left_link;
     if (__x!=leftmost()) 
       {
	 printf("\n Leftmost is not properly set");
	 printf("\n X\n");

	 print_values(__x);
	 printf("\n Leftmost\n");

	 print_values(leftmost());
	 return false;
       }
     __x=__root();
     while(__x->left_link!=0)
       if (__x->right_link!=0) __x=__x->right_link;
       else __x=__x->mid_link;
     if (__x!=rightmost())
       {
	 printf("\n Rightmost is not properly set");
	 printf("\n X\n");
	 print_values(__x);
	 printf("\n Rightmost\n");

	 print_values(rightmost());
	 return false;
       }
     return verify_23tree(__root());
   }
 
 void print_values(_Base_ptr __x)
   {
     printf("\n Node value %d   (%d)", *(__x->value),__x);
     printf("\n Left Value %d  (%d) and Mid Value %d   (%d)", *(__x->left_value),__x->left_link,*(__x->mid_value),__x->mid_link);
     if (__x->right_link!=0)
       printf(" and Right Value %d  (%d)",*(__x->right_value),__x->right_link);
   }
 public:
 void print()
   {
     printf("\n");
     if (empty()) {printf("\n Empty\n");return;}
     for(iterator it=begin();it!=end();it++)
       //              printf(" %d (%d)",*it,it.__node);
       printf(" %d",*it);
     printf("\n");

   }
 void print_tree_node(_Base_ptr __x)
   {
     if (__x->isLeaf())
       {
	 	 printf(" %d",*(__x->value));
		 //     printf(" %d (%d)",*(__x->value),__x);

	 return;
       }
     else
       {
	 if (__x->left_link!=0) print_tree_node(__x->left_link);
	 if (__x->mid_link!=0) print_tree_node(__x->mid_link);
	 if (__x->right_link!=0) print_tree_node(__x->right_link);

       }
   }
 void print_tree()
   {
     if (empty()) {printf("\n Empty \n");return;}
     printf("\n");
     print_tree_node(__root());
     printf("\n");
   }
 vector<_Value> serialize()
   {
     vector<_Value> v;
     serialize_node(__root(),v);
     return v;
   }
 void serialize_node(_Base_ptr __x,vector<_Value> &v)
   {
     if (__x->isLeaf())
       {
	 v.push_back(*(__x->value));
	 return;
       }
     else
       {
	 if (__x->left_link!=0) serialize_node(__x->left_link,v);
	 if (__x->mid_link!=0) serialize_node(__x->mid_link,v);
	 if (__x->right_link!=0) serialize_node(__x->right_link,v);
       }
   }

 void print_n()
   {
     if (empty()) {printf("\n Empty \n");return;}

     size_t s=size();
      printf("\n");
     iterator it=begin();
     size_t i=0;
     while(i<s)
       {
	 //	 printf(" %d (%d)",*it,it.__node);
	 printf(" %d",*it);
	 i++;
	 it++;
       }
     printf("\n");
   }

 void set_iterators_by_traversing()
   {
     _Base_ptr __previous,__leftmost_node;
     __leftmost_node=__root();
     while(!__leftmost_node->isLeaf()) __leftmost_node=__leftmost_node->left_link;
     __previous=__leftmost_node;
     traverse_tree(__root(),__previous);
     __header->left_link=__leftmost_node;
     __leftmost_node->mid_link=__header;
     __previous->right_link=__header;
     __header->right_link=__previous;
     
   }
 void traverse_tree(_Base_ptr __x,_Base_ptr & __previous)
   {
     if (__x->isLeaf())
       {
	 __previous->right_link=__x;
	 __x->mid_link=__previous;
	 __previous=__x;
	 return;
       }
     else
       {
	 if (__x->left_link!=0) traverse_tree(__x->left_link,__previous);
	 if (__x->mid_link!=0) traverse_tree(__x->mid_link,__previous);
	 if (__x->right_link!=0) traverse_tree(__x->right_link,__previous);
       }
     return;
   }



};


template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool
operator==(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  return __x.size() == __y.size() &&
         equal(__x.begin(), __x.end(), __y.begin());
}


template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool  
operator<(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
          const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  return lexicographical_compare(__x.begin(), __x.end(), 
                                 __y.begin(), __y.end());
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool
operator!=(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator>(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
          const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return __y < __x;
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator<=(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline bool 
operator>=(const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
           const _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y) {
  return !(__x < __y);
}



template <class _Key, class _Value, class _KeyOfValue, 
          class _Compare, class _Alloc>
inline void 
swap(_23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __x, 
     _23tree<_Key,_Value,_KeyOfValue,_Compare,_Alloc>& __y)
{
  __x.swap(__y);
}
