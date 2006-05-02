#ifndef SchedulerDefines_h
#define SchedulerDefines_h
#include "Defines.h"

namespace stapl {   



#ifndef PID
typedef short PID;
#endif


#ifndef CID
typedef int CID;
#endif

#ifndef INVALID_PID
#define INVALID_PID -99
#endif

#define NINFINITE -1000000
#define INFINITE 1000000

#ifndef MAX_MACHINE_LEVEL
#define MAX_MACHINE_LEVEL 1
#endif

//used in IR mapping, number of tries to find a valid pair to swap
#define TOTALTRY 10

enum GenClustAlgType {WFM,CPM,DSC,FLB,FCP,HEM,OTHER_CLUST,NIL_CLUST};          
enum GenAllocAlgType {RAND,GA,LLB,OTHER_ALLOC,NIL_ALLOC};          

} //end namespace

using namespace stapl;

#ifndef VID
typedef int VID;
#endif

#ifndef IVID
typedef vector<VID>::iterator IVID;
#endif

#ifndef CIVID
typedef vector<VID>::const_iterator CIVID;
#endif

#endif
