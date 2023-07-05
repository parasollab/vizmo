#ifndef VIZMO_EXCEPTIONS_H_
#define VIZMO_EXCEPTIONS_H_

#include <string>
#include "Utilities/PMPLExceptions.h"

////////////////////////////////////////////////////////////////////////////////
/// \brief Specialization of PMPLException for build errors.
////////////////////////////////////////////////////////////////////////////////
class BuildException : public PMPLException {

  public:

    BuildException(const std::string& _where, const std::string& _message) :
        PMPLException("BuildError", _where, _message) {}
};


////////////////////////////////////////////////////////////////////////////////
/// \brief Specialization of PMPLException for rendering errors.
////////////////////////////////////////////////////////////////////////////////
class DrawException : public PMPLException {

  public:

    DrawException(const std::string& _where, const std::string& _message) :
        PMPLException("DrawError", _where, _message) {}
};

#endif
