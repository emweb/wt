/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/WDateTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/QueryModel>

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

  boost::posix_time::ptime start
    = boost::posix_time::microsec_clock::local_time();

  typedef boost::tuple<dbo::ptr<Perf2::Post>, double> PostWithSum;

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
  for (unsigned i = 0; i < total_objects; i++) {
    boost::tie(post, sum) = model.resultRow(i);
    total_sum += sum;
  }

  boost::posix_time::ptime
    end = boost::posix_time::microsec_clock::local_time();

  boost::posix_time::time_duration d = end - start;

  std::cerr << (double)d.total_microseconds() / total_objects
            << " microseconds per object" << std::endl;

  BOOST_REQUIRE(total_sum == expected);

  return d.total_seconds();
}

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
                  "FROM \"post\" \"p2\" WHERE \"p2\".\"id\" <= \"p1\".\"id\") "
      "FROM \"post\" \"p1\" "
      "WHERE (SELECT sum(\"p2\".\"counter\") * (avg(\"p2\".\"counter\")) \"sum_total\" "
                  "FROM \"post\" \"p2\" WHERE \"p2\".\"id\" <= \"p1\".\"id\") > 0";

  long time_required = 0L;
  long expected;
  int current_objects = 0;
  for (int i = start_total_objects;
      (i <= total_added_objects) && ((time_required * 6) < benchmark_time_limit); i *= 2) {
    expected = i * (i + 1) / 2;
    int total_objects = i;

    dbo::Transaction t(session);

    std::cerr << "Loading " << total_objects << " objects in database."
              << std::endl;

    for (unsigned i = 0; i < total_objects - current_objects; ++i) {
      Perf2::Post *p = new Perf2::Post();
      p->counter = 1;
      session.add(p);
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
