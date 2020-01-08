/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include "DboFixture.h"

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

struct DboBenchmarkFixture : DboFixtureBase
{
  DboBenchmarkFixture() :
    DboFixtureBase(false)
  {
    session_->mapClass<Perf::Post>("post");

    try {
      session_->dropTables();
    } catch (...) {
    }

    session_->createTables();
  }
};

namespace Wt {
  namespace Dbo {

    /*
     * Customize the mapping: disable version field and surrogate ID
     */
    template<>
    struct dbo_traits<Perf::Post> : public dbo_default_traits {
      typedef long IdType;
      static const char *surrogateIdField() { return nullptr; }
      static const char *versionField() { return nullptr; }
      static IdType invalidId() { return -1; }
    };

  }
}

BOOST_AUTO_TEST_CASE( performance_test )
{
  DboBenchmarkFixture f;

  dbo::Session &session = *(f.session_);

  dbo::Transaction t(session);

  const unsigned total_objects = 10000;
  const std::string text = "some text?";

  std::cerr << "Loading " << total_objects << " objects in database."
            << std::endl;
  for (unsigned i = 0; i < total_objects; ++i) {
    auto p = Wt::cpp14::make_unique<Perf::Post>();

    p->id = i;
    p->text = text;
    p->creation_date = Wt::WDateTime::currentDateTime().addSecs(-(int)i * 60 * 60);
    p->last_change_date = Wt::WDateTime::currentDateTime();
 
    for (unsigned k = 0; k < 10; ++k)
      p->counter[k] = i + k + 1;

    session.add(std::move(p));
  }

  t.commit();

  std::cerr << "Measuring selection ..." << std::endl;

  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

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

  std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

  std::cerr << "Took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / times
            << " ms per 500 selects." << std::endl;

  //session.dropTables();
}

