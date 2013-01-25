/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Dbo/backend/MySQL>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Firebird>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>
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


struct Dbo3Fixture
{
  Dbo3Fixture()
  {
#ifdef SQLITE3
    connection_ = new dbo::backend::Sqlite3(":memory:");
#endif // SQLITE3

#ifdef POSTGRES
    connection_ = new dbo::backend::Postgres
        ("user=postgres_test password=postgres_test port=5432 dbname=wt_test");
#endif // POSTGRES

#ifdef MYSQL
    connection_ = new dbo::backend::MySQL("wt_test_db", "test_user",
                                          "test_pw", "localhost", 3306);
#endif // MYSQL
#ifdef FIREBIRD
    std::string file;
#ifdef WIN32
    file = "C:\\opt\\db\\firebird\\wt_test.fdb";
#else
    file = "/opt/db/firebird/wt_test.fdb";
#endif

    connection_ = new dbo::backend::Firebird ("localhost",
                                              file,
                                              "test_user", "test_pwd",
                                              "", "", "");
#endif // FIREBIRD

    connection_->setProperty("show-queries", "true");

    session_ = new dbo::Session();
    session_->setConnection(*connection_);

//    session_->mapClass<Customer>("customer");
//    session_->mapClass<CustomerAddress>("customer_address");

    //session_->createTables();
  }

  ~Dbo3Fixture()
  {
    session_->dropTables();

    delete session_;
    delete connection_;
  }

  dbo::SqlConnection *connection_;
  dbo::Session *session_;
};

BOOST_AUTO_TEST_CASE( dbo3_test1 )
{
  Dbo3Fixture f;

  dbo::Session& session = *f.session_;

  // Really short names required here to make firebird test
  // succeed (max 31 char identifiers)
  session.mapClass<Customer>("c");
  session.mapClass<CustomerAddress>("ca");

  session.createTables();
}
