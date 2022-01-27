/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>

#include "DboFixture.h"

namespace dbo = Wt::Dbo;

class Painting
{
public:
  std::string name;
  std::string artist;
  int price;

  template<typename Action>
  void persist(Action &a)
  {
    dbo::field(a, name, "name");
    dbo::field(a, artist, "artist");
    dbo::field(a, price, "price");
  }
};

struct Dbo9Fixture : DboFixtureBase {
  Dbo9Fixture()
  {
    session_->mapClass<Painting>("painting");

    try {
      session_->dropTables();
    } catch (...) {
    }
    session_->createTables();
  }
};

BOOST_AUTO_TEST_SUITE( DBO_TEST_SUITE_NAME )

BOOST_AUTO_TEST_CASE( dbo9_test1_subquery )
{
  Dbo9Fixture f;
  dbo::Session &session = *f.session_;

  {
    dbo::Transaction t(session);

    dbo::ptr<Painting> p1 = session.addNew<Painting>();
    p1.modify()->name = "Zonnebloemen";
    p1.modify()->artist = "Van Gogh";
    p1.modify()->price = 1000;

    dbo::ptr<Painting> p2 = session.addNew<Painting>();
    p2.modify()->name = "De sterrennacht";
    p2.modify()->artist = "Van Gogh";
    p2.modify()->price = 5000;

    dbo::ptr<Painting> p3 = session.addNew<Painting>();
    p3.modify()->name = "Het melkmeisje";
    p3.modify()->artist = "Vermeer";
    p3.modify()->price = 2000;

    dbo::ptr<Painting> p4 = session.addNew<Painting>();
    p4.modify()->name = "Meisje met de parel";
    p4.modify()->artist = "Vermeer";
    p4.modify()->price = 3000;
  }

  {
    dbo::Transaction t(session);

    auto q = session.query<dbo::ptr<Painting>>(
      "select p from \"painting\" as p "
      "where p.price > (select MAX(p.price) from \"painting\" as p "
      "                 where p.artist = ?)")
      .bind("Vermeer");
    auto results = q.resultList();
    BOOST_TEST(results.size() == 1);
    auto result = results.front();
    BOOST_TEST(result->name == "De sterrennacht");
    BOOST_TEST(result->artist == "Van Gogh");
    BOOST_TEST(result->price == 5000);
  }

  {
    dbo::Transaction t(session);

    auto maxPriceQuery = session.query<int>("select MAX(p.price) from \"painting\" as p")
      .where("p.artist = ?").bind("Vermeer");
    auto q = session.query<dbo::ptr<Painting>>("select p from \"painting\" as p")
      .where("p.price > (" + maxPriceQuery.asString() + ")").bindSubqueryValues(maxPriceQuery);
    auto results = q.resultList();
    BOOST_TEST(results.size() == 1);
    auto result = results.front();
    BOOST_TEST(result->name == "De sterrennacht");
    BOOST_TEST(result->artist == "Van Gogh");
    BOOST_TEST(result->price == 5000);
  }
}

BOOST_AUTO_TEST_SUITE_END()
