/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( stackedwidget_currentIndex )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto stack = std::make_unique<Wt::WStackedWidget>();
  // No widgets yet, so currentWidget() returns nullptr, and currentIndex() is -1
  BOOST_REQUIRE_EQUAL(-1, stack->currentIndex());
  BOOST_REQUIRE_EQUAL(nullptr, stack->currentWidget());
  // Adding the first widget causes currentIndex() to change to 0
  auto text = stack->addNew<Wt::WText>(Wt::utf8("Text"));
  BOOST_REQUIRE_EQUAL(0, stack->currentIndex());
  BOOST_REQUIRE_EQUAL(text, stack->currentWidget());
  // Adding another widget, current widget stays the first added
  stack->addNew<Wt::WText>(Wt::utf8("Text 2"));
  BOOST_REQUIRE_EQUAL(0, stack->currentIndex());
  BOOST_REQUIRE_EQUAL(text, stack->currentWidget());
}
