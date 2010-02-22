// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>

#include "DboTest.h"

namespace dbo = Wt::Dbo;

class A;
class B;
class C;

typedef dbo::collection<dbo::ptr<A> > As;
typedef dbo::collection<dbo::ptr<B> > Bs;
typedef dbo::collection<dbo::ptr<C> > Cs;

class A : public dbo::Dbo {
public:
  dbo::ptr<B> b;
  dbo::ptr<A> parent;

  Wt::WDateTime  date;
  Wt::WString    wstring;
  std::string    string;
  int            i;
  float          f;
  double         d;

  bool operator== (const A& other) const {
    return date == other.date
      && wstring == other.wstring
      && string == other.string
      && i == other.i
      && b == other.b
      && f == other.f
      && d == other.d
      && parent == other.parent;
  }

  As             asManyToOne;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, date, "date");
    dbo::field(a, wstring, "wstring");
    dbo::field(a, string, "string");
    dbo::field(a, i, "i");
    dbo::field(a, f, "f");
    dbo::field(a, d, "d");

    dbo::belongsTo(a, b, "b");

    dbo::belongsTo(a, parent, "a_parent");
    dbo::hasMany(a, asManyToOne, dbo::ManyToOne, "a_parent");
  }
};

class B {
public:
  enum State {
    State1 = 0,
    State2 = 1
  };

  std::string name;
  State state;

  As    asManyToOne;
  Cs    csManyToMany;

  B() { }

  B(const std::string& aName, State aState)
    : name(aName), state(aState)
  { }

  bool operator== (const B& other) const {
    return name == other.name
      && state == other.state;
  }

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, state, "state");
    dbo::field(a, name, "name");

    dbo::hasMany(a, asManyToOne, dbo::ManyToOne,  "b");
    dbo::hasMany(a, csManyToMany, dbo::ManyToMany, "b_c", "the_b");
  }
};

class C {
public:
  std::string name;
  
  Bs    bsManyToMany;

  C() { }

  C(const std::string& aName)
    : name(aName)
  { }

  bool operator== (const C& other) const {
    return name == other.name;
  }

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");

    dbo::hasMany(a, bsManyToMany, dbo::ManyToMany, "b_c", "the_c");
  }
};

void DboTest::setup()
{
  connection_ = new dbo::backend::Sqlite3(":memory:");

  session_ = new dbo::Session();
  session_->setConnection(*connection_);

  session_->mapClass<A>("table_a");
  session_->mapClass<B>("table_b");
  session_->mapClass<C>("table_c");

  session_->createTables();
}

void DboTest::teardown()
{
  delete session_;
  delete connection_;
}

void DboTest::test1()
{
  setup();

  A a1;
  a1.date = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
  a1.wstring = "Hello";
  a1.string = "There";
  a1.i = 42;
  a1.f = 42.42;
  a1.d = 42.424242;

  /* Create an A, check that it is found during the same transaction  */
  {
    dbo::Transaction t(*session_);
    dbo::ptr<A> ptrA = session_->add(new A(a1));

    BOOST_REQUIRE(ptrA->session() == session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    BOOST_REQUIRE(*a2 == a1);

    t.commit();

  }

  /* Check that A is found during other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();
    BOOST_REQUIRE(*a2 == a1);

    t.commit();
  }

  /* Remove the A, check it is no longer found during the same transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 1);
    dbo::ptr<A> a2 = *allAs.begin();

    a2.remove();

    BOOST_REQUIRE(allAs.size() == 0);

    t.commit();
  }

  /* Check it is no longer found during other transaction */
  {
    dbo::Transaction t(*session_);

    As allAs = session_->find<A>();
    BOOST_REQUIRE(allAs.size() == 0);

    t.commit();
  }

  teardown();
}

void DboTest::test2()
{
  setup();

  {
    A a1;
    a1.date = Wt::WDateTime(Wt::WDate(2009, 10, 1), Wt::WTime(12, 11, 31));
    a1.wstring = "Hello";
    a1.string = "There";
    a1.i = 42;

    B b1;
    b1.name = "b1";
    b1.state = B::State1;

    /* Create an A + B  */
    {
      dbo::Transaction t(*session_);
      a1.b = session_->add(new B(b1));
      dbo::ptr<A> a = session_->add(new A(a1));

      As allAs = session_->find<A>();
      BOOST_REQUIRE(allAs.size() == 1);
      dbo::ptr<A> a2 = *allAs.begin();
      BOOST_REQUIRE(*a2 == a1);

      t.commit();
    }

    /* Check that A + B are found in other transaction */
    {
      dbo::Transaction t(*session_);

      As allAs = session_->find<A>();
      BOOST_REQUIRE(allAs.size() == 1);
      dbo::ptr<A> a2 = *allAs.begin();
      BOOST_REQUIRE(*a2 == a1);
      BOOST_REQUIRE(*a2->b == b1);

      t.commit();
    }
  }

  teardown();
}

void DboTest::test3()
{
  setup();

  {
    /* Create B's many-to-many C's  */
    {
      dbo::Transaction t(*session_);

      dbo::ptr<B> b1 = session_->add(new B("b1", B::State1));
      dbo::ptr<B> b2 = session_->add(new B("b2", B::State2));
      dbo::ptr<B> b3 = session_->add(new B("b3", B::State1));

      dbo::ptr<C> c1 = session_->add(new C("c1"));
      dbo::ptr<C> c2 = session_->add(new C("c2"));
      dbo::ptr<C> c3 = session_->add(new C("c3"));

      b1.modify()->csManyToMany.insert(c1);

      BOOST_REQUIRE(b1->csManyToMany.size() == 1);
      BOOST_REQUIRE(c1->bsManyToMany.size() == 1);

      b1.modify()->csManyToMany.insert(c2);

      BOOST_REQUIRE(b1->csManyToMany.size() == 2);
      BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
      BOOST_REQUIRE(c2->bsManyToMany.size() == 1);
      BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

      b1.modify()->csManyToMany.erase(c2);

      BOOST_REQUIRE(b1->csManyToMany.size() == 1);
      BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
      BOOST_REQUIRE(c2->bsManyToMany.size() == 0);
      BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

      b1.modify()->csManyToMany.insert(c2);
      b1.modify()->csManyToMany.erase(c2);

      BOOST_REQUIRE(b1->csManyToMany.size() == 1);
      BOOST_REQUIRE(c1->bsManyToMany.size() == 1);
      BOOST_REQUIRE(c2->bsManyToMany.size() == 0);
      BOOST_REQUIRE(c3->bsManyToMany.size() == 0);

      t.commit();
    }

    {
      dbo::Transaction t(*session_);

      dbo::ptr<B> b1 = session_->query< dbo::ptr<B> >
	("select B from table_b B where B.name = ?").bind("b1");

      dbo::ptr<C> c1 = session_->find<C>("where name = ?").bind("c1");

      BOOST_REQUIRE(b1->csManyToMany.size() == 1);
      BOOST_REQUIRE(c1->bsManyToMany.size() == 1);

      t.commit();
    }
  }

  teardown();
}

void DboTest::test4()
{
  setup();

  {
    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a1(new A());
      a1.modify()->date = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					Wt::WTime(12, 11, 31));
      a1.modify()->wstring = "Hello";
      a1.modify()->string = "There";
      a1.modify()->i = 42;

      dbo::ptr<A> a2(new A(*a1));
      a2.modify()->wstring = "Oh my god";
      a2.modify()->i = 142;

      dbo::ptr<B> b(new B());
      b.modify()->name = "b";
      b.modify()->state = B::State1;

      a1.modify()->b = b;
      a2.modify()->b = b;

      session_->add(a1);
      session_->add(a2);
      session_->add(b);

      typedef dbo::ptr_tuple<B, A>::type BA;
      typedef dbo::collection<BA> BAs;

      BAs bas = session_->query<BA>
	("select B, A from table_b B join table_a A on A.b_id = B.id "
	 "order by A.i");

      BOOST_REQUIRE(bas.size() == 2);

      int ii = 0;
      for (BAs::const_iterator i = bas.begin(); i != bas.end(); ++i) {
	dbo::ptr<A> a_result;
	dbo::ptr<B> b_result;
	boost::tie(b_result, a_result) = *i;

	if (ii == 0) {
	  BOOST_REQUIRE(a_result == a1);
	  BOOST_REQUIRE(b_result == b);
	} else if (ii == 1) {
	  BOOST_REQUIRE(a_result == a2);
	  BOOST_REQUIRE(b_result == b);
	}

	++ii;
      }

      BOOST_REQUIRE(ii == 2);

      t.commit();
    }
  }

  teardown();
}

void DboTest::test5()
{
  setup();

  {
    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a1(new A());
      a1.modify()->date = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					Wt::WTime(12, 11, 31));
      a1.modify()->wstring = "Hello";
      a1.modify()->string = "There";
      a1.modify()->i = 42;

      dbo::ptr<A> a2(new A(*a1));
      a2.modify()->wstring = "Oh my god";
      a2.modify()->i = 142;

      dbo::ptr<B> b(new B());
      b.modify()->name = "b";
      b.modify()->state = B::State1;

      a1.modify()->b = b;
      a2.modify()->b = b;

      session_->add(a1);
      session_->add(a2);
      session_->add(b);

      session_->flush();

      t.rollback();

      a1.remove();
      a2.remove();
      b.remove();
    }

    {
      dbo::Transaction t(*session_);

      t.commit();
    }
  }

  teardown();
}

void DboTest::test6()
{
  setup();

  {
    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a1(new A());
      a1.modify()->date = Wt::WDateTime(Wt::WDate(2009, 10, 1),
					Wt::WTime(12, 11, 31));
      a1.modify()->wstring = "Hello";
      a1.modify()->string = "There";
      a1.modify()->i = 42;

      session_->add(a1);

      t.commit();
    }

    {
      dbo::Transaction t(*session_);

      dbo::ptr<A> a1 = session_->find<A>();
      a1.modify()->i = 41;
      a1.flush();
      t.rollback();

      a1.reread();

      dbo::Transaction t2(*session_);

      BOOST_REQUIRE(a1->i == 42);

      t2.commit();
    }
  }

  teardown();
}

DboTest::DboTest()
  : test_suite("dbotest_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test1, this)));
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test2, this)));
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test3, this)));
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test4, this)));
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test5, this)));
  add(BOOST_TEST_CASE(boost::bind(&DboTest::test6, this)));
}
