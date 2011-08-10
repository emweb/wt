/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

using namespace Wt;

BOOST_AUTO_TEST_CASE( standarditems_test1 )
{
  WStandardItemModel *model = new WStandardItemModel();

  BOOST_REQUIRE(model->columnCount() == 0);
  BOOST_REQUIRE(model->rowCount() == 0);

  std::vector<WStandardItem *> items;
  items.push_back(new WStandardItem());
  items.push_back(0);

  model->insertColumn(0, items);

  BOOST_REQUIRE(model->columnCount() == 1);
  BOOST_REQUIRE(model->rowCount() == 2);

  model->clear();

  delete model;
}
