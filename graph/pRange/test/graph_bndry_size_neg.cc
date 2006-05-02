#include <stdlib.h>
#include <iostream>
#include <pGraph.h>
#include <pRange.h>
#include <runtime.h>

#include "stapl_bool.h"

using namespace stapl;
using namespace std;

/* Simple struct used in negative tests */
struct dumb {
  int x;
  int y;
};


void stapl_main(int argc, char **argv) {

  // Type definitions for the test
  typedef pGraph<PDG<DDGTask, DDGTask>,
                 PNMG<DDGTask, DDGTask>,
                 PWG<DDGTask, DDGTask>,
                 DDGTask, DDGTask> pgraph;
  typedef graph_boundary<pgraph>     boundary;
  typedef stapl::pRange<boundary, pgraph> prange;

  // Finding the thread id, number of threads, and the number of 
  // elements per thread assuming there are 64 elements total.
  int id = stapl::get_thread_id();
  int nThreads = stapl::get_num_threads();

  int num_local_elems = 64 / nThreads;
  if (id < 64 % nThreads) ++num_local_elems;

  // Initial output to show what test is running
  if (id == 0) {
    cout << "graph boundary size negative with " 
         << nThreads << " threads... ";
  }

  // Instantiate the pContainer 
  pgraph g;

  // Initialize the elements of the pContainer
  int local_start = num_local_elems * id;

  for (int i = local_start; i < local_start + num_local_elems; ++i) {
    DDGTask t(i);
    g.AddVertex(t, i);
  }

  rmi_fence();

  g.AddEdge( 0,  1); g.AddEdge( 1,  2); g.AddEdge( 2,  3); g.AddEdge( 3,  4);
  g.AddEdge( 4,  5); g.AddEdge( 5,  6); g.AddEdge( 6,  7);

  g.AddEdge( 8,  9); g.AddEdge( 9, 10); g.AddEdge(10, 11); g.AddEdge(11, 12);
  g.AddEdge(12, 13); g.AddEdge(13, 14); g.AddEdge(14, 15);

  g.AddEdge(16, 17); g.AddEdge(17, 18); g.AddEdge(18, 19); g.AddEdge(19, 20);
  g.AddEdge(20, 21); g.AddEdge(21, 22); g.AddEdge(22, 23);

  g.AddEdge(24, 25); g.AddEdge(25, 26); g.AddEdge(26, 27); g.AddEdge(27, 28);
  g.AddEdge(28, 29); g.AddEdge(29, 30); g.AddEdge(30, 31);

  g.AddEdge(32, 33); g.AddEdge(33, 34); g.AddEdge(34, 35); g.AddEdge(35, 36);
  g.AddEdge(36, 37); g.AddEdge(37, 38); g.AddEdge(38, 39);

  g.AddEdge(40, 41); g.AddEdge(41, 42); g.AddEdge(42, 43); g.AddEdge(43, 44);
  g.AddEdge(44, 45); g.AddEdge(45, 46); g.AddEdge(46, 47);

  g.AddEdge(48, 49); g.AddEdge(49, 50); g.AddEdge(50, 51); g.AddEdge(51, 52);
  g.AddEdge(52, 53); g.AddEdge(53, 54); g.AddEdge(54, 55);

  g.AddEdge(56, 57); g.AddEdge(57, 58); g.AddEdge(58, 59); g.AddEdge(59, 60);
  g.AddEdge(60, 61); g.AddEdge(61, 62); g.AddEdge(62, 63);

  g.AddEdge( 0,  8); g.AddEdge( 8, 16); g.AddEdge(16, 24); g.AddEdge(24, 32);
  g.AddEdge(32, 40); g.AddEdge(40, 48); g.AddEdge(48, 56);

  g.AddEdge( 1,  9); g.AddEdge( 9, 17); g.AddEdge(17, 25); g.AddEdge(25, 33);
  g.AddEdge(33, 41); g.AddEdge(41, 49); g.AddEdge(49, 57);

  g.AddEdge( 2, 10); g.AddEdge(10, 18); g.AddEdge(18, 26); g.AddEdge(26, 34);
  g.AddEdge(34, 42); g.AddEdge(42, 50); g.AddEdge(50, 58);

  g.AddEdge( 3, 11); g.AddEdge(11, 19); g.AddEdge(19, 27); g.AddEdge(27, 35);
  g.AddEdge(35, 43); g.AddEdge(43, 51); g.AddEdge(51, 59);

  g.AddEdge( 4, 12); g.AddEdge(12, 20); g.AddEdge(20, 28); g.AddEdge(28, 36);
  g.AddEdge(36, 44); g.AddEdge(44, 52); g.AddEdge(52, 60);

  g.AddEdge( 5, 13); g.AddEdge(13, 21); g.AddEdge(21, 29); g.AddEdge(29, 37);
  g.AddEdge(37, 45); g.AddEdge(45, 53); g.AddEdge(53, 61);

  g.AddEdge( 6, 14); g.AddEdge(14, 22); g.AddEdge(22, 30); g.AddEdge(30, 38);
  g.AddEdge(38, 46); g.AddEdge(46, 54); g.AddEdge(54, 62);

  g.AddEdge( 7, 15); g.AddEdge(15, 23); g.AddEdge(23, 31); g.AddEdge(31, 39);
  g.AddEdge(39, 47); g.AddEdge(47, 55); g.AddEdge(55, 63);


  // Instantiate one boundary per thread that contains all the local elements.
  // This is hard coded for the 1 and 4 thread cases.  The tests won't handle
  // any other number of threads.
  
  boundary bound;
  if (nThreads == 1) {
    hash_multimap<int, int, hash<int> > edge_list;
    bound = boundary(edge_list, &g, 0);
  } else if (nThreads == 4) {
    hash_multimap<int, int, hash<int> > edge_list;
    if (id == 0) {
      edge_list.insert(pair<int, int>( 8, 16)); 
      edge_list.insert(pair<int, int>( 9, 17)); 
      edge_list.insert(pair<int, int>(10, 18)); 
      edge_list.insert(pair<int, int>(11, 19)); 
      edge_list.insert(pair<int, int>(12, 20)); 
      edge_list.insert(pair<int, int>(13, 21)); 
      edge_list.insert(pair<int, int>(14, 22)); 
      edge_list.insert(pair<int, int>(15, 23)); 
    } else if (id == 1) {
      edge_list.insert(pair<int, int>(24, 32)); 
      edge_list.insert(pair<int, int>(25, 33)); 
      edge_list.insert(pair<int, int>(26, 34)); 
      edge_list.insert(pair<int, int>(27, 35)); 
      edge_list.insert(pair<int, int>(28, 36)); 
      edge_list.insert(pair<int, int>(29, 37)); 
      edge_list.insert(pair<int, int>(30, 38)); 
      edge_list.insert(pair<int, int>(31, 39)); 
      edge_list.insert(pair<int, int>(32, 40)); 
    } else if (id == 2) {
      edge_list.insert(pair<int, int>(40, 48)); 
      edge_list.insert(pair<int, int>(41, 49)); 
      edge_list.insert(pair<int, int>(42, 50)); 
      edge_list.insert(pair<int, int>(43, 51)); 
      edge_list.insert(pair<int, int>(44, 52)); 
      edge_list.insert(pair<int, int>(45, 53)); 
      edge_list.insert(pair<int, int>(46, 54)); 
      edge_list.insert(pair<int, int>(47, 55)); 
      edge_list.insert(pair<int, int>(48, 56)); 
    }
    bound = boundary(edge_list, &g, local_start);
  } else {
    stapl_assert(0,"Incorrect number of threads.  Test only valid for 1 or 4.");
  }


  // Test 1: assigne the return value to a non-integer variable.
  dumb x = bound.size();


  // Report whether or not the test passed
  stapl_bool test_result; //insert test for correctness as constructor argument
  test_result.reduce();

  if (id == 0) {
    if (test_result.value())
      cout << "Passed\n";
    else
      cout << "Failed\n";
  }
}
