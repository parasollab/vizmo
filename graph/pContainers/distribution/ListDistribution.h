
#ifndef LISTDISTRIBUTION_H
#define LISTDISTRIBUTION_H

#include "LinearOrderDistribution.h"

namespace stapl {
/**
 * @addtogroup plist
 * @{
 **/


/**
 *List distribution inherits the functionality of LinearOrderDistribution. Optimizations
 *specific to the list pcontainer can be implemented here.
 *
 *Status: the ListDistribution inherits all of its functionality from LinearOrderDistribution
 *which in turn inherits its functionality from the BaseDistribution. There are no methods 
 *implemented here except the constructor that perform the registration.
*/
template<class T>
class ListDistribution : public LinearOrderDistribution<typename list<stapl::splBaseElement<T> >::iterator>
{
 public:


  typedef typename list<splBaseElement<T> >::iterator List_Iterator_type;
  typedef pair<List_Iterator_type,List_Iterator_type> List_Range_type;
  
  using LinearOrderDistribution<List_Iterator_type>::element_location_map; 
  using LinearOrderDistribution<List_Iterator_type>::element_location_cache; 
  using LinearOrderDistribution<List_Iterator_type>::myid;
  using LinearOrderDistribution<List_Iterator_type>::nprocs;
  
  //===========================
  //constructors & destructors
  //===========================
  /**@brief 
   *Default constructor.
   */
  ListDistribution() {
    this->register_this(this);
    rmi_fence();
  }
  /**@brief
   *Destructor.
   */
  ~ListDistribution() {
    rmiHandle handle = this->getHandle();
    if (handle > -1)
      stapl::unregister_rmi_object(handle);
    rmi_fence(); 
  }
        
 //===========================
 //public methods
 //===========================
};

} //end namespace stapl
//@}
#endif



