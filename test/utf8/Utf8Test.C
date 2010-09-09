// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include "Utf8Test.h"

#include <Wt/WString>
// #include <iostream>

void Utf8Test::test()
{
  std::wstring w = L"This costs 100\x20AC (greek \x0194 special \x103A7)";
  Wt::WString ws = w;
  // std::cerr << ws.toUTF8() << std::endl;
  BOOST_REQUIRE(ws.value() == w);
  BOOST_REQUIRE(ws.toUTF8().length() == w.length() + 2 + 1 + 3);
}

Utf8Test::Utf8Test()
  : test_suite("utf8_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&Utf8Test::test, this)));
}
