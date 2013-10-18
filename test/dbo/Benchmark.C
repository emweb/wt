/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/backend/MySQL>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Firebird>
#include <Wt/WDateTime>
#include <Wt/Dbo/WtSqlTraits>

namespace dbo = Wt::Dbo;

/*
 * Small benchmark inspired on:
 * http://www.codesynthesis.com/~boris/blog/2011/04/06/performance-odb-cxx-orm-vs-cs-orm/
 *
 * (We get about same performance for Sqlite3, but slower performance
 *  for Postgres -- twice as slow, all because we do not use binary I/O
 *  in the backend, and we pay the price for datetime parsing)
 */
namespace Perf {

class Post {
public:
  long id;
  std::string text;

  Wt::WDateTime creation_date;
  Wt::WDateTime last_change_date;

  int counter[10];

  template<class Action>
  void persist(Action& a)
  {
    /*
     * Concatenating this string becomes a bottleneck, persist()
     * is called alot...
     */
    static const char *counterFields[]
      = { "counter1", "counter2", "counter3", "counter4", "counter5",
	  "counter6", "counter7", "counter8", "counter9", "counter10" };

    dbo::id(a, id, "id");
    dbo::field(a, text, "text");
    dbo::field(a, creation_date, "creation_date");
    dbo::field(a, last_change_date, "last_change_date");

    for (int i = 0; i < 10; ++i)
      dbo::field(a, counter[i], counterFields[i]);
  }
};

}

namespace Wt {
  namespace Dbo {

    /*
     * Customize the mapping: disable version field and surrogate ID
     */
    template<>
    struct dbo_traits<Perf::Post> : public dbo_default_traits {
      typedef long IdType;
      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
      static IdType invalidId() { return -1; }
    };

  }
}


BOOST_AUTO_TEST_CASE( performance_test )
{
#ifdef SQLITE3
  dbo::backend::Sqlite3 connection(":memory:");
  connection.setDateTimeStorage(dbo::SqlDateTime,
				dbo::backend::Sqlite3::UnixTimeAsInteger);
#endif // SQLITE3

#ifdef POSTGRES
  dbo::backend::Postgres connection
    ("user=postgres_test password=postgres_test port=5432 dbname=wt_test");
#endif // POSTGRES


#ifdef MYSQL
    dbo::backend::MySQL connection("wt_test_db", "test_user",
                                   "test_pw", "localhost", 3306);
#endif // MYSQL

#ifdef FIREBIRD
    std::string file;
#ifdef WIN32
    file = "C:\\opt\\db\\firebird\\wt_test.fdb";
#else
    file = "/opt/db/firebird/wt_test.fdb";
#endif

  dbo::backend::Firebird connection("localhost", 
				    file, 
				    "test_user", "test_pwd", 
				    "", "", "");
#endif // FIREBIRD

  // connection.setProperty("show-queries", "true");

  dbo::Session session;
  session.setConnection(connection);

  session.mapClass<Perf::Post>("post");

  try {
    session.dropTables();
  } catch (...) {
  }

  session.createTables();

  dbo::Transaction t(session);

  const unsigned total_objects = 10000;
  const std::string text = "some text?";

  std::cerr << "Loading " << total_objects << " objects in database."
	    << std::endl;
  for (unsigned i = 0; i < total_objects; ++i) {
    Perf::Post *p = new Perf::Post();

    p->id = i;
    p->text = text;
    p->creation_date = Wt::WDateTime::currentDateTime().addSecs(-i * 60 * 60);
    p->last_change_date = Wt::WDateTime::currentDateTime();
 
    for (unsigned k = 0; k < 10; ++k)
      p->counter[k] = i + k + 1;

    session.add(p);
  }

  t.commit();

  std::cerr << "Measuring selection ..." << std::endl;

  boost::posix_time::ptime start
    = boost::posix_time::microsec_clock::local_time();

  const unsigned times = 100;
  for (unsigned i = 0; i < times; ++i) {
    dbo::Transaction t(session);

    dbo::ptr<Perf::Post> p;

    for (unsigned long i = 0; i < 500; ++i) {
      unsigned long id = std::rand() % total_objects;
      p = session.load<Perf::Post>(id);
    }

    t.commit();
  }

  boost::posix_time::ptime
    end = boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration d = end - start;

  std::cerr << "Took: " << (double)d.total_microseconds() / 1000 / times
	    << " ms per 500 selects." << std::endl;

  session.dropTables();
}

