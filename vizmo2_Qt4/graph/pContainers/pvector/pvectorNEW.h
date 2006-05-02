//*************************************************************************
/*!
	\file pvectorNEW.h
	\author  Alin Jula	
	\date  05/21/03
	\ingroup stapl
	\brief STAPL pvector container class. It contains parallelized public methods.

	STAPL - project
	Texas A&M University,College Station, TX, USA
*/
//*************************************************************************



#ifndef _STAPL_PVECTOR_H
#define _STAPL_PVECTOR_H
#include <vector>
#include "VectorPart.h"
#include <LinearOrderPContainer.h>
#include "../distribution/VectorDistribution.h"
#include "../base/GlobalIterator.h"

using namespace _STLP_STD;



namespace stapl {


template <class _Tp>
  class pvectorNew :  public LinearOrderPContainer<VectorPart<_Tp,pair<int,int> >,VectorDistribution<_Tp> >
{

 public:
  void define_type(stapl::typer &t)  {
    stapl_assert(1,"pVector define_type used.\n");
  }

  /* This type */
  typedef pvectorNew<_Tp> this_type;
  /* Base class type */
  typedef   LinearOrderPContainer<VectorPart<_Tp,pair<int,int> >,VectorDistribution<_Tp > > _Base_type;
  /* Overdefine the value_type from the base for ease of use */
  typedef typename _Base_type::value_type value_type;
  typedef typename _Base_type::Container_type Container_type;
  typedef VectorPart<_Tp,pair<int,int> > pContainer_Part_type;
  typedef pair<int,int> Element_Set_type;

 /** Element type */
  //  typedef _Tp value_type;
  /** Pointer to element type */
  //  typedef value_type* pointer;
  /** Const pointer to element type*/
  //  typedef const value_type* const_pointer;
  /** Global Iterator to element type*/
  //  typedef GlobalIterator<this_type> iterator;
  /** Gloabl Const Iterator to element type*/
  // typedef const  GlobalIterator<this_type>  const_iterator;
  /** Local Iterator to element type*/
  //  typedef typename vector<value_type>::iterator local_iterator;
  /** Local Const Iterator to element type*/
  // typedef const typename vector<value_type>::iterator const_local_iterator;

  
  //==============================================
  // Constructors 
  //================================================

  // Default destructor 
  pvectorNew() {printf("\n PVECTOR_NEW Default Constructor");this->register_this(this);}

  pvectorNew(size_t __n): _Base_type(__n) {printf("\n PVECTOR_NEW - N Constructor");this->register_this(this);}

  pvectorNew(size_t __n, const _Tp& __value) :_Base_type(__n,__value){printf("\n PVECTOR_NEW - (N,value) Constructor");this->register_this(this);}
  
  pvectorNew(const pvectorNew<_Tp> & __x):_Base_type(__x)
    {
      printf("\n Copy constructor");
      this->register_this(this);
    }


   //=======================================
      //size statistics
      //=======================================
  // size, local_size,empty and local_empty inherited from BasePContainer 




   
  //=======================================
  //PContainer checks
  //=======================================

  /**check if _gid is a real local element
   */
  bool IsLocal(GID _gid) const
    {return true;}
  
  /**Called only for remote nodes
     check local cache first, then check gid map
     cache locally after find.
     return InvalidLocation() if not find
  */
  PID FindRemotePid(GID _gid) 
    {return 0;}
  
  //=======================================
  //PContainer Elements bookkeeping using Global ID
  //=======================================

  /**adds an element, generates gid automatically, adds to last part 
     if the pcontainer is not empty.
     *\b LOCAL
     */
  GID AddElement(const value_type& _t)
    {return 0;}
  
  /**add an element with a given gid, to last part if the pcontainer is not empty.
   *\b LOCAL
   */
  void AddElement(const value_type& _t, GID _gid)
    {}
  
  /**add elements from a sequential container
   *\b LOCAL
   */
  void AddElements(const Container_type& _data) 
    {}
  
  const value_type GetElement(GID _gid)
    {return value_type();}
  
  void SetElement(GID _gid, const value_type& _t)
    {}
  
  void DeleteElement(GID _gid)
    {}

  void BuildSubContainer( const vector<Element_Set_type>&, 
				      vector<pContainer_Part_type>&) {}
};


}// end namespace stapl


#endif
