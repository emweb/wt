// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WObject.h>

class MyObject : public Wt::WObject
{
public:
  MyObject() { }

  void foo() { 
    result = 1;
  }

  void fooc() const { 
    result = 1;
  }

  void bar(int v) { 
    result = v;
  }

  static int result;
};

int MyObject::result = 0;

BOOST_AUTO_TEST_CASE( test_bind1 )
{
  std::unique_ptr<MyObject> o(new MyObject);

  auto b1 = o->bindSafe(&MyObject::foo);
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 1);

  o.reset();
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 0);
}

BOOST_AUTO_TEST_CASE( test_bind2 )
{
  std::unique_ptr<MyObject> o(new MyObject);

  MyObject *oo = o.get();
  auto b1 = o->bindSafe([oo]() { oo->foo(); });
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 1);

  o.reset();
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 0);
}

BOOST_AUTO_TEST_CASE( test_bind3 )
{
  std::unique_ptr<MyObject> o(new MyObject);

  auto b1 = o->bindSafe(&MyObject::bar);
  MyObject::result = 0;
  b1(42);
  BOOST_REQUIRE(MyObject::result == 42);

  o.reset();
  MyObject::result = 0;
  b1(42);
  BOOST_REQUIRE(MyObject::result == 0);
}

BOOST_AUTO_TEST_CASE( test_bind4 )
{
  std::unique_ptr<MyObject> o(new MyObject);

  MyObject *oo = o.get();
  auto b1 = o->bindSafe([oo](int v) { oo->bar(v); });
  MyObject::result = 0;
  b1(42);
  BOOST_REQUIRE(MyObject::result == 42);

  o.reset();
  MyObject::result = 0;
  b1(42);
  BOOST_REQUIRE(MyObject::result == 0);
}

BOOST_AUTO_TEST_CASE( test_bind5 )
{
  std::unique_ptr<MyObject> o(new MyObject);
  const MyObject *co = o.get();

  auto b1 = co->bindSafe(&MyObject::fooc);
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 1);

  o.reset();
  MyObject::result = 0;
  b1();
  BOOST_REQUIRE(MyObject::result == 0);
}
