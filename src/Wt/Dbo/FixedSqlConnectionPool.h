// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_FIXED_SQL_CONNECTION_POOL_H_
#define WT_DBO_FIXED_SQL_CONNECTION_POOL_H_

#include <Wt/Dbo/SqlConnectionPool.h>

#include <chrono>
#include <vector>

namespace Wt {
  namespace Dbo {

/*! \class FixedSqlConnectionPool Wt/Dbo/FixedSqlConnectionPool.h Wt/Dbo/FixedSqlConnectionPool.h
 *  \brief A connection pool of fixed size.
 *
 * This provides a connection pool of fixed size: its size is determined at
 * startup time, and the pool will not grow as more connections are needed.
 *
 * This is adequate when the number of threads (which need different
 * connections to work with) is also bounded, like when using a fixed
 * size thread pool. This is for example the case when used in
 * conjunction with %Wt. Note that you do not need as many connections
 * as sessions, since Session will only use a connection while
 * processing a transaction.
 *
 * \ingroup dbo
 */
class WTDBO_API FixedSqlConnectionPool : public SqlConnectionPool
{
public:
  /*! \brief Creates a fixed connection pool.
   *
   * The pool is initialized with the provided \p connection, which is
   * cloned (\p size - 1) times.
   */
  FixedSqlConnectionPool(std::unique_ptr<SqlConnection> connection, int size);

  /*! \brief Set a timeout to get a connection.
   *
   * When the connection pool has no available connection,
   * it will wait the given duration.
   *
   * On timeout, handleTimeout() is called, which throws an exception
   * by default.
   *
   * By default, there is no timeout.
   */
  void setTimeout(std::chrono::steady_clock::duration timeout);

  /*! \brief Get the timeout to get a connection.
   *
   * \sa setTimeout()
   */
  std::chrono::steady_clock::duration timeout() const;
  
  virtual ~FixedSqlConnectionPool();
  virtual std::unique_ptr<SqlConnection> getConnection() override;
  virtual void returnConnection(std::unique_ptr<SqlConnection>) override;
  virtual void prepareForDropTables() const override;

protected:
  /*! \brief Handle a timeout that occured while getting a connection.
   *
   * The default implementation throws an Exception.
   *
   * If the function returns cleanly, it is assumed that something has been done to fix
   * the situation (e.g. connections have been added to the pool): the timeout is reset
   * and another attempt is made to obtain a connection.
   */
  virtual void handleTimeout();
  
private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

  }
}

#endif // WT_DBO_SQL_CONNECTION_POOL_H_
