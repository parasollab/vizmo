#include "EulerAngle.h"

namespace mathtool {

    ostream & operator<<(ostream & out, const EulerAngle & angle){
      out<<"( "<<angle.Alpha()*180/PI<<", "<<angle.Beta()*180/PI<<", "<<angle.Gamma()*180/PI<<")";
      return out;
    }

}//end of nprmlib namespace
