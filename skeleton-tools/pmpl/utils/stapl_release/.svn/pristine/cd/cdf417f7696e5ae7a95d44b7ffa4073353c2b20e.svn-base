/*
// Copyright (c) 2000-2009, Texas Engineering Experiment Station (TEES), a
// component of the Texas A&M University System.

// All rights reserved.

// The information and source code contained herein is the exclusive
// property of TEES and may not be disclosed, examined or reproduced
// in whole or in part without explicit written authorization from TEES.
*/


#ifndef STAPL_RUNTIME_CONFIG_COMPILER_HPP
#define STAPL_RUNTIME_CONFIG_COMPILER_HPP

//////////////////////////////////////////////////////////////////////
/// @file
/// Detects compilers and their capabilities.
///
/// @ingroup runtimeConfig
//////////////////////////////////////////////////////////////////////

#include <boost/config/select_compiler_config.hpp>

//////////////////////////////////////////////////////////////////////
/// @def STAPL_ALIGNAS(x)
/// @brief Specifies the alignment of @p x.
///
/// @ingroup runtimeConfig
//////////////////////////////////////////////////////////////////////

#if defined(__GNUC__)    || \
    defined(BOOST_INTEL) || \
    defined(__IBMCPP__)  || \
    defined(__SUNPRO_CC) || \
    defined(__PATHSCALE__)
# define STAPL_ALIGNAS(x) __attribute__((aligned(x)))
#elif defined(__HP_aCC) || \
      defined(BOOST_MSVC)
# define STAPL_ALIGNAS(x) __declspec(align(x))
#else
# error "Provide support for STAPL_ALIGNAS()."
# define STAPL_ALIGNAS(x)
#endif

#endif
