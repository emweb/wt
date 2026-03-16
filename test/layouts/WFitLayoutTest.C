/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WFitLayout.h>
#include <Wt/Test/WTestEnvironment.h>

#include <memory>

using namespace Wt;

BOOST_AUTO_TEST_CASE( test_fit_layout_itemAt_range )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication testApp(environment);

  auto container = testApp.root()->addNew<WContainerWidget>();
  auto layout = container->setLayout(std::make_unique<WFitLayout>());

  BOOST_CHECK(layout->itemAt(0) == nullptr);

  auto item = layout->addWidget(std::make_unique<WContainerWidget>());

  BOOST_CHECK(layout->itemAt(0)->widget() == item);
}
