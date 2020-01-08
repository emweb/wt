// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WEXCEPTION_H_
#define WT_WEXCEPTION_H_

#include <string>
#include <Wt/WDllDefs.h>

namespace Wt {

extern WT_API std::string backtrace();

/*! \class WException Wt/WException.h
 *  \brief Base class for exceptions thrown by %Wt.
 */
class WT_API WException : public std::exception
{
public:
  /*! \brief Creates an exception.
   */
  explicit WException(const std::string& what);

  /*! \brief Creates an exception.
   */
  WException(const std::string& what, const std::exception& wrapped);

  /*! \brief Destructor
   */
  virtual ~WException() throw();

  /*! \brief Returns the message.
   */
  virtual const char *what() const throw() override;

  /*! \brief Sets the message.
   */
  void setMessage(const std::string& msg);

private:
  std::string what_;
};

}

#endif // WT_WEXCEPTION_H_
