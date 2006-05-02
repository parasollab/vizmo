/*!
	\file AbstractBasePContainer.h
	\date Jan. 20, 03
	\ingroup stapl
	\brief Abstract base class to provide interface functions to the USER
	basic pContainer using global ID as interfaces 


*/

#ifndef ABSTRACTBASEPCONTAINER_H
#define ABSTRACTBASEPCONTAINER_H

#include "DistributionDefines.h"

namespace stapl {

  template<class PCONTAINERPART>
    class AbstractBasePContainer 
    {
    public:
      //=======================================
      //types
      //=======================================
      //user's data type
      typedef typename PCONTAINERPART::value_type value_type;

      //sequential container with user data type 
      typedef typename PCONTAINERPART::Container_type Container_type;

      //=======================================
      //constructors 
      //=======================================
      /**default constructor 
       *\b LOCAL
       */
      AbstractBasePContainer() 
	{}

      /**constructor with n default elements, distributed evenly*/
      AbstractBasePContainer(int n) {}

      /**constructor with n copies of elements T, distributed evenly*/
      AbstractBasePContainer(int n, const value_type& _data) {}

      /**destructor 
       *\b LOCAL
       */
      ~AbstractBasePContainer() {}

      //=======================================
      //size statistics
      //=======================================

      /**global size 
       *\b GLOBAL
       */
      virtual size_t size()  = 0;

      /**local size
       *\b LOCAL
       */
      virtual size_t local_size() const = 0;
  
      /**global empty 
       *\b GLOBAL
       */
      virtual bool empty() = 0;
  
      /**local empty
       *\b LOCAL
       */
      virtual bool local_empty() const = 0;

      //=======================================
      //PContainer checks
      //=======================================

      /**check if _gid is a real local element
       */
      virtual bool IsLocal(GID _gid) const = 0;

      /**Called only for remote nodes
	 check local cache first, then check gid map
	 cache locally after find.
	 return InvalidLocation() if not find
      */
      virtual PID FindRemotePid(GID _gid)  = 0;

      //=======================================
      //PContainer Elements bookkeeping using Global ID
      //=======================================

      /**add an element, generate gid automaticly, add to last part 
	 if the pcontainer is not empty.
	 *\b LOCAL
	 */
      virtual GID AddElement(const value_type& _t) = 0;

      /**add an element with a given gid, to last part if the pcontainer is not empty.
       *\b LOCAL
       */
      virtual void AddElement(const value_type& _t, GID _gid) = 0;

      /**add elements from a sequential container
       *\b LOCAL
       */
      //virtual void AddElements(const Container_type& _data)  = 0;

      virtual const value_type GetElement(GID _gid) const = 0;

      virtual void SetElement(GID _gid, const value_type& _t) = 0;

      virtual void DeleteElement(GID _gid) = 0;

    }; 

} //end namespace stapl

#endif
