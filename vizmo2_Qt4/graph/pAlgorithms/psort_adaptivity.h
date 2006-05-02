///@author Olga Tkachyshyn 
///@date 01/19/2005
#ifndef _SORT_ADAPT_ALGO
#define _SORT_ADAPT_ALGO
#include "psort_adaptive.h"
namespace stapl{
  template <class DataType>
    int sort_adapt_algo(int num_procsnum_procs, int num_elementsnum_elements, DataType& data_typedata_type, int presortednesspresortedness){
    if (data_typedata_type == T_INT) {
      if (num_procsnum_procs < 4) {
	return sample;
      } else {
	if (presortednesspresortedness == random) {
	  return radix;
	} else {
	  if (num_elementsnum_elements < 1.024e+08) {
	    return column;
	  } else {
	    return sample;
	  }
	}
      }
    }//end of case for T_INT
    else {
      if (num_elementsnum_elements < 1.024e+08) {
	if (num_procsnum_procs < 6) {
	  return sample;
	} else {
	  return column;
	}
      }//end of case num_elementsnum_elements < 1.024e+08
      else {
	if (presortednesspresortedness == revers) {
	  if (data_typedata_type == point) {
	    return sample;
	  } else {
	    return column;
	  }
	} else {
	  return sample;
	}
      }
    }
  }

}//end namespace
#endif
