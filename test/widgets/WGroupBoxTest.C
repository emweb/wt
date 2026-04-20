/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WGroupBox.h>
#include <Wt/WHBoxLayout.h>

BOOST_AUTO_TEST_CASE(test_WGroupBox_addWidget)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");

  BOOST_REQUIRE_EQUAL(groupBox->widget(0), nullptr);
  BOOST_CHECK_EQUAL(groupBox->count(), 0);

   // Check adding a widget works correctly
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_CHECK_EQUAL(groupBox->count(), 1);

  // Check adding a widget adds it at the end.
  auto widget2 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_CHECK_EQUAL(groupBox->widget(0), widget1);
  BOOST_CHECK_EQUAL(groupBox->widget(1), widget2);
  BOOST_CHECK_EQUAL(groupBox->count(), 2);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_addWidget_when_has_layout)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  groupBox->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 0);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), nullptr);

  // Check that adding a widget does not add it.
  groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_CHECK_EQUAL(groupBox->count(), 0);
  BOOST_CHECK_EQUAL(groupBox->widget(0), nullptr);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_indexOf)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());
  auto widget2 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 2);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_REQUIRE_EQUAL(groupBox->widget(1), widget2);

  // Check that we can find the correct index of the widgets
  BOOST_CHECK_EQUAL(groupBox->indexOf(widget1), 0);
  BOOST_CHECK_EQUAL(groupBox->indexOf(widget2), 1);

  // Check that a widget that is not in the group box returns -1
  auto unrelatedWidget = std::make_unique<Wt::WContainerWidget>();
  BOOST_CHECK_EQUAL(groupBox->indexOf(unrelatedWidget.get()), -1);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_insertWidget)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());
  auto widget3 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 2);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_REQUIRE_EQUAL(groupBox->widget(1), widget3);

  // Check that inserting a widget inserts it at the correct index
  auto widget2 = groupBox->insertWidget(1, std::make_unique<Wt::WContainerWidget>());

  BOOST_CHECK_EQUAL(groupBox->count(), 3);

  BOOST_CHECK_EQUAL(groupBox->widget(0), widget1);
  BOOST_CHECK_EQUAL(groupBox->widget(1), widget2);
  BOOST_CHECK_EQUAL(groupBox->widget(2), widget3);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_insertWidget_when_has_layout)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  groupBox->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 0);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), nullptr);

  // Check that inserting a widget does not add it.
  groupBox->insertWidget(0, std::make_unique<Wt::WContainerWidget>());

  BOOST_CHECK_EQUAL(groupBox->count(), 0);
  BOOST_CHECK_EQUAL(groupBox->widget(0), nullptr);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_insertBefore)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());
  auto widget3 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 2);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_REQUIRE_EQUAL(groupBox->widget(1), widget3);

  // Check that inserting a widget inserts it at the correct index
  auto widget2 = groupBox->insertBefore(std::make_unique<Wt::WContainerWidget>(), widget3);

  BOOST_CHECK_EQUAL(groupBox->count(), 3);

  BOOST_CHECK_EQUAL(groupBox->widget(0), widget1);
  BOOST_CHECK_EQUAL(groupBox->widget(1), widget2);
  BOOST_CHECK_EQUAL(groupBox->widget(2), widget3);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_insertBefore_non_existent_widget)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());
  auto widget2 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 2);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_REQUIRE_EQUAL(groupBox->widget(1), widget2);

  // Check that inserting a widget before an unrelated widget adds it at the end.
  auto unrelatedWidget = std::make_unique<Wt::WContainerWidget>();
  auto widget3 = groupBox->insertBefore(std::make_unique<Wt::WContainerWidget>(), unrelatedWidget.get());

  BOOST_CHECK_EQUAL(groupBox->count(), 3);

  BOOST_CHECK_EQUAL(groupBox->widget(0), widget1);
  BOOST_CHECK_EQUAL(groupBox->widget(1), widget2);
  BOOST_CHECK_EQUAL(groupBox->widget(2), widget3);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_insertBefore_when_has_layout)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  groupBox->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 0);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), nullptr);

  // Check that inserting a widget does not add it.
  auto unrelatedWidget = std::make_unique<Wt::WContainerWidget>();
  groupBox->insertBefore(std::make_unique<Wt::WContainerWidget>(), unrelatedWidget.get());

  BOOST_CHECK_EQUAL(groupBox->count(), 0);
  BOOST_CHECK_EQUAL(groupBox->widget(0), nullptr);
}

BOOST_AUTO_TEST_CASE(test_WGroupBox_setLayout)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  auto groupBox = std::make_unique<Wt::WGroupBox>("Group box title");
  auto widget1 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());
  auto widget2 = groupBox->addWidget(std::make_unique<Wt::WContainerWidget>());

  BOOST_REQUIRE_EQUAL(groupBox->count(), 2);
  BOOST_REQUIRE_EQUAL(groupBox->widget(0), widget1);
  BOOST_REQUIRE_EQUAL(groupBox->widget(1), widget2);

  // Check adding a layout removes the widgets.
  groupBox->setLayout(std::make_unique<Wt::WHBoxLayout>());

  BOOST_CHECK_EQUAL(groupBox->count(), 0);
  BOOST_CHECK_EQUAL(groupBox->indexOf(widget1), -1);
  BOOST_CHECK_EQUAL(groupBox->indexOf(widget2), -1);
  BOOST_CHECK_EQUAL(groupBox->widget(0), nullptr);
  BOOST_CHECK_EQUAL(groupBox->widget(1), nullptr);
}








