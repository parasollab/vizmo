#ifndef TESTTASK_H
#define TESTTASK_H

#include <runtime.h>

#ifndef MAXWEIGHT
#define MAXWEIGHT 1000000
#endif

//===================================================================
//===================================================================
class Task {

  //===================================================================
  //  Data
  //===================================================================
 protected:
  int taskwt;
 
 public:
  void define_type(stapl::typer &t){
    t.local(taskwt);
  }
  //===================================================================
  //  Constructors and Destructor
  //===================================================================
  Task() : taskwt(0) {}
 
  Task(int _vwt) {
    taskwt = _vwt;
  } 

  ~Task() {}
	
  //===================================================================
  //  Other Methods
  //===================================================================

  void SetT(const Task& _t) {
    taskwt = _t.taskwt;
  } 

  ///Getting Data information
  int  GetWeight() const  {return taskwt;}

  ///Getting Data information
  int GetVertexWeight() {return taskwt;}
  
  ///Modify data
  int SetWeight(int _wt) {taskwt = _wt;return OK;}


  double GetWeight1()  {return (double)taskwt;}
  double GetWeight2(int& arg) {return (double)taskwt;}

  double GetWeightConst1() const {return (double)taskwt;}
  double GetWeightConst2(int& arg) const {return (double)taskwt;}

  int SetWeight1(int& _wt) {taskwt = _wt;return OK;}
  int SetWeight2(int& _wt,double& d) {taskwt = _wt;return OK;}
  int SetWeight3(int& _wt,double& d,long& a) {taskwt = _wt;return OK;}
  //===================================================================
  // Operators
  //===================================================================

  inline bool operator== (const Task &task) const {  
    return  (taskwt == task.taskwt);
  }

  inline Task& operator= (const Task &task) {
    taskwt = task.taskwt;
    return *this;
  }

};

//===================================================================
//===================================================================
class Weight2 {

  //===================================================================
  //  Data
  //===================================================================
 protected:
  double edgewt;

 public:
  void define_type(stapl::typer &t) {
    t.local(edgewt);
  }
  //===================================================================

  //===================================================================
  //  Constructors and Destructor
  //===================================================================
  Weight2() : edgewt(0) {}
 
  Weight2(double _ewt) {
    edgewt = _ewt;
  } 
  
  ~Weight2() {}

  //===================================================================
  //  Other Methods
  //===================================================================
  
  //Getting Data information
  
  double GetWeight() const {return edgewt;}
  double GetWeight(int x)  {return edgewt;}  
  double GetWeight() {return edgewt;}
  
  double GetWeight1() {return edgewt;}
  double GetWeight2(int& a) {return edgewt;}
  double GetWeightConst1() {return edgewt;}
  double GetWeightConst2(int& a) {return edgewt;}
  
  ///Modify data, for setweightfield?

  int SetWeight(int _wt){
    edgewt = _wt;
    return OK;
  }

  int SetWeight1(int& _wt){
    edgewt = _wt;
    return OK;
  }
  int SetWeight2(int& _wt, double& d){
    edgewt = _wt;
    return OK;
  }
  int SetWeight3(int& _wt,double& d, long& a){
    edgewt = _wt;
    return OK;
  }

  ///Modify data
  int SetWeight(double& _wt){
    edgewt = _wt;
    return OK;
  }

  static int MaxWeight(){
    return MAXWEIGHT;
  }

  //===================================================================
  // Operators
  //===================================================================

  inline bool operator== (const Weight2& weight) const {
    return  (edgewt == weight.edgewt);
  }

  inline Weight2& operator= (const Weight2& weight) {
    edgewt = weight.edgewt;
    return *this;
  }
};

//---------------------------------------------
// Input/Output operators for Task
//---------------------------------------------
inline istream& operator >> (istream& s, Task& t) {
  int wt;
  s >> wt;
  t.SetWeight(wt);
  return s;
}

inline ostream& operator << (ostream& s,const Task& t) {
  return s << t.GetWeight()<<" ";
}

//---------------------------------------------
// Input/Output operators for Weight2
//---------------------------------------------
inline istream& operator >> (istream& s, Weight2& w) { 
  int wt;
  s >> wt;
  w.SetWeight(wt);
  return s;
}

inline ostream& operator << (ostream& s, const Weight2& w) {
  return s << w.GetWeight();
}

#endif
