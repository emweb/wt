// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Paul Harrison
 */
#ifndef WT_DBO_BACKEND_MYSQL_H_
#define WT_DBO_BACKEND_MYSQL_H_

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/SqlStatement.h>
#include <Wt/Dbo/backend/WDboMySQLDllDefs.h>

namespace Wt {
  namespace Dbo {
    namespace backend {

class MySQL_impl;

/*! \class MySQL Wt/Dbo/backend/MySQL.h
 *  \brief A MySQL connection
 *
 * This class provides the backend implementation for mariadb databases.
 * It has been tested against MySQL 5.6.
 *
 * In order to work properly with Wt::Dbo, MySQL must be configured with
 * InnoDB (for MySQL) or XtraDB (for mariadb) as the default database
 * engine - so that the transaction based functionality works.
 *
 * \note There is a bug in the implementation of milliseconds in mariadb C
 *       client which affects WTime and posix::time_duration values -- it
 *       goes berserk when fractional part = 0.
 *
 * \ingroup dbo
 */
class WTDBOMYSQL_API MySQL : public SqlConnection
{
public:
  /*! \brief Opens a new MySQL backend connection.
   *
   *  For the connection parameter description, please refer to the
   *  connect() method.
   *
   *  \param fractionalSecondsPart The number of fractional units (0 to 6).
   *         A value of -1 indicates that the fractional part is not stored.
   *         Fractional seconds part are supported for MySQL 5.6.4
   *         http://dev.mysql.com/doc/refman/5.6/en/fractional-seconds.html
   */
  MySQL(const std::string &db, const std::string &dbuser="root",
        const std::string &dbpasswd="", const std::string dbhost="localhost",
        unsigned int dbport = 0,
        const std::string &dbsocket ="/var/run/mysqld/mysqld.sock",
        int fractionalSecondsPart = -1);

  /*! \brief Copies a MySQL connection.
   *
   * This creates a new connection with the same settings as another
   * connection.
   *
   * \sa clone()
   */
  MySQL(const MySQL& other);

  /*! \brief Destructor.
   *
   * Closes the connection.
   */
  ~MySQL();

  /*! \brief Returns a copy of the connection.
   */
  virtual std::unique_ptr<SqlConnection> clone() const override;

  /*! \brief Tries to connect.
   *
   *  \param db The database name.
   *  \param dbuser The username for the database connection -
   *         defaults to "root".
   *  \param dbpasswd The password for the database conection - defaults to an
   *         empty string.
   *  \param dbhost The hostname of the database - defaults to localhost.
   *  \param dbport The portnumber - defaults to a default port.
   *  \param dbsocket The socket to use.
   *
   * Throws an exception if there was a problem, otherwise true.
   */
  bool connect(const std::string &db, const std::string &dbuser="root",
        const std::string &dbpasswd="", const std::string &dbhost="localhost",
        unsigned int dbport = 0,
        const std::string &dbsocket ="/var/run/mysqld/mysqld.sock");

  /*! \brief Returns the underlying connection.
   */
  MySQL_impl *connection() { return impl_; }

  void checkConnection();

  virtual void executeSql(const std::string &sql) override;

  virtual void startTransaction() override;
  virtual void commitTransaction() override;
  virtual void rollbackTransaction() override;

  virtual std::unique_ptr<SqlStatement> prepareStatement(const std::string& sql) override;

  /** @name Methods that return dialect information
   */
  //!@{
  virtual std::string autoincrementSql() const override;
  virtual std::string autoincrementType() const override;
  virtual std::string autoincrementInsertSuffix(const std::string& id) const override;
  virtual std::vector<std::string>
    autoincrementCreateSequenceSql(const std::string &table,
                                   const std::string &id) const override;
  virtual std::vector<std::string>
    autoincrementDropSequenceSql(const std::string &table,
                                 const std::string &id) const override;

  virtual const char *dateTimeType(SqlDateTimeType type) const override;
  virtual const char *blobType() const override;
  virtual bool supportAlterTable() const override;
  virtual const char *alterTableConstraintString() const override;
  virtual bool requireSubqueryAlias() const override {return true;}
  //!@}

  /*! \brief Returns the supported fractional seconds part
  *
  * By default return -1: fractional part is not stored.
  * Fractional seconds part is supported for MySQL 5.6.4
  * http://dev.mysql.com/doc/refman/5.6/en/fractional-seconds.html
  *
  * \sa setFractionalSecondsPart()
  */
  int getFractionalSecondsPart() const;

  /*! \brief Set the supported fractional seconds part
  *
  * The fractional seconds part can be also set in the constructor
  * Fractional seconds part is supported for MySQL 5.6.4
  * http://dev.mysql.com/doc/refman/5.6/en/fractional-seconds.html
  *
  * \param fractionalSecondsPart Must be in the range 0 to 6.
  *
  * \sa setFractionalSecondsPart()
  */
  void setFractionalSecondsPart(int fractionalSecondsPart);

private:
  std::string dbname_;
  std::string dbuser_;
  std::string dbpasswd_;
  std::string dbhost_;
  std::string dbsocket_;
  unsigned int dbport_;

  int fractionalSecondsPart_;
  std::string dateType_, timeType_;

  MySQL_impl* impl_; // MySQL connection handle

  void init();
};

    }
  }
}

#endif // WT_DBO_BACKEND_MYSQL_H_
