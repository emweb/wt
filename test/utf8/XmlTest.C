// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/bind.hpp>
#include <iostream>
#include <Wt/WMessageResourceBundle>

#include "XmlTest.h"

void XmlTest::test()
{
  Wt::WMessageResourceBundle bundle;
  bundle.use("test");

  std::string result;

  if (!bundle.resolveKey("test1", result))
    return; // test.xml file not found

  BOOST_REQUIRE(bundle.resolveKey("test1", result));
  BOOST_REQUIRE(result == "<br/>");

  BOOST_REQUIRE(bundle.resolveKey("test2", result));
  BOOST_REQUIRE(result == "<div></div>");
}

XmlTest::XmlTest()
  : test_suite("xml_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&XmlTest::test, this)));
}
