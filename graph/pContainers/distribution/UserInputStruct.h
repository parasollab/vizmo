#ifndef USERINPUTSTRCT_h
#define USERINPUTSTRCT_h

#include "DistributionDefines.h"
#include "SchedulerDefines.h"
#include "MachineInfo.h"

struct Alg {
  Alg() {}
  Alg(void* _ddg, GenClustAlgType _c, GenAllocAlgType _a) 
    : cat(_c), aat(_a) {
    ddgptr = _ddg;
  }
  void* ddgptr;
  GenClustAlgType cat;
  GenAllocAlgType aat;

  //for GA only
  int niter;
  int totaliter;
};

struct Percentage {
  vector<double> percentiles;
};

struct Uniform {
};

struct Given_Part {
  vector<TASKPOOL> data_part;
  GenAllocAlgType aat;
};

struct Given_Map {
  map<PID,TASKPOOL> data_map;
};

struct Given_Dist {
  vector<string> file_names;
};
  
struct Nil_Policy {
  bool hasDDG;
  void* ddgptr;
  int performance_degree;
};

struct PartitionInputs {
  PartitionInputs() { 
    valid_inputs = false;
  }

  void Set(PartitionPolicy _p, void* _d, void* _i, const MachineInfo& _mi) {
    partpolicy = _p; 
    dataptr = _d;
    inputs =_i;
    valid_inputs = true;
    machineinfo = _mi;
  }

  bool valid_inputs;
  PartitionPolicy partpolicy;
  void* dataptr;
  void* inputs;  //ptr to above selected struct
  MachineInfo machineinfo;
};

#endif
