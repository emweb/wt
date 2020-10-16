/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/QueryModel.h>

#include "DboFixture.h"

namespace dbo = Wt::Dbo;

namespace {

const unsigned total_added_objects = 20000;
const unsigned start_total_objects = 500;
const unsigned benchmark_time_limit = 10; //seconds (for a benchmark round)

}

namespace Perf2 {

class Post {
public:
  int counter;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, counter, "counter");
  }
};

}

struct DboBenchmark2Fixture : DboFixtureBase
{
  DboBenchmark2Fixture() :
    DboFixtureBase(false)
  {
    session_->mapClass<Perf2::Post>("post");

    try {
      session_->dropTables();
    } catch (...) {
    }

    session_->createTables();
  }
};


long benchmarkQuery(
    dbo::Session& session,
    const std::string& query_sql,
    int total_objects,
    int batch_size,
    long expected,
    bool force_select_count)
{
  dbo::Transaction t(session);

  /*
   * clear Dbo Session cache
   */
  session.rereadAll();

  /*boost::posix_time::ptime start
    = boost::posix_time::microsec_clock::local_time();*/
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

  typedef std::tuple<dbo::ptr<Perf2::Post>, double> PostWithSum;

  dbo::Query<PostWithSum> query = session.query<PostWithSum>(query_sql);

  dbo::QueryModel< PostWithSum > model;

  model.setQuery(query);
  model.addAllFieldsAsColumns();

  std::cerr << "QueryModel " << total_objects
    << " rows in batches of " << batch_size
    << (force_select_count ? " (force count): " : " : ");

  long total_sum = 0L;

  BOOST_REQUIRE(model.columnCount() == 4); // id, version, counter, sum_total

  /*
   * FOR TESTING ONLY: Force select count by setting batch size to zero
   */
  if (force_select_count)
    model.setBatchSize(0);
  else
    model.setBatchSize(batch_size);

  BOOST_REQUIRE(model.rowCount() == total_objects);

  if (force_select_count)
    model.setBatchSize(batch_size);

  long sum;
  dbo::ptr<Perf2::Post> post;
  for (int i = 0; i < total_objects; i++) {
    std::tie(post, sum) = model.resultRow(i);
    total_sum += sum;
  }

  /*boost::posix_time::ptime
    end = boost::posix_time::microsec_clock::local_time();*/
  std::chrono::system_clock::time_point end = std::chrono::system_clock::now();

  //boost::posix_time::time_duration d = end - start;

  std::cerr << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / total_objects
            << " microseconds per object" << std::endl;

  BOOST_REQUIRE(total_sum == expected);

  //return d.total_seconds();
  return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}

BOOST_AUTO_TEST_SUITE( DBO_TEST_SUITE_NAME )

BOOST_AUTO_TEST_CASE( performance_test2 )
{
  DboBenchmark2Fixture f;

  dbo::Session &session = *(f.session_);

  /*
   * This query is expensive to count() because of the WHERE clause,
   * at least with Postgres and Sqlite3. SELECT statements that use
   * WITH clauses or WINDOW functions can also be expensive to count.
   */
  std::string query_post_with_sum =
      "SELECT \"p1\", (SELECT sum(\"p2\".\"counter\") * (avg(\"p2\".\"counter\")) \"sum_total\" "
                  "FROM \"post\" \"p2\" WHERE \"p2\".\"id\" <= \"p1\".\"id\") AS \"subq1\" "
      "FROM \"post\" \"p1\" "
      "WHERE (SELECT sum(\"p2\".\"counter\") * (avg(\"p2\".\"counter\")) \"sum_total\" "
                  "FROM \"post\" \"p2\" WHERE \"p2\".\"id\" <= \"p1\".\"id\") > 0";

  long time_required = 0L;
  long expected;
  int current_objects = 0;
  for (unsigned i = start_total_objects;
      (i <= total_added_objects) && ((time_required * 6) < benchmark_time_limit); i *= 2) {
    expected = i * (i + 1) / 2;
    int total_objects = i;

    dbo::Transaction t(session);

    std::cerr << "Loading " << total_objects << " objects in database."
              << std::endl;

    for (int i = 0; i < total_objects - current_objects; ++i) {
      auto p = std::make_unique<Perf2::Post>();
      p->counter = 1;
      session.add(std::move(p));
    }

    t.commit();
    current_objects = total_objects;

    /*
     * Three cases:
     *
     * 1. Batch size is greater than total rows in table. One SELECT.
     * 2. Batch size is equal to number of rows in table. All rows are fetched in
     *    single SELECT, but a second SELECT for count is still required because in
     *    general case, Dbo is not sure whether or not more rows exist.
     * 3. Batch size is less than number of rows. A SELECT for count is required.
     */
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects + 1, expected, 0);
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects + 0, expected, 0);
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects - 1, expected, 0);

    std::cerr << std::endl;

    /*
     * Repeat above tests, but force a separate select for counting.
     */
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects + 1, expected, 1);
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects + 0, expected, 1);
    time_required = benchmarkQuery(
        session, query_post_with_sum, total_objects, total_objects - 1, expected, 1);

    std::cerr << std::endl;
  }
}

BOOST_AUTO_TEST_SUITE_END()
