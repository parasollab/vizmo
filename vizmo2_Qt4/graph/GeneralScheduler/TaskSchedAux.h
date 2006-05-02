#ifndef TASKSCHEDAUX_H
#define TASKSCHEDAUX_H
//======================================================================
//
// Typedefs for FLB algorithm
//
//======================================================================
#include <runtime.h>
#include "SchedulerDefines.h"

namespace scheduler {

class TaskTriplet
{
  friend ostream& operator << (ostream &, const TaskTriplet &);
public:
  double priority1;
  double priority2;
  int id;

  TaskTriplet(double p1, double p2, int i) {
    priority1 = p1;
    priority2 = p2;
    id = i;
  }
  bool
  operator == (const TaskTriplet &tt) const {
    return id == tt.id;
  }
};

 class ProcPair
{
  friend ostream& operator << (ostream &, const ProcPair &);
public:
  double priority;
  int id;
  void define_type(stapl::typer &t) {
    t.local(priority);
    t.local(id);
  }
  ProcPair() : priority(0.0), id(0) {}
  ProcPair(double p, int i) : priority(p), id(i) {}
  ~ProcPair() {}

  bool
  operator == (const ProcPair &pp) const {
    return id == pp.id;
  }
};

class TT_Cmp
{
public:
  bool operator () (const TaskTriplet &t1, const TaskTriplet &t2) const
  {
    if (t1.priority1 == t2.priority1)
      return t1.priority2 > t2.priority2;
    return t1.priority1 < t2.priority1;
  }
};

class TT_GT
{
public:
  bool operator () (const TaskTriplet &t1, const TaskTriplet &t2) const
  {
    if (t1.priority1 == t2.priority1)
      return t1.priority2 > t2.priority2;
    return t1.priority1 > t2.priority1;
  }
};

class PP_LT
{
public:
  bool operator () (const ProcPair &p1, const ProcPair &p2) const
  {
    return p1.priority < p2.priority;
  }
};

class PP_GT
{
public:
  bool operator () (const ProcPair &p1, const ProcPair &p2) const
  {
    return p1.priority > p2.priority;
  }
};

} //end namespace

inline ostream&
operator <<(ostream &os, const scheduler::TaskTriplet &t) {
  os << "(" << t.priority1 << ", " << t.priority2 << ", " << t.id << ")";
  return os;
}

inline ostream&
operator <<(ostream &os, const scheduler::ProcPair &t) {
  os << "(" << t.priority << ", " << t.id << ")";
  return os;
}

#endif
