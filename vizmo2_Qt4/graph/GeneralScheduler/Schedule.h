/* //////////////////////////////////////////////
   Schedule.h

   by Ping An	Dec., 14, 2000

   Classes defined:

   Taskset
   Data: PID pid; 
   vector< VID > taskset;

   Schedule
   Data: 
      int numTasks;
      int numTaskSets;
      PID lastTaskSetID;
      P_TS_MAP TaskSet_Map;
      V_P_MAP TaskSetPID_Map;
      double makespan;
   char *AlgName; 

   Data structures, operators, and functions used in class TaskScheduler

   //////////////////////////////////////////////// */
#ifndef Schedule_h
#define Schedule_h

#include "SchedulerDefines.h"

namespace scheduler {


  //===================================================================
  //===================================================================
  class TaskSet
    {
    public:

      //==================================================================
      //  Constructors and Destructor
      //==================================================================
      TaskSet();
      TaskSet(PID);
      TaskSet(VID, PID);
      TaskSet(vector<VID>&, PID);
      ~TaskSet();

      //===================================================================
      //  Other Methods
      //===================================================================
      vector< VID > GetTaskSet() const { return taskset; };
      PID GetPid() const { return pid; };
      int GetPpid() const { return ppid; };

      void SetTaskSet(const vector<VID> _ts ) { taskset=_ts; };
      void SetPid(const PID _p) { pid=_p; };
      void SetPpid(const PID _p) { ppid=_p; };

      //Adding and Deleting Tasks from taskset

      void AddTask(VID);
      void AddTask(vector<VID>&);
      void DeleteTask(VID);
      void DeleteTask(vector<VID>);
      void EraseTaskSet();

      //Checking a Task in a TaskSet or not

      bool IsTask(VID,IVID&);
      int GetSize();
      bool Empty();
      int size() { return taskset.size(); }

      //Output taskset information

      void DisplayTaskSet();
      void WriteTaskSet(ostream&);

      //===================================================================
      //  Data
      //===================================================================

      PID pid;    //cluster id
      vector< VID > taskset;
      //===================================================================
 
    protected:
      int ppid;   //mapped proc. id

    };


#ifndef P_TS_MAP
      typedef map<PID, TaskSet> P_TS_MAP;
#endif
  
#ifndef P_TS_MAPITER
      typedef map<PID, TaskSet>::iterator P_TS_MAPITER;
#endif

      typedef P_TS_MAPITER GII;

#ifndef V_P_MAP
      typedef map<VID, PID> V_P_MAP;
#endif

#ifndef V_P_MAPITER
      typedef map<VID, PID>::iterator V_P_MAPITER;
#endif



  //===================================================================
  //===================================================================
  class Schedule {
      //===================================================================
      //  Data
      //===================================================================
  public:
      double makespan;
      int numTasks;
      int numTaskSets;
      PID lastTaskSetID;
      P_TS_MAP TaskSet_Map;
      V_P_MAP TaskSetPID_Map;

      P_TS_MAP& tasksetvector() { return TaskSet_Map; }
                             
      string AlgName;
      VID lvid;
      PID lpid;
    public:

      //===================================================================
      //  Constructors and Destructor
      //===================================================================
      Schedule();
      Schedule(int);    //Reserve space for tasksetvector
      Schedule(int,PID);   
      Schedule(map<PID,vector<VID> >& dmap);
      ~Schedule();

      void SetSchedule(map<PID,vector<VID> >& dmap);
     //===================================================================
      //  Other Methods
      //===================================================================

     int size() { return TaskSet_Map.size(); }

      void DisplayScheduleSizes() {
        GII ts_iter = TaskSet_Map.begin();
        GII ts_end = TaskSet_Map.end();
        for (; ts_iter != ts_end; ts_iter++) {
          cout<<"pid = "<<ts_iter->first<<" size "<<ts_iter->second.taskset.size()<<endl;
        }
      }


      //Adding and Deleting Tasks & TaskSets

      PID AddTaskSet();
      PID AddTaskSetwithPID(PID);
      PID AddTaskSet(VID);
      PID AddTaskSet(vector<VID> &);
      PID AddTask(PID, VID);
      PID AddTask(PID, vector<VID> &);
	
      
      void AddSchedule(Schedule& s);

      void DeleteTask(VID);
      void DeleteTask(vector<VID> &);
      void DeleteTaskSet(PID);
      void Clear();

      // Checking TaskSet & Predicator

      bool IsTaskSet(PID);
      PID GetTaskSetPID(VID);
      TaskSet GetTaskSetbyPID(PID);
      TaskSet GetTaskSetbyVID(VID);
      int GetNumTaskSets();
      int GetNumTasks();

      bool IsSameProcessor(VID &, VID &);
      bool CheckSchedule(const vector<VID> &, vector<VID> &, vector<VID> &);
      double GetLoadBalancingFactor(); 


      void SetName(string);
      string GetName();

      void ScheduleTransform( map<VID,VID>& vidmap);

      // Output taskset information
      void ReadSchedule(const char *);
      void ReadSchedule(istream &);
      void DisplaySchedule();
      void WriteSchedule(const char *);
      void WriteSchedule(ostream &);
    };

}  //end namespace
#endif

