#ifndef VIZMOEXCEPTIONS_H_
#define VIZMOEXCEPTIONS_H_

#include "Utilities/PMPLExceptions.h"

class BuildException : public PMPLException {
  public:
    BuildException(const string& _where, const string& _message) :
      PMPLException("BuildError", _where, _message) {}
};

class DrawException : public PMPLException {
  public:
    DrawException(const string& _where, const string& _message) :
      PMPLException("DrawError", _where, _message) {}
};

#endif
