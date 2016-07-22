#ifndef VIZMO_EXCEPTIONS_H_
#define VIZMO_EXCEPTIONS_H_

#include "Utilities/PMPLExceptions.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Specialization of PMPLException for build errors.
////////////////////////////////////////////////////////////////////////////////
class BuildException : public PMPLException {

  public:

    BuildException(const string& _where, const string& _message) :
        PMPLException("BuildError", _where, _message) {}
};


////////////////////////////////////////////////////////////////////////////////
/// \brief Specialization of PMPLException for rendering errors.
////////////////////////////////////////////////////////////////////////////////
class DrawException : public PMPLException {

  public:

    DrawException(const string& _where, const string& _message) :
        PMPLException("DrawError", _where, _message) {}
};

#endif
