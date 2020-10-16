#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/Dbo/backend/MySQL.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/backend/Firebird.h>
#include <Wt/Dbo/backend/MSSQLServer.h>
#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>
#include <Wt/Dbo/QueryModel.h>

namespace dbo = Wt::Dbo;

#if defined(SQLITE3)
#define DBO_TEST_SUITE_NAME Sqlite3_Test_Suite
#elif defined(POSTGRES)
#define DBO_TEST_SUITE_NAME PostgreSQL_Test_Suite
#elif defined(MYSQL)
#define DBO_TEST_SUITE_NAME MariaDB_MySQL_Test_Suite
#elif defined(FIREBIRD)
#define DBO_TEST_SUITE_NAME Firebird_Test_Suite
#elif defined(MSSQLSERVER)
#define DBO_TEST_SUITE_NAME MSSqlServer_Test_Suite
#endif

struct DboFixtureBase
{

  std::unique_ptr<dbo::SqlConnectionPool> connectionPool_;
  dbo::Session *session_;

  DboFixtureBase(bool showQueries = true)
  {
    static bool logged = false;
    std::unique_ptr<dbo::SqlConnection> connection;

#ifdef SQLITE3
    if (!logged) {
      std::cerr << "DboTest.C created a Sqlite3 connector" << std::endl;
      logged = true;
    }

    dbo::backend::Sqlite3 *sqlite3 = new dbo::backend::Sqlite3(":memory:");
    sqlite3->setDateTimeStorage
      (dbo::SqlDateTimeType::Date,
       dbo::backend::DateTimeStorage::JulianDaysAsReal);
    connection = std::unique_ptr<dbo::SqlConnection>(sqlite3);
#endif // SQLITE3

#ifdef POSTGRES
    if (!logged) {
      std::cerr << "DboTest.C created a Postgres connector" << std::endl;
      logged = true;
    }

    connection = std::unique_ptr<dbo::SqlConnection>(new dbo::backend::Postgres
        ("host=db user=postgres_test password=postgres_test port=5432 dbname=wt_test"));

#endif // POSTGRES");

#ifdef MYSQL
    if (!logged) {
      std::cerr << "DboTest.C created a MySQL connector" << std::endl;
      logged = true;
    }

    std::unique_ptr<dbo::backend::MySQL> mysql(
        new dbo::backend::MySQL("wt_test_db", "test_user", "test_pw", "db", 3306));
    mysql->setFractionalSecondsPart(3);
    connection = std::move(mysql);
#endif // MYSQL

#ifdef FIREBIRD
    // gsec.exe -user sysdba -pass masterkey
    // add test_user -pw test_pwd
    // isql.exe
    // create database 'C:\opt\db\firebird\wt_test.fdb' user 'test_user' password 'test_pwd'
    std::string file;
#ifdef WT_WIN32
    file = "C:\\opt\\db\\firebird\\wt_test.fdb";
#else
    file = "/firebird/data/wt_test.fdb";
#endif

    if (!logged) {
      std::cerr << "DboTest.C created a Firebird connector" << std::endl;
      logged = true;
    }

    connection = std::unique_ptr<dbo::SqlConnection>(
          new dbo::backend::Firebird ("db",
                                      file,
                                      "test_user", "test_pwd",
                                      "", "", ""));
#endif // FIREBIRD

#ifdef MSSQLSERVER
    if (!logged) {
      std::cerr << "DboTest.C created a Microsoft SQL Server connector" << std::endl;
      logged = true;
    }

    connection = std::unique_ptr<dbo::SqlConnection>(
	new dbo::backend::MSSQLServer(
	"Driver={ODBC Driver 17 for SQL Server};"
	"Server=db;"
	"UID=sa;"
	"PWD={hereIsMyPassword_1234};"));
#endif // MSSQLSERVER

    if (showQueries)
      connection->setProperty("show-queries", "true");
    connectionPool_ = std::unique_ptr<dbo::SqlConnectionPool>(
          new dbo::FixedSqlConnectionPool(std::move(connection), 5));

    session_ = new dbo::Session();
    session_->setConnectionPool(*connectionPool_);
  }

  ~DboFixtureBase()
  {
    try {
      session_->dropTables();
    } catch (...) {

    }

    delete session_;
  }
};
