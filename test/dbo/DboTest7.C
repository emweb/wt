/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>

#include "DboFixture.h"

#ifdef MYSQL
using ResultType = int;
#else
using ResultType = double;
#endif

namespace Wt {
namespace Dbo {

template<>
struct query_result_traits<std::vector<ResultType>>
{
  static void getFields(Session &session,
                        std::vector<std::string> *aliases,
                        std::vector<FieldInfo> &result)
  {
    if (!aliases)
      throw std::logic_error("Session::query(): not enough aliases for results");
    while (aliases->size() > 0)
      query_result_traits<ResultType>::getFields(session, aliases, result);
  }

  static std::vector<ResultType> load(Session &session,
                                  SqlStatement &statement,
                                  int &column)
  {
    std::vector<ResultType> result;
    for (int i = 0; i < statement.columnCount(); ++i)
      result.push_back(query_result_traits<ResultType>::load(session, statement, column));
    return result;
  }

  static void getValues(const std::vector<ResultType> &result,
                        std::vector<cpp17::any> &values)
  {
    for (ResultType value : result)
      values.push_back(value);
  }

  // no setValue

  static std::vector<ResultType> create()
  {
    return std::vector<ResultType>();
  }

  // no add

  // no remove

  static long long id(const std::vector<ResultType> &result)
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

    auto query = session_->query<std::vector<ResultType>>("select 1, 2, 3, 4"
#ifdef FIREBIRD
        " from RDB$DATABASE"
#endif
        );
    auto result = query.resultValue();

    BOOST_CHECK_EQUAL(result.size(), 4);

    BOOST_CHECK_EQUAL(result[0], 1);
    BOOST_CHECK_EQUAL(result[1], 2);
    BOOST_CHECK_EQUAL(result[2], 3);
    BOOST_CHECK_EQUAL(result[3], 4);
  }
}
