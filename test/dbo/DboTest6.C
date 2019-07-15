/*
 * Copyright (C) 2019 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>

#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Serializer>
#include <Wt/Json/Value>

#include "DboFixture.h"

#ifdef POSTGRES

class Emails {
public:
  std::string emails;

  template<typename Action>
  void persist(Action &a) {
    Wt::Dbo::field(a, emails, "emails");
  }
};

struct Dbo6Fixture : DboFixtureBase
{
  Dbo6Fixture()
  {
    session_->mapClass<Emails>("table_emails");

    try {
      session_->dropTables();
    } catch (...) {
    }

    dbo::Transaction transaction(*session_);
    session_->execute("CREATE TABLE \"table_emails\" ("
                      "" "\"id\" bigserial,"
                      "" "\"version\" integer,"
                      "" "\"emails\" json"
                      ")");
  }
};

BOOST_AUTO_TEST_CASE( dbo6_test1 )
{
  Dbo6Fixture f;

  dbo::Session& session = *f.session_;

  {
    Wt::Json::Object o;
    Wt::Json::Array ar;
    ar.push_back(Wt::Json::Value("test@example.com"));
    o["emails"] = ar;

    std::string const emailsStr = Wt::Json::serialize(o);

    dbo::Transaction transaction(session);
    Emails *emails = new Emails();
    Wt::Dbo::ptr<Emails> e = session.add(emails);
    e.modify()->emails = emailsStr;
  }

  {
    dbo::Transaction transaction(session);
    Wt::Dbo::ptr<Emails> result = session.query<Wt::Dbo::ptr<Emails> >("SELECT e FROM \"table_emails\" e WHERE \"emails\"->'emails' ?? ?").bind("test@example.com");

    Wt::Json::Object o;
    Wt::Json::parse(result->emails, o);
    Wt::Json::Array ar = o["emails"];

    BOOST_REQUIRE(ar[0] == "test@example.com");
  }

  {
    dbo::Transaction transaction(session);
    Wt::Dbo::ptr<Emails> result = session.query<Wt::Dbo::ptr<Emails> >("SELECT e FROM \"table_emails\" e WHERE \"emails\"->'emails' ?? ?").bind("test2@example.com");

    BOOST_REQUIRE(!result);
  }
}

#endif // POSTGRES
