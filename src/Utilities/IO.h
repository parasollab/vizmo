#ifndef IO_H_
#define IO_H_

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
using namespace std;

#include "Utilities/IOUtils.h"

#include "Models/RegionModel.h"

#include "VizmoExceptions.h"
#include "Color.h"

//parse filename out of map header
string ParseMapHeader(const string& _filename);

//optionally read a color from a comment line
Color4 GetColorFromComment(istream& _is);

void VDAddRegion(const RegionModel* _region);
void VDRemoveRegion(const RegionModel* _region);
void AddInitialRegions();

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

#endif
