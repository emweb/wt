// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_EXCEPTION_H_
#define WT_EXCEPTION_H_

#include <string>
#include <Wt/WDllDefs.h>

namespace Wt {

class WT_API WtException : public std::exception
{
public:
  WtException(const std::string what);
  WtException(const std::string what, const std::exception& wrapped);
  ~WtException() throw();

  const char *what() const throw() { return what_.c_str(); }

private:
  std::string what_;
};

}

#endif // WT_EXCEPTION_H_
