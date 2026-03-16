/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( test_vbox_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WVBoxLayout>());

  auto item1 = layout->addWidget(std::make_unique<WContainerWidget>());
  auto item2 = layout->addWidget(std::make_unique<WContainerWidget>());


  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == item1);
  BOOST_CHECK(layout->itemAt(1)->widget() == item2);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
}

BOOST_AUTO_TEST_CASE( test_hbox_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WHBoxLayout>());

  auto item1 = layout->addWidget(std::make_unique<WContainerWidget>());
  auto item2 = layout->addWidget(std::make_unique<WContainerWidget>());


  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == item1);
  BOOST_CHECK(layout->itemAt(1)->widget() == item2);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
}
