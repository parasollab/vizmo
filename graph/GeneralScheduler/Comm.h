/*!
  \file Comm.h
  \author Ping An
  \date Dec. 14, 00
  \ingroup Scheduler
  \brief a general edge in a DDG for Scheduler 

  Stores message size/communication cost and other infos
    double messagesize;
    COMMWEIGHT commcost;
    COMMINFO comminfo;
*/

#ifndef Comm_h
#define Comm_h

#ifdef PARALLEL_SCHEDULER
#include <runtime.h>
#endif
#include "SchedulerDefines.h"

namespace scheduler {

//===================================================================
//===================================================================
template<class COMMWEIGHT, class COMMINFO>
class Comm {

  //===================================================================
  //  Data
  //===================================================================
 public:

  COMMWEIGHT messagesize;
  COMMWEIGHT commcost;
  COMMINFO comminfo;

public:
#ifdef PARALLEL_SCHEDULER
  void define_type(stapl::typer &t) {
      t.local(messagesize);
      t.local(comminfo);
  }
#endif
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
 public:
  Comm(): messagesize(0),commcost(0)
    {}

  Comm(COMMWEIGHT _msize): messagesize(_msize)
    {}  

  Comm(COMMWEIGHT _msize,COMMINFO _ci): messagesize(_msize),comminfo(_ci)
    {}  

  Comm(COMMWEIGHT _msize, COMMWEIGHT _ccost, COMMINFO _ci) :
    messagesize(_msize),
    commcost(_ccost),
    comminfo(_ci)
   {}

  Comm(const Comm& ccost) : 
    messagesize(ccost.messagesize),
    commcost(ccost.commcost),
    comminfo(ccost.comminfo)
    {}

  ~Comm() {}

  //===================================================================
  //  Other Methods
  //===================================================================

  //!Getting Data & Statisitcs information
  const COMMWEIGHT& GetMessageSize() const { return messagesize; }
  const COMMWEIGHT& GetCommCost() const {return commcost; }
  const COMMINFO& GetCommInfo() const { return comminfo; }

  //!Modifying data
  void SetMessageSize(const COMMWEIGHT & _msize) { messagesize = _msize; }
  void SetCommCost(const COMMWEIGHT& _ccost) { commcost = _ccost; }
  void SetCommInfo(const COMMINFO& _m) { comminfo = _m; }

  static Comm InvalidWeight() { Comm comm; return comm; }

  //===================================================================
  // Operators
  //===================================================================

  bool operator== ( const Comm &ccost) const {
  return (messagesize == ccost.messagesize &&
	   commcost == ccost.commcost &&
	   comminfo == ccost.comminfo );
  }

  Comm& operator= (const Comm& ccost) {
    messagesize = ccost.messagesize;
    commcost = ccost.commcost;
    comminfo = ccost.comminfo;
    return *this;
  }
};

} //end namespace

   inline istream& operator>> (istream& s, scheduler::Comm<double,int> &comm) {
	char ss[10];
	s >> ss;
        double ms=atof(ss);
//	s >>ms;
	comm.SetMessageSize(ms);
        return s;
   };

   inline ostream& operator<< (ostream& s, const scheduler::Comm<double,int>
&comm)
{
        return s << "messagesize:"<<comm.GetMessageSize()<<
	  " commcost:"<<comm.GetCommCost() <<" ";
   };

   inline istream& operator>> (istream& s, scheduler::Comm<double,double> &comm) {
	char ss[10];
	s >> ss;  
      double ms=atof(ss);
//	s >>ms;
	comm.SetMessageSize(ms);
        return s;
   };

   inline ostream& operator<< (ostream& s, 
			       const scheduler::Comm<double,double> &comm) {
     return s <<comm.GetMessageSize();
   };

/*    inline ostream& operator<< (ostream& s, const scheduler::Comm<double,double> */
/* &comm) */
/* { */
/*         return s << "messagesize:"<<comm.GetMessageSize()<< */
/* 	  " commcost:"<<comm.GetCommCost() <<" "; */
/*    }; */


#endif
