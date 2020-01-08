// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_CALL_H_
#define WT_DBO_CALL_H_

#include <Wt/Dbo/WDboDllDefs.h>
#include <string>
#include <Wt/Dbo/WDboDllDefs.h>

namespace Wt {
  namespace Dbo {

class Session;
class SqlStatement;

/*! \class Call Wt/Dbo/Call.h Wt/Dbo/Call.h
 *  \brief A database call.
 *
 * A call can be used to execute a database command (e.g. an update,
 * or a stored procedure call).
 *
 * \sa Query
 */
class WTDBO_API Call
{
public:
  /*! \brief Destructor.
   *
   * This executes the call if it wasn't run() yet, and the call has not
   * been copied.
   */
  ~Call() noexcept(false);

  /*! \brief Copy constructor.
   *
   * This transfer the call "token" to the copy.
   */
  Call(const Call& other);

  /*! \brief Binds a value to the next positional marker.
   *
   * This binds the \p value to the next positional marker.
   */
  template<typename T> Call& bind(const T& value);

  /*! \brief Runs the database call.
   *
   * This may throw an exception if there was a problem with the SQL
   * command.
   */
  void run();

private:
  bool copied_, run_;
  SqlStatement *statement_;
  int column_;

  Call(Session& session, const std::string& sql);

  friend class Session;
  template <class C> friend class collection;
};

  }
}

#endif // WT_DBO_CALL
