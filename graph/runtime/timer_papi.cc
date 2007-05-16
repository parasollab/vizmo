/**
 * This provides preliminary support for PAPI using the existing STAPL start/stop_timer
 * infrastructure.  As such, it is currently limited to counting one event at a time.
 * The event may be selected at run time by setting the environment variable STAPL_TIMER_PAPI
 * to a PAPI event name.  PAPI event names can be found here:
 *   http://icl.cs.utk.edu/projects/papi/files/documentation/PAPI_Software_Spec_30b.htm#common
 * or in /usr/include/papiStdEventDefs.h (assumes PAPI is installed in /usr)
 * or from the output of the "avail" example (in /usr/share/doc/papi-3.0/tests/ctests on
 * those systems using my PAPI RPMs)
 *
 * NOTE: 11Nov2004/jkp - the web page above is wrong/old and the header file is not easy to
 *                       comprehend... so your only real option is avail... I've attached the
 *                       output at the bottom of this file for easy reference (for now).
 *
 * To use this timer you will need to specifiy -DSTAPL_TIMER_PAPI at compile time.
 * You also need to specifiy where PAPI can be found.  You can use USER_CFLAGS and USER_LIB
 * to pass these values...  e.g.
 *
 *   make [usual stuff]  USER_CFLAGS="-DSTAPL_TIMER_PAPI -I/home/jkp/papi/include" USER_LIB="-L/home/jkp/papi/lib -lpapi"
 *
 * then, at run time you can select the counter... e.g. 
 *
 *   export STAPL_TIMER_PAPI="PAPI_L1_TCA"
 *   ./mySTAPLprogram
 *
 * will report "Level 1 Total Cache Assesses".  If no variable is set, the default 
 * is the total clock cycles (which seems to only be accessible through PAPI_get_real_cyc()
 * and isn't listed as an event in the above... the event PAPI_TOT_CYC (total cpu cycles)
 * counts only the user's process (doesn't count cycles when user process is idle/waiting).
 * [I'll inquire as to why... it would be nice if it real cycles were avail through the API :/]
 *
 * Note: if you are using MPI, you will need to use mpirun's -x option to pass the variable... e.g.
 *
 *   mpirun -np 2 -x STAPL_TIMER_PAPI ./mySTAPLprogram
 *
**/

#include "Defines.h"
#include "timer_papi.h"

#define WHICH_TIMER_USE_REAL_CYC 1

namespace stapl
{

  // someday this will be a proper class and we can clean all this out of the namespace - jkp
  typedef long long int timer;
  int which_timer = -1;
  int timer_active = 0;

#ifdef LATERWHENMORESUPPORTED
  void select_timer()
  {
    char a[4][30] = { "", "", "", "" }, b[30], *cp;
    int len, i = 0;
    char *s = getenv("STAPL_TIMER_PAPI");
    if (!s)
      s = "PAPI_TOT_CYC PAPI_TOT_INS";
    if (s) {
      cp = s;
      while (cp != NULL && *cp != '\0') {
        cp = strstr(cp, "PAPI");
        if (cp != NULL) {
          len = strcspn(cp, " \0");     // find space
          strncpy(a[i], cp, len);
          // debug - printf("a[%d]=%s  (len=%d)\n", i, a[i], len);
          i++;
          cp += len;
        }
      }
    } else {                    // use default timers
    }
  }
#else
  int select_timer(void)
  {
    int which = WHICH_TIMER_USE_REAL_CYC;       // default is high-res/cycle counter... not the same as PAPI_TOT_CYC
    char *s = getenv("STAPL_TIMER_PAPI");
    if (s)
    {
      if (PAPI_event_name_to_code(s, &which) != PAPI_OK) {
        cerr << "select_timer() - PAPI_event_name_to_code(" << s <<
            ") failed" << endl;
        exit(1);
      }
      if (PAPI_query_event(which) != PAPI_OK)
      {
        cerr << "select_timer() - PAPI_query_event() failed... counter 0x"
            << hex << which << " is not available on this system" << endl;
        exit(1);
      }
    }
    return which;
  }
#endif

  void init_timer(void)
  {
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
      cerr << "PAPI library init error" << endl;
      exit(1);
    }
    which_timer = select_timer();
  }

  void print_timer(void)
  {
    char evname[PAPI_MAX_STR_LEN];
    PAPI_event_info_t info;
    if (which_timer != WHICH_TIMER_USE_REAL_CYC) {
      if (PAPI_event_code_to_name(which_timer, evname) != PAPI_OK) {
        cerr << "print_timer() - PAPI_event_code_to_name() failed" << endl;
        exit(1);
      }
      if (PAPI_get_event_info(which_timer, &info) != PAPI_OK) {
        cerr << "print_timer() - PAPI_get_event_info() failed" << endl;
        exit(1);
      } else
        cout << "start_timer: using PAPI_start_counters(" << evname <<
            " = 0x" << hex << which_timer << " = " << info.
            long_descr << ")" << endl;
    } else
      cout <<
          "start_timer: using PAPI_start_counters(PAPI_get_real_cyc())" <<
          endl;
  }

  timer start_timer(void)
  {
    timer time = 0;             // a dummy value or PAPI_get_real_cyc() 
    if (which_timer == -1)
      init_timer();
#ifdef _PRINT_TIMER
    print_timer();
#endif
    if (timer_active) {
      cerr <<
          "start_timer() - error: tried to start timer before stopping previous one???"
          << endl;
      exit(1);
    } else
      timer_active = 1;

    if (which_timer == WHICH_TIMER_USE_REAL_CYC)
      time = PAPI_get_real_cyc();
    else {
      if (PAPI_start_counters(&which_timer, 1) != PAPI_OK) {
        cerr << "start_timer() - PAPI_start_counters() failed" << endl;
        exit(1);
      }
    }
    return time;
  }

  double stop_timer(timer time1)
  {
    double diff = 0;
    timer count;
    if (!timer_active) {
      cerr <<
          "stop_timer() - error: tried to stop timer before starting it???"
          << endl;
      exit(1);
    } else
      timer_active = 0;

    if (which_timer != -1) {
      if (which_timer == WHICH_TIMER_USE_REAL_CYC) {
        timer time2 = PAPI_get_real_cyc();
        diff = time2 - time1;
      } else {
        if (PAPI_OK != PAPI_stop_counters(&count, 1)) {
          cerr << "stoptimer() - PAPI_stop_counters() failed" << endl;
          exit(1);
        }
        diff = count;
      }
    }
    return diff;
  }
}                               // end namespace

/*** For reference until web page updated... event names are the same on all platforms
     (though the event may not be available...)

Test case avail.c: Available events and hardware information.
-------------------------------------------------------------------------
Vendor string and code   : AuthenticAMD (2)
Model string and code    : AMD K7 (9)
CPU Revision             : 0.000000
CPU Megahertz            : 1830.024048
CPU's in this Node       : 1
Nodes in this System     : 1
Total CPU's              : 1
Number Hardware Counters : 4
Max Multiplex Counters   : 32
-------------------------------------------------------------------------
Name		Code		Avail	Deriv	Description (Note)
PAPI_L1_DCM	0x80000000	Yes	No	Level 1 data cache misses ()
PAPI_L1_ICM	0x80000001	Yes	No	Level 1 instruction cache misses ()
PAPI_L2_DCM	0x80000002	Yes	No	Level 2 data cache misses ()
PAPI_L2_ICM	0x80000003	Yes	No	Level 2 instruction cache misses ()
PAPI_L3_DCM	0x80000004	No	No	Level 3 data cache misses ()
PAPI_L3_ICM	0x80000005	No	No	Level 3 instruction cache misses ()
PAPI_L1_TCM	0x80000006	Yes	Yes	Level 1 cache misses ()
PAPI_L2_TCM	0x80000007	Yes	Yes	Level 2 cache misses ()
PAPI_L3_TCM	0x80000008	No	No	Level 3 cache misses ()
PAPI_CA_SNP	0x80000009	No	No	Requests for a snoop ()
PAPI_CA_SHR	0x8000000a	No	No	Requests for exclusive access to shared cache line ()
PAPI_CA_CLN	0x8000000b	No	No	Requests for exclusive access to clean cache line ()
PAPI_CA_INV	0x8000000c	No	No	Requests for cache line invalidation ()
PAPI_CA_ITV	0x8000000d	No	No	Requests for cache line intervention ()
PAPI_L3_LDM	0x8000000e	No	No	Level 3 load misses ()
PAPI_L3_STM	0x8000000f	No	No	Level 3 store misses ()
PAPI_BRU_IDL	0x80000010	No	No	Cycles branch units are idle ()
PAPI_FXU_IDL	0x80000011	No	No	Cycles integer units are idle ()
PAPI_FPU_IDL	0x80000012	No	No	Cycles floating point units are idle ()
PAPI_LSU_IDL	0x80000013	No	No	Cycles load/store units are idle ()
PAPI_TLB_DM	0x80000014	Yes	No	Data translation lookaside buffer misses ()
PAPI_TLB_IM	0x80000015	Yes	No	Instruction translation lookaside buffer misses ()
PAPI_TLB_TL	0x80000016	Yes	Yes	Total translation lookaside buffer misses ()
PAPI_L1_LDM	0x80000017	Yes	No	Level 1 load misses ()
PAPI_L1_STM	0x80000018	Yes	No	Level 1 store misses ()
PAPI_L2_LDM	0x80000019	Yes	No	Level 2 load misses ()
PAPI_L2_STM	0x8000001a	Yes	No	Level 2 store misses ()
PAPI_BTAC_M	0x8000001b	No	No	Branch target address cache misses ()
PAPI_PRF_DM	0x8000001c	No	No	Data prefetch cache misses ()
PAPI_L3_DCH	0x8000001d	No	No	Level 3 data cache hits ()
PAPI_TLB_SD	0x8000001e	No	No	Translation lookaside buffer shootdowns ()
PAPI_CSR_FAL	0x8000001f	No	No	Failed store conditional instructions ()
PAPI_CSR_SUC	0x80000020	No	No	Successful store conditional instructions ()
PAPI_CSR_TOT	0x80000021	No	No	Total store conditional instructions ()
PAPI_MEM_SCY	0x80000022	No	No	Cycles Stalled Waiting for memory accesses ()
PAPI_MEM_RCY	0x80000023	No	No	Cycles Stalled Waiting for memory Reads ()
PAPI_MEM_WCY	0x80000024	No	No	Cycles Stalled Waiting for memory writes ()
PAPI_STL_ICY	0x80000025	No	No	Cycles with no instruction issue ()
PAPI_FUL_ICY	0x80000026	No	No	Cycles with maximum instruction issue ()
PAPI_STL_CCY	0x80000027	No	No	Cycles with no instructions completed ()
PAPI_FUL_CCY	0x80000028	No	No	Cycles with maximum instructions completed ()
PAPI_HW_INT	0x80000029	Yes	No	Hardware interrupts ()
PAPI_BR_UCN	0x8000002a	Yes	No	Unconditional branch instructions ()
PAPI_BR_CN	0x8000002b	Yes	No	Conditional branch instructions ()
PAPI_BR_TKN	0x8000002c	Yes	No	Conditional branch instructions taken ()
PAPI_BR_NTK	0x8000002d	Yes	Yes	Conditional branch instructions not taken ()
PAPI_BR_MSP	0x8000002e	Yes	No	Conditional branch instructions mispredicted ()
PAPI_BR_PRC	0x8000002f	Yes	Yes	Conditional branch instructions correctly predicted ()
PAPI_FMA_INS	0x80000030	No	No	FMA instructions completed ()
PAPI_TOT_IIS	0x80000031	No	No	Instructions issued ()
PAPI_TOT_INS	0x80000032	Yes	No	Instructions completed ()
PAPI_INT_INS	0x80000033	No	No	Integer instructions ()
PAPI_FP_INS	0x80000034	No	No	Floating point instructions ()
PAPI_LD_INS	0x80000035	No	No	Load instructions ()
PAPI_SR_INS	0x80000036	No	No	Store instructions ()
PAPI_BR_INS	0x80000037	Yes	No	Branch instructions ()
PAPI_VEC_INS	0x80000038	No	No	Vector/SIMD instructions ()
PAPI_RES_STL	0x80000039	Yes	No	Cycles stalled on any resource ()
PAPI_FP_STAL	0x8000003a	No	No	Cycles the FP unit(s) are stalled ()
PAPI_TOT_CYC	0x8000003b	Yes	No	Total cycles ()
PAPI_LST_INS	0x8000003c	No	No	Load/store instructions completed ()
PAPI_SYC_INS	0x8000003d	No	No	Synchronization instructions completed ()
PAPI_L1_DCH	0x8000003e	Yes	Yes	Level 1 data cache hits ()
PAPI_L2_DCH	0x8000003f	Yes	Yes	Level 2 data cache hits ()
PAPI_L1_DCA	0x80000040	Yes	No	Level 1 data cache accesses ()
PAPI_L2_DCA	0x80000041	Yes	No	Level 2 data cache accesses ()
PAPI_L3_DCA	0x80000042	No	No	Level 3 data cache accesses ()
PAPI_L1_DCR	0x80000043	No	No	Level 1 data cache reads ()
PAPI_L2_DCR	0x80000044	Yes	Yes	Level 2 data cache reads ()
PAPI_L3_DCR	0x80000045	No	No	Level 3 data cache reads ()
PAPI_L1_DCW	0x80000046	No	No	Level 1 data cache writes ()
PAPI_L2_DCW	0x80000047	Yes	Yes	Level 2 data cache writes ()
PAPI_L3_DCW	0x80000048	No	No	Level 3 data cache writes ()
PAPI_L1_ICH	0x80000049	No	No	Level 1 instruction cache hits ()
PAPI_L2_ICH	0x8000004a	No	No	Level 2 instruction cache hits ()
PAPI_L3_ICH	0x8000004b	No	No	Level 3 instruction cache hits ()
PAPI_L1_ICA	0x8000004c	Yes	No	Level 1 instruction cache accesses ()
PAPI_L2_ICA	0x8000004d	Yes	No	Level 2 instruction cache accesses ()
PAPI_L3_ICA	0x8000004e	No	No	Level 3 instruction cache accesses ()
PAPI_L1_ICR	0x8000004f	Yes	No	Level 1 instruction cache reads ()
PAPI_L2_ICR	0x80000050	No	No	Level 2 instruction cache reads ()
PAPI_L3_ICR	0x80000051	No	No	Level 3 instruction cache reads ()
PAPI_L1_ICW	0x80000052	No	No	Level 1 instruction cache writes ()
PAPI_L2_ICW	0x80000053	No	No	Level 2 instruction cache writes ()
PAPI_L3_ICW	0x80000054	No	No	Level 3 instruction cache writes ()
PAPI_L1_TCH	0x80000055	No	No	Level 1 total cache hits ()
PAPI_L2_TCH	0x80000056	No	No	Level 2 total cache hits ()
PAPI_L3_TCH	0x80000057	No	No	Level 3 total cache hits ()
PAPI_L1_TCA	0x80000058	Yes	Yes	Level 1 total cache accesses ()
PAPI_L2_TCA	0x80000059	No	No	Level 2 total cache accesses ()
PAPI_L3_TCA	0x8000005a	No	No	Level 3 total cache accesses ()
PAPI_L1_TCR	0x8000005b	No	No	Level 1 total cache reads ()
PAPI_L2_TCR	0x8000005c	No	No	Level 2 total cache reads ()
PAPI_L3_TCR	0x8000005d	No	No	Level 3 total cache reads ()
PAPI_L1_TCW	0x8000005e	No	No	Level 1 total cache writes ()
PAPI_L2_TCW	0x8000005f	No	No	Level 2 total cache writes ()
PAPI_L3_TCW	0x80000060	No	No	Level 3 total cache writes ()
PAPI_FML_INS	0x80000061	No	No	Floating point multiply instructions ()
PAPI_FAD_INS	0x80000062	No	No	Floating point add instructions ()
PAPI_FDV_INS	0x80000063	No	No	Floating point divide instructions ()
PAPI_FSQ_INS	0x80000064	No	No	Floating point square root instructions ()
PAPI_FNV_INS	0x80000065	No	No	Floating point inverse instructions ()
PAPI_FP_OPS	0x80000066	No	No	Floating point operations ()
-------------------------------------------------------------------------
avail.c                                  PASSED

***/

// EOF 
