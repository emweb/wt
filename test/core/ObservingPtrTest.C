// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Core/observable.hpp>
#include <Wt/Core/observing_ptr.hpp>

#include <type_traits>

using observable = Wt::Core::observable;

template<typename T>
using observing_ptr = Wt::Core::observing_ptr<T>;

struct A : observable {
};

struct B : A {
};

BOOST_AUTO_TEST_CASE( test_observing_ptr1 )
{
  observing_ptr<A> p;

  BOOST_REQUIRE(!p);
  BOOST_REQUIRE(p.get() == nullptr);
  BOOST_REQUIRE(!p.observedDeleted());

  {
    A a;
    p = &a;

    BOOST_REQUIRE(p);
    BOOST_REQUIRE(p.get() == &a);
    BOOST_REQUIRE(!p.observedDeleted());
  }

  BOOST_REQUIRE(!p);
  BOOST_REQUIRE(p.get() == nullptr);
  BOOST_REQUIRE(p.observedDeleted());

  p = nullptr;

  BOOST_REQUIRE(!p);
  BOOST_REQUIRE(p.get() == nullptr);
  BOOST_REQUIRE(!p.observedDeleted());

  static_assert(std::is_nothrow_move_constructible<observing_ptr<A>>::value,
                "observing_ptr<A> should currently not be move constructible");
  static_assert(std::is_nothrow_move_assignable<observing_ptr<A>>::value,
                "observing_ptr<A> should currently not be move assignable");
}

BOOST_AUTO_TEST_CASE( test_observing_ptr2 )
{
  observing_ptr<A> p_a;
  observing_ptr<B> p_b;

  BOOST_REQUIRE(!p_a.observedDeleted());
  BOOST_REQUIRE(!p_b.observedDeleted());

  {
    B b;
    p_b = &b;
    p_a = p_b;

    BOOST_REQUIRE(p_a);
    BOOST_REQUIRE(p_b);

    BOOST_REQUIRE(p_a.get() == &b);
    BOOST_REQUIRE(p_b.get() == &b);

    BOOST_REQUIRE(!p_a.observedDeleted());
    BOOST_REQUIRE(!p_b.observedDeleted());
  }

  BOOST_REQUIRE(p_a.get() == nullptr);
  BOOST_REQUIRE(p_b.get() == nullptr);

  BOOST_REQUIRE(p_a.observedDeleted());
  BOOST_REQUIRE(p_b.observedDeleted());

  static_assert(std::is_convertible<B*,A*>::value,
                "B* should be convertible to A*");
  static_assert(std::is_convertible<observing_ptr<B>,observing_ptr<A>>::value,
                "observing_ptr<Derived> should be convertible to observing_ptr<Base>");
  static_assert(!std::is_convertible<observing_ptr<A>,observing_ptr<B>>::value,
                "observing_ptr<Base> should not be convertible to observing_ptr<Derived>");
}

BOOST_AUTO_TEST_CASE( test_observing_ptr3 )
{
  observing_ptr<A> p;

  A a1;
  {
    A a2;
    p = &a2; // First, make it point to a2

    p = &a1; // Then, make it point to a1

    // a2 goes away
  }

  BOOST_REQUIRE(!p.observedDeleted());
  BOOST_REQUIRE(p.get() == &a1);
}
