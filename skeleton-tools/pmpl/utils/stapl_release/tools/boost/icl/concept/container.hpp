/*-----------------------------------------------------------------------------+    
Copyright (c) 2010-2010: Joachim Faulhaber
+------------------------------------------------------------------------------+
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENCE.txt or copy at
           http://www.boost.org/LICENSE_1_0.txt)
+-----------------------------------------------------------------------------*/

// Adding template parameter to swap so that when std::swap and boost::icl::swap
// are both valid for the parameters being passed
// (e.g., std::tuple<boost::interval_set<int> > ) the std::swap implementation
// will be used as it requires fewer substitutions.  This modification was based
// on the approach discussed in https://svn.boost.org/trac/boost/ticket/2839 .
// https://svn.boost.org/trac/boost/ticket/9624 was opened to report the issue.

#ifndef BOOST_ICL_CONCEPT_CONTAINER_HPP_JOFA_100923
#define BOOST_ICL_CONCEPT_CONTAINER_HPP_JOFA_100923

#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/icl/type_traits/is_container.hpp>
#include <boost/icl/type_traits/is_icl_container.hpp>

namespace boost{ namespace icl
{

//==============================================================================
//= Emptieness
//==============================================================================

/** Tests if the container is empty. 
    Complexity: constant. */
template<class Type>
typename enable_if<is_container<Type>, bool>::type
is_empty(const Type& object)
{
    return object.begin()==object.end();
}


/** All content of the container is dropped. 
    Complexity: linear. */
template<class Type>
typename enable_if<is_container<Type>, void>::type
clear(Type& object)
{
    object.erase(object.begin(), object.end());
}

//==============================================================================
//= Size
//==============================================================================

template<class Type> 
typename enable_if<mpl::and_< is_container<Type>
                            , mpl::not_<is_icl_container<Type> > >
                  , std::size_t>::type
iterative_size(const Type& object)
{ 
    return object.size(); 
}

//==============================================================================
//= Swap
//==============================================================================

template<class Type1, class Type2>
typename enable_if<is_container<Type1>, void>::type
swap(Type1& left, Type2& right)
{
    left.swap(right);
}

//==============================================================================
//= Iteration
//==============================================================================

template<class Type>
typename enable_if<is_container<Type>, typename Type::iterator>::type
cyclic_prior(Type& object, typename Type::iterator it_)
{ return it_ == object.begin() ? object.end() : --it_; }

template<class Type>
typename enable_if<is_container<Type>, typename Type::const_iterator>::type
cyclic_prior(const Type& object, typename Type::const_iterator it_)
{ return it_ == object.begin() ? object.end() : --it_; }



}} // namespace boost icl

#endif


