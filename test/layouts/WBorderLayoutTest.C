/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WBorderLayout.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( test_border_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WBorderLayout>());

  auto itemCenter = layout->addWidget(std::make_unique<WContainerWidget>(), LayoutPosition::Center);
  auto itemNorth = layout->addWidget(std::make_unique<WContainerWidget>(), LayoutPosition::North);

  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == itemNorth);
  BOOST_CHECK(layout->itemAt(1)->widget() == itemCenter);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
  BOOST_CHECK(layout->itemAt(3) == nullptr);
  BOOST_CHECK(layout->itemAt(4) == nullptr);
  BOOST_CHECK(layout->itemAt(5) == nullptr);
}
