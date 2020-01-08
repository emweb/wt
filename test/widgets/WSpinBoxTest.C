/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WSpinBox.h>
#include <Wt/Test/WTestEnvironment.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( spinbox_validate )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  {
    auto sb = cpp14::make_unique<WSpinBox>();
    sb->setRange(100, 200);
    sb->setValue(150);
    ValidationState result = sb->validate();
    BOOST_REQUIRE(result == ValidationState::Valid);
  }
}
