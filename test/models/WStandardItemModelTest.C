/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>

using namespace Wt;

std::unique_ptr<WStandardItemModel> createPopulatedModel(int rows, int cols)
{
  std::unique_ptr<WStandardItemModel> model = std::make_unique<WStandardItemModel>(rows, cols);

  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      model->item(row, col)->setText(WString("Row: {1} - Col: {2}").arg(row).arg(col));
    }
  }

  BOOST_REQUIRE(model->rowCount() == rows);
  BOOST_REQUIRE(model->columnCount() == cols);

  return model;
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_clear_test )
{
  // Test whether clearing the model removes all items

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 1);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  model->clear();

  BOOST_REQUIRE(model->rowCount() == 0);
  BOOST_REQUIRE(model->columnCount() == 0);
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_indexFromItem_test )
{
  // Test whether the index of an item is correctly retrieved

  std::unique_ptr<WStandardItemModel> model(new WStandardItemModel());
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.emplace_back();
  std::unique_ptr<WStandardItem> item2Ptr = std::make_unique<WStandardItem>();
  WStandardItem* item2 = item2Ptr.get();
  items.push_back(std::move(item2Ptr));

  model->insertColumn(0, std::move(items));

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  WModelIndex index = model->indexFromItem(item2);

  BOOST_TEST(index.row() == 1);
  BOOST_TEST(index.column() == 0);
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_itemFromIndex_test )
{
  // Test whether the item is correctly retrieved from the index

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(0, 0);
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.emplace_back();
  std::unique_ptr<WStandardItem> item2Ptr = std::make_unique<WStandardItem>();
  WStandardItem* item2 = item2Ptr.get();
  items.push_back(std::move(item2Ptr));

  model->insertColumn(0, std::move(items));

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  WStandardItem* item = model->itemFromIndex(model->indexFromItem(item2));

  BOOST_TEST(item == item2);
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_appendColumn_test )
{
  // Test whether appending a column of items changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.emplace_back();
  items.emplace_back();

  model->appendColumn(std::move(items));

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 3);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(!model->item(0, 2));
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(!model->item(1, 2));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_appendRow_item_test )
{
  // Test whether appending a row of a single item changes the model correctly.
  // The item in the "expected" second column will not exist.

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);
  std::unique_ptr<WStandardItem> item = std::make_unique<WStandardItem>();

  model->appendRow(std::move(item));

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(model->item(2, 0)->text().empty());
  BOOST_TEST(!model->item(2, 1));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_appendRow_vector_test )
{
  // Test whether appending a row of items changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.push_back(std::make_unique<WStandardItem>());
  items.push_back(std::make_unique<WStandardItem>());

  model->appendRow(std::move(items));

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(model->item(2, 0)->text().empty());
  BOOST_TEST(model->item(2, 1)->text().empty());
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertColumn_test )
{
  // Test whether inserting a column of items changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.push_back(std::make_unique<WStandardItem>());
  items.push_back(std::make_unique<WStandardItem>());

  model->insertColumn(1, std::move(items));

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 3);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text().empty());
  BOOST_TEST(model->item(0, 2)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text().empty());
  BOOST_TEST(model->item(1, 2)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertRow_item_test )
{
  // Test whether inserting a row of a single item changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  model->insertRow(1, std::make_unique<WStandardItem>());

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text().empty());
  BOOST_TEST(!model->item(1, 1));
  BOOST_TEST(model->item(2, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(2, 1)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertRow_vector_test )
{
  // Test whether inserting a row of items changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);
  std::vector<std::unique_ptr<WStandardItem>> items;
  items.push_back(std::make_unique<WStandardItem>());
  items.push_back(std::make_unique<WStandardItem>());

  model->insertRow(1, std::move(items));

  BOOST_REQUIRE(model->rowCount() == 3);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text().empty());
  BOOST_TEST(model->item(1, 1)->text().empty());
  BOOST_TEST(model->item(2, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(2, 1)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_takeRow_test )
{
  // Test whether taking a row removes it from the model

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  std::vector<std::unique_ptr<WStandardItem>> row = model->takeRow(0);

  BOOST_REQUIRE(model->rowCount() == 1);
  BOOST_REQUIRE(model->columnCount() == 2);
  BOOST_REQUIRE(row.size() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(row[0]->text() == "Row: 0 - Col: 0");
  BOOST_TEST(row[1]->text() == "Row: 0 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_takeColumn_test )
{
  // Test whether taking a column removes it from the model

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  std::vector<std::unique_ptr<WStandardItem>> column = model->takeColumn(0);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);
  BOOST_REQUIRE(column.size() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(column[0]->text() == "Row: 0 - Col: 0");
  BOOST_TEST(column[1]->text() == "Row: 1 - Col: 0");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_takeItem_test )
{
  // Test whether taking an item removes it from the model

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  std::unique_ptr<WStandardItem> item = model->takeItem(0, 1);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);
  BOOST_REQUIRE(item);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(!model->item(0, 1));
  BOOST_TEST(item->text() == "Row: 0 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertColumns_insert_test )
{
  // Test whether inserting multiple columns changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool inserted = model->insertColumns(1, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 4);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text().empty());
  BOOST_TEST(model->item(0, 2)->text().empty());
  BOOST_TEST(model->item(0, 3)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text().empty());
  BOOST_TEST(model->item(1, 2)->text().empty());
  BOOST_TEST(model->item(1, 3)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertColumns_append_test )
{
  // Test whether appending multiple columns changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool inserted = model->insertColumns(2, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 4);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(0, 2)->text().empty());
  BOOST_TEST(model->item(0, 3)->text().empty());
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(model->item(1, 2)->text().empty());
  BOOST_TEST(model->item(1, 3)->text().empty());
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertColumns_nonexistent_index_test )
{
  // Test whether inserting multiple columns on an out-of-range index
  // does not change the model.

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  BOOST_CHECK_THROW(model->insertColumns(3, 2), Wt::WException);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(!model->item(0, 2));
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(!model->item(2, 0));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertRows_insert_test )
{
  // Test whether inserting multiple rows changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool inserted = model->insertRows(1, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 4);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text().empty());
  BOOST_TEST(model->item(1, 1)->text().empty());
  BOOST_TEST(model->item(2, 0)->text().empty());
  BOOST_TEST(model->item(2, 1)->text().empty());
  BOOST_TEST(model->item(3, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(3, 1)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertRows_append_test )
{
  // Test whether appending multiple rows changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool inserted = model->insertRows(2, 2);

  BOOST_REQUIRE(inserted);
  BOOST_REQUIRE(model->rowCount() == 4);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(model->item(2, 0)->text().empty());
  BOOST_TEST(model->item(2, 1)->text().empty());
  BOOST_TEST(model->item(3, 0)->text().empty());
  BOOST_TEST(model->item(3, 1)->text().empty());
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_insertRows_nonexistent_test )
{
  // Test whether inserting multiple rows on an out-of-range index
  // does not change the model.

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  BOOST_CHECK_THROW(model->insertRows(3, 2), Wt::WException);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(!model->item(0, 2));
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(!model->item(2, 0));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeColumns_full_test )
{
  // Test whether removing all columns changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeColumns(0, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 0);
  BOOST_REQUIRE(model->columnCount() == 0);

  BOOST_TEST(!model->item(0, 0));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeColumns_partial_test )
{
  // Tests whether the model correctly removes a single column at the end

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeColumns(1, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 1);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(!model->item(0, 1));
  BOOST_TEST(!model->item(1, 1));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeColumns_none_test )
{
  // Tests whether the model correctly ignores removing out-of-range indices

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeColumns(3, 1);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeRows_full_test )
{
  // Test whether removing all rows changes the model correctly

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeRows(0, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 0);
  BOOST_REQUIRE(model->columnCount() == 0);

  BOOST_TEST(!model->item(0, 0));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeRows_partial_test )
{
  // Tests whether the model correctly removes a single row at the end

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeRows(1, 2);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 1);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(!model->item(1, 0));
  BOOST_TEST(!model->item(1, 1));
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_removeRows_none_test )
{
  // Tests whether the model correctly ignores removing out-of-range indices

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  bool removed = model->removeRows(3, 1);

  BOOST_REQUIRE(removed);
  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");
}

BOOST_AUTO_TEST_CASE( WStandardItemModel_sort_test )
{
  // Tests whether the model correctly sorts columns

  std::unique_ptr<WStandardItemModel> model = createPopulatedModel(2, 2);

  model->sort(1, SortOrder::Ascending);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 0 - Col: 1");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 1 - Col: 1");

  model->sort(1, SortOrder::Descending);

  BOOST_REQUIRE(model->rowCount() == 2);
  BOOST_REQUIRE(model->columnCount() == 2);

  BOOST_TEST(model->item(0, 0)->text() == "Row: 1 - Col: 0");
  BOOST_TEST(model->item(1, 0)->text() == "Row: 0 - Col: 0");
  BOOST_TEST(model->item(0, 1)->text() == "Row: 1 - Col: 1");
  BOOST_TEST(model->item(1, 1)->text() == "Row: 0 - Col: 1");
}
