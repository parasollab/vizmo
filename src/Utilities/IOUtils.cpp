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
string
ReadFieldString(istream& _is, const string& _where, const string& _error, bool _toUpper) {
  string s = ReadField<string>(_is, _where, _error);
  if(_toUpper)
    transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

//optionally read a color from a comment line
Color4
GetColorFromComment(istream& _is) {
  string line;
  while(!_is.eof()) {
    char c;
    while(isspace(_is.peek()))
      _is.get(c);

    c = _is.peek();
    if(!IsCommentLine(c))
      return Color4(0.5, 0.5, 0.5, 1);
    else{
      getline(_is, line);
      //colors begin with VIZMO_COLOR
      if(line[7] == 'C'){
        size_t loc = line.find(" ");
        string sub = line.substr(loc+1);
        istringstream iss(sub);
        float r, g, b;
        if(!(iss >> r >> g >> b))
          throw ParseException(WHERE, "Cannot parse MultiBody color");
        return Color4(r, g, b, 1);
      }
    }
  }
  return Color4();
}

