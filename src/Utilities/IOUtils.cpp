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

//discard all commented lines util the next uncommented line is found
void
GoToNext(istream& _is) {
  string line;
  while(!_is.eof()) {
    char c;
    while(isspace(_is.peek()))
      _is.get(c);

    c = _is.peek();
    if(!IsCommentLine(c))
      return;
    else
      getline(_is, line);
  }
}

//GetPathName from given filename
string
GetPathName(const string& _filename) {
  size_t pos = _filename.rfind('/');
  return pos == string::npos ? "" : _filename.substr(0, pos+1);
}

//read the string using above ReadField and tranform it to upper case
string ReadFieldString(istream& _is, string _error, bool _toUpper) {
  string s = ReadField<string>(_is, _error);
  if(_toUpper)
    transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

