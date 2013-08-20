#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <sstream>
#include <stdexcept>
using namespace std;

template<typename X, typename Y, typename Z>
string WhereAt(X x, Y y, Z z) {
  ostringstream oss;
  oss << "\n\tFile: " << x << "\n\tFunction: " << y << "\n\tLine: " << z;
  return oss.str();
};

#define WHERE WhereAt(__FILE__, __PRETTY_FUNCTION__, __LINE__)

class VizmoException : public runtime_error {
  public:
    VizmoException(const string& _type, const string& _where, const string& _message) :
      runtime_error("\nError:\n\t" + _type +
          "\nWhere:" + _where +
          "\nWhy:\n\t" + _message +
          "\n") {
      }
};

class ParseException : public VizmoException {
  public:
    ParseException(const string& _where, const string& _message) :
      VizmoException("ParseError", _where, _message) {}
};

class WriteException : public VizmoException {
  public:
    WriteException(const string& _where, const string& _message) :
      VizmoException("WriteError", _where, _message) {}
};

class BuildException : public VizmoException {
  public:
    BuildException(const string& _where, const string& _message) :
      VizmoException("BuildError", _where, _message) {}
};

class DrawException : public VizmoException {
  public:
    DrawException(const string& _where, const string& _message) :
      VizmoException("DrawError", _where, _message) {}
};

#endif
