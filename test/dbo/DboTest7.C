/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>

#include "DboFixture.h"

namespace Wt {
namespace Dbo {

template<>
struct query_result_traits<std::vector<double>>
{
  static void getFields(Session &session,
                        std::vector<std::string> *aliases,
                        std::vector<FieldInfo> &result)
  {
    if (!aliases)
      throw std::logic_error("Session::query(): not enough aliases for results");
    while (aliases->size() > 0)
      query_result_traits<double>::getFields(session, aliases, result);
  }

  static std::vector<double> load(Session &session,
                                  SqlStatement &statement,
                                  int &column)
  {
    std::vector<double> result;
    for (int i = 0; i < statement.columnCount(); ++i)
      result.push_back(query_result_traits<double>::load(session, statement, column));
    return result;
  }

  static void getValues(const std::vector<double> &result,
                        std::vector<cpp17::any> &values)
  {
    for (double value : result)
      values.push_back(value);
  }

  // no setValue

  static std::vector<double> create()
  {
    return std::vector<double>();
  }

  // no add

  // no remove

  static long long id(const std::vector<double> &result)
  {
    (void) result;
    return -1;
  }

  // no findById
};

}
}

struct Dbo7Fixture : DboFixtureBase
{
  Dbo7Fixture()
  { }
};

BOOST_AUTO_TEST_CASE( dbo7_test1 )
{
  Dbo7Fixture f;
  dbo::Session *session_ = f.session_;

  {
    dbo::Transaction t(*session_);

    auto query = session_->query<std::vector<double>>("select 1, 2, 3, 4"
#ifdef FIREBIRD
        " from RDB$DATABASE"
#endif
        );
    auto result = query.resultValue();

    BOOST_REQUIRE(result.size() == 4);

    BOOST_REQUIRE(result[0] == 1);
    BOOST_REQUIRE(result[1] == 2);
    BOOST_REQUIRE(result[2] == 3);
    BOOST_REQUIRE(result[3] == 4);
  }
}
