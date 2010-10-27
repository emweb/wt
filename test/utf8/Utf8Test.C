// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include "Utf8Test.h"

#include <Wt/WString>
#include <iostream>

void Utf8Test::test()
{
#ifndef WIN32
  std::wstring w = L"This costs 100\x20AC (greek \x0194 special \x103A7)";
  Wt::WString ws = w;
  // std::cerr << ws.toUTF8() << std::endl;
  BOOST_REQUIRE(ws.value() == w);
  BOOST_REQUIRE(ws.toUTF8().length() == w.length() + 2 + 1 + 3);
#else
  std::wstring w = L"This costs 100\x20AC (greek \x0194)";
  Wt::WString ws = w;
  // std::cerr << ws.toUTF8() << std::endl;
  BOOST_REQUIRE(ws.value() == w);
  BOOST_REQUIRE(ws.toUTF8().length() == w.length() + 2 + 1);
#endif
}

void Utf8Test::test2()
{
  std::wstring w = L"This costs 100\x20AC (greek \x0194)";

  Wt::WString ws = w;
  std::string s = ws.narrow();

  // The following will work only if locale is classic (not UTF8)
  BOOST_REQUIRE(s == "This costs 100? (greek ?)");
}

void Utf8Test::test3()
{
  std::wstring w = L"\x20AC\x20AC\x20AC\x20AC (greek \x0194)";

  Wt::WString ws = w;

  std::locale l(std::locale("C"), std::locale(""),
		std::locale::collate | std::locale::ctype);

  std::string s = ws.narrow(l);

  BOOST_REQUIRE(s == ws.toUTF8());
}
 

Utf8Test::Utf8Test()
  : test_suite("utf8_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&Utf8Test::test, this)));
  add(BOOST_TEST_CASE(boost::bind(&Utf8Test::test2, this)));
  add(BOOST_TEST_CASE(boost::bind(&Utf8Test::test3, this)));
}
