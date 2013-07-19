#ifndef EXCEPTIONS_H_
#define EXCEPTIONS_H_

#include <string>
#include <sstream>
#include <exception>
using namespace std;

class VizmoException : public exception {
  public:
    VizmoException(const string& _type, const string& _where, const string& _message) :
      m_type(_type), m_where(_where), m_message(_message) {
      }
    virtual ~VizmoException() throw() {}

    virtual const char* what() const throw() {
      return ("Error::" + m_type + "::" + m_where + "::" + m_message).c_str();
    }

  private:
    string m_type, m_where, m_message;
};

class ParseException : public VizmoException {
  public:
    ParseException(const string& _where, const string& _message) :
      VizmoException("ParseError", _where, _message) {}
};

class BuildException : public VizmoException {
  public:
    BuildException(const string& _where, const string& _message) :
      VizmoException("BuildError", _where, _message) {}
};

#endif
