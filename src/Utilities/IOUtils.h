#ifndef IOUTILS_H_
#define IOUTILS_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Exceptions.h"
#include "Color.h"

//determine if a file exists or not
bool FileExists(string _filename, bool _err = true);

//discard all commented lines util the next uncommented line is found
void GoToNext(istream& _is);

//is given char a start of command line? comments denoted with # character
inline bool IsCommentLine(char _c) {return _c == '#';}

//GetPathName from given filename
string GetPathName(const string& _filename);

//read type by eating all white space. If type cannot be read report the error
//provided
template <class T>
T
ReadField(istream& _is, const string& _where, const string& _error) {
  char c;
  string line;
  T element = T();
  while(_is) {
    c = _is.peek();
    if(c == '#') {
      getline(_is, line);
    }
    else if(!isspace(c)) {
      if (!(_is >> element))
        throw ParseException(_where, _error);
      else
        break;
    }
    else
      _is.get(c);
  }
  if(_is.eof())
    throw ParseException(_where, "End of file reached");

  return element;
};

//read the string using above ReadField and tranform it to upper case
string ReadFieldString(istream& _is, const string& _where, const string& _error, bool _toUpper = true);

//optionally read a color from a comment line
Color4 GetColorFromComment(istream& _is);

#endif
