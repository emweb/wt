// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WINVALIDFORMATEXCEPTION_H_
#define WT_WINVALIDFORMATEXCEPTION_H_

#include "Wt/WDllDefs.h"
#include "Wt/WException.h"

#include <string>

namespace Wt {

/*! \class WInvalidFormatException Wt/Exception/WInvalidFormatException.h
 *  \brief The exception thrown when a supplied format was not correct.
 *
 * If the developer or user is required to enter a format, this format
 * should adhere to certain conventions. Formatting can be interpreted as
 * a regex, a self-defined format (see: WDate::toString() for example),
 * or any input that supplies the rules for another field or value.
 */
class WT_API WInvalidFormatException : public Wt::WException
{
public:
  //! \brief Creates the invalid format exception.
  explicit WInvalidFormatException(const std::string& what);

  /*! \brief Creates the invalid format exception from another exception.
   *
   * It is possible to wrap the WInvalidFormatException around an
   * existing exception. The wrapped exception's description (see:
   * https://en.cppreference.com/w/cpp/error/exception/what) will then
   * be appended to this exception's description.
   */
  WInvalidFormatException(const std::string& what, const std::exception& wrapped);
};
}

#endif // WT_WINVALIDFORMATEXCEPTION_H_
