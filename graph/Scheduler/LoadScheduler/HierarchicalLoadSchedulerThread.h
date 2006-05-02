#ifndef _HLS_Thread_h_
#define _HLS_Thread_h_

#include "Transferable_pList.h"
#include "Transferable_pRange.h"
#include "Transferable_pRange.h"
#include "pRange.h"

void * Hierarchical_Load_Scheduler_Run(void *hls_address)
{
	
	BaseScheduler * bsch = (BaseScheduler*) hls_address;
	
	return bsch->Schedule();
	
	
}

#endif
