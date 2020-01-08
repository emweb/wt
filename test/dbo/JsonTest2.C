/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifdef SQLITE3

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "DboFixture.h"

namespace dbo = Wt::Dbo;

namespace JsonDboTest2 {

class HasJson {
public:
  std::string name;
  Wt::Json::Object object;
  Wt::Json::Array array;

  template <class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
    dbo::field(a, object, "object");
    dbo::field(a, array, "array");
  }
};

struct DboFixture : DboFixtureBase
{
  DboFixture() :
    DboFixtureBase()
  {
    session_->mapClass<HasJson>("has_json");

    try {
      session_->dropTables();
    } catch (...) {
    }

    //session_->dropTables();

    session_->createTables();
  }
};

BOOST_AUTO_TEST_CASE( dbo_json_sql_traits )
{
  DboFixture f;
  dbo::Session& session = *f.session_;
  dbo::Transaction transaction(session);

  auto hj = std::unique_ptr<HasJson>(new HasJson());
  hj->name = "john";
  session.add(std::move(hj));
  dbo::ptr<HasJson> hjptr = session
    .find<HasJson>("where \"name\" = ?")
    .bind("john");

  BOOST_REQUIRE(hjptr->object == Wt::Json::Object::Empty);
  BOOST_REQUIRE(hjptr->array == Wt::Json::Array::Empty);

  hjptr.modify()->array.push_back(1);
  BOOST_REQUIRE(hjptr->array[0] == Wt::Json::Value(1));

  Wt::Json::Object obj;
  obj["attribute"] = "value";
  hjptr.modify()->array.push_back(obj);
  hjptr.modify()->object = obj;
  BOOST_REQUIRE(hjptr->object.get("attribute") == "value");
  BOOST_REQUIRE(hjptr->object == Wt::Json::Object(hjptr->array.at(1)));

  hjptr.modify()->object["a2"] = "v2";
  BOOST_REQUIRE(hjptr->object != Wt::Json::Object(hjptr->array.at(1)));


}

}

#endif
