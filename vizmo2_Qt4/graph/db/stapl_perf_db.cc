#include "db/stapl_perf_db.h"
void stapl_main(int argc, char **argv)
{
  stapl::stapl_perf_db("test");
  cout << "Hello world" << endl;
  sleep(10);
  cout << "Goodbye" << endl;
}

// EOF - stapl_perf_db.cc
