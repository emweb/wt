#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/backend/MySQL>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Firebird>
#include <Wt/Dbo/FixedSqlConnectionPool>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>
#include <Wt/Dbo/QueryModel>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace dbo = Wt::Dbo;

struct DboFixtureBase
{

  dbo::SqlConnectionPool *connectionPool_;
  dbo::Session *session_;

  DboFixtureBase(bool showQueries = true)
  {
    static bool logged = false;
    dbo::SqlConnection *connection;

#ifdef SQLITE3
    if (!logged) {
      std::cerr << "DboTest.C created a Sqlite3 connector" << std::endl;
      logged = true;
    }

    dbo::backend::Sqlite3 *sqlite3 = new dbo::backend::Sqlite3(":memory:");
    sqlite3->setDateTimeStorage(dbo::SqlDate,
				dbo::backend::Sqlite3::JulianDaysAsReal);
    connection = sqlite3;
#endif // SQLITE3

#ifdef POSTGRES
    if (!logged) {
      std::cerr << "DboTest.C created a Postgres connector" << std::endl;
      logged = true;
    }

    connection = new dbo::backend::Postgres
        ("user=postgres_test password=postgres_test port=5432 dbname=wt_test");
    // use host=vendetta for testing.
#endif // POSTGRES");

#ifdef MYSQL
    if (!logged) {
      std::cerr << "DboTest.C created a MySQL connector" << std::endl;
      logged = true;
    }

    connection = new dbo::backend::MySQL("wt_test_db", "test_user",
                                            "test_pw", "vendetta", 3306);
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
    file = "/opt/db/firebird/wt_test.fdb";
#endif

    if (!logged) {
      std::cerr << "DboTest.C created a Firebird connector" << std::endl;
      logged = true;
    }

    connection = new dbo::backend::Firebird ("vendetta",
					     file, 
					     "test_user", "test_pwd", 
					     "", "", "");
#endif // FIREBIRD

    if (showQueries)
      connection->setProperty("show-queries", "true");
    connectionPool_ = new dbo::FixedSqlConnectionPool(connection, 5);

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
    delete connectionPool_;
  }
};
