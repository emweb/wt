/*
 * Copyright (C) 2019 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>

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
    session_->execute("CREATE EXTENSION IF NOT EXISTS hstore");
    session_->execute("CREATE TABLE \"table_emails\" ("
                      "" "\"id\" bigserial,"
                      "" "\"version\" integer,"
                      "" "\"emails\" hstore"
                      ")");
  }
};

BOOST_AUTO_TEST_CASE( dbo6_test1 )
{
  Dbo6Fixture f;

  dbo::Session& session = *f.session_;

  {
    char const * const emailsStr = "test => test@example.com";

    dbo::Transaction transaction(session);
    Wt::Dbo::ptr<Emails> e = session.addNew<Emails>();
    e.modify()->emails = emailsStr;
  }

  {
    dbo::Transaction transaction(session);
    Wt::Dbo::ptr<Emails> result = session.query<Wt::Dbo::ptr<Emails> >("SELECT e FROM \"table_emails\" e WHERE \"emails\" ?? ?").bind("test");

    BOOST_REQUIRE(result);
  }

  {
    dbo::Transaction transaction(session);
    Wt::Dbo::ptr<Emails> result = session.query<Wt::Dbo::ptr<Emails> >("SELECT e FROM \"table_emails\" e WHERE \"emails\" ?? ?").bind("test2");

    BOOST_REQUIRE(!result);
  }
}

#endif // POSTGRES
