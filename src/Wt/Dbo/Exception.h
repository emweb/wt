// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_EXCEPTION_H_
#define WT_DBO_EXCEPTION_H_

#include <string>
#include <stdexcept>
#include <Wt/Dbo/WDboDllDefs.h>

namespace Wt {
  namespace Dbo {

extern WTDBO_API std::string backtrace();

/*! \class Exception Wt/Dbo/Exception.h Wt/Dbo/Exception.h
 *  \brief %Exception base class for %Wt::%Dbo.
 *
 * \ingroup dbo
 */
class WTDBO_API Exception : public std::runtime_error
{
public:
  /*! \brief Constructor.
   */
  Exception(const std::string& error, const std::string& code = std::string());

  virtual ~Exception() throw();

  /*! \brief A (backend-specific) error code.
   *
   * For native SQL errors, a native backend code may be available
   * (see the backend documentation for details). This is typically
   * the (semi-standardized) SQLSTATE code value.
   *
   * When not available, an empty string is returned.
   */
  std::string code() const { return code_; }

private:
  std::string code_;
};

/*! \class StaleObjectException Wt/Dbo/Exception.h Wt/Dbo/Exception.h
 *  \brief %Exception thrown when %Wt::%Dbo detects a concurrent modification
 *
 * %Wt::%Dbo uses optimistic locking for detecting and preventing
 * concurrent modification of database objects. When trying to save an
 * object that has been modified concurrently by another session, since
 * it was read from the database, this exception is thrown.
 *
 * This exception is thrown during flushing from Session::flush() or
 * ptr::flush(). Since flushing will also be done automatically when
 * needed (e.g. before running a query or before committing a
 * transaction), you should be prepared to catch this exception from most
 * library API calls.
 *
 * \note We should perhaps also have a ptr::isStale() method to find out
 *       what database object is stale ?
 *
 * \ingroup dbo
 */
class WTDBO_API StaleObjectException : public Exception
{
public:
  /*! \brief Constructor.
   */
  StaleObjectException(const std::string& id, const char *table, int version);
};

/*! \class ObjectNotFoundException Wt/Dbo/Exception.h Wt/Dbo/Exception.h
 *  \brief %Exception thrown when trying to load a non-existing object.
 *
 * This %Exception is thrown by Session::load() when trying to load an object
 * that does not exist.
 *
 * \ingroup dbo
 */
class WTDBO_API ObjectNotFoundException : public Exception
{
public:
  /*! \brief Constructor.
   */
  ObjectNotFoundException(const char *table, const std::string& id);
};

/*! \class NoUniqueResultException Wt/Dbo/Exception.h Wt/Dbo/Exception.h
 *  \brief %Exception thrown when a query unexpectedly finds a non-unique result.
 *
 * This %Exception is thrown by Query::resultValue() when the query has
 * more than one result.
 *
 * \ingroup dbo
 */
class WTDBO_API NoUniqueResultException : public Exception
{
public:
  /*! \brief Constructor.
   */
  NoUniqueResultException();
};

  }
}

#endif // WT_DBO_EXCEPTION_H_
