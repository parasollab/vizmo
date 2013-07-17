#include "IOUtils.h"
#include <iostream>
#include <fstream>

//determine if a file exists or not
bool
FileExists(string _filename) {
  ifstream ifs(_filename.c_str());
  if(!ifs.good()) {
    cerr << "File (" << _filename << ") not found";
    return false;
  }
  return true;
}
