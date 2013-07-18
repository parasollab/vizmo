#include "IOUtils.h"
#include <iostream>
#include <fstream>

//determine if a file exists or not
bool
FileExists(string _filename, bool _err) {
  ifstream ifs(_filename.c_str());
  if(!ifs.good()) {
    if(_err) cerr << "File (" << _filename << ") not found";
    return false;
  }
  return true;
}
