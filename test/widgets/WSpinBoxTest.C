/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication>
#include <Wt/WSpinBox>
#include <Wt/Test/WTestEnvironment>

using namespace Wt;

BOOST_AUTO_TEST_CASE( spinbox_validate )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  {
    WSpinBox *sb = new WSpinBox();
    sb->setRange(100, 200);
    sb->setValue(150);
    WValidator::State result = sb->validate();
    BOOST_REQUIRE(result == WValidator::Valid);
  }
}
