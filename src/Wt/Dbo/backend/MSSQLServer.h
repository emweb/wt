// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_BACKEND_MSSQLSERVER_H_
#define WT_DBO_BACKEND_MSSQLSERVER_H_

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>
#include <Wt/Dbo/backend/WDboMSSQLServerDllDefs.h>

namespace Wt {
  namespace Dbo {
    namespace backend {

/*! \class MSSQLServer Wt/Dbo/backend/MSSQLServer Wt/Dbo/backend/MSSQLServer
 *  \brief A Microsoft SQL Server connection
 *
 * This class provides the backend implementation for Microsoft SQL Server databases.
 *
 * \ingroup dbo
 */
class WTDBOMSSQLSERVER_API MSSQLServer : public SqlConnection
{
public:
  /*! \brief Creates a new Microsoft SQL Server backend connection.
   *
   * The connection is not yet open, and requires a connect() before it
   * can be used.
   */
  MSSQLServer();

  /*! \brief Creates a new Microsoft SQL Server backend connection.
   *
   * For info about the connection string, see the connect() method.
   *
   * \sa connect()
   */
  MSSQLServer(const std::string &connectionString);

  /*! \brief Copy constructor.
   *
   * This creates a new backend connection with the same settings
   * as another connection.
   *
   * \sa clone()
   */
  MSSQLServer(const MSSQLServer& other);

  /*! \brief Destructor.
   *
   * Closes the connection.
   */
  virtual ~MSSQLServer();

  virtual std::unique_ptr<Wt::Dbo::SqlConnection> clone() const override;

  /*! \brief Tries to connect.
   *
   * Throws an exception if there was a problem, otherwise returns true.
   *
   * The connection string is the connection string that should be passed
   * to SQLDriverConnectW to connect to the Microsoft SQL Server database.
   *
   * The \p connectionString should be UTF-8 encoded.
   *
   * Example connection string:
   *
   * \code
   * Driver={ODBC Driver 13 for SQL Server};
   * Server=localhost; 
   * UID=SA;
   * PWD={example password};
   * Database=example_db;
   * \endcode
   *
   * You could also specify a DSN (Data Source Name) if you have it configured.
   *
   * See the
   * <a href="https://docs.microsoft.com/en-us/sql/odbc/reference/syntax/sqldriverconnect-function">SQLDriverConnect</a>
   * function documentation on MSDN for more info.
   */
  bool connect(const std::string &connectionString);

  virtual void executeSql(const std::string &sql) override;

  virtual void startTransaction() override;
  virtual void commitTransaction() override;
  virtual void rollbackTransaction() override;
  
  virtual std::unique_ptr<SqlStatement> prepareStatement(const std::string &sql) override;

  /** @name Methods that return dialect information
   */
  //!@{
  virtual std::string autoincrementSql() const override;
  virtual std::vector<std::string> autoincrementCreateSequenceSql(const std::string &table, const std::string &id) const override;
  virtual std::vector<std::string> autoincrementDropSequenceSql(const std::string &table, const std::string &id) const override;
  virtual std::string autoincrementType() const override;
  virtual std::string autoincrementInsertInfix(const std::string &id) const override;
  virtual std::string autoincrementInsertSuffix(const std::string &id) const override;
  virtual const char *dateTimeType(SqlDateTimeType type) const override;
  virtual const char *blobType() const override;
  virtual bool requireSubqueryAlias() const override;
  virtual const char *booleanType() const override;
  virtual bool supportAlterTable() const override;
  virtual std::string textType(int size) const override;
  virtual LimitQuery limitQueryMethod() const override;
  //!@}

private:
  struct Impl;
  Impl *impl_;

  friend class MSSQLServerStatement;
};

    }
  }
}

#endif // WT_DBO_BACKEND_MSSQLSERVER_H_
