/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>

#include "DboFixture.h"

#include <cmath>

#include <string>
namespace dbo = Wt::Dbo;

class CustomerAddress;

class Customer
{
public:

  Wt::Dbo::collection< Wt::Dbo::ptr<CustomerAddress> > addresses;
  Wt::Dbo::ptr<CustomerAddress> default_address;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::hasMany(action, addresses, Wt::Dbo::ManyToOne);
    Wt::Dbo::belongsTo(action, default_address, "default_customer_address");
  }
};


class CustomerAddress
{
public:
  Wt::Dbo::ptr<Customer> customer;
  //Wt::Dbo::collection< Wt::Dbo::ptr<Customer> > customers;
  Wt::Dbo::weak_ptr<Customer> default_customer;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::belongsTo(action, customer);
    //Wt::Dbo::hasMany(action, customers, Wt::Dbo::ManyToOne, "default_customer_address");
    Wt::Dbo::hasOne(action, default_customer, "default_customer_address");

  }
};


class FuncTest : public Wt::Dbo::Dbo<FuncTest>
{
public:
        int intC;
        double doubleC;

        template<class Action>
        void persist(Action &a)
        {
                Wt::Dbo::field(a, intC, "intC");
                Wt::Dbo::field(a, doubleC, "doubleC");
        }
        static const char *TableName()
        {
                return "func";
        }
};

struct Dbo3Fixture : DboFixtureBase
{
  Dbo3Fixture() :
    DboFixtureBase()
  {
    session_->mapClass<Customer>("c");
    session_->mapClass<CustomerAddress>("ca");
    session_->mapClass<FuncTest>(FuncTest::TableName());

    try {
      session_->dropTables(); //todo:remove
    } catch (...) {
    }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;

    session_->createTables();
  }
};

BOOST_AUTO_TEST_CASE( dbo3_test2 )
{
  Dbo3Fixture f;

  dbo::Session& session = *f.session_;
  {
    dbo::Transaction transaction(session);
    auto f1 = std::make_unique<FuncTest>();
    f1->intC = 1;
    f1->doubleC = 1.1;
    session.add(std::move(f1));

    auto f2 = std::make_unique<FuncTest>();
    f2->intC = 2;
    f2->doubleC = 2.2;
    session.add(std::move(f2));

    auto f3 = std::make_unique<FuncTest>();
    f3->intC = 3;
    f3->doubleC = 3.3;
    session.add(std::move(f3));

    transaction.commit();
  }

  typedef std::tuple<int, double> tupel;

  dbo::Transaction transaction(session);

  tupel tupe;

#ifdef SQLITE3
  /*
   * Odd enough Sqlite3 thinks this is legal SQL, but postgres
   * complains (rightfully) that you can't combine aggregate and
   * non-aggregate fields.
   */
  tupe = session.query<tupel>(std::string(
          "SELECT SUM(\"intC\") as sc, \"doubleC\" FROM \"") +
			      FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(std::get<0>(tupe) == 6);
#endif

  tupe = session.query<tupel>(std::string(
          "SELECT SUM(\"intC\") as sc, SUM(\"doubleC\") as dc FROM \"") +
           FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(std::get<0>(tupe) == 6);

  // NOTE: can fail w/valgrind due to precision of emulated floating point
  BOOST_REQUIRE(std::abs(std::get<1>(tupe) - 6.6) < 0.001);

  double d = session.query<double>(std::string(
          "SELECT SUM(\"doubleC\" * \"intC\") as sc FROM \"") +
           FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(std::abs(d - 15.4) < 0.001);//d == 15.4

  d = session.query<double>(std::string(
          "SELECT MAX(\"doubleC\" * \"intC\") as m FROM \"") +
           FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(std::abs(d - 9.9) < 0.001);

  double i = session.query<double>(std::string(
          "SELECT AVG(\"intC\") as a FROM \"") +
          FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(i == 2.0);

  i = session.query<int>(std::string(
          "SELECT COUNT(\"intC\") as c FROM \"") +
          FuncTest::TableName() + "\"").limit(1);

  BOOST_REQUIRE(i == 3);

  transaction.commit();
}
