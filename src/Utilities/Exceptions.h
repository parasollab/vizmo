#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <sstream>
#include <exception>
using namespace std;

template<typename X, typename Y, typename Z>
string WhereAt(X x, Y y, Z z) {
  ostringstream oss;
  oss << "\n\tFile: " << x << "\n\tFunction: " << y << "\n\tLine: " << z;
  return oss.str();
};

#define WHERE WhereAt(__FILE__, __PRETTY_FUNCTION__, __LINE__)

class VizmoException : public exception {
  public:
    VizmoException(const string& _type, const string& _where, const string& _message) :
      m_type(_type), m_where(_where), m_message(_message) {
      }
    virtual ~VizmoException() throw() {}

    virtual const char* what() const throw() {
      return
        ("\nError:\n\t" + m_type +
        "\nWhere:" + m_where +
        "\nWhy:\n\t" + m_message +
        "\n").c_str();
    }

  private:
    string m_type, m_where, m_message;
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
