
//===================================================================
//===================================================================
class Task {
public:

//===================================================================
//  Data
//===================================================================

int taskwt;

//===================================================================

//===================================================================
//  Constructors and Destructor
//===================================================================
        Task() {};
 
		
        Task(int _vwt) {
        	taskwt = _vwt;
       	};  
        ~Task() {};

//===================================================================
// Operators
//===================================================================

   inline bool operator== (const Task &task) const {  
     return  (taskwt == task.taskwt);
   };

   inline Task& operator= (const Task &task) {
     taskwt = task.taskwt;
     return *this;
   };

//===================================================================
//  Other Methods
//===================================================================

        //Getting Data information

	int GetTaskWeight() {return taskwt;}; 

	//Modify data
	void SetTaskWeight(int _wt) {taskwt = _wt;};

	static Task InvalidData() {
		Task c(-1);
		return c;
	};

protected:
private:
};
//===================================================================
//===================================================================
class Weight2 {
public:

//===================================================================
//  Data
//===================================================================

double edgewt;

//===================================================================

//===================================================================
//  Constructors and Destructor
//===================================================================
        Weight2() {};
 
		
        Weight2(double _ewt) {
        	edgewt = _ewt;
       	};  
        ~Weight2() {};

//===================================================================
// Operators
//===================================================================

   inline bool operator== (const Weight2 &weight)
                {  return  (edgewt == weight.edgewt);
};

   inline Weight2& operator= (const Weight2 &weight)
                {       edgewt = weight.edgewt;
                return *this;
};

//===================================================================
//  Other Methods
//===================================================================

        //Getting Data information

   double Weight() {return edgewt;}; 

	//Modify data
	void SetEdgeWeight(double _wt) {edgewt = _wt;};

	static Weight2 InvalidWeight() {
		Weight2 c(-1);
		return c;
	};
	static int MaxWeight(){
	  return 10000000;
	}
protected:
private:
};

//---------------------------------------------
// Input/Output operators for Task
//---------------------------------------------
  inline istream& operator >> (istream& s, Task& t) { 
     s >> t.taskwt;
	return s;
  };

  inline ostream& operator << (ostream& s, const Task& t) {
  return s << t.taskwt;
  };

//---------------------------------------------
// Input/Output operators for Weight
//---------------------------------------------
  inline istream& operator >> (istream& s, Weight2& w) { 
    s >> w.edgewt;
  	return s;
  };

  inline ostream& operator << (ostream& s, const Weight2& w) {
    return s << w.edgewt;
  };

