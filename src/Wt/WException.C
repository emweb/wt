/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException"

namespace Wt {

WException::WException(const std::string& what)
  : what_(what)
{ }

WException::WException(const std::string& what,
		       const std::exception& wrapped)
  : what_(what + "\nCaused by exception: " + wrapped.what())
{ }

WException::~WException() throw()
{ }

void WException::setMessage(const std::string& message)
{
  what_ = message;
}

const char *WException::what() const throw()
{
  return what_.c_str();
}

}
