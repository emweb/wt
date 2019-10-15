// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Hilary Cheng
 */
#ifndef WT_DBO_BACKEND_POSTGRES_H_
#define WT_DBO_BACKEND_POSTGRES_H_

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>
#include <Wt/Dbo/backend/WDboPostgresDllDefs.h>

#include <chrono>

struct pg_conn;
typedef struct pg_conn PGconn;

namespace Wt {
  namespace Dbo {
    namespace backend {

/*! \class Postgres Wt/Dbo/backend/Postgres.h Wt/Dbo/backend/Postgres.h
 *  \brief A PostgreSQL connection
 *
 * This class provides the backend implementation for PostgreSQL databases.
 *
 * When applicable, exceptions from the backend will return the
 * five-character SQLSTATE error codes, as in
 * http://www.postgresql.org/docs/8.1/static/errcodes-appendix.html, in
 * Exception::code().
 *
 * \ingroup dbo
 */
class WTDBOPOSTGRES_API Postgres : public SqlConnection
{
public:
  /*! \brief Creates new PostgreSQL backend connection.
   *
   * The connection is not yet open, and requires a connect() before it
   * can be used.
   */
  Postgres();

  /*! \brief Opens a new PostgreSQL backend connection.
   *
   * The \p db may be any of the values supported by PQconnectdb().
   */
  Postgres(const std::string& db);

  /*! \brief Copies a PostgreSQL connection.
   *
   * This creates a new connection with the same settings as another
   * connection.
   *
   * \sa clone()
   */
  Postgres(const Postgres& other);

  /*! \brief Destructor.
   *
   * Closes the connection.
   */
  ~Postgres();

  virtual std::unique_ptr<SqlConnection> clone() const override;

  /*! \brief Tries to connect.
   *
   * Throws an exception if there was a problem, otherwise true.
   * An example connecion string could be: 
   * "host=127.0.0.1 user=test password=test port=5432 dbname=test"
   */
  bool connect(const std::string& db);

  /*! \brief Disconnects.
   *
   * This disconnects from the server. Any subsequent action on the connection
   * will result in an automatic reconnect.
   *
   * \sa reconnect()
   */
  void disconnect();

  /*! \brief Reconnect.
   *
   * This will try to reconnect a previously disconnected connection. If the connection
   * is still open, it will first disconnect.
   */
  bool reconnect();

  /*! \brief Returns the underlying connection.
   */
  PGconn *connection() { return conn_; }

  /*! \brief Sets a timeout.
   *
   * Sets a timeout for queries. When the query exceeds this timeout, the connection
   * is closed using disconnect() and an exception is thrown.
   *
   * In practice, as a result, the connection (and statements) can still be used again
   * when a successful reconnect() is performed.
   *
   * A value of 0 disables the timeout handling, allowing operations
   * to take as much time is they require.
   *
   * The default value is 0.
   */
  void setTimeout(std::chrono::microseconds timeout);

  /*! \brief Returns the timeout.
   *
   * \sa setTimeout()
   */
  std::chrono::microseconds timeout() const { return timeout_; }

  /*! \brief Sets the maximum lifetime for the underlying connection.
   *
   * The maximum lifetime is specified in seconds. If set to a value >
   * 0, then the underlying connection object will be closed and
   * reopened when the connection has been open longer than this
   * lifetime.
   *
   * The default value is -1 (unlimited lifetime).
   */
  void setMaximumLifetime(std::chrono::seconds seconds);

  virtual void executeSql(const std::string &sql) override;

  virtual void startTransaction() override;
  virtual void commitTransaction() override;
  virtual void rollbackTransaction() override;

  virtual std::unique_ptr<SqlStatement> prepareStatement(const std::string& sql) override;

  /** @name Methods that return dialect information
   */
  //!@{
  virtual std::string autoincrementSql() const override;
  virtual std::vector<std::string>
    autoincrementCreateSequenceSql(const std::string &table,
                                   const std::string &id) const override;
  virtual std::vector<std::string> 
    autoincrementDropSequenceSql(const std::string &table,
                                 const std::string &id) const override;
  virtual std::string autoincrementType() const override;
  virtual std::string autoincrementInsertSuffix(const std::string& id) const override;
  virtual const char *dateTimeType(SqlDateTimeType type) const override;
  virtual const char *blobType() const override;
  virtual bool supportAlterTable() const override;
  virtual bool supportDeferrableFKConstraint() const override;
  virtual bool requireSubqueryAlias() const override;
  //!@}
  
  void checkConnection(std::chrono::seconds margin);

private:
  std::string connInfo_;
  PGconn *conn_;
  std::chrono::microseconds timeout_;
  std::chrono::seconds maximumLifetime_;
  std::chrono::steady_clock::time_point connectTime_;

  void exec(const std::string& sql, bool showQuery);
};

    }
  }
}

#endif // WT_DBO_BACKEND_POSTGRES_H_
