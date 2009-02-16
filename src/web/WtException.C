/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WtException.h"

namespace Wt {

WtException::WtException(const std::string what)
  : what_(what)
{ }

WtException::WtException(const std::string what,
			 const std::exception& wrapped)
  : what_(what + "\nCaused by exception:" + wrapped.what())
{ }

WtException::~WtException() throw()
{ }

}
