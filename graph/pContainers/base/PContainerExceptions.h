#ifndef PCONTAINER_EXCEPTIONS
#define PCONTAINER_EXCEPTIONS
#include "DistributionDefines.h"

namespace stapl {

  class ElementNotFound
    {
    public:
      GID gid;

      ElementNotFound(GID x)
	{
	  gid=x;
	}

      void Report() 
	{
	  cout<<"Exception: Element with GID "<<gid<<" not found."<<endl;
	}

    };

} //end namespace
#endif
