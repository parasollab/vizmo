/////////////////////////////////////////////////////////////////////
//
//  Defines.h
//
//  General Description
//  This an example of how the includes should look like in order to have compatibility
//  with different STLs
//  Created
//      Gabriel Tanase
//
/////////////////////////////////////////////////////////////////////

//include standard headers
//#include "../template_instantiation.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <assert.h>

//not using STL_PORT
#ifndef  _STL_PORT

  #if defined(__GNUC__) //gcc
    #if __GNUC__ < 3
      #warning includes for gcc < 3 when not using STLport have not been tested
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include <algo.h>
      #include <list.h>
      #include <vector.h>
      #include <deque.h>
      #include <stack.h>
      #include <map.h>
      #include <iterator.h>
      #include <string>
      #include <queue.h>
      namespace Sgi {  // inherit globals
        #warning namespaces for gcc < 3 when not using STLport have not been tested
        using ::hash_map;
        //JKP - there should be more "using"s here if ever this section of code is used in the future
      }; 
    #else //gcc >= 3
      #include <iostream>
      #include <fstream>
      #include <iomanip>
      #include <ext/algorithm>
      #include <list>   
      #include <vector>
      #include <deque>  
      #include <stack> 
      #include <map> 
      #include <ext/iterator>
      #include <string>
      #include <queue>
      #include <ext/hash_map>
      #if __GNUC_MINOR__ == 0 && !defined(__blrts__)
        namespace Sgi = std;               // GCC 3.0
      #else
        namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later
      #endif
    #endif
    using namespace Sgi;    // pick things up from Sgi
    using namespace std;    // but override with things from std
    //using namespace stapl;  // finally, STAPL definitions dominate (for ARMI)

  #else //not gcc
    #if defined(__KCC)
      #define __compiler_found__
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include <algorithm>    
      #include <list>     
      #include <vector>       
      #include <deque>        
      #include <stack>        
      #include <map> 
      #include <iterator>
      #include <string>
      #include <queue>
    #endif

    #if defined(hppa) && defined(__cplusplus) //c++ on parasol
      #define __compiler_found__
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include <algorithm>    
      #include <list>     
      #include <vector.h>
      #include <deque.h>  
      #include <stack>  
      #include <map.h> 
      #include <iterator>
      #include <queue.h>
    #endif

    #if defined(__HP_aCC) //aCC
      #define __compiler_found__
      #include <iostream.h>
      #include <fstream>
      #include <iomanip>
      #include <algorithm>    
      #include <list>     
      #include <vector>       
      #include <deque>        
      #include <stack>        
      #include <map> 
      #include <iterator>
      #include <queue>
    #endif

    #if defined(_DARWIN)
      #define __compiler_found__
      #include <iostream>
      #include <fstream>
      #include <iomanip>
      #include <algorithm>    
      #include <list>     
      #include <vector>       
      #include <deque>        
      #include <stack>        
      #include <map> 
      #include <iterator>
      #include <queue>
    #endif

    #if defined(__sgi) //K2, MIPSpro Compiler
      #define __compiler_found__
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include <algo.h>
      #include <list.h>
      #include <vector.h>
      #include <deque.h>
      #include <stack.h>
      #include <map.h>
      #include <iterator.h>
      #include <string>
      #include <queue>
    #endif

    #if defined(__blrts__) //IBM xlc Compiler
      #define __compiler_found__
      #include <iostream>
      #include <fstream>
      #include <iomanip>
      #include <ext/algorithm>
      #include <list>   
      #include <vector>
      #include <deque>  
      #include <stack> 
      #include <map> 
      #include <ext/iterator>
      #include <string>
      #include <queue>
      #include <ext/hash_map>
      namespace Sgi = ::__gnu_cxx; 
      using namespace Sgi;    // pick things up from Sgi
      using namespace std;    // but override with things from std
      //using namespace stapl;  // finally, STAPL definitions dominate (for ARMI)
    #endif

    #if !defined(__compiler_found__) //default includes
      #include <iostream.h>
      #include <fstream.h>
      #include <iomanip.h>
      #include <algo.h>
      #include <list.h>
      #include <vector.h>
      #include <deque.h>
      #include <stack.h>
      #include <map.h>
      #include <iterator.h>
      #include <string>
      #include <queue.h>
    #endif

  #endif //end !__GNUC__

//STL_PORT 
#else
  #include <iostream>
  #include <ostream>
  #include <fstream>
  #include <iomanip>
  #include <algorithm>    
  #include <list>     
  #include <vector>       
  #include <deque>        
  #include <queue>        
  #include <stack>   
  #include <map>
  #include <set>
  #include <iterator>
  #include <hash_map>
  //using namespace _STLP_STD;
  using namespace std;
#endif

#ifndef OK
#define OK  0
#endif

#ifndef ERROR
#define ERROR -1
#endif
