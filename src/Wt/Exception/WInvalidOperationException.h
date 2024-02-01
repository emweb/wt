// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WINVALIDOPERATIONEXCEPTION_H_
#define WT_WINVALIDOPERATIONEXCEPTION_H_

#include "Wt/WDllDefs.h"
#include "Wt/WException.h"

#include <string>

namespace Wt {

/*! \class WInvalidOperationException Wt/Exception/WInvalidOperationException.h
 *  \brief The exception thrown when an numeric operation could not execute.
 *
 * When a numeric operation is called, this can encounter an exception.
 * This exception can for example be a division be 0.
 */
class WT_API WInvalidOperationException : public Wt::WException
{
public:
  //! \brief Creates the invalid operation exception.
  explicit WInvalidOperationException(const std::string& what);

  /*! \brief Creates the invalid operation exception from another exception.
   *
   * It is possible to wrap the WInvalidOperationException around an
   * existing exception. The wrapped exception's description (see:
   * https://en.cppreference.com/w/cpp/error/exception/what) will then
   * be appended to this exception's description.
   */
  WInvalidOperationException(const std::string& what, const std::exception& wrapped);
};
}

#endif // WT_WINVALIDOPERATIONEXCEPTION_H_
