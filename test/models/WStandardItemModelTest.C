/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( standarditems_test1 )
{
  std::unique_ptr<WStandardItemModel> model(new WStandardItemModel());

  BOOST_REQUIRE(model->columnCount() == 0);
  BOOST_REQUIRE(model->rowCount() == 0);

  std::vector<std::unique_ptr<WStandardItem>> items;
  items.emplace_back();
  items.emplace_back();

  model->insertColumn(0, std::move(items));

  BOOST_REQUIRE(model->columnCount() == 1);
  BOOST_REQUIRE(model->rowCount() == 2);

  model->clear();
}
