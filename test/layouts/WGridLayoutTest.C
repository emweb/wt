/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WGridLayout.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( test_grid_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WGridLayout>());

  auto item00 = layout->addWidget(std::make_unique<WContainerWidget>(), 0, 0);
  auto item01 = layout->addWidget(std::make_unique<WContainerWidget>(), 0, 1);
  auto item11 = layout->addWidget(std::make_unique<WContainerWidget>(), 1, 1);

  BOOST_CHECK(layout->itemAt(-1) == nullptr);
  BOOST_CHECK(layout->itemAt(0)->widget() == item00);
  BOOST_CHECK(layout->itemAt(1)->widget() == item01);
  BOOST_CHECK(layout->itemAt(2) == nullptr);
  BOOST_CHECK(layout->itemAt(3)->widget() == item11);
  BOOST_CHECK(layout->itemAt(4) == nullptr);
}
