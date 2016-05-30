/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// These tests get a bit weird semantically, try not to pay too much attention to that.

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>

#include "DboFixture.h"

#include <string>
namespace dbo = Wt::Dbo;

class Car;
class Door;
class Person;
class Shoe;

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<Person> : public dbo_default_traits {
      static const char *surrogateIdField() {
	return 0;
      }
    };

  }
}

class Person
{
public:
  int name;
  Wt::Dbo::collection< Wt::Dbo::ptr<Person> > friends1_side1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Person> > friends1_side2;
  Wt::Dbo::collection< Wt::Dbo::ptr<Person> > friends2_side1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Person> > friends2_side2;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > cars1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > cars2;
  Wt::Dbo::weak_ptr<Shoe> leftShoe;
  Wt::Dbo::weak_ptr<Shoe> rightShoe;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::id(action, name, "name");
    Wt::Dbo::hasMany(action, friends1_side1, Wt::Dbo::ManyToMany, "friends1", "side1");
    Wt::Dbo::hasMany(action, friends1_side2, Wt::Dbo::ManyToMany, "friends1", "side2");
    Wt::Dbo::hasMany(action, friends2_side1, Wt::Dbo::ManyToMany, "friends2", ">side1");
    Wt::Dbo::hasMany(action, friends2_side2, Wt::Dbo::ManyToMany, "friends2", ">side2");
    Wt::Dbo::hasMany(action, cars1, Wt::Dbo::ManyToOne, "owner1");
    Wt::Dbo::hasMany(action, cars2, Wt::Dbo::ManyToOne, ">owner2");
    Wt::Dbo::hasOne(action, leftShoe, "owner1");
    Wt::Dbo::hasOne(action, rightShoe, ">owner2");
  }
};

class Car
{
public:
  Wt::Dbo::ptr<Person> owner1;
  Wt::Dbo::ptr<Person> owner2;
  Wt::Dbo::collection< Wt::Dbo::ptr<Door> > doors1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Door> > doors2;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > friends1_side1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > friends1_side2;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > friends2_side1;
  Wt::Dbo::collection< Wt::Dbo::ptr<Car> > friends2_side2;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::belongsTo(action, owner1, "owner1");
    Wt::Dbo::belongsTo(action, owner2, ">owner2");
    Wt::Dbo::hasMany(action, doors1, Wt::Dbo::ManyToOne, "car1");
    Wt::Dbo::hasMany(action, doors2, Wt::Dbo::ManyToOne, ">car2");
    Wt::Dbo::hasMany(action, friends1_side1, Wt::Dbo::ManyToMany, "car_friends1", "side1");
    Wt::Dbo::hasMany(action, friends1_side2, Wt::Dbo::ManyToMany, "car_friends1", "side2");
    Wt::Dbo::hasMany(action, friends2_side1, Wt::Dbo::ManyToMany, "car_friends2", ">side1");
    Wt::Dbo::hasMany(action, friends2_side2, Wt::Dbo::ManyToMany, "car_friends2", ">side2");
  }
};

class Door
{
public:
  Wt::WString color;
  Wt::Dbo::ptr<Car> car1;
  Wt::Dbo::ptr<Car> car2;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::field(action, color, "color");
    Wt::Dbo::belongsTo(action, car1, "car1");
    Wt::Dbo::belongsTo(action, car2, ">car2");
  }
};

class Shoe
{
public:
  Wt::Dbo::ptr<Person> owner1;
  Wt::Dbo::ptr<Person> owner2;

  template<class Action>
  void persist(Action &action)
  {
    Wt::Dbo::belongsTo(action, owner1, "owner1");
    Wt::Dbo::belongsTo(action, owner2, ">owner2");
  }
};


struct Dbo4Fixture : DboFixtureBase
{
  Dbo4Fixture() :
    DboFixtureBase()
  {
    session_->mapClass<Person>("p");
    session_->mapClass<Car>("c");
    session_->mapClass<Door>("d");
    session_->mapClass<Shoe>("s");

    try {
      session_->dropTables(); //todo:remove
    } catch (...) {
    }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;

    session_->createTables();
  }
};

BOOST_AUTO_TEST_CASE( dbo4_test1 )
{
  Dbo4Fixture f;

  dbo::Session& session = *f.session_;
  {
    dbo::Transaction transaction(session);
    Car *c = new Car;
    session.add(c);
    Shoe *s = new Shoe;
    session.add(s);
    Door *d = new Door;
    session.add(d);
    Person *p = new Person;
    session.add(p);
    transaction.commit();
  }

  {
    std::vector<dbo::FieldInfo> fields = session.find<Car>().fields();
    BOOST_REQUIRE(fields.size() == 4);
    BOOST_REQUIRE(fields[0].name() == "id");
    BOOST_REQUIRE(fields[1].name() == "version");
    BOOST_REQUIRE(fields[2].name() == "owner1_name");
    BOOST_REQUIRE(fields[3].name() == "owner2");
  }

  {
    std::vector<dbo::FieldInfo> fields = session.find<Door>().fields();
    BOOST_REQUIRE(fields.size() == 5);
    BOOST_REQUIRE(fields[0].name() == "id");
    BOOST_REQUIRE(fields[1].name() == "version");
    BOOST_REQUIRE(fields[2].name() == "color");
    BOOST_REQUIRE(fields[3].name() == "car1_id");
    BOOST_REQUIRE(fields[4].name() == "car2");
  }

  {
    std::vector<dbo::FieldInfo> fields = session.find<Person>().fields();
    BOOST_REQUIRE(fields.size() == 2);
    BOOST_REQUIRE(fields[0].name() == "version");
    BOOST_REQUIRE(fields[1].name() == "name");
  }

  {
    std::vector<dbo::FieldInfo> fields = session.find<Shoe>().fields();
    BOOST_REQUIRE(fields.size() == 4);
    BOOST_REQUIRE(fields[0].name() == "id");
    BOOST_REQUIRE(fields[1].name() == "version");
    BOOST_REQUIRE(fields[2].name() == "owner1_name");
    BOOST_REQUIRE(fields[3].name() == "owner2");
  }

  // None of the following should throw!
  Wt::Dbo::SqlConnection *connection = f.connectionPool_->getConnection();
  connection->executeSql("SELECT side1_name,side2_name FROM \"friends1\"");
  connection->executeSql("SELECT side1,side2 FROM \"friends2\"");
  connection->executeSql("SELECT side1_id,side2_id FROM \"car_friends1\"");
  connection->executeSql("SELECT side1,side2 FROM \"car_friends2\"");
  f.connectionPool_->returnConnection(connection);
}
