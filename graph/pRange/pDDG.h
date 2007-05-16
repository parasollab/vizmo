#ifndef P_DDG_H
#define P_DDG_H

/*!
  \file pDDG.h
  \author Timmie Smith
  \date 11/20/02
  \brief This file continas the DDG class.

  pDDG is used by pRange to store subrange dependence information.  It is 
  deived from pGraph, and uses pGraph algorithms wherver possible to perform
  the desired task.

  The pGraph requires that the items stored in the vertices and edges are actual classes and not fundamental data types simple vertex and weight classes are provided that basically wrap an integer with the correct interfaces.
*/

#include <utility>
#include <vector>
#include <algorithm>

#include <runtime.h>
#include <Graph.h>
#include <pGraph.h>

using std::vector;
using std::pair;
using std::make_pair;

namespace stapl {

#ifndef MAXWEIGHT
#define MAXWEIGHT 1000000
#endif

/**
 * @addtogroup pDDG
 *
 * @{
 */

/**
 * DDGTask is a simple class that encapsultes an integer.  The class can be
 * used as the data type stored in the vertex of the pDDG if more complex 
 * data isn't needed in the vertex.
 */
class DDGTask {
protected:

  /**
   * \brief global id of the subrange this vertex represents in the pDDG.
   */
  int taskid;
 
public:

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \param t typer object used by ARMI to automatically pack the class
   */
  void define_type(stapl::typer& t){
    t.local(taskid);
  }

  /**
   * \brief Default constructor
   */
  DDGTask() : taskid(0) {}

  /**
   * \brief Constructor to initialize task with user data.
   * \param _vwt global id of the subrange the vertex represents
   */
  DDGTask(int id) {
    taskid = id;
  } 

  /**
   * \brief Default destructor
   */
  ~DDGTask() {}
 
  /**
   * \brief Get the global id of the subrange.
   * \return the global id stored inside the DDGTask
   */
  int GetId() const {return taskid;} 


  /**
   * \brief Set the global id of the subrange.
   * \param _wt the global id of the subrange this vertex represents
   */
  int SetId(int id) {
    taskid = id; 
    return OK;
  }

  /**
   * Equality operator
   */
  inline bool operator== (const DDGTask& task) const {  
    return  (taskid == task.taskid);
  }

  /**
   * Assignment operator
   */
  inline DDGTask& operator= (const DDGTask& task) {
    taskid = task.taskid;
    return *this;
  }
};


/**
 * DDGWeight is a simple class that encapsultes an integer.  The class can 
 * be used as the data type stored in the vertex of the pDDG if more 
 * complex data isn't needed in the vertex.
 */
class DDGWeight {
protected:

  /**
   * \brief weight stored on the edge of the pDDG.
   */
  double edgewt;

public:

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \param t typer object used by ARMI to automatically pack the class
   */
  void define_type(stapl::typer& t) {
    t.local(edgewt);
  }

  /**
   * \brief Default constructor
   */
  DDGWeight() : edgewt(0.0) {}

  /**
   * \brief Constructor to initialize task with user data.
   * \param wt Weight of the edge in the graph
   */
  DDGWeight(double wt) {
    edgewt = wt;
  } 

  /**
   * \brief Constructor to initialize task with user data. Provided 
   * because the pGraph requires that the class used to represent the
   * weight have a constructor that accepts an integer argument.
   * \param wt Weight of the edge in the graph
   */
  DDGWeight(int wt) {
    edgewt = wt;
  } 

  /**
   * \brief Default constructor
   */
  ~DDGWeight() {}

  /**
   * \brief Get the weight stored on the edge.
   */
  double GetWeight() const { return edgewt; }

  /**
   * \brief Set the weight stored on the edge.
   * \param wt the value to assign to the edge weight.
   */
  void SetWeight(double wt){
    edgewt = wt;
  }

  /**
   * \brief Equality operator
   */
  inline bool operator== (const DDGWeight& weight) const {
    return  (edgewt == weight.edgewt);
  }

  /**
   * \brief Assignment operator
   * \param weight DDGWeight used to initialize the weight
   */
  inline DDGWeight& operator= (const DDGWeight& weight) {
    edgewt = weight.edgewt;
    return *this;
  }
};

/**
 * \brief Input operator for the DDGTask
 */
inline istream& operator >> (istream& s, DDGTask& t) {
  int id;
  s >> id;
  t.SetId(id);
  return s;
}

/**
 * \brief Output operator for the DDGTask
 */
inline ostream& operator << (ostream& s, const DDGTask& t) {
  return s << t.GetId();
}

/**
 * \brief Input operator for the DDGWeight
 */
inline istream& operator >> (istream& s, DDGWeight& w) { 
  double wt;
  s >> wt;
  w.SetWeight(wt);
  return s;
}

/**
 * \brief Output operator for the DDGWeight
 */
inline ostream& operator << (ostream& s, const DDGWeight& w) {
  return s << w.GetWeight();
}


/**
 * Base DDG from which all DDG classes should be derived.
 */
class BaseDDG {
public:
  virtual void add_task(const int) = 0;
  virtual void remove_task(const int) = 0;
  virtual void add_dependence(const int, const int) = 0;
  virtual bool remove_dependence(const int, const int) = 0;
  virtual void reset() = 0;
  virtual bool finished() = 0;
  virtual std::vector<int> get_ready() = 0;
  virtual void processed(const int) = 0;
/*
  virtual bool critical(const int) = 0;

  template <class WorkFunction>
  int has_priority(const std::vector<int>&, WorkFunction&) = 0;
*/
};


/**
 * DDG is a sequential DDG class that is derived from the STAPL Graph.  
 * The class is also deived from BaseDDG to ensure the necessary methods 
 * are implemented.
 */
template <class vertex_data, class edge_data>
class DDG : public Graph<DG<vertex_data, edge_data>, 
                         NMG<vertex_data, edge_data>, 
                         WG<vertex_data, edge_data>, 
                         vertex_data, edge_data>, 
            public BaseDDG {
private:

  /**
   * \brief The number of predecessors of every vertex in the DDG.  
   *
   * It is stored as a vector of pairs that stores a vertex id and the 
   * number of predecessors for that vertex.
   */
  map<int,int> base_num_predecessors;

  /** 
   * \brief The number of predecessors that must be processed before each 
   * vertex can be processed.  
   *
   * This vector is used while the DDG is being travesed.  It is 
   * initialized by base_num_predecessors.
   */
  map<int,int> predecessors_left;

  /** 
   * \brief The vector is a list of tasks that have all incoming 
   * dependencies satisfied.
   */
  vector<int> ready_tasks;

  /**
   * \brief The number of vertices in the local portion of the DDG that 
   * have been processed.
   */
  int processed_count;

public:
  /**
   * \brief Type of the Graph from which the DDG is derived.
   */
  typedef Graph<DG<vertex_data, edge_data>, NMG<vertex_data, edge_data>,
                WG<vertex_data, edge_data>, vertex_data, edge_data>   GraphType;

  /**
   * \brief the type of the DDG.
   */
  typedef DDG<vertex_data, edge_data> DDGType;

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \param t typer object used by ARMI to automatically pack the class
   */
  void define_type(stapl::typer& t) {
    stapl_assert(0,"ERROR: DDG define_type used.\n");
  }

  /**
   * Default constructor
   */
  DDG() 
    : GraphType(), base_num_predecessors(), predecessors_left(),
      ready_tasks(), processed_count(0)
  { }

  /**
   * \brief Construct a DDG from a Graph.
   *
   * The constructor registers the object with ARMI after copying the 
   * values of all the data members from the Graph passed in to the 
   * constructor except the handle.
   *
   * \param g Graph used to initialize the DDG
   */
  DDG(const GraphType& g) 
    : GraphType(g), base_num_predecessors(), predecessors_left(),
      ready_tasks(), processed_count(0)
  { }

  /**
   * \brief Assignment operator
   *
   * The data members defined in the class are copied after the Graph data
   * members are copied.
   *
   * \param g DDG used to initialize the members of the DDG.
   */
  inline DDGType& operator=(const DDGType& g) {
    GraphType::operator=(g);
    base_num_predecessors = g.base_num_predecessors;
    predecessors_left = g.predecessors_left;
    ready_tasks = g.ready_tasks;
    processed_count = g.processed_count;
  }

  /**
   * Default destructor
   */
  ~DDG() { }

  /**
   * Equality operator
   */
  inline bool operator== (const DDGType& g) const {  
    return  ((base_num_predecessors == g.base_num_predecessors) &&
             (predecessors_left == g.predecessors_left) &&
             (ready_tasks == g.ready_tasks) &&
             (processed_count == g.processed_count));
  }

  /**
   * \brief Add a task to the DDG.
   *
   * \param vid The global id of the task to add to the DDG.
   */
  void add_task(int vid) {
    vertex_data temp(vid);
    this->AddVertex(temp, vid);
    base_num_predecessors[vid] = 0;
    predecessors_left[vid] = 0;
    ready_tasks.push_back(vid);
  }

  /**
   * \brief Remove a task from the DDG.
   *
   * \param vid The global id of the task to remove from the DDG.
   */
  inline void _erase_pred_entries(int _v) {
    base_num_predecessors.erase(_v);
    predecessors_left.erase(_v);
  }

  void remove_task(int vid) {
    this->DeleteVertex(vid);
    _erase_pred_entries(vid);
  }

  /**
   * \brief Remove all edges from the DDG.
   */
  void clear();

  /**
   * \brief Add edges to the DDG that cause a sequential traversal of the 
   * DDG.
   *
   * \warning This method is not implemented yet.
   */
  void setSequential();

  /**
   * \brief Add edges to the DDG that cause a sequential traversal of the 
   * DDG.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info Graph with connectivity information used to 
   * setup a sequential traversal of the DDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setSequential(int vid, GraphType& connectivity_info);

  /**
   * \brief Add edges to the DDG that would cause the vertices to be 
   * traversed in a breadth-first sequence.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info Graph with connectivity information used to 
   * setup a sequential traversal of the DDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setBFS(int vid, GraphType& connectivity_info);


  /**
   * \brief Add edges to the DDG that would cause the vertices to be 
   * traversed in a depth-first sequence.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info Graph with connectivity information used to 
   * setup a sequential traversal of the DDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setDFS(int vid, GraphType& connectivity_info);

  /**
   * \brief Add a directed edge between two vertices to mark a dependence 
   * between them.
   *
   * \param v1id The id of the vertex that must be processed before the 
   * second vertex.
   * \param v2id The id of the vertex that is processed after the first 
   * vertex in the dependence.
   * \return bool indicating whether the edge was successfully added to the
   * DDG.
   */
  void add_dependence(int v1id, int v2id) { 
    int success = this->AddEdge(v1id, v2id, edge_data()); 
    if (success == OK) {
      ++base_num_predecessors[v2id];
      ++predecessors_left[v2id];
    } else {
#ifdef STAPL_DEBUG
      cerr << "Edge not added to pDDG!\n";
#endif
    }
  }

  /**
   * \brief Remove a dependence from the DDG.
   *
   * \param v1id The id of the vertex that must be processed before the 
   * second vertex.
   * \param v2id The id of the vertex that is processed after the first 
   * vertex in the dependence.
   * \return bool indicating whether the edge was successfully removed from
   * the DDG.
   */
  bool remove_dependence(int v1id, int v2id) { 
    this->DeleteEdge(v1id, v2id); 
    --base_num_predecessors[v2id];
    if (predecessors_left[v2id] > 0)
      --predecessors_left[v2id];
    if (predecessors_left[v2id] == 0)
      ready_tasks.push_back(v2id);
  }

  /**
   * \brief Get the user data stored in a vertex of the DDG.
   *
   * \param vid global id of the vertex that contains the desired data.
   * \return The user data stored within the specified vertex.
   */
  vertex_data& getData(int vid) {
    typename GraphType::VI data;
    if(IsVertex(vid, &data) == false) {
      cerr << "No data has local id " << vid << endl;
      exit(1);
    } else {
      return data->data;
    }
  }

  /**
   * \brief Calculates the base number of predecessors of each local 
   * vertex. 
   *
   * Calls to rmi_fence() are required because async_rmi() calls are used 
   * to obtain values from other threads.
   */
  void initialize_base_num_predecessors() {
    base_num_predecessors.clear();
    for (typename GraphType::VI vi=this->begin();  vi!=this->end(); ++vi) {
      base_num_predecessors[vi->vid] = 0;
    }

    for (typename GraphType::VI vi=this->begin();  vi!=this->end(); ++vi) {
      for (typename GraphType::EI ei = vi->edgelist.begin(); 
                                  ei != vi->edgelist.end();   
                                ++ei) {
        map<int,int>::iterator i = base_num_predecessors.find(ei->vertex2id);
        if (i != base_num_predecessors.end())
          (*i).second += 1;
      }
    }
  }

  /** 
   * \brief Use the values from base_num_predecessors to initialize the 
   * number of dependencies yet to be satisfied for each vertex. 
   *
   * \note initialize_base_num_predecessors() must be called before this 
   * function.
   */
  void initialize_predecessors_left()
  {
    predecessors_left.clear();
    for (map<int,int>::iterator i = base_num_predecessors.begin();
                                i != base_num_predecessors.end();
                              ++i) {
      predecessors_left[(*i).first] = (*i).second;
      if ((*i).second == 0)
        ready_tasks.push_back((*i).first);
    }
  }

  /**
   * \brief reset the DDG so it can be reused in another computation.
   *
   * The number of predecessors processed for each vertex is reset, the 
   * ready queue is cleard, and the count of processed vertices is reset to
   * 0.
   */
  inline void reset() { 
    ready_tasks.clear();
    initialize_predecessors_left();
    processed_count = 0;
  }

  /**
   * \brief Get the vertex ids of the vertices whose predecessors have been
   * processed.
   * \return A vector containing the global ids of the vertices that are 
   * ready to process.
   */
  inline std::vector<int> get_ready() { 
    std::vector<int> ready = ready_tasks;
    ready_tasks.clear();
    return ready;
  }

  /**
   * \brief mark a vertex as having been processed.
   *
   * The predecessor counf of each of the vertex's successors is 
   * decremented and the number of processed vertices is incremented.
   *
   * \param id the global id of the vertex to mark as processed.
   */
  inline void processed(const int id) {
    decrement_successors(id);
    ++processed_count;
  }

  /**
   * \brief Check if the DDG has been completely processed.
   *
   * \return bool indicating whether the DDG has been processed.
   */
  inline bool finished() {
    return (processed_count == this->size());
  }

  /**  
   * \brief Get the number of unprocessed predecessors for a given vertex.
   *
   * \param _v The global id of the vertex whose predecessors will be 
   * checked. 
   * \return the number of predecessors yet to be satisfied for a vertex. 
   */
  int get_predecessors_left(int _v) {  
    map<int,int>::iterator it = predecessors_left.find(_v);
    if (it != predecessors_left.end())
      return (*it).second;
    else
      return -1;
  }


  /** 
   * \brief Decrement the predecessors_left value for all vertices in the 
   * adjancecy list of the vertex with the global id passed to the 
   * function.
   *
   * \param v The global id of the vertex that has been processed.
   */
  void decrement_successors(int v) {
    typename GraphType::VI vi;
    if (GraphType::IsVertex(v,&vi)) {
      for(typename GraphType::EI ei = vi->edgelist.begin();   
                                 ei!=vi->edgelist.end();   
                               ++ei) {
        map<int,int>::iterator i = predecessors_left.find(ei->vertex2id);
        if ( i != predecessors_left.end()) {
          --((*i).second);
          if ( 0 == (*i).second) {
            ready_tasks.push_back((*i).first);
          }
        }
      }
    }
  }

};


/**
 * pDDG is a distributed DDG class that is derived from the STAPL pGraph.  
 * The class is also deived from DDG to ensure the necessary methods are 
 * implemented.
 */
template <class vertex_data, class edge_data>
class pDDG : public pGraph<PDG<vertex_data, edge_data>, 
                           PMG<vertex_data, edge_data>, 
                           PWG<vertex_data, edge_data>, 
                           vertex_data, edge_data>, 
             public BaseDDG {
private:
  /**
   * \brief Handle used by RMI communication calls.
   *
   * This object is instantiated in the constructors by calls to 
   * register_rmi_object().
   */
  stapl::rmiHandle pDDG_Handle;


  /**
   * \brief The number of predecessors of every vertex in the pDDG.  
   *
   * It is stored as a vector of pairs that stores a vertex id and the 
   * number of predecessors for that vertex.
   */
  map<int,int> base_num_predecessors;

  /** 
   * \brief The number of predecessors that must be processed before each 
   * vertex can be processed.  
   *
   * This vector is used while the pDDG is being travesed.  It is 
   * initialized by base_num_predecessors.
   */
  map<int,int> predecessors_left;

  /** 
   * \brief The vector is a list of tasks that have all incoming 
   * dependencies satisfied.
   */
  vector<int> ready_tasks;

  /**
   * \brief The number of vertices in the local portion of the pDDG that 
   * have been processed.
   */
  int processed_count;

public:
  /**
   * \brief Type of the pGraph from which the pDDG is derived.
   */
  typedef pGraph<PDG<vertex_data, edge_data>, PMG<vertex_data, edge_data>,
                 PWG<vertex_data, edge_data>, vertex_data, edge_data>     GraphType;

  /**
   * \brief the type of the pDDG.
   */
  typedef pDDG< vertex_data, edge_data> DDGType;

  /**
   * \brief Function required in order to use STAPL communication 
   * primitives.
   *
   * \param t typer object used by ARMI to automatically pack the class
   */
  void define_type(stapl::typer& t) {
    stapl_assert(0,"ERROR: pDDG define_type used.\n");
  }

  /**
   * Default constructor
   *
   * \param p pointer to the parent pRange the pDDG is defined on.  If 
   * there is no parent pRange then the pDDG is defined on the root pRange,
   * and is registered with ARMI to allow the distributed parts of the pDDG
   * to communicate.
   */
  pDDG(void *p = NULL) 
    : GraphType(), base_num_predecessors(), predecessors_left(),
      ready_tasks(), processed_count(0), pDDG_Handle(-1)
  {
    if (p == NULL)
      pDDG_Handle = stapl::register_rmi_object(this);
  }

  /**
   * \brief Construct a pDDG from a pGraph.
   *
   * The constructor registers the object with ARMI after copying the 
   * values of all the data members from the pGraph passed in to the 
   * constructor except the handle.
   *
   * \param g pGraph used to initialize the pDDG
   */
  pDDG(const GraphType& g) 
    : GraphType(g), base_num_predecessors(), predecessors_left(),
      ready_tasks(), processed_count(0), pDDG_Handle(-1)
  { 
    pDDG_Handle = stapl::register_rmi_object(this);
  }

  /**
   * \brief Copy constructor
   *
   * The constructor uses all values from the pDDG passed in except the 
   * rmiHandle.
   *
   * \param g pDDG used to initialize the members of the pDDG.
   */
  pDDG(const DDGType& g) : GraphType::Base(g) {
    base_num_predecessors = g.base_num_predecessors;
    predecessors_left = g.predecessors_left;
    ready_tasks = g.ready_tasks;
    processed_count = g.processed_count;
    pDDG_Handle = stapl::register_rmi_object(this);
  }

  /**
   * \brief Assignment operator
   *
   * The data members defined in the class, with the exception of the 
   * rmiHandle, are copied after the pGraph data members are copied.  The 
   * rmiHandle is not copied since the pDDG is already registered with 
   * ARMI.
   *
   * \param g pDDG used to initialize the members of the pDDG.
   */
  inline DDGType& operator=(const DDGType& g) {
    GraphType::operator=(g);
    base_num_predecessors = g.base_num_predecessors;
    predecessors_left = g.predecessors_left;
    ready_tasks = g.ready_tasks;
    processed_count = g.processed_count;
  }

  /**
   * Default destructor
   *
   * The destructor unregisters the pDDG if it was registered with ARMI.
   */
  ~pDDG() {
    if (pDDG_Handle != -1)
      unregister_rmi_object(pDDG_Handle);
  }

  /**
   * Equality operator
   */
  inline bool operator== (const DDGType& g) const {  
    return  ((base_num_predecessors == g.base_num_predecessors) &&
             (predecessors_left == g.predecessors_left) &&
             (ready_tasks == g.ready_tasks) &&
             (processed_count == g.processed_count));
  }

  /**
   * \brief Add a task to the DDG.
   *
   * \param vid The global id of the task to add to the DDG.
   */
  void add_task(int vid) {
    vertex_data temp(vid);
    this->AddVertex(temp, vid);
    base_num_predecessors[vid] = 0;
    predecessors_left[vid] = 0;
    ready_tasks.push_back(vid);
  }

  /**
   * \brief Remove a task from the DDG.
   *
   * \param vid The global id of the task to remove from the DDG.
   */
  inline void _erase_pred_entries(int _v) {
    base_num_predecessors.erase(_v);
    predecessors_left.erase(_v);
  }

  void remove_task(int vid) {
    Location l = this->dist.Lookup(vid);
    this->DeleteVertex(vid);
    if (l.pid == stapl::get_thread_id()) {
      _erase_pred_entries(vid);
    } else if (l.ValidLocation()) {
      async_rmi(l.pid, pDDG_Handle, &pDDG::_erase_pred_entries, vid);
    }
  }

  /**
   * \brief Remove all edges from the pDDG.
   */
  void clear();

  /**
   * \brief Add edges to the pDDG that cause a sequential traversal of the 
   * pDDG.
   *
   * \warning This method is not implemented yet.
   */
  void setSequential();

  /**
   * \brief Add edges to the pDDG that cause a sequential traversal of the 
   * pDDG.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info pGraph with connectivity information used to 
   * setup a sequential traversal of the pDDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setSequential(int vid, GraphType& connectivity_info);

  /**
   * \brief Add edges to the pDDG that would cause the vertices to be 
   * traversed in a breadth-first sequence.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info pGraph with connectivity information used to 
   * setup a sequential traversal of the pDDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setBFS(int vid, GraphType& connectivity_info);


  /**
   * \brief Add edges to the pDDG that would cause the vertices to be 
   * traversed in a depth-first sequence.
   *
   * \param vid The global id of the vertex that should be the first vertex
   * processed in the sequential traversal.
   * \param connectivity_info pGraph with connectivity information used to 
   * setup a sequential traversal of the pDDG vertices
   *
   * \warning This method is not implemented yet.
   */
  void setDFS(int vid, GraphType& connectivity_info);

  /**
   * \brief Add a directed edge between two vertices to mark a dependence 
   * between them.
   *
   * \param v1id The id of the vertex that must be processed before the 
   * second vertex.
   * \param v2id The id of the vertex that is processed after the first 
   * vertex in the dependence.
   * \return bool indicating whether the edge was successfully added to the
   * pDDG.
   */
  inline void _increment_pred_entries(int _v) {
    map<int,int>::iterator i = base_num_predecessors.find(_v);
    if (i != base_num_predecessors.end())
      (*i).second += 1;

    map<int,int>::iterator j = predecessors_left.find(_v);
    if (j != predecessors_left.end())
      (*j).second += 1;
  }

  inline void add_dependence(int v1id, int v2id) { 
    Location l = this->dist.Lookup(v2id);
    int success = this->AddEdge(v1id, v2id, edge_data());
    if (success == OK) {
      if (l.pid == stapl::get_thread_id()) {
        _increment_pred_entries(v2id);  
      } else {  
        async_rmi(l.pid,pDDG_Handle,&pDDG::_increment_pred_entries,v2id);  
      }
    } else {
#ifdef STAPL_DEBUG
      cerr << "Edge not added to pDDG!\n";
#endif
    }
  }

  /**
   * \brief Remove a dependence from the pDDG.
   *
   * \param v1id The id of the vertex that must be processed before the 
   * second vertex.
   * \param v2id The id of the vertex that is processed after the first 
   * vertex in the dependence.
   * \return bool indicating whether the edge was successfully removed from
   * the pDDG.
   */
  inline void _decrement_pred_entries(int _v) {
    map<int,int>::iterator i = base_num_predecessors.find(_v);
    if (i != base_num_predecessors.end())
      (*i).second -= 1;

    map<int,int>::iterator j = predecessors_left.find(_v);
    if (j != predecessors_left.end())
      (*j).second -= 1;
    if (j->second <= 0)
      ready_tasks.push_back(j->first);
  }

  inline bool remove_dependence(int v1id, int v2id) { 
    Location l = this->dist.Lookup(v2id);
    int success = this->DeleteEdge(v1id, v2id);
    if (success == OK) {
      if (l.pid == stapl::get_thread_id()) {
        _decrement_pred_entries(v2id);  
      } else {  
        async_rmi(l.pid,pDDG_Handle,&pDDG::_decrement_pred_entries,v2id);  
      }
    }
  }

  /**
   * \brief Get the user data stored in a vertex of the pDDG.
   *
   * \param vid global id of the vertex that contains the desired data.
   * \return The user data stored within the specified vertex.
   */
  inline vertex_data& getData(int vid) {
    typename GraphType::VI data;
    if(IsVertex(vid, &data) == false) {
      cerr << "No data has local id " << vid << endl;
      exit(1);
    } else {
      return data->data;
    }
  }

  /** 
   * \brief Internal function used by  initialize_base_num_predecessors()
   *
   * The method is invoked on remoted threads through calls to async_rmi if
   * the desired vetex isn't local to the thread.
   */
  inline void _increment_base_num_predecessors(int _v) {
    map<int,int>::iterator i = base_num_predecessors.find(_v);
    if (i != base_num_predecessors.end())
      (*i).second += 1;
  }

  /**
   * \brief Calculates the base number of predecessors of each local 
   * vertex. 
   *
   * Calls to rmi_fence() are required because async_rmi() calls are used 
   * to obtain values from other threads.
   */
  void initialize_base_num_predecessors() {
    base_num_predecessors.clear();
    for (typename GraphType::VI vi=this->begin();  vi!=this->end(); ++vi)
      base_num_predecessors[vi->vid] = 0;
    stapl::rmi_fence();

    for (typename GraphType::VI vi=this->begin();  vi!=this->end(); ++vi) {
      for (typename GraphType::EI ei = vi->edgelist.begin(); 
                                   ei != vi->edgelist.end();   
                                 ++ei) {
        Location l = this->dist.Lookup(ei->vertex2id);
        if (l.pid == stapl::get_thread_id()) {
          _increment_base_num_predecessors(ei->vertex2id);  
        }
        else {  
          stapl::async_rmi(l.pid,pDDG_Handle,&pDDG::_increment_base_num_predecessors,ei->vertex2id);  
        }
      }
    }
    stapl::rmi_fence();
  }

  /** 
   * \brief Use the values from base_num_predecessors to initialize the 
   * number of dependencies yet to be satisfied for each vertex. 
   *
   * \note initialize_base_num_predecessors() must be called before this 
   * function.
   */
  void initialize_predecessors_left()
  {
    predecessors_left.clear();
    for (map<int,int>::iterator i = base_num_predecessors.begin();
                                i != base_num_predecessors.end();
                              ++i) {
      predecessors_left[(*i).first] = (*i).second;
      if ((*i).second == 0) {
        ready_tasks.push_back((*i).first);
      }
    }
  }

  /**
   * \brief reset the pDDG so it can be reused in another computation.
   *
   * The number of predecessors processed for each vertex is reset, the 
   * ready queue is cleard, and the count of processed vertices is reset to
   * 0.
   */
  inline void reset() { 
    ready_tasks.clear();
    initialize_predecessors_left();
    processed_count = 0;
  }

  /**
   * \brief Get the vertex ids of teh vertices whose predecessors have been
   * processed.
   * \return A vector containing the global ids of the vertices that are 
   * ready to process.
   */
  inline std::vector<int> get_ready() { 
    std::vector<int> ready = ready_tasks;
    ready_tasks.clear();
    return ready;
  }

  /**
   * \brief mark a vertex as having been processed.
   *
   * The predecessor counf of each of the vertex's successors is 
   * decremented and the number of processed vertices is incremented.
   *
   * \param id the global id of the vertex to mark as processed.
   */
  inline void processed(const int id) {
    decrement_successors(id);
    ++processed_count;
  }

  /**
   * \brief Check if the local part of the pDDG has been completely 
   * processed.
   *
   * This indicates only the local completion.  It should be used as part 
   * of a larger computation to check the global completion of the pDDG.
   *
   * \return bool indicating whether the local part has been processed.
   */
  inline bool finished() {
    return (processed_count == this->local_size());
  }

  /**  
   * \brief Get the number of unprocessed predecessors for a given vertex.
   *
   * \param _v The global id of the vertex whose predecessors will be 
   * checked. 
   * \return the number of predecessors yet to be satisfied for a vertex. 
   */
  int get_predecessors_left(int _v) {  
    Location l = this->dist.Lookup(_v);
    
    if (l.pid == stapl::get_thread_id())
      return _get_predecessors_left(_v);
    else
      return stapl::sync_rmi(l.pid,pDDG_Handle,&pDDG::_get_predecessors_left,_v);
  }

  /** 
   * \brief Internal function used by get_predecessors_left().
   *
   * The method is invoked on remoted threads through calls to sync_rmi if 
   * the desired vetex isn't local to the thread.
   */
  inline int _get_predecessors_left(int _v) {
    map<int,int>::iterator it = predecessors_left.find(_v);
    if(it != predecessors_left.end())
      return (*it).second;
    else
      return -1;
  }


  /** 
   * \brief Decrement the predecessors_left value for all vertices in the 
   * adjancecy list of the vertex with the global id passed to the 
   * function.
   *
   * \param v The global id of the vertex that has been processed.
   */
  void decrement_successors(int v) {
    Location loc = this->dist.Lookup(v);

    if (loc.pid == stapl::get_thread_id()) {
      typename GraphType::VI vi;
      if (GraphType::IsVertex(v,&vi)) {
        for(typename GraphType::EI ei = vi->edgelist.begin();   
                                    ei!=vi->edgelist.end();   
                                  ++ei) {
          Location l = this->dist.Lookup(ei->vertex2id);
          if (l.pid == stapl::get_thread_id()) {
            _decrement_num_predecessors(ei->vertex2id);
          } else {
            stapl::async_rmi(l.pid,pDDG_Handle,&pDDG::_decrement_num_predecessors,ei->vertex2id);
          }
      
        }
      }
    } else {
      stapl::async_rmi(loc.pid,pDDG_Handle,&pDDG::decrement_successors,v);
    }

  }

  /** 
   * \brief Internal function used by decrement_successors
   *
   * The method is invoked on remoted threads through calls to async_rmi if
   * the desired vetex isn't local to the thread.
   */
  inline void _decrement_num_predecessors(int _v) {
    map<int,int>::iterator it = predecessors_left.find(_v);
    if (it != predecessors_left.end()) {
      --((*it).second);
      if (0 == (*it).second)
        ready_tasks.push_back((*it).first);
    }
  }

  //Note: off processor dependencies will store the pair (pid, vid) on the local  processor to allow easy access to info needed for RMI.

  //Question: Should the subranges be stored here still?
  //A vector of subranges allows DDGs to be easily constructed by the user 
  //without having to get the subranges and add them into the DDG.

  //Something we could do is when the user calls setDDG and passes the DDG 
  //to the pRange the subranges are moved from the vector into the DDG.
};


/**
 * @}
 */

}
#endif
