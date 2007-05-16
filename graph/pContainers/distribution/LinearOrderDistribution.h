#ifndef LINEARORDERDISTRIBUTION_H
#define LINEARORDERDISTRIBUTION_H

#include "BaseDistribution.h"

/**
 * @addtogroup linearorderpcontainer
 * @{
 **/


namespace stapl {
/**
 *Linear order distribution class is intended to be a specialized class derived
 *from BaseDistribution with specific optimization for linera order containers
 * like  pVector, pList.
*/
template<class ITER>
class LinearOrderDistribution : public BaseDistribution<pair<ITER,ITER> >
{
 public:
  typedef pair<ITER,ITER> Linear_Range_type;  

 public:
  //===========================
  //constructors & destructors
  //===========================
  /**@brief
   *Default constructor.
   */
  LinearOrderDistribution() {}
  /**@brief
   *Destructor.
   */
  ~LinearOrderDistribution() {}
        
 //===========================
 //public methods
 //===========================
  /**@brief 
   *Assignment operator.
   */
  LinearOrderDistribution& operator= (const LinearOrderDistribution& w) {
    this->element_location_map  = w.element_location_map;
    this->element_location_cache  = w.element_location_cache;
    this->dist_version=w.dist_version;
    for (int i = 0; i < MAX_MACHINE_LEVEL; ++i)
      this->cids[i] = w.cids[i];
    this->partbrdyinfo=w.partbrdyinfo;
    this->validbdinfo = w.validbdinfo;  
     return *this;
  }
};

} //end namespace stapl

//@}
#endif



